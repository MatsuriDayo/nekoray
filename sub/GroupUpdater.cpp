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

#define FIRST_OR_SECOND(a, b) a.isEmpty() ? b : a

namespace NekoRay::sub {

    GroupUpdater *groupUpdater = new GroupUpdater;

    void RawUpdater_FixEnt(const QSharedPointer<ProxyEntity> &ent) {
        if (ent == nullptr) return;
        auto stream = fmt::GetStreamSettings(ent->bean.get());
        if (stream == nullptr) return;
        // 1. "security"
        if (stream->security == "none" || stream->security == "0" || stream->security == "false") {
            stream->security = "";
        } else if (stream->security == "xtls" || stream->security == "1" || stream->security == "true") {
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

        QSharedPointer<ProxyEntity> ent;
        bool needFix = true;

        // Nekoray format
        if (str.startsWith("nekoray://")) {
            needFix = false;
            auto link = QUrl(str);
            if (!link.isValid()) return;
            ent = ProfileManager::NewProxyEntity(link.host());
            if (ent->bean->version == -114514) return;
            auto j = DecodeB64IfValid(link.fragment().toUtf8(), QByteArray::Base64UrlEncoding);
            if (j.isEmpty()) return;
            ent->bean->FromJsonBytes(j);
            MW_show_log("nekoray format: " + ent->bean->DisplayTypeAndName());
        }

        // SOCKS
        if (str.startsWith("socks5://") || str.startsWith("socks4://") ||
            str.startsWith("socks4a://") || str.startsWith("socks://")) {
            ent = ProfileManager::NewProxyEntity("socks");
            auto ok = ent->SocksHTTPBean()->TryParseLink(str);
            if (!ok) return;
        }

        // HTTP
        if (str.startsWith("http://") || str.startsWith("https://")) {
            ent = ProfileManager::NewProxyEntity("http");
            auto ok = ent->SocksHTTPBean()->TryParseLink(str);
            if (!ok) return;
        }

        // ShadowSocks
        if (str.startsWith("ss://")) {
            ent = ProfileManager::NewProxyEntity("shadowsocks");
            auto ok = ent->ShadowSocksBean()->TryParseLink(str);
            if (!ok) return;
        }

        // VMess
        if (str.startsWith("vmess://")) {
            ent = ProfileManager::NewProxyEntity("vmess");
            auto ok = ent->VMessBean()->TryParseLink(str);
            if (!ok) return;
        }

        // VLESS
        if (str.startsWith("vless://")) {
            ent = ProfileManager::NewProxyEntity("vless");
            auto ok = ent->TrojanVLESSBean()->TryParseLink(str);
            if (!ok) return;
        }

        // Trojan
        if (str.startsWith("trojan://")) {
            ent = ProfileManager::NewProxyEntity("trojan");
            auto ok = ent->TrojanVLESSBean()->TryParseLink(str);
            if (!ok) return;
        }

        // Naive
        if (str.startsWith("naive+")) {
            needFix = false;
            ent = ProfileManager::NewProxyEntity("naive");
            auto ok = ent->NaiveBean()->TryParseLink(str);
            if (!ok) return;
        }

        // Hysteria
        if (str.startsWith("hysteria://")) {
            needFix = false;
            // https://github.com/HyNetwork/hysteria/wiki/URI-Scheme
            ent = ProfileManager::NewProxyEntity("custom");
            auto bean = ent->CustomBean();
            auto url = QUrl(str);
            auto query = QUrlQuery(url.query());
            if (url.host().isEmpty() || url.port() == -1 || !query.hasQueryItem("upmbps")) return;
            bean->name = url.fragment();
            bean->serverAddress = url.host();
            bean->serverPort = url.port();
            bean->core = "hysteria";
            bean->command = QString(Preset::Hysteria::command).split(" ");
            auto result = QString2QJsonObject(Preset::Hysteria::config);
            result["server_name"] = url.host(); // default sni
            result["obfs"] = query.queryItemValue("obfsParam");
            result["insecure"] = query.queryItemValue("insecure") == "1";
            result["up_mbps"] = query.queryItemValue("upmbps").toInt();
            result["down_mbps"] = query.queryItemValue("downmbps").toInt();
            result["protocol"] = query.hasQueryItem("protocol") ? query.queryItemValue("protocol") : "udp";
            if (query.hasQueryItem("auth")) result["auth_str"] = query.queryItemValue("auth");
            if (query.hasQueryItem("alpn")) result["alpn"] = query.queryItemValue("alpn");
            if (query.hasQueryItem("peer")) result["server_name"] = query.queryItemValue("peer");
            bean->config_simple = QJsonObject2QString(result, false);
        }

        if (ent == nullptr) return;

        // Fix
        if (needFix) RawUpdater_FixEnt(ent);

        // End
        profileManager->AddProfile(ent, gid_add_to);
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
                if (type == "ss" || type == "ssr") type = "shadowsocks";
                if (type == "socks5") type = "socks";

                auto ent = ProfileManager::NewProxyEntity(type);
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
                } else if (type == "trojan") {
                    needFix = true;
                    auto bean = ent->TrojanVLESSBean();
                    bean->password = Node2QString(proxy["password"]);
                    bean->stream->security = "tls";
                    bean->stream->network = Node2QString(proxy["network"], "tcp");
                    bean->stream->sni = FIRST_OR_SECOND(Node2QString(proxy["sni"]), Node2QString(proxy["servername"]));
                    if (Node2Bool(proxy["skip-cert-verify"])) bean->stream->allow_insecure = true;

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
                    }

                    auto grpc = NodeChild(proxy, {"grpc-opts", "grpc-opt"});
                    if (grpc.IsMap()) {
                        bean->stream->path = Node2QString(grpc["grpc-service-name"]);
                    }
                } else if (type == "vmess") {
                    needFix = true;
                    auto bean = ent->VMessBean();
                    bean->uuid = Node2QString(proxy["uuid"]);
                    bean->aid = Node2Int(proxy["alterId"]);
                    bean->security = Node2QString(proxy["cipher"]);
                    bean->stream->network = Node2QString(proxy["network"], "tcp").replace("h2", "http");
                    bean->stream->sni = FIRST_OR_SECOND(Node2QString(proxy["sni"]), Node2QString(proxy["servername"]));
                    if (Node2Bool(proxy["tls"])) bean->stream->security = "tls";
                    if (Node2Bool(proxy["skip-cert-verify"])) bean->stream->allow_insecure = true;

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
                } else {
                    continue;
                }

                if (needFix) RawUpdater_FixEnt(ent);
                profileManager->AddProfile(ent, gid_add_to);
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

        if (_sub_gid < 0 && (content.startsWith("http://") || content.startsWith("https://"))) {
            auto items = QStringList{QObject::tr("As Subscription"), QObject::tr("As link")};
            bool ok;
            auto a = QInputDialog::getItem(nullptr,
                                           QObject::tr("url detected"),
                                           QObject::tr("%1\nHow to update?").arg(content),
                                           items, 0, false, &ok);
            if (!ok) return;
            if (items.indexOf(a) == 0) asURL = true;
        }

        runOnNewThread([=] {
            Update(str, _sub_gid, asURL);
            emit asyncUpdateCallback(_sub_gid);
            if (finish != nullptr) finish();
        });
    }

    void GroupUpdater::Update(const QString &_str, int _sub_gid, bool _not_sub_as_url) {
        // 创建 rawUpdater
        NekoRay::dataStore->imported_count = 0;
        auto rawUpdater = std::make_unique<RawUpdater>();
        rawUpdater->gid_add_to = _sub_gid;

        // 准备
        QString sub_user_info;
        bool asURL = _sub_gid >= 0 || _not_sub_as_url; // 把 _str 当作 url 处理（下载内容）
        auto content = _str.trimmed();
        auto group = profileManager->GetGroup(_sub_gid);
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
        }

        QList<QSharedPointer<ProxyEntity>> in;         // 更新前
        QList<QSharedPointer<ProxyEntity>> out_all;    // 更新前 + 更新后
        QList<QSharedPointer<ProxyEntity>> out;        // 更新后
        QList<QSharedPointer<ProxyEntity>> only_in;    // 只在更新前有的
        QList<QSharedPointer<ProxyEntity>> only_out;   // 只在更新后有的
        QList<QSharedPointer<ProxyEntity>> update_del; // 更新前后都有的，删除更新后多余的

        // 订阅解析前
        if (group != nullptr) {
            in = group->Profiles();
            group->last_update = QDateTime::currentSecsSinceEpoch();
            group->info = sub_user_info;
            group->order.clear();
            group->Save();
            //
            if (dataStore->sub_clear) {
                MW_show_log(QObject::tr("Clearing servers..."));
                for (const auto &profile: in) {
                    profileManager->DeleteProfile(profile->id);
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

            ProfileFilter::OnlyInSrc_ByPointer(out_all, in, out);
            ProfileFilter::OnlyInSrc(in, out, only_in);
            ProfileFilter::OnlyInSrc(out, in, only_out);
            ProfileFilter::Common(in, out, update_del, false, true);
            update_del += only_in;
            if (dataStore->sub_clear) update_del = {};

            for (const auto &ent: update_del) {
                profileManager->DeleteProfile(ent->id);
            }

            QString notice_added;
            for (const auto &ent: only_out) {
                notice_added += ent->bean->DisplayTypeAndName() + "\n";
            }
            QString notice_deleted;
            for (const auto &ent: only_in) {
                notice_deleted += ent->bean->DisplayTypeAndName() + "\n";
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
            NekoRay::dataStore->imported_count = rawUpdater->update_counter;
            MW_dialog_message("SubUpdater", "finish");
        }
    }
} // namespace NekoRay::sub
