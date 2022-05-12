#include "ConfigBuilder.hpp"
#include "db/Database.hpp"

namespace NekoRay::fmt {

    void ApplyCustomOutboundJsonSettings(const QJsonObject &custom, QJsonObject &outbound) {
        // 合并
        if (custom.isEmpty()) return;
        for (const auto &key: custom.keys()) {
            if (outbound.contains(key)) {
                auto v = custom[key];
                auto v_orig = outbound[key];
                if (v.isObject() && v_orig.isObject()) {// isObject 则合并？
                    auto vo = v.toObject();
                    QJsonObject vo_orig = v_orig.toObject();
                    ApplyCustomOutboundJsonSettings(vo, vo_orig);
                    outbound[key] = vo_orig;
                } else {
                    outbound[key] = v;
                }
            } else {
                outbound[key] = custom[key];
            }
        }
    }

    QSharedPointer<BuildConfigResult> BuildConfig(const QSharedPointer<ProxyEntity> &ent, bool forTest) {
        auto result = QSharedPointer<BuildConfigResult>(new BuildConfigResult());
        auto status = QSharedPointer<BuildConfigStatus>(new BuildConfigStatus);
        status->result = result;

        // Log
        result->coreConfig.insert("log", QJsonObject{{"loglevel", dataStore->log_level}});

        // Inbounds
        bool dnsIn = false;
        QJsonArray inbounds;

        QJsonObject sniffing{{"destOverride", QJsonArray{"http", "tls"}},
                             {"enabled",      true},
                             {"metadataOnly", true},
                             {"routeOnly",    dataStore->sniffing_mode == SniffingMode::TO_DNS},};

        // socks-in
        if (InRange(dataStore->inbound_socks_port, 0, 65535) && !forTest) {
            QJsonObject socksInbound;
            socksInbound["tag"] = "socks-in";
            socksInbound["protocol"] = "socks";
            socksInbound["listen"] = dataStore->inbound_address;
            socksInbound["port"] = dataStore->inbound_socks_port;
            socksInbound["settings"] = QJsonObject({{"auth", "noauth"},
                                                    {"udp",  true},});
            if (dataStore->sniffing_mode != SniffingMode::DISABLE) socksInbound["sniffing"] = sniffing;
            inbounds += socksInbound;
        }
        // http-in
        if (InRange(dataStore->inbound_http_port, 0, 65535) && !forTest) {
            QJsonObject socksInbound;
            socksInbound["tag"] = "http-in";
            socksInbound["protocol"] = "http";
            socksInbound["listen"] = dataStore->inbound_address;
            socksInbound["port"] = dataStore->inbound_http_port;
            if (dataStore->sniffing_mode != SniffingMode::DISABLE) socksInbound["sniffing"] = sniffing;
            inbounds += socksInbound;
        }

        result->coreConfig.insert("inbounds", inbounds);

        // Outbounds
        QList<QSharedPointer<ProxyEntity>> ents;
        if (ent->type == "chain") {
            auto list = ent->ChainBean()->list;
            std::reverse(std::begin(list), std::end(list));
            for (auto id: list) {
                ents += profileManager->GetProfile(id);
                if (ents.last() == nullptr) {
                    result->error = QString("chain missing ent: %1").arg(id);
                    return result;
                }
                if (ents.last()->type == "chain") {
                    result->error = QString("chain in chain is not allowed: %1").arg(id);
                    return result;
                }
            }
        } else {
            ents += ent;
        }

        BuildChain(false, ents, status);
        if (!result->error.isEmpty()) return result;

        // direct & bypass & block
        status->outbounds += QJsonObject{{"protocol", "freedom"},
                                         {"tag",      "direct"},};
        status->outbounds += QJsonObject{{"protocol", "freedom"},
                                         {"tag",      "bypass"},};
        status->outbounds += QJsonObject{{"protocol", "blackhole"},
                                         {"tag",      "block"},};

        if (dnsIn) {
            QJsonObject dnsOut;
            dnsOut["protocol"] = "dns";
            dnsOut["tag"] = "dns-out";
            QJsonObject dnsOut_settings;
            dnsOut_settings["userLevel"] = 1;
            dnsOut["settings"] = dnsOut_settings;
            status->outbounds += dnsOut;
        }

        result->coreConfig.insert("outbounds", status->outbounds);

        // dns domain user rules
        for (const auto &line: SplitLines(dataStore->routing->proxy_domain)) {
            if (line.startsWith("#")) continue;
            if (dataStore->dns_routing) status->domainListDNSRemote += line;
            status->domainListRemote += line;
        }
        for (const auto &line: SplitLines(dataStore->routing->direct_domain)) {
            if (line.startsWith("#")) continue;
            if (dataStore->dns_routing) status->domainListDNSDirect += line;
            status->domainListDirect += line;
        }

        // final add DNS
        QJsonObject dns;
        QJsonArray dnsServers;

        // remote
        QJsonObject dnsServerRemote;
        dnsServerRemote["address"] = dataStore->remote_dns;
        dnsServerRemote["domains"] = status->domainListDNSRemote;
        if (!forTest) dnsServers += dnsServerRemote;

        //direct
        auto directDnsAddress = dataStore->direct_dns;
        if (directDnsAddress.contains("https://")) {
            directDnsAddress.replace("https://", "https+local://");
        } else {
            status->routingRules += QJsonObject({{"type", "field"},
                                                 {"ip",   QJsonArray{directDnsAddress}},});
        }
        dnsServers += QJsonObject{{"address",      directDnsAddress},
                                  {"domains",      status->domainListDNSDirect},
                                  {"skipFallback", true},};

        dns["disableFallbackIfMatch"] = true;
        dns["hosts"] = status->hosts;
        dns["servers"] = dnsServers;
        result->coreConfig.insert("dns", dns);

        // Routing
        QJsonObject routing;
        routing["domainStrategy"] = dataStore->domain_strategy;

        if (dnsIn) {
            QJsonObject routingRules_dns_in;
            routingRules_dns_in["type"] = "field";
            routingRules_dns_in["inboundTag"] = QList2QJsonArray(QList<QString>({"dns-in"}));
            routingRules_dns_in["outboundTag"] = "dns-out";
            status->routingRules += routingRules_dns_in;
        }

        // ip user rule
        // proxy
        QJsonObject routingRule_tmp;
        routingRule_tmp["type"] = "field";
        routingRule_tmp["outboundTag"] = "proxy";
        for (const auto &line: SplitLines(dataStore->routing->proxy_ip)) {
            if (line.startsWith("#")) continue;
            status->ipListRemote += line;
        }
        // final add proxy route
        if (!status->ipListRemote.isEmpty()) {
            auto routingRule_proxy_ip = routingRule_tmp;
            routingRule_proxy_ip["ip"] = status->ipListRemote;
            status->routingRules += routingRule_proxy_ip;
        }
        if (!status->domainListRemote.isEmpty()) {
            auto routingRule_proxy_domain = routingRule_tmp;
            routingRule_proxy_domain["domain"] = status->domainListRemote;
            status->routingRules += routingRule_proxy_domain;
        }

        // bypass
        routingRule_tmp["outboundTag"] = "bypass";
        for (const auto &line: SplitLines(dataStore->routing->direct_ip)) {
            if (line.startsWith("#")) continue;
            status->ipListDirect += line;
        }
        // final add bypass route
        if (!status->ipListDirect.isEmpty()) {
            auto routingRule_direct_ip = routingRule_tmp;
            routingRule_direct_ip["ip"] = status->ipListDirect;
            status->routingRules += routingRule_direct_ip;
        }
        if (!status->domainListDirect.isEmpty()) {
            auto routingRule_direct_domain = routingRule_tmp;
            routingRule_direct_domain["domain"] = status->domainListDirect;
            status->routingRules += routingRule_direct_domain;
        }

        routing["rules"] = status->routingRules;
        result->coreConfig.insert("routing", routing);

        // Policy & stats
        QJsonObject policy;
        QJsonObject levels;
        QJsonObject level1;
        level1["connIdle"] = 30;
        levels["1"] = level1;
        policy["levels"] = levels;

        QJsonObject policySystem;
        policySystem["statsOutboundDownlink"] = true;
        policySystem["statsOutboundUplink"] = true;
        policy["system"] = policySystem;
        result->coreConfig.insert("policy", policy);
        result->coreConfig.insert("stats", QJsonObject());

        return result;
    }

//    QString genDomainStrategy(bool noAsIs) {
//        if (dataStore->outbound_domain_strategy != "AsIs") return dataStore->outbound_domain_strategy;
//        QString ret;
//        if (!dataStore->destination_override && !noAsIs) ret = "AsIs";
//        if (dataStore->ipv6_mode == IPv6Mode::DISABLE) ret = "UseIPv4";
//        if (dataStore->ipv6_mode == IPv6Mode::PREFER)ret = "PreferIPv6";
//        if (dataStore->ipv6_mode == IPv6Mode::ONLY)ret = "UseIPv6";
//        if (ret.isEmpty()) ret = "PreferIPv4";
//        return ret;
//    }

    QString BuildChain(bool extra, const QList<QSharedPointer<ProxyEntity>> &ents,
                       const QSharedPointer<BuildConfigStatus> &status) {
        QString chainOutboundTag;
        QString pastTag;
        int index = 0;

        for (const auto &ent: ents) {
            auto mainObj = ent->bean->BuildCoreObj();
            if (!mainObj.error.isEmpty()) { // rejected
                status->result->error = mainObj.error;
                return "";
            }
            auto outbound = mainObj.outbound;

            // tagOut: v2ray outbound tagOut for a profile
            // profile1 (in)  tagOut=global-proxy-(id)
            // profile2       tagOut=proxy-(id)
            // profile3 (out) tagOut=proxy / proxy-(chainID)
            auto tagOut = index == 0 && !extra ? "proxy" : QString("proxy-%1").arg(ent->id);

            // needGlobal: can only contain one?
            bool needGlobal = false;

            // first profile set as global
            if (index == ents.length() - 1) {
                ent->bean->isFirstProfile = true;
                needGlobal = true;
                if (index != 0) tagOut = "global-" + tagOut;
            }

            if (needGlobal) {
                if (status->globalProfiles.contains(ent->id)) {
                    continue;
                }
                status->globalProfiles += ent->id;
            }

            // chain rules
            ent->traffic_data->id = ent->id;
            if (index > 0) {
                // chain route/proxy rules
                if (!ents[index - 1]->bean->NeedExternal()) {
                    auto replaced = status->outbounds.last().toObject();
                    replaced["proxySettings"] = QJsonObject{
                            {"tag",            tagOut},
                            {"transportLayer", true},
                    };
                    status->outbounds.removeLast();
                    status->outbounds += replaced;
                } else {
                    status->routingRules += QJsonObject{
                            {"type",        "field"},
                            {"inboundTag",  QJsonArray{pastTag}},
                            {"outboundTag", tagOut},
                    };
                }
            } else {
                // index == 0 means last profile in chain / not chain
                chainOutboundTag = tagOut;
                status->result->outboundStat = ent->traffic_data;
            }
            ent->traffic_data->tag = tagOut.toStdString();
            status->result->outboundStats += ent->traffic_data;

            outbound["tag"] = tagOut;
            outbound["domainStrategy"] = dataStore->outbound_domain_strategy;

            // apply mux
            if (dataStore->mux_cool > 0) {
                if (ent->type == "vmess" || ent->type == "trojan") {//常见的*ray后端
                    outbound["mux"] = QJsonObject{
                            {"enabled",     true},
                            {"concurrency", dataStore->mux_cool},
                    };
                }
            }

            // apply custom
            auto custom_item = ent->bean->_get("custom");
            if (custom_item != nullptr) {
                ApplyCustomOutboundJsonSettings(QString2QJsonObject(*((QString *) custom_item->ptr)), outbound);
            }

            // Bypass Lookup for the first profile
            if (ent->bean->isFirstProfile && !IsIpAddress(ent->bean->serverAddress)) {
                status->domainListDNSDirect += "full:" + ent->bean->serverAddress;
            }

            status->outbounds += outbound;
            pastTag = tagOut;
            index++;
        }
        return chainOutboundTag;
    }

}