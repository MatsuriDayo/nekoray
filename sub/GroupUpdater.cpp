#include "db/Database.hpp"
#include "db/ProfileFilter.hpp"
#include "fmt/includes.h"
#include "fmt/Preset.hpp"
#include "main/QJS.hpp"
#include "main/HTTPRequestHelper.hpp"

#include "GroupUpdater.hpp"

#include <QInputDialog>
#include <QUrlQuery>

#ifndef NKR_NO_YAML

#include <yaml-cpp/yaml.h>

#endif

namespace NekoGui_sub {

    GroupUpdater *groupUpdater = new GroupUpdater;

    void RawUpdater_FixEnt(const std::shared_ptr<NekoGui::ProxyEntity> &ent) {
        if (ent == nullptr) return;
        auto stream = NekoGui_fmt::GetStreamSettings(ent->bean.get());
        if (stream == nullptr) return;
        // 1. "security"
        if (stream->security == "none" || stream->security == "0" || stream->security == "false") {
            stream->security = "";
        } else if (stream->security == "1" || stream->security == "true") {
            stream->security = "tls";
        }
        // 2. TLS SNI: v2rayN config builder generate sni like this, so set sni here for their format.
        if (stream->security == "tls" && IsIpAddress(ent->bean->serverAddress) && (!stream->host.isEmpty()) && stream->sni.isEmpty()) {
            stream->sni = stream->host;
        }
    }

    void RawUpdater::update(const QString &str) {
        // Base64 encoded subscription
        if (auto str2 = DecodeB64IfValid(str); !str2.isEmpty()) {
            update(str2);
            return;
        }

        // Clash
        if (str.contains("proxies:")) {
            updateClash(str);
            return;
        }

        // Multi line
        if (str.count("\n") > 0) {
            auto list = str.split("\n");
            for (const auto &str2: list) {
                update(str2.trimmed());
            }
            return;
        }

        std::shared_ptr<NekoGui::ProxyEntity> ent;
        bool needFix = true;

        // Nekoray format
        if (str.startsWith("nekoray://")) {
            needFix = false;
            auto link = QUrl(str);
            if (!link.isValid()) return;
            ent = NekoGui::ProfileManager::NewProxyEntity(link.host());
            if (ent->bean->version == -114514) return;
            auto j = DecodeB64IfValid(link.fragment().toUtf8(), QByteArray::Base64UrlEncoding);
            if (j.isEmpty()) return;
            ent->bean->FromJsonBytes(j);
            MW_show_log("nekoray format: " + ent->bean->DisplayTypeAndName());
        }

        // SOCKS
        if (str.startsWith("socks5://") || str.startsWith("socks4://") ||
            str.startsWith("socks4a://") || str.startsWith("socks://")) {
            ent = NekoGui::ProfileManager::NewProxyEntity("socks");
            auto ok = ent->SocksHTTPBean()->TryParseLink(str);
            if (!ok) return;
        }

        // HTTP
        if (str.startsWith("http://") || str.startsWith("https://")) {
            ent = NekoGui::ProfileManager::NewProxyEntity("http");
            auto ok = ent->SocksHTTPBean()->TryParseLink(str);
            if (!ok) return;
        }

        // ShadowSocks
        if (str.startsWith("ss://")) {
            ent = NekoGui::ProfileManager::NewProxyEntity("shadowsocks");
            auto ok = ent->ShadowSocksBean()->TryParseLink(str);
            if (!ok) return;
        }

        // VMess
        if (str.startsWith("vmess://")) {
            ent = NekoGui::ProfileManager::NewProxyEntity("vmess");
            auto ok = ent->VMessBean()->TryParseLink(str);
            if (!ok) return;
        }

        // VLESS
        if (str.startsWith("vless://")) {
            ent = NekoGui::ProfileManager::NewProxyEntity("vless");
            auto ok = ent->TrojanVLESSBean()->TryParseLink(str);
            if (!ok) return;
        }

        // Trojan
        if (str.startsWith("trojan://")) {
            ent = NekoGui::ProfileManager::NewProxyEntity("trojan");
            auto ok = ent->TrojanVLESSBean()->TryParseLink(str);
            if (!ok) return;
        }

        // Naive
        if (str.startsWith("naive+")) {
            needFix = false;
            ent = NekoGui::ProfileManager::NewProxyEntity("naive");
            auto ok = ent->NaiveBean()->TryParseLink(str);
            if (!ok) return;
        }

        // Hysteria
        if (str.startsWith("hysteria://")) {
            needFix = false;
            ent = NekoGui::ProfileManager::NewProxyEntity("hysteria");
            auto ok = ent->QUICBean()->TryParseLink(str);
            if (!ok) return;
        }

        if (ent == nullptr) return;

        // Fix
        if (needFix) RawUpdater_FixEnt(ent);

        // End
        NekoGui::profileManager->AddProfile(ent, gid_add_to);
        update_counter++;
    }

#ifndef NKR_NO_YAML

    QString Node2QString(const YAML::Node &n, const QString &def = "") {
        try {
            return n.as<std::string>().c_str();
        } catch (const YAML::Exception &ex) {
            qDebug() << ex.what();
            return def;
        }
    }

    QStringList Node2QStringList(const YAML::Node &n) {
        try {
            if (n.IsSequence()) {
                QStringList list;
                for (auto item: n) {
                    list << item.as<std::string>().c_str();
                }
                return list;
            } else {
                return {};
            }
        } catch (const YAML::Exception &ex) {
            qDebug() << ex.what();
            return {};
        }
    }

    int Node2Int(const YAML::Node &n, const int &def = 0) {
        try {
            return n.as<int>();
        } catch (const YAML::Exception &ex) {
            qDebug() << ex.what();
            return def;
        }
    }

    bool Node2Bool(const YAML::Node &n, const bool &def = false) {
        try {
            return n.as<bool>();
        } catch (const YAML::Exception &ex) {
            try {
                return n.as<int>();
            } catch (const YAML::Exception &ex2) {
                ex2.what();
            }
            qDebug() << ex.what();
            return def;
        }
    }

    // NodeChild returns the first defined children or Null Node
    YAML::Node NodeChild(const YAML::Node &n, const std::list<std::string> &keys) {
        for (const auto &key: keys) {
            auto child = n[key];
            if (child.IsDefined()) return child;
        }
        return {};
    }

#endif

    // https://github.com/Dreamacro/clash/wiki/configuration
    void RawUpdater::updateClash(const QString &str) {
#ifndef NKR_NO_YAML
        try {
            auto proxies = YAML::Load(str.toStdString())["proxies"];
            for (auto proxy: proxies) {
                auto type = Node2QString(proxy["type"]);
                auto type_clash = type;

                if (type == "ss" || type == "ssr") type = "shadowsocks";
                if (type == "socks5") type = "socks";

                auto ent = NekoGui::ProfileManager::NewProxyEntity(type);
                if (ent->bean->version == -114514) continue;
                bool needFix = false;

                // common
                ent->bean->name = Node2QString(proxy["name"]);
                ent->bean->serverAddress = Node2QString(proxy["server"]);
                ent->bean->serverPort = Node2Int(proxy["port"]);

                if (type == "shadowsocks") {
                    auto bean = ent->ShadowSocksBean();
                    bean->method = Node2QString(proxy["cipher"]).replace("dummy", "none");
                    bean->password = Node2QString(proxy["password"]);
                    auto plugin_n = proxy["plugin"];
                    auto pluginOpts_n = proxy["plugin-opts"];

                    // UDP over TCP
                    if (Node2Bool(proxy["udp-over-tcp"])) {
                        bean->uot = Node2Int(proxy["udp-over-tcp-version"]);
                        if (bean->uot == 0) bean->uot = 2;
                    }

                    if (plugin_n.IsDefined() && pluginOpts_n.IsDefined()) {
                        QStringList ssPlugin;
                        auto plugin = Node2QString(plugin_n);
                        if (plugin == "obfs") {
                            ssPlugin << "obfs-local";
                            ssPlugin << "obfs=" + Node2QString(pluginOpts_n["mode"]);
                            ssPlugin << "obfs-host=" + Node2QString(pluginOpts_n["host"]);
                        } else if (plugin == "v2ray-plugin") {
                            auto mode = Node2QString(pluginOpts_n["mode"]);
                            auto host = Node2QString(pluginOpts_n["host"]);
                            auto path = Node2QString(pluginOpts_n["path"]);
                            ssPlugin << "v2ray-plugin";
                            if (!mode.isEmpty() && mode != "websocket") ssPlugin << "mode=" + mode;
                            if (Node2Bool(pluginOpts_n["tls"])) ssPlugin << "tls";
                            if (!host.isEmpty()) ssPlugin << "host=" + host;
                            if (!path.isEmpty()) ssPlugin << "path=" + path;
                            // clash only: skip-cert-verify
                            // clash only: headers
                            // clash: mux=?
                        }
                        bean->plugin = ssPlugin.join(";");
                    }
                    auto protocol_n = proxy["protocol"];
                    if (protocol_n.IsDefined()) {
                        continue; // SSR
                    }
                } else if (type == "socks" || type == "http") {
                    auto bean = ent->SocksHTTPBean();
                    bean->username = Node2QString(proxy["username"]);
                    bean->password = Node2QString(proxy["password"]);
                    if (Node2Bool(proxy["tls"])) bean->stream->security = "tls";
                    if (Node2Bool(proxy["skip-cert-verify"])) bean->stream->allow_insecure = true;
                } else if (type == "trojan" || type == "vless") {
                    needFix = true;
                    auto bean = ent->TrojanVLESSBean();
                    if (type == "vless") {
                        bean->flow = Node2QString(proxy["flow"]);
                        bean->password = Node2QString(proxy["uuid"]);
                        // meta packet encoding
                        if (Node2Bool(proxy["packet-addr"])) {
                            bean->stream->packet_encoding = "packetaddr";
                        } else {
                            // For VLESS, default to use xudp
                            bean->stream->packet_encoding = "xudp";
                        }
                    } else {
                        bean->password = Node2QString(proxy["password"]);
                    }
                    bean->stream->security = "tls";
                    bean->stream->network = Node2QString(proxy["network"], "tcp");
                    bean->stream->sni = FIRST_OR_SECOND(Node2QString(proxy["sni"]), Node2QString(proxy["servername"]));
                    bean->stream->alpn = Node2QStringList(proxy["alpn"]).join(",");
                    bean->stream->allow_insecure = Node2Bool(proxy["skip-cert-verify"]);
                    bean->stream->utlsFingerprint = Node2QString(proxy["client-fingerprint"]);

                    // opts
                    auto ws = NodeChild(proxy, {"ws-opts", "ws-opt"});
                    if (ws.IsMap()) {
                        auto headers = ws["headers"];
                        for (auto header: headers) {
                            if (Node2QString(header.first).toLower() == "host") {
                                bean->stream->host = Node2QString(header.second);
                            }
                        }
                        bean->stream->path = Node2QString(ws["path"]);
                        bean->stream->ws_early_data_length = Node2Int(ws["max-early-data"]);
                        bean->stream->ws_early_data_name = Node2QString(ws["early-data-header-name"]);
                    }

                    auto grpc = NodeChild(proxy, {"grpc-opts", "grpc-opt"});
                    if (grpc.IsMap()) {
                        bean->stream->path = Node2QString(grpc["grpc-service-name"]);
                    }

                    auto reality = NodeChild(proxy, {"reality-opts"});
                    if (reality.IsMap()) {
                        bean->stream->reality_pbk = Node2QString(reality["public-key"]);
                        bean->stream->reality_sid = Node2QString(reality["short-id"]);
                    }
                } else if (type == "vmess") {
                    needFix = true;
                    auto bean = ent->VMessBean();
                    bean->uuid = Node2QString(proxy["uuid"]);
                    bean->aid = Node2Int(proxy["alterId"]);
                    bean->security = Node2QString(proxy["cipher"], bean->security);
                    bean->stream->network = Node2QString(proxy["network"], "tcp").replace("h2", "http");
                    bean->stream->sni = FIRST_OR_SECOND(Node2QString(proxy["sni"]), Node2QString(proxy["servername"]));
                    bean->stream->alpn = Node2QStringList(proxy["alpn"]).join(",");
                    if (Node2Bool(proxy["tls"])) bean->stream->security = "tls";
                    if (Node2Bool(proxy["skip-cert-verify"])) bean->stream->allow_insecure = true;

                    // meta packet encoding
                    if (Node2Bool(proxy["xudp"])) bean->stream->packet_encoding = "xudp";
                    if (Node2Bool(proxy["packet-addr"])) bean->stream->packet_encoding = "packetaddr";

                    // opts
                    auto ws = NodeChild(proxy, {"ws-opts", "ws-opt"});
                    if (ws.IsMap()) {
                        auto headers = ws["headers"];
                        for (auto header: headers) {
                            if (Node2QString(header.first).toLower() == "host") {
                                bean->stream->host = Node2QString(header.second);
                            }
                        }
                        bean->stream->path = Node2QString(ws["path"]);
                        bean->stream->ws_early_data_length = Node2Int(ws["max-early-data"]);
                        bean->stream->ws_early_data_name = Node2QString(ws["early-data-header-name"]);
                    }

                    auto grpc = NodeChild(proxy, {"grpc-opts", "grpc-opt"});
                    if (grpc.IsMap()) {
                        bean->stream->path = Node2QString(grpc["grpc-service-name"]);
                    }

                    auto h2 = NodeChild(proxy, {"h2-opts", "h2-opt"});
                    if (h2.IsMap()) {
                        auto hosts = h2["host"];
                        for (auto host: hosts) {
                            bean->stream->host = Node2QString(host);
                            break;
                        }
                        bean->stream->path = Node2QString(h2["path"]);
                    }

                    auto tcp_http = NodeChild(proxy, {"http-opts", "http-opt"});
                    if (tcp_http.IsMap()) {
                        bean->stream->network = "tcp";
                        bean->stream->header_type = "http";
                        auto headers = tcp_http["headers"];
                        for (auto header: headers) {
                            if (Node2QString(header.first).toLower() == "host") {
                                bean->stream->host = Node2QString(header.second[0]);
                            }
                            break;
                        }
                        auto paths = tcp_http["path"];
                        for (auto path: paths) {
                            bean->stream->path = Node2QString(path);
                            break;
                        }
                    }
                } else if (type == "hysteria") {
                    auto bean = ent->QUICBean();

                    bean->hopPort = Node2QString(proxy["ports"]);
                    if (bean->serverPort == 0) bean->hopPort = Node2QString(proxy["port"]);

                    bean->allowInsecure = Node2Bool(proxy["skip-cert-verify"]);
                    auto alpn = Node2QStringList(proxy["alpn"]);
                    bean->caText = Node2QString(proxy["ca-str"]);
                    if (!alpn.isEmpty()) bean->alpn = alpn[0];
                    bean->sni = Node2QString(proxy["sni"]);

                    auto auth_str = FIRST_OR_SECOND(Node2QString(proxy["auth_str"]), Node2QString(proxy["auth-str"]));
                    auto auth = Node2QString(proxy["auth"]);
                    if (!auth_str.isEmpty()) {
                        bean->authPayloadType = NekoGui_fmt::QUICBean::hysteria_auth_string;
                        bean->authPayload = auth_str;
                    }
                    if (!auth.isEmpty()) {
                        bean->authPayloadType = NekoGui_fmt::QUICBean::hysteria_auth_base64;
                        bean->authPayload = auth;
                    }
                    bean->obfsPassword = Node2QString(proxy["obfs"]);

                    if (Node2Bool(proxy["disable_mtu_discovery"]) || Node2Bool(proxy["disable-mtu-discovery"])) bean->disableMtuDiscovery = true;
                    bean->streamReceiveWindow = Node2Int(proxy["recv-window"]);
                    bean->connectionReceiveWindow = Node2Int(proxy["recv-window-conn"]);

                    auto upMbps = Node2QString(proxy["up"]).split(" ")[0].toInt();
                    auto downMbps = Node2QString(proxy["down"]).split(" ")[0].toInt();
                    if (upMbps > 0) bean->uploadMbps = upMbps;
                    if (downMbps > 0) bean->downloadMbps = downMbps;
                } else if (type == "tuic") {
                    auto bean = ent->QUICBean();

                    bean->uuid = Node2QString(proxy["uuid"]);
                    bean->password = Node2QString(proxy["password"]);

                    if (Node2Int(proxy["heartbeat-interval"]) != 0) {
                        bean->heartbeat = Int2String(Node2Int(proxy["heartbeat-interval"])) + "ms";
                    }

                    bean->udpRelayMode = Node2QString(proxy["udp-relay-mode"], bean->udpRelayMode);
                    bean->congestionControl = Node2QString(proxy["congestion-controller"], bean->congestionControl);

                    bean->disableSni = Node2Bool(proxy["disable-sni"]);
                    bean->zeroRttHandshake = Node2Bool(proxy["reduce-rtt"]);
                    bean->allowInsecure = Node2Bool(proxy["skip-cert-verify"]);
                    bean->alpn = Node2QStringList(proxy["alpn"]).join(",");
                    bean->caText = Node2QString(proxy["ca-str"]);
                    bean->sni = Node2QString(proxy["sni"]);

                    if (!Node2QString(proxy["ip"]).isEmpty()) {
                        if (bean->sni.isEmpty()) bean->sni = bean->serverAddress;
                        bean->serverAddress = Node2QString(proxy["ip"]);
                    }
                } else {
                    continue;
                }

                if (needFix) RawUpdater_FixEnt(ent);
                NekoGui::profileManager->AddProfile(ent, gid_add_to);
                update_counter++;
            }
        } catch (const YAML::Exception &ex) {
            runOnUiThread([=] {
                MessageBoxWarning("YAML Exception", ex.what());
            });
        }
#endif
    }

    // 在新的 thread 运行
    void GroupUpdater::AsyncUpdate(const QString &str, int _sub_gid, const std::function<void()> &finish) {
        auto content = str.trimmed();
        bool asURL = false;
        bool createNewGroup = false;

        if (_sub_gid < 0 && (content.startsWith("http://") || content.startsWith("https://"))) {
            auto items = QStringList{
                QObject::tr("As Subscription (add to this group)"),
                QObject::tr("As Subscription (create new group)"),
                QObject::tr("As link"),
            };
            bool ok;
            auto a = QInputDialog::getItem(nullptr,
                                           QObject::tr("url detected"),
                                           QObject::tr("%1\nHow to update?").arg(content),
                                           items, 0, false, &ok);
            if (!ok) return;
            if (items.indexOf(a) <= 1) asURL = true;
            if (items.indexOf(a) == 1) createNewGroup = true;
        }

        runOnNewThread([=] {
            auto gid = _sub_gid;
            if (createNewGroup) {
                auto group = NekoGui::ProfileManager::NewGroup();
                group->name = QUrl(str).host();
                group->url = str;
                NekoGui::profileManager->AddGroup(group);
                gid = group->id;
                MW_dialog_message("SubUpdater", "NewGroup");
            }
            Update(str, gid, asURL);
            emit asyncUpdateCallback(gid);
            if (finish != nullptr) finish();
        });
    }

    void GroupUpdater::Update(const QString &_str, int _sub_gid, bool _not_sub_as_url) {
        // 创建 rawUpdater
        NekoGui::dataStore->imported_count = 0;
        auto rawUpdater = std::make_unique<RawUpdater>();
        rawUpdater->gid_add_to = _sub_gid;

        // 准备
        QString sub_user_info;
        bool asURL = _sub_gid >= 0 || _not_sub_as_url; // 把 _str 当作 url 处理（下载内容）
        auto content = _str.trimmed();
        auto group = NekoGui::profileManager->GetGroup(_sub_gid);
        if (group != nullptr && group->archive) return;

        // 网络请求
        if (asURL) {
            auto groupName = group == nullptr ? content : group->name;
            MW_show_log(">>>>>>>> " + QObject::tr("Requesting subscription: %1").arg(groupName));

            auto resp = NetworkRequestHelper::HttpGet(content);
            if (!resp.error.isEmpty()) {
                MW_show_log("<<<<<<<< " + QObject::tr("Requesting subscription %1 error: %2").arg(groupName, resp.error + "\n" + resp.data));
                return;
            }

            content = resp.data;
            sub_user_info = NetworkRequestHelper::GetHeader(resp.header, "Subscription-UserInfo");

            MW_show_log("<<<<<<<< " + QObject::tr("Subscription request fininshed: %1").arg(groupName));
        }

        QList<std::shared_ptr<NekoGui::ProxyEntity>> in;         // 更新前
        QList<std::shared_ptr<NekoGui::ProxyEntity>> out_all;    // 更新前 + 更新后
        QList<std::shared_ptr<NekoGui::ProxyEntity>> out;        // 更新后
        QList<std::shared_ptr<NekoGui::ProxyEntity>> only_in;    // 只在更新前有的
        QList<std::shared_ptr<NekoGui::ProxyEntity>> only_out;   // 只在更新后有的
        QList<std::shared_ptr<NekoGui::ProxyEntity>> update_del; // 更新前后都有的，删除更新后多余的

        // 订阅解析前
        if (group != nullptr) {
            in = group->Profiles();
            group->sub_last_update = QDateTime::currentMSecsSinceEpoch() / 1000;
            group->info = sub_user_info;
            group->order.clear();
            group->Save();
            //
            if (NekoGui::dataStore->sub_clear) {
                MW_show_log(QObject::tr("Clearing servers..."));
                for (const auto &profile: in) {
                    NekoGui::profileManager->DeleteProfile(profile->id);
                }
            }
        }

        // hook.js
        auto source = qjs::ReadHookJS();
        if (!source.isEmpty()) {
            qjs::QJS js(source);
            auto js_result = js.EvalFunction("hook.hook_import", content);
            if (content != js_result) {
                MW_show_log("hook.js modified your import content.");
                content = js_result;
            }
        }

        // 解析并添加 profile
        rawUpdater->update(content);

        if (group != nullptr) {
            out_all = group->Profiles();

            // 第一次判定：除了 自定义字段 其他全部相同的，视为相同配置

            NekoGui::ProfileFilter::OnlyInSrc_ByPointer(out_all, in, out);
            NekoGui::ProfileFilter::OnlyInSrc(in, out, only_in);
            NekoGui::ProfileFilter::OnlyInSrc(out, in, only_out);
            NekoGui::ProfileFilter::Common(in, out, update_del, false, true);

            // 第二次判定：是否只更改了 名称 或 地址端口

#define key_without_name QJsonObject2QString(ent->bean->ToJson({"name", "c_cfg", "c_out"}), true) + ent->bean->DisplayType()
#define key_without_serverAddr QJsonObject2QString(ent->bean->ToJson({"addr", "port", "c_cfg", "c_out"}), true) + ent->bean->DisplayType()

            QString notice_added;
            QString notice_deleted;
            std::map<QString, std::shared_ptr<NekoGui::ProxyEntity>> only_out_without_name;
            std::map<QString, std::shared_ptr<NekoGui::ProxyEntity>> only_out_without_serverAddr;

            for (const auto &ent: only_out) {
                only_out_without_name[key_without_name] = ent;
                only_out_without_serverAddr[key_without_serverAddr] = ent;
                notice_added += "[+] " + ent->bean->DisplayTypeAndName() + "\n";
            }

            for (const auto &ent: only_in) {
                // qDebug() << ent->bean->name << key_without_name;
                notice_deleted += "[-] " + ent->bean->DisplayTypeAndName() + "\n";
                if (only_out_without_name.count(key_without_name)) {
                    auto updated = only_out_without_name[key_without_name];
                    ent->bean->name = updated->bean->name;
                    ent->Save();
                    update_del += updated;
                } else if (only_out_without_serverAddr.count(key_without_serverAddr)) {
                    auto updated = only_out_without_serverAddr[key_without_serverAddr];
                    ent->bean->serverAddress = updated->bean->serverAddress;
                    ent->bean->serverPort = updated->bean->serverPort;
                    ent->Save();
                    update_del += updated;
                } else {
                    update_del += ent;
                }
            }

            // Delete unused & show message

            for (const auto &ent: update_del) {
                NekoGui::profileManager->DeleteProfile(ent->id);
            }

            auto change = "\n" + QObject::tr("Added %1 profiles:\n%2\nDeleted %3 Profiles:\n%4")
                                     .arg(only_out.length())
                                     .arg(notice_added)
                                     .arg(only_in.length())
                                     .arg(notice_deleted);
            if (only_out.length() + only_in.length() == 0) change = QObject::tr("Nothing");
            MW_show_log("<<<<<<<< " + QObject::tr("Change of %1:").arg(group->name) + " " + change);
            MW_dialog_message("SubUpdater", "finish-dingyue");
        } else {
            NekoGui::dataStore->imported_count = rawUpdater->update_counter;
            MW_dialog_message("SubUpdater", "finish");
        }
    }
} // namespace NekoGui_sub

bool UI_update_all_groups_Updating = false;

#define should_skip_group(g) (g == nullptr || g->url.isEmpty() || g->archive || (onlyAllowed && g->skip_auto_update))

void serialUpdateSubscription(const QList<int> &groupsTabOrder, int _order, bool onlyAllowed) {
    // calculate next group
    int nextOrder = _order;
    std::shared_ptr<NekoGui::Group> nextGroup;
    forever {
        nextOrder += 1;
        if (nextOrder >= groupsTabOrder.size()) break;
        auto nextGid = groupsTabOrder[nextOrder];
        nextGroup = NekoGui::profileManager->GetGroup(nextGid);
        if (should_skip_group(nextGroup)) continue;
        break;
    }

    // calculate this group
    auto group = NekoGui::profileManager->GetGroup(groupsTabOrder[_order]);
    if (group == nullptr) {
        UI_update_all_groups_Updating = false;
        return;
    }

    // v2.2: listener is moved to GroupItem, no refresh here.
    NekoGui_sub::groupUpdater->AsyncUpdate(group->url, group->id, [=] {
        if (nextGroup == nullptr) {
            UI_update_all_groups_Updating = false;
        } else {
            serialUpdateSubscription(groupsTabOrder, nextOrder, onlyAllowed);
        }
    });
}

void UI_update_all_groups(bool onlyAllowed) {
    if (UI_update_all_groups_Updating) {
        MW_show_log("The last subscription update has not exited.");
        return;
    }
    // first: freeze group order
    auto groupsTabOrder = NekoGui::profileManager->groupsTabOrder;
    for (const auto &gid: groupsTabOrder) {
        auto group = NekoGui::profileManager->GetGroup(gid);
        if (should_skip_group(group)) continue;
        // start
        UI_update_all_groups_Updating = true;
        serialUpdateSubscription(groupsTabOrder, groupsTabOrder.indexOf(gid), onlyAllowed);
    }
}
