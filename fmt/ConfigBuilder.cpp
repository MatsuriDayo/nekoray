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
        auto logObj = QJsonObject{{"loglevel", dataStore->log_level}};
        result->coreConfig.insert("log", logObj);

        // Inbounds
        QJsonObject sniffing{{"destOverride", dataStore->fake_dns ?
                                              QJsonArray{"fakedns", "http", "tls"} : QJsonArray{"http", "tls"}},
                             {"enabled",      true},
                             {"metadataOnly", false},
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
            if (dataStore->fake_dns || dataStore->sniffing_mode != SniffingMode::DISABLE) {
                socksInbound["sniffing"] = sniffing;
            }
            status->inbounds += socksInbound;
        }
        // http-in
        if (InRange(dataStore->inbound_http_port, 0, 65535) && !forTest) {
            QJsonObject socksInbound;
            socksInbound["tag"] = "http-in";
            socksInbound["protocol"] = "http";
            socksInbound["listen"] = dataStore->inbound_address;
            socksInbound["port"] = dataStore->inbound_http_port;
            if (dataStore->sniffing_mode != SniffingMode::DISABLE) socksInbound["sniffing"] = sniffing;
            status->inbounds += socksInbound;
        }

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

        // DNS Routing (tun2socks 用到，防污染)
        if (dataStore->dns_routing) {
            QJsonObject dnsOut;
            dnsOut["protocol"] = "dns";
            dnsOut["tag"] = "dns-out";
            QJsonObject dnsOut_settings;
            dnsOut_settings["network"] = "tcp";
            dnsOut_settings["port"] = 53;
            dnsOut_settings["address"] = "1.0.0.1";
            dnsOut_settings["userLevel"] = 1;
            dnsOut["settings"] = dnsOut_settings;

            status->outbounds += dnsOut;
            status->routingRules += QJsonObject{
                    {"type",        "field"},
                    {"port",        "53"},
                    {"outboundTag", "dns-out"},
            };
        }

        // block for tun
        status->routingRules += QJsonObject{{"type",        "field"},
                                            {"ip",          QJsonArray{"224.0.0.0/3", "169.254.0.0/16",},},
                                            {"outboundTag", "block"},};
        status->routingRules += QJsonObject{{"type",        "field"},
                                            {"port",        "135-139"},
                                            {"outboundTag", "block"},};

        // custom inbound
        QJSONARRAY_ADD(status->inbounds, QString2QJsonObject(dataStore->custom_inbound)["inbounds"].toArray())

        result->coreConfig.insert("inbounds", status->inbounds);
        result->coreConfig.insert("outbounds", status->outbounds);

        // dns domain user rule
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
        for (const auto &line: SplitLines(dataStore->routing->block_domain)) {
            if (line.startsWith("#")) continue;
            status->domainListBlock += line;
        }

        // final add DNS
        QJsonObject dns;
        QJsonArray dnsServers;

        // FakeDNS
        QJsonObject dnsServerFake;
        dnsServerFake["address"] = "fakedns";
        dnsServerFake["domains"] = status->domainListDNSRemote;
        if (dataStore->fake_dns && !forTest) dnsServers += dnsServerFake;

        // remote
        QJsonObject dnsServerRemote;
        dnsServerRemote["address"] = dataStore->remote_dns;
        dnsServerRemote["domains"] = status->domainListDNSRemote;
        if (!forTest) dnsServers += dnsServerRemote;

        //direct
        auto directDnsAddress = dataStore->direct_dns;
        if (directDnsAddress.contains("://")) {
            auto directDnsIp = SubStrBefore(SubStrAfter(directDnsAddress, "://"), "/");
            if (IsIpAddress(directDnsIp)) {
                status->routingRules.push_front(QJsonObject{
                        {"type",        "field"},
                        {"ip",          QJsonArray{directDnsIp}},
                        {"outboundTag", "direct"},
                });
            } else {
                status->routingRules.push_front(QJsonObject{
                        {"type",        "field"},
                        {"domain",      QJsonArray{directDnsIp}},
                        {"outboundTag", "direct"},
                });
            }
        } else if (directDnsAddress != "localhost") {
            status->routingRules.push_front(QJsonObject{
                    {"type",        "field"},
                    {"ip",          QJsonArray{directDnsAddress}},
                    {"outboundTag", "direct"},
            });
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
        routing["domainMatcher"] = dataStore->domain_matcher == DomainMatcher::MPH ? "mph" : "linear";

        // ip user rule
        QJsonObject routingRule_tmp;
        routingRule_tmp["type"] = "field";

        // block
        routingRule_tmp["outboundTag"] = "block";
        for (const auto &line: SplitLines(dataStore->routing->block_ip)) {
            if (line.startsWith("#")) continue;
            status->ipListBlock += line;
        }
        // final add block route
        if (!status->ipListBlock.isEmpty()) {
            auto tmp = routingRule_tmp;
            tmp["ip"] = status->ipListBlock;
            status->routingRules += tmp;
        }
        if (!status->domainListBlock.isEmpty()) {
            auto tmp = routingRule_tmp;
            tmp["domain"] = status->domainListBlock;
            status->routingRules += tmp;
        }

        // proxy
        routingRule_tmp["outboundTag"] = "proxy";
        for (const auto &line: SplitLines(dataStore->routing->proxy_ip)) {
            if (line.startsWith("#")) continue;
            status->ipListRemote += line;
        }
        // final add proxy route
        if (!status->ipListRemote.isEmpty()) {
            auto tmp = routingRule_tmp;
            tmp["ip"] = status->ipListRemote;
            status->routingRules += tmp;
        }
        if (!status->domainListRemote.isEmpty()) {
            auto tmp = routingRule_tmp;
            tmp["domain"] = status->domainListRemote;
            status->routingRules += tmp;
        }

        // bypass
        routingRule_tmp["outboundTag"] = "bypass";
        for (const auto &line: SplitLines(dataStore->routing->direct_ip)) {
            if (line.startsWith("#")) continue;
            status->ipListDirect += line;
        }
        // final add bypass route
        if (!status->ipListDirect.isEmpty()) {
            auto tmp = routingRule_tmp;
            tmp["ip"] = status->ipListDirect;
            status->routingRules += tmp;
        }
        if (!status->domainListDirect.isEmpty()) {
            auto tmp = routingRule_tmp;
            tmp["domain"] = status->domainListDirect;
            status->routingRules += tmp;
        }

        // final add routing rule
        // custom routing rule
        auto routingRules = QString2QJsonObject(dataStore->custom_route)["rules"].toArray();
        QJSONARRAY_ADD(routingRules, status->routingRules)
        routing["rules"] = routingRules;
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
            } else {
                ent->bean->isFirstProfile = false;
            }

            if (needGlobal) {
                if (status->globalProfiles.contains(ent->id)) {
                    continue;
                }
                status->globalProfiles += ent->id;
            }

            if (index > 0) {
                // chain rules: past
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
                            {"inboundTag",  QJsonArray{pastTag + "-mapping"}},
                            {"outboundTag", tagOut},
                    };
                }
            } else {
                // index == 0 means last profile in chain / not chain
                chainOutboundTag = tagOut;
                status->result->outboundStat = ent->traffic_data;
            }

            // chain rules: this
            auto mapping_port = MkPort();
            if (ent->bean->NeedExternal()) {
                status->inbounds += QJsonObject{
                        {"protocol", "dokodemo-door"},
                        {"tag",      tagOut + "-mapping"},
                        {"listen",   "127.0.0.1"},
                        {"port",     mapping_port},
                        {"settings", QJsonObject{ // to
                                {"address", ent->bean->serverAddress},
                                {"port",    ent->bean->serverPort},
                                {"network", "tcp,udp"},
                        }},
                };
                // no chain rule and not outbound, so need to set to direct
                if (ent->bean->isFirstProfile) {
                    status->routingRules += QJsonObject{
                            {"type",        "field"},
                            {"inboundTag",  QJsonArray{tagOut + "-mapping"}},
                            {"outboundTag", "direct"},
                    };
                }
            }

            // Outbound

            QJsonObject outbound;
            CoreObjOutboundBuildResult coreR;
            ExternalBuildResult extR;

            if (ent->bean->NeedExternal()) {
                auto ext_socks_port = MkPort();
                extR = ent->bean->BuildExternal(mapping_port, ext_socks_port);
                if (!extR.error.isEmpty()) { // rejected
                    status->result->error = extR.error;
                    return "";
                }

                // SOCKS OUTBOUND
                outbound["protocol"] = "socks";
                QJsonObject settings;
                QJsonArray servers;
                QJsonObject server;
                server["address"] = "127.0.0.1";
                server["port"] = ext_socks_port;
                servers.push_back(server);
                settings["servers"] = servers;
                outbound["settings"] = settings;

                // EXTERNAL PROCESS
                auto extC = new sys::ExternalProcess(ent->bean->DisplayType(),
                                                     extR.program, extR.arguments, extR.env);
                status->result->ext += extC;
            } else {
                coreR = ent->bean->BuildCoreObj();
                if (!coreR.error.isEmpty()) { // rejected
                    status->result->error = coreR.error;
                    return "";
                }
                outbound = coreR.outbound;
            }

            // outbound misc
            outbound["tag"] = tagOut;
            outbound["domainStrategy"] = dataStore->outbound_domain_strategy;
            ent->traffic_data->id = ent->id;
            ent->traffic_data->tag = tagOut.toStdString();
            status->result->outboundStats += ent->traffic_data;

            // apply mux
            if (dataStore->mux_cool > 0) {
                //常见的*ray后端
                if (ent->type == "vmess" || ent->type == "trojan" || ent->type == "vless") {
                    auto muxObj = QJsonObject{
                            {"enabled",     true},
                            {"concurrency", dataStore->mux_cool},
                    };
                    auto stream = GetStreamSettings(ent->bean);
                    if (stream != nullptr && !stream->packet_encoding.isEmpty()) {
                        muxObj["packetEncoding"] = stream->packet_encoding;
                    }
                    outbound["mux"] = muxObj;
                }
            }

            // apply custom outbound settings
            auto custom_item = ent->bean->_get("custom");
            if (custom_item != nullptr) {
                ApplyCustomOutboundJsonSettings(QString2QJsonObject(*((QString *) custom_item->ptr)), outbound);
            }

            // Bypass Lookup for the first profile
            if (ent->bean->isFirstProfile && !IsIpAddress(ent->bean->serverAddress)) {
                if (dataStore->enhance_resolve_server_domain) {
                    status->result->tryDomains += ent->bean->serverAddress;
                } else {
                    status->domainListDNSDirect += "full:" + ent->bean->serverAddress;
                }
            }

            status->outbounds += outbound;
            pastTag = tagOut;
            index++;
        }

        return chainOutboundTag;
    }

}