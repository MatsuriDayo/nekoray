#include "qv2ray/v2/utils/HTTPRequestHelper.hpp"

#include "db/Database.hpp"
#include "db/ProfileFilter.hpp"
#include "fmt/includes.h"

#include "GroupUpdater.hpp"

#include <QInputDialog>

#ifndef NKR_NO_EXTERNAL

#include <yaml-cpp/yaml.h>

#endif

#define FIRST_OR_SECOND(a, b) a.isEmpty() ? b : a

namespace NekoRay::sub {

    GroupUpdater *groupUpdater = new GroupUpdater;

    void RawUpdater::update(const QString &str) {
        // Base64 encoded subscription
        if (auto str2 = DecodeB64IfValid(str);!str2.isEmpty()) {
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

        // Nekoray format
        if (str.startsWith("nekoray://")) {
            auto link = QUrl(str);
            if (!link.isValid()) return;
            ent = ProfileManager::NewProxyEntity(link.host());
            if (ent->bean->version == -114514) return;
            auto j = DecodeB64IfValid(link.fragment().toUtf8(), QByteArray::Base64UrlEncoding);
            if (j.isEmpty()) return;
            ent->bean->FromJsonBytes(j.toUtf8());
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

        // VMess
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
        if (str.startsWith("https+naive://")) {
            ent = ProfileManager::NewProxyEntity("naive");
            auto ok = ent->NaiveBean()->TryParseLink(str);
            if (!ok) return;
        }

        // End
        if (ent == nullptr) return;
        profileManager->AddProfile(ent, gid_add_to);
        update_counter++;
    }

#ifndef NKR_NO_EXTERNAL

    QString Node2QString(const YAML::Node &n, const QString &def = "") {
        try {
            return n.as<std::string>().c_str();
        } catch (const YAML::Exception &ex) {
            return def;
        }
    }

    int Node2Int(const YAML::Node &n, const int &def = 0) {
        try {
            return n.as<int>();
        } catch (const YAML::Exception &ex) {
            return def;
        }
    }

    bool Node2Bool(const YAML::Node &n, const bool &def = false) {
        try {
            return n.as<bool>();
        } catch (const YAML::Exception &ex) {
            return def;
        }
    }

#endif

// https://github.com/Dreamacro/clash/wiki/configuration
    void RawUpdater::updateClash(const QString &str) {
#ifndef NKR_NO_EXTERNAL
        try {
            auto proxies = YAML::Load(str.toStdString())["proxies"];
            for (auto proxy: proxies) {
                auto type = Node2QString(proxy["type"]);
                if (type == "ss" || type == "ssr") type = "shadowsocks";
                if (type == "socks5") type = "socks";

                auto ent = ProfileManager::NewProxyEntity(type);
                if (ent->bean->version == -114514) continue;

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
                        if (Node2QString(plugin_n) == "obfs") {
                            bean->plugin = "obfs-local;obfs=" + Node2QString(pluginOpts_n["mode"]) + ";obfs-host=" +
                                           Node2QString(pluginOpts_n["host"]);
                        }
                    }
                    auto protocol_n = proxy["protocol"];
                    if (protocol_n.IsDefined()) {
                        continue; // SSR
                    }
                } else if (type == "socks" || type == "http") {
                    auto bean = ent->SocksHTTPBean();
                    bean->password = Node2QString(proxy["username"]);
                    bean->password = Node2QString(proxy["password"]);
                    if (Node2Bool(proxy["tls"])) bean->stream->security = "tls";
                    if (Node2Bool(proxy["skip-cert-verify"])) bean->stream->allow_insecure = true;
                } else if (type == "trojan") {
                    auto bean = ent->TrojanVLESSBean();
                    bean->password = Node2QString(proxy["password"]);
                    bean->stream->security = "tls";
                    bean->stream->network = Node2QString(proxy["network"], "tcp");
                    bean->stream->sni = FIRST_OR_SECOND(Node2QString(proxy["sni"]), Node2QString(proxy["servername"]));
                    if (Node2Bool(proxy["skip-cert-verify"])) bean->stream->allow_insecure = true;
                } else if (type == "vmess") {
                    auto bean = ent->VMessBean();
                    bean->uuid = Node2QString(proxy["uuid"]);
                    bean->aid = Node2Int(proxy["alterId"]);
                    bean->security = Node2QString(proxy["cipher"]);
                    bean->stream->network = Node2QString(proxy["network"], "tcp");
                    bean->stream->sni = FIRST_OR_SECOND(Node2QString(proxy["sni"]), Node2QString(proxy["servername"]));
                    if (Node2Bool(proxy["tls"])) bean->stream->security = "tls";
                    if (Node2Bool(proxy["skip-cert-verify"])) bean->stream->allow_insecure = true;

                    auto ws = proxy["ws-opts"];
                    if (ws.IsMap()) {
                        auto headers = ws["headers"];
                        for (auto header: headers) {
                            if (Node2QString(header.first).toLower() == "host") {
                                bean->stream->host = Node2QString(header.second);
                            }
                        }
                        bean->stream->path = Node2QString(ws["path"]);
                    }

                    auto grpc = proxy["grpc-opts"];
                    if (grpc.IsMap()) {
                        bean->stream->path = Node2QString(grpc["grpc-service-name"]);
                    }

                    auto h2 = proxy["h2-opts"];
                    if (h2.IsMap()) {
                        auto hosts = ws["host"];
                        for (auto host: hosts) {
                            bean->stream->host = Node2QString(host);
                            break;
                        }
                        bean->stream->path = Node2QString(h2["path"]);
                    }
                } else {
                    continue;
                }

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

    // 不要刷新，下载导入完会自己刷新
    void GroupUpdater::AsyncUpdate(const QString &str, int _sub_gid,
                                   QObject *caller, const std::function<void()> &callback) {
        if (caller != nullptr && callback != nullptr) {
            connectOnce(this, &GroupUpdater::AsyncUpdateCallback, caller,
                        [=](QObject *receiver) {
                            if (receiver == caller) callback();
                        });
        }

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
            emit AsyncUpdateCallback(caller);
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
            showLog(">>>>>>>> " + QObject::tr("Requesting subscription: %1").arg(groupName));

            auto resp = NetworkRequestHelper::HttpGet(content);
            if (!resp.error.isEmpty()) {
                showLog("<<<<<<<< " + QObject::tr("Requesting subscription %1 error: %2")
                        .arg(groupName, resp.error + "\n" + resp.data));
                return;
            }

            content = resp.data;
            sub_user_info = NetworkRequestHelper::GetHeader(resp.header, "Subscription-UserInfo");
        }

        QList<QSharedPointer<ProxyEntity>> in; // 更新前
        QList<QSharedPointer<ProxyEntity>> out_all; // 更新前 + 更新后
        QList<QSharedPointer<ProxyEntity>> out; // 更新后
        QList<QSharedPointer<ProxyEntity>> only_in; // 只在更新前有的
        QList<QSharedPointer<ProxyEntity>> only_out; // 只在更新后有的
        QList<QSharedPointer<ProxyEntity>> update_del; // 更新前后都有的，删除更新后多余的

        // 订阅解析前
        if (group != nullptr) {
            in = group->Profiles();
            group->last_update = QDateTime::currentSecsSinceEpoch();
            group->info = sub_user_info;
            group->order.clear();
            group->Save();
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

            runOnUiThread([=] {
                auto change = "\n" + QObject::tr("Added %1 profiles:\n%2\nDeleted %3 Profiles:\n%4")
                        .arg(only_out.length()).arg(notice_added)
                        .arg(only_in.length()).arg(notice_deleted);
                if (only_out.length() + only_in.length() == 0) change = QObject::tr("Nothing");
                showLog("<<<<<<<< " + QObject::tr("Change of %1:").arg(group->name) + " " + change);
                dialog_message("SubUpdater", "finish-dingyue");
            });
        } else {
            NekoRay::dataStore->imported_count = rawUpdater->update_counter;
            runOnUiThread([=] {
                dialog_message("SubUpdater", "finish");
            });
        }
    }
}
