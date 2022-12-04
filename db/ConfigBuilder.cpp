#include "db/ConfigBuilder.hpp"
#include "db/Database.hpp"
#include "fmt/includes.h"
#include "fmt/Preset.hpp"
#include "main/QJS.hpp"

#include <QApplication>
#include <QFile>
#include <QFileInfo>

namespace NekoRay {

    // Common

    QSharedPointer<BuildConfigResult> BuildConfig(const QSharedPointer<ProxyEntity> &ent, bool forTest, bool forExport) {
        QSharedPointer<BuildConfigResult> result;
        if (IS_NEKO_BOX) {
            result = BuildConfigSingBox(ent, forTest, forExport);
        } else {
            result = BuildConfigV2Ray(ent, forTest, forExport);
        }
        // hook.js
        if (result->error.isEmpty()) {
            auto source = qjs::ReadHookJS();
            if (!source.isEmpty()) {
                qjs::QJS js(source);
                auto js_result = js.EvalFunction("hook.hook_core_config", QJsonObject2QString(result->coreConfig, true));
                auto js_result_json = QString2QJsonObject(js_result);
                if (!js_result_json.isEmpty() && result->coreConfig != js_result_json) {
                    MW_show_log("hook.js modified your " + software_core_name + " json config.");
                    result->coreConfig = js_result_json;
                }
            }
        }
        return result;
    }

    QString BuildChain(int chainId, const QSharedPointer<BuildConfigStatus> &status) {
        // Make list
        QList<QSharedPointer<ProxyEntity>> ents;
        auto ent = status->ent;
        auto result = status->result;

        if (ent->type == "chain") {
            auto list = ent->ChainBean()->list;
            std::reverse(std::begin(list), std::end(list));
            for (auto id: list) {
                ents += profileManager->GetProfile(id);
                if (ents.last() == nullptr) {
                    result->error = QString("chain missing ent: %1").arg(id);
                    return {};
                }
                if (ents.last()->type == "chain") {
                    result->error = QString("chain in chain is not allowed: %1").arg(id);
                    return {};
                }
            }
        } else {
            ents += ent;
        }

        // BuildChain
        QString chainTagOut = BuildChainInternal(0, ents, status);

        // Chain ent traffic stat
        if (ents.length() > 1) {
            status->ent->traffic_data->id = status->ent->id;
            status->ent->traffic_data->tag = chainTagOut.toStdString();
            status->result->outboundStats += status->ent->traffic_data;
        }

        return chainTagOut;
    }

    // V2Ray

    void ApplyCustomOutboundJsonSettings(const QJsonObject &custom, QJsonObject &outbound) {
        // 合并
        if (custom.isEmpty()) return;
        for (const auto &key: custom.keys()) {
            if (outbound.contains(key)) {
                auto v = custom[key];
                auto v_orig = outbound[key];
                if (v.isObject() && v_orig.isObject()) { // isObject 则合并？
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

    QSharedPointer<BuildConfigResult> BuildConfigV2Ray(const QSharedPointer<ProxyEntity> &ent, bool forTest, bool forExport) {
        auto result = QSharedPointer<BuildConfigResult>(new BuildConfigResult);
        auto status = QSharedPointer<BuildConfigStatus>(new BuildConfigStatus);
        status->ent = ent;
        status->result = result;

        // Log
        auto logObj = QJsonObject{{"loglevel", dataStore->log_level}};
        result->coreConfig.insert("log", logObj);

        // Inbounds
        QJsonObject sniffing{
            {"destOverride", dataStore->fake_dns ? QJsonArray{"fakedns", "http", "tls", "quic"}
                                                 : QJsonArray{"http", "tls", "quic"}},
            {"enabled", true},
            {"metadataOnly", false},
            {"routeOnly", dataStore->sniffing_mode == SniffingMode::FOR_ROUTING},
        };

        // socks-in
        if (InRange(dataStore->inbound_socks_port, 0, 65535) && !forTest) {
            QJsonObject socksInbound;
            socksInbound["tag"] = "socks-in";
            socksInbound["protocol"] = "socks";
            socksInbound["listen"] = dataStore->inbound_address;
            socksInbound["port"] = dataStore->inbound_socks_port;
            socksInbound["settings"] = QJsonObject{
                {"auth", "noauth"},
                {"udp", true},
            };
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
            if (dataStore->sniffing_mode != SniffingMode::DISABLE) {
                socksInbound["sniffing"] = sniffing;
            }
            status->inbounds += socksInbound;
        }

        // Outbounds
        auto tagProxy = BuildChain(0, status);
        if (!result->error.isEmpty()) return result;

        // direct & bypass & block
        status->outbounds += QJsonObject{
            {"protocol", "freedom"},
            {"tag", "direct"},
        };
        status->outbounds += QJsonObject{
            {"protocol", "freedom"},
            {"tag", "bypass"},
        };
        status->outbounds += QJsonObject{
            {"protocol", "blackhole"},
            {"tag", "block"},
        };

        // block for tun
        if (!forTest) {
            status->routingRules += QJsonObject{
                {"type", "field"},
                {
                    "ip",
                    QJsonArray{
                        "224.0.0.0/3",
                        "169.254.0.0/16",
                    },
                },
                {"outboundTag", "block"},
            };
            status->routingRules += QJsonObject{
                {"type", "field"},
                {"port", "135-139"},
                {"outboundTag", "block"},
            };
        }

        // DNS Routing (tun2socks 用到，防污染)
        if (dataStore->dns_routing && !forTest) {
            QJsonObject dnsOut;
            dnsOut["protocol"] = "dns";
            dnsOut["tag"] = "dns-out";
            QJsonObject dnsOut_settings;
            dnsOut_settings["network"] = "tcp";
            dnsOut_settings["port"] = 53;
            dnsOut_settings["address"] = "8.8.8.8";
            dnsOut_settings["userLevel"] = 1;
            dnsOut["settings"] = dnsOut_settings;
            dnsOut["proxySettings"] = QJsonObject{{"tag", tagProxy},
                                                  {"transportLayer", true}};

            status->outbounds += dnsOut;
            status->routingRules += QJsonObject{
                {"type", "field"},
                {"port", "53"},
                {"inboundTag", QJsonArray{"socks-in", "http-in"}},
                {"outboundTag", "dns-out"},
            };
            status->routingRules += QJsonObject{
                {"type", "field"},
                {"inboundTag", QJsonArray{"dns-in"}},
                {"outboundTag", "dns-out"},
            };
        }

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

        // Remote or FakeDNS
        QJsonObject dnsServerRemote;
        dnsServerRemote["address"] = dataStore->fake_dns ? "fakedns" : dataStore->remote_dns;
        dnsServerRemote["domains"] = status->domainListDNSRemote;
        if (!forTest) dnsServers += dnsServerRemote;

        // Direct
        auto directDnsAddress = dataStore->direct_dns;
        if (directDnsAddress.contains("://")) {
            auto directDnsIp = SubStrBefore(SubStrAfter(directDnsAddress, "://"), "/");
            if (IsIpAddress(directDnsIp)) {
                status->routingRules.push_front(QJsonObject{
                    {"type", "field"},
                    {"ip", QJsonArray{directDnsIp}},
                    {"outboundTag", "direct"},
                });
            } else {
                status->routingRules.push_front(QJsonObject{
                    {"type", "field"},
                    {"domain", QJsonArray{directDnsIp}},
                    {"outboundTag", "direct"},
                });
            }
        } else if (directDnsAddress != "localhost") {
            status->routingRules.push_front(QJsonObject{
                {"type", "field"},
                {"ip", QJsonArray{directDnsAddress}},
                {"outboundTag", "direct"},
            });
        }
        dnsServers += QJsonObject{
            {"address", directDnsAddress},
            {"domains", status->domainListDNSDirect},
            {"skipFallback", true},
        };

        dns["disableFallbackIfMatch"] = true;
        dns["servers"] = dnsServers;
        dns["tag"] = "dns";
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
        routingRule_tmp["outboundTag"] = tagProxy;
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
        auto routingRules = QString2QJsonObject(dataStore->routing->custom)["rules"].toArray();
        QJSONARRAY_ADD(routingRules, QString2QJsonObject(dataStore->custom_route_global)["rules"].toArray())
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

    QString BuildChainInternal(int chainId, const QList<QSharedPointer<ProxyEntity>> &ents,
                               const QSharedPointer<BuildConfigStatus> &status) {
        QString chainTag = "c-" + Int2String(chainId);
        QString chainTagOut;
        bool muxApplied = false;

        QString pastTag;
        int pastExternalStat = 0;
        int index = 0;

        for (const auto &ent: ents) {
            // tagOut: v2ray outbound tag for a profile
            // profile2 (in) (global)   tag g-(id)
            // profile1                 tag (chainTag)-(id)
            // profile0 (out)           tag (chainTag)-(id) / single: chainTag=g-(id)
            auto tagOut = chainTag + "-" + Int2String(ent->id);

            // needGlobal: can only contain one?
            bool needGlobal = false;

            // first profile set as global
            auto isFirstProfile = index == ents.length() - 1;
            if (isFirstProfile) {
                needGlobal = true;
                tagOut = "g-" + Int2String(ent->id);
            }

            // last profile set as "proxy"
            if (chainId == 0 && index == 0) {
                needGlobal = false;
                tagOut = "proxy";
            }

            // ignoreConnTag
            if (index != 0) {
                status->result->ignoreConnTag << tagOut;
            }

            if (needGlobal) {
                if (status->globalProfiles.contains(ent->id)) {
                    continue;
                }
                status->globalProfiles += ent->id;
            }

            if (index > 0) {
                // chain rules: past
                if (pastExternalStat == 0) {
                    auto replaced = status->outbounds.last().toObject();
                    if (IS_NEKO_BOX) {
                        replaced["detour"] = tagOut;
                    } else {
                        replaced["proxySettings"] = QJsonObject{
                            {"tag", tagOut},
                            {"transportLayer", true},
                        };
                    }
                    status->outbounds.removeLast();
                    status->outbounds += replaced;
                } else {
                    if (IS_NEKO_BOX) {
                        status->routingRules += QJsonObject{
                            {"inbound", QJsonArray{pastTag + "-mapping"}},
                            {"outbound", tagOut},
                        };
                    } else {
                        status->routingRules += QJsonObject{
                            {"type", "field"},
                            {"inboundTag", QJsonArray{pastTag + "-mapping"}},
                            {"outboundTag", tagOut},
                        };
                    }
                }
            } else {
                // index == 0 means last profile in chain / not chain
                chainTagOut = tagOut;
                status->result->outboundStat = ent->traffic_data;
            }

            // chain rules: this
            auto mapping_port = MkPort();
            auto thisExternalStat = ent->bean->NeedExternal(isFirstProfile, dataStore->running_spmode == SystemProxyMode::VPN);
            if (thisExternalStat == 2) dataStore->need_keep_vpn_off = true;
            if (thisExternalStat == 1) {
                // mapping
                if (IS_NEKO_BOX) {
                    status->inbounds += QJsonObject{
                        {"type", "direct"},
                        {"tag", tagOut + "-mapping"},
                        {"listen", "127.0.0.1"},
                        {"listen_port", mapping_port},
                        {"override_address", ent->bean->serverAddress},
                        {"override_port", ent->bean->serverPort},
                    };
                } else {
                    status->inbounds += QJsonObject{
                        {"protocol", "dokodemo-door"},
                        {"tag", tagOut + "-mapping"},
                        {"listen", "127.0.0.1"},
                        {"port", mapping_port},
                        {"settings", QJsonObject{
                                         // to
                                         {"address", ent->bean->serverAddress},
                                         {"port", ent->bean->serverPort},
                                         {"network", "tcp,udp"},
                                     }},
                    };
                }
                // no chain rule and not outbound, so need to set to direct
                if (isFirstProfile) {
                    if (IS_NEKO_BOX) {
                        status->routingRules += QJsonObject{
                            {"inbound", QJsonArray{tagOut + "-mapping"}},
                            {"outbound", "direct"},
                        };
                    } else {
                        status->routingRules += QJsonObject{
                            {"type", "field"},
                            {"inboundTag", QJsonArray{tagOut + "-mapping"}},
                            {"outboundTag", "direct"},
                        };
                    }
                }
            }

            // Outbound

            QJsonObject outbound;
            fmt::CoreObjOutboundBuildResult coreR;
            fmt::ExternalBuildResult extR;

            if (thisExternalStat > 0) {
                auto ext_socks_port = MkPort();
                extR = ent->bean->BuildExternal(mapping_port, ext_socks_port, thisExternalStat);
                if (extR.program.isEmpty()) {
                    status->result->error = QObject::tr("Core not found: %1").arg(ent->bean->DisplayType());
                    return {};
                }
                if (!extR.error.isEmpty()) { // rejected
                    status->result->error = extR.error;
                    return {};
                }

                // SOCKS OUTBOUND
                if (IS_NEKO_BOX) {
                    outbound["type"] = "socks";
                    outbound["server"] = "127.0.0.1";
                    outbound["server_port"] = ext_socks_port;
                } else {
                    outbound["protocol"] = "socks";
                    QJsonObject settings;
                    QJsonArray servers;
                    QJsonObject server;
                    server["address"] = "127.0.0.1";
                    server["port"] = ext_socks_port;
                    servers.push_back(server);
                    settings["servers"] = servers;
                    outbound["settings"] = settings;
                }

                // EXTERNAL PROCESS
                auto extC = new sys::ExternalProcess();
                extC->tag = ent->bean->DisplayType();
                extC->program = extR.program;
                extC->arguments = extR.arguments;
                extC->env = extR.env;
                status->result->ext += extC;
            } else {
                coreR = IS_NEKO_BOX ? ent->bean->BuildCoreObjSingBox() : ent->bean->BuildCoreObjV2Ray();
                if (coreR.outbound.isEmpty()) {
                    status->result->error = "unsupported outbound";
                    return {};
                }
                if (!coreR.error.isEmpty()) { // rejected
                    status->result->error = coreR.error;
                    return {};
                }
                outbound = coreR.outbound;
            }

            // outbound misc
            outbound["tag"] = tagOut;
            ent->traffic_data->id = ent->id;
            ent->traffic_data->tag = tagOut.toStdString();
            status->result->outboundStats += ent->traffic_data;

            if (IS_NEKO_BOX) {
                // TODO no such field?
                auto ds = dataStore->outbound_domain_strategy;
                if (ds == "UseIPv4") {
                    ds = "ipv4_only";
                } else if (ds == "UseIPv6") {
                    ds = "ipv6_only";
                } else if (ds == "PreferIPv4") {
                    ds = "prefer_ipv4";
                } else if (ds == "PreferIPv6") {
                    ds = "prefer_ipv6";
                } else {
                    ds = "";
                }
                outbound["domain_strategy"] = ds;
                // TODO apply mux
            } else {
                outbound["domainStrategy"] = dataStore->outbound_domain_strategy;
                // apply mux
                if (dataStore->mux_cool > 0 && !muxApplied) {
                    // TODO refactor mux settings
                    if (ent->type == "vmess" || ent->type == "trojan" || ent->type == "vless") {
                        auto muxObj = QJsonObject{
                            {"enabled", true},
                            {"concurrency", dataStore->mux_cool},
                        };
                        auto stream = GetStreamSettings(ent->bean.data());
                        if (stream != nullptr && !stream->packet_encoding.isEmpty()) {
                            muxObj["packetEncoding"] = stream->packet_encoding;
                        }
                        outbound["mux"] = muxObj;
                        muxApplied = true;
                    }
                }
            }

            // apply custom outbound settings
            auto custom_item = ent->bean->_get("custom");
            if (custom_item != nullptr) {
                ApplyCustomOutboundJsonSettings(QString2QJsonObject(*((QString *) custom_item->ptr)), outbound);
            }

            // Bypass Lookup for the first profile
            if (isFirstProfile && !IsIpAddress(ent->bean->serverAddress)) {
                if (dataStore->enhance_resolve_server_domain && !IS_NEKO_BOX) {
                    status->result->tryDomains += ent->bean->serverAddress;
                } else {
                    status->domainListDNSDirect += "full:" + ent->bean->serverAddress;
                }
            }

            status->outbounds += outbound;
            pastTag = tagOut;
            pastExternalStat = thisExternalStat;
            index++;
        }

        return chainTagOut;
    }

    // SingBox

    QSharedPointer<BuildConfigResult> BuildConfigSingBox(const QSharedPointer<ProxyEntity> &ent, bool forTest, bool forExport) {
        auto result = QSharedPointer<BuildConfigResult>(new BuildConfigResult);
        auto status = QSharedPointer<BuildConfigStatus>(new BuildConfigStatus);
        status->ent = ent;
        status->result = result;

        // Log
        result->coreConfig["log"] = QJsonObject{{"level", dataStore->log_level}};

        // Inbounds

        // mixed-in
        if (InRange(dataStore->inbound_socks_port, 0, 65535) && !forTest) {
            QJsonObject socksInbound;
            socksInbound["tag"] = "mixed-in";
            socksInbound["type"] = "mixed";
            socksInbound["listen"] = dataStore->inbound_address;
            socksInbound["listen_port"] = dataStore->inbound_socks_port;
            if (dataStore->sniffing_mode != SniffingMode::DISABLE) {
                socksInbound["sniff"] = true;
                socksInbound["sniff_override_destination"] = dataStore->sniffing_mode == SniffingMode::FOR_DESTINATION;
            }
            status->inbounds += socksInbound;
        }

        // Outbounds
        auto tagProxy = BuildChain(0, status);
        if (!result->error.isEmpty()) return result;

        // direct & bypass & block
        status->outbounds += QJsonObject{
            {"type", "direct"},
            {"tag", "direct"},
        };
        status->outbounds += QJsonObject{
            {"type", "direct"},
            {"tag", "bypass"},
        };
        status->outbounds += QJsonObject{
            {"type", "block"},
            {"tag", "block"},
        };
        status->outbounds += QJsonObject{
            {"type", "dns"},
            {"tag", "dns-out"},
        };

        // custom inbound
        QJSONARRAY_ADD(status->inbounds, QString2QJsonObject(dataStore->custom_inbound)["inbounds"].toArray())

        result->coreConfig.insert("inbounds", status->inbounds);
        result->coreConfig.insert("outbounds", status->outbounds);

        // dns domain user rule
        for (const auto &line: SplitLines(dataStore->routing->proxy_domain)) {
            if (line.startsWith("#")) continue;
            status->domainListDNSRemote += line;
            status->domainListRemote += line;
        }
        for (const auto &line: SplitLines(dataStore->routing->direct_domain)) {
            if (line.startsWith("#")) continue;
            status->domainListDNSDirect += line;
            status->domainListDirect += line;
        }
        for (const auto &line: SplitLines(dataStore->routing->block_domain)) {
            if (line.startsWith("#")) continue;
            status->domainListBlock += line;
        }

        //
        auto make_rule = [&](const QJsonArray &arr, bool isIP = false) {
            QJsonObject rule;
            QJsonArray ips;
            QJsonArray geoips;
            QJsonArray domain_keyword;
            QJsonArray domain_subdomain;
            QJsonArray domain_full;
            QJsonArray domain_suffix;
            QJsonArray geosites;
            for (const auto &domain_: arr) {
                auto domain = domain_.toString();
                if (isIP) {
                    if (domain.startsWith("geoip:")) {
                        geoips += domain.replace("geoip:", "");
                    } else {
                        ips += domain;
                    }
                } else {
                    if (domain.startsWith("geosite:")) {
                        geosites += domain.replace("geosite:", "");
                    } else if (domain.startsWith("full:")) {
                        domain_full += domain.replace("full:", "");
                    } else if (domain.startsWith("domain:")) {
                        domain_suffix += domain.replace("domain:", "");
                    } else {
                        domain_keyword += domain;
                    }
                }
            }
            if (isIP) {
                if (ips.isEmpty() && geoips.isEmpty()) return rule;
                rule["ip_cidr"] = ips;
                rule["geoip"] = geoips;
            } else {
                if (domain_keyword.isEmpty() && domain_subdomain.isEmpty() && domain_full.isEmpty() && geosites.isEmpty()) {
                    return rule;
                }
                rule["domain"] = domain_full;
                rule["domain_suffix"] = domain_suffix;
                rule["domain_keyword"] = domain_keyword;
                rule["geosite"] = geosites;
            }
            return rule;
        };

        // final add DNS
        QJsonObject dns;
        QJsonArray dnsServers;
        QJsonArray dnsRules;

        // Remote
        if (!forTest)
            dnsServers += QJsonObject{
                {"tag", "dns-remote"},
                {"address_resolver", "dns-underlying"},
                {"address", dataStore->remote_dns},
                {"detour", tagProxy},
            };

        // neko only
        auto underlyingStr = forExport ? "local" : "underlying://0.0.0.0";

        // Direct
        auto directDNSAddress = dataStore->direct_dns;
        if (directDNSAddress == "localhost") directDNSAddress = underlyingStr;
        if (!forTest)
            dnsServers += QJsonObject{
                {"tag", "dns-direct"},
                {"address_resolver", "dns-underlying"},
                {"address", directDNSAddress.replace("+local://", "://")},
                {"detour", "direct"},
            };

        // Underlying 100% Working DNS
        dnsServers += QJsonObject{
            {"tag", "dns-underlying"},
            {"address", underlyingStr},
            {"detour", "direct"},
        };

        // DNS rules
        auto add_rule_dns = [&](const QJsonArray &arr, const QString &server) {
            auto rule = make_rule(arr, false);
            if (rule.isEmpty()) return;
            rule["server"] = server;
            dnsRules += rule;
        };

        add_rule_dns(status->domainListDNSRemote, "dns-remote");
        add_rule_dns(status->domainListDNSDirect, "dns-direct");

        dns["servers"] = dnsServers;
        dns["rules"] = dnsRules;
        result->coreConfig.insert("dns", dns);

        // Routing

        // custom routing rule (top)
        auto routingRules = QString2QJsonObject(dataStore->routing->custom)["rules"].toArray();

        // dns hijack
        routingRules += QJsonObject{{"protocol", "dns"},
                                    {"outbound", "dns-out"}};

        auto add_rule_route = [&](const QJsonArray &arr, bool isIP, const QString &out) {
            auto rule = make_rule(arr, isIP);
            if (rule.isEmpty()) return;
            rule["outbound"] = out;
            routingRules += rule;
        };

        // ip user rule
        for (const auto &line: SplitLines(dataStore->routing->block_ip)) {
            if (line.startsWith("#")) continue;
            status->ipListBlock += line;
        }
        for (const auto &line: SplitLines(dataStore->routing->proxy_ip)) {
            if (line.startsWith("#")) continue;
            status->ipListRemote += line;
        }
        for (const auto &line: SplitLines(dataStore->routing->direct_ip)) {
            if (line.startsWith("#")) continue;
            status->ipListDirect += line;
        }
        add_rule_route(status->ipListBlock, true, "block");
        add_rule_route(status->ipListRemote, true, tagProxy);
        add_rule_route(status->ipListDirect, true, "bypass");

        // domain user rule
        add_rule_route(status->domainListBlock, false, "block");
        add_rule_route(status->domainListRemote, false, tagProxy);
        add_rule_route(status->domainListDirect, false, "bypass");

        // geopath
        auto geoip = FindCoreAsset("geoip.db");
        auto geosite = FindCoreAsset("geosite.db");
        if (geoip.isEmpty()) result->error = +"geoip.db not found";
        if (geosite.isEmpty()) result->error = +"geosite.db not found";

        // final add routing rule
        QJSONARRAY_ADD(routingRules, QString2QJsonObject(dataStore->custom_route_global)["rules"].toArray())
        QJSONARRAY_ADD(routingRules, status->routingRules)
        auto routeObj = QJsonObject{
            {"rules", routingRules},
            {"auto_detect_interface", true},
            {
                "geoip",
                QJsonObject{
                    {"path", geoip},
                },
            },
            {
                "geosite",
                QJsonObject{
                    {"path", geosite},
                },
            }};
        if (forExport) {
            routeObj.remove("geoip");
            routeObj.remove("geosite");
            routeObj.remove("auto_detect_interface");
        }
        result->coreConfig.insert("route", routeObj);

        // api
        if (!forTest && !forExport && dataStore->traffic_loop_interval > 0) {
            result->coreConfig.insert("experimental", QJsonObject{
                                                          {"v2ray_api", QJsonObject{
                                                                            {"listen", "127.0.0.1:" + Int2String(dataStore->inbound_socks_port + 10)},
                                                                            {"stats", QJsonObject{
                                                                                          {"enabled", true},
                                                                                          {"outbounds", QJsonArray{tagProxy, "bypass"}},
                                                                                      }}}},
                                                      });
        }

        return result;
    }

    QString WriteVPNSingBoxConfig() {
        //
        QString process_name_rule = dataStore->vpn_bypass_process.trimmed();
        if (!process_name_rule.isEmpty()) {
            auto arr = SplitLines(process_name_rule);
            QJsonObject rule{{"outbound", "direct"},
                             {"process_name", QList2QJsonArray(arr)}};
            process_name_rule = "," + QJsonObject2QString(rule, false);
        }
        QString cidr_rule = dataStore->vpn_bypass_cidr.trimmed();
        if (!cidr_rule.isEmpty()) {
            auto arr = SplitLines(cidr_rule);
            QJsonObject rule{{"outbound", "direct"},
                             {"ip_cidr", QList2QJsonArray(arr)}};
            cidr_rule = "," + QJsonObject2QString(rule, false);
        }
        //
        auto tun_name = "nekoray-tun";
#ifdef Q_OS_MACOS
        tun_name = "utun9";
#endif
        // gen config
        auto configFn = ":/neko/vpn/sing-box-vpn.json";
        if (QFile::exists("vpn/sing-box-vpn.json")) configFn = "vpn/sing-box-vpn.json";
        auto config = ReadFileText(configFn)
                          .replace("%IPV6_ADDRESS%", dataStore->vpn_ipv6 ? R"("inet6_address": "fdfe:dcba:9876::1/126",)" : "")
                          .replace("%MTU%", Int2String(dataStore->vpn_mtu))
                          .replace("%STACK%", Preset::SingBox::VpnImplementation.value(dataStore->vpn_implementation))
                          .replace("%PROCESS_NAME_RULE%", process_name_rule)
                          .replace("%CIDR_RULE%", cidr_rule)
                          .replace("%TUN_NAME%", tun_name)
                          .replace("%STRICT_ROUTE%", dataStore->vpn_strict_route ? "true" : "false")
                          .replace("%PORT%", Int2String(dataStore->inbound_socks_port));
        // hook.js
        auto source = qjs::ReadHookJS();
        if (!source.isEmpty()) {
            qjs::QJS js(source);
            auto js_result = js.EvalFunction("hook.hook_vpn_config", config);
            if (config != js_result) {
                MW_show_log("hook.js modified your VPN config.");
                config = js_result;
            }
        }
        // write config
        QFile file;
        file.setFileName(QFileInfo(configFn).fileName());
        file.open(QIODevice::ReadWrite | QIODevice::Truncate);
        file.write(config.toUtf8());
        file.close();
        return QFileInfo(file).absoluteFilePath();
    }

    QString WriteVPNLinuxScript(const QString &protectPath, const QString &configPath) {
        // gen script
        auto scriptFn = ":/neko/vpn/vpn-run-root.sh";
        if (QFile::exists("vpn/vpn-run-root.sh")) scriptFn = "vpn/vpn-run-root.sh";
        auto script = ReadFileText(scriptFn)
                          .replace("$PORT", Int2String(dataStore->inbound_socks_port))
                          .replace("./nekobox_core", QApplication::applicationDirPath() + "/nekobox_core")
                          .replace("$PROTECT_LISTEN_PATH", protectPath)
                          .replace("$CONFIG_PATH", configPath)
                          .replace("$TABLE_FWMARK", "514");
        // hook.js
        auto source = qjs::ReadHookJS();
        if (!source.isEmpty()) {
            qjs::QJS js(source);
            auto js_result = js.EvalFunction("hook.hook_vpn_script", script);
            if (script != js_result) {
                MW_show_log("hook.js modified your VPN script.");
                script = js_result;
            }
        }
        // write script
        QFile file2;
        file2.setFileName(QFileInfo(scriptFn).fileName());
        file2.open(QIODevice::ReadWrite | QIODevice::Truncate);
        file2.write(script.toUtf8());
        file2.close();
        return QFileInfo(file2).absoluteFilePath();
    }

} // namespace NekoRay