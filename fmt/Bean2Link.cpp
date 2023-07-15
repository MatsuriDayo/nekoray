#include "db/ProxyEntity.hpp"
#include "fmt/includes.h"

#include <QUrlQuery>

namespace NekoGui_fmt {
    QString SocksHttpBean::ToShareLink() {
        QUrl url;
        if (socks_http_type == type_HTTP) { // http
            if (stream->security == "tls") {
                url.setScheme("https");
            } else {
                url.setScheme("http");
            }
        } else {
            url.setScheme(QString("socks%1").arg(socks_http_type));
        }
        if (!name.isEmpty()) url.setFragment(name);
        if (!username.isEmpty()) url.setUserName(username);
        if (!password.isEmpty()) url.setPassword(password);
        url.setHost(serverAddress);
        url.setPort(serverPort);
        return url.toString(QUrl::FullyEncoded);
    }

    QString TrojanVLESSBean::ToShareLink() {
        QUrl url;
        QUrlQuery query;
        url.setScheme(proxy_type == proxy_VLESS ? "vless" : "trojan");
        url.setUserName(password);
        url.setHost(serverAddress);
        url.setPort(serverPort);
        if (!name.isEmpty()) url.setFragment(name);

        //  security
        auto security = stream->security;
        if (security == "tls" && !stream->reality_pbk.trimmed().isEmpty()) security = "reality";
        query.addQueryItem("security", security);

        if (!stream->sni.isEmpty()) query.addQueryItem("sni", stream->sni);
        if (stream->allow_insecure) query.addQueryItem("allowInsecure", "1");
        if (!stream->utlsFingerprint.isEmpty()) query.addQueryItem("fp", stream->utlsFingerprint);

        if (security == "reality") {
            query.addQueryItem("pbk", stream->reality_pbk);
            if (!stream->reality_sid.isEmpty()) query.addQueryItem("sid", stream->reality_sid);
            if (!stream->reality_spx.isEmpty()) query.addQueryItem("spx", stream->reality_spx);
        }

        // type
        query.addQueryItem("type", stream->network);

        if (stream->network == "ws" || stream->network == "http") {
            if (!stream->path.isEmpty()) query.addQueryItem("path", stream->path);
            if (!stream->host.isEmpty()) query.addQueryItem("host", stream->host);
        } else if (stream->network == "grpc") {
            if (!stream->path.isEmpty()) query.addQueryItem("serviceName", stream->path);
        } else if (stream->network == "tcp") {
            if (stream->header_type == "http") {
                query.addQueryItem("headerType", "http");
                query.addQueryItem("host", stream->host);
            }
        }

        // protocol
        if (proxy_type == proxy_VLESS) {
            if (!flow.isEmpty()) {
                query.addQueryItem("flow", flow);
            }
        }

        url.setQuery(query);
        return url.toString(QUrl::FullyEncoded);
    }

    const char* fixShadowsocksUserNameEncodeMagic = "fixShadowsocksUserNameEncodeMagic-holder-for-QUrl";

    QString ShadowSocksBean::ToShareLink() {
        QUrl url;
        url.setScheme("ss");
        if (method.startsWith("2022-")) {
            url.setUserName(fixShadowsocksUserNameEncodeMagic);
        } else {
            auto method_password = method + ":" + password;
            url.setUserName(method_password.toUtf8().toBase64(QByteArray::Base64Option::Base64UrlEncoding));
        }
        url.setHost(serverAddress);
        url.setPort(serverPort);
        if (!name.isEmpty()) url.setFragment(name);
        QUrlQuery q;
        if (!plugin.isEmpty()) q.addQueryItem("plugin", plugin);
        if (!q.isEmpty()) url.setQuery(q);
        //
        auto link = url.toString(QUrl::FullyEncoded);
        link = link.replace(fixShadowsocksUserNameEncodeMagic, method + ":" + QUrl::toPercentEncoding(password));
        return link;
    }

    QString VMessBean::ToShareLink() {
        QJsonObject N{
            {"v", "2"},
            {"ps", name},
            {"add", serverAddress},
            {"port", Int2String(serverPort)},
            {"id", uuid},
            {"aid", Int2String(aid)},
            {"net", stream->network},
            {"host", stream->host},
            {"path", stream->path},
            {"type", stream->header_type},
            {"scy", security},
            {"tls", stream->security == "tls" ? "tls" : ""},
            {"sni", stream->sni},
        };
        return "vmess://" + QJsonObject2QString(N, true).toUtf8().toBase64();
    }

    QString NaiveBean::ToShareLink() {
        QUrl url;
        url.setScheme("naive+" + protocol);
        url.setUserName(username);
        url.setPassword(password);
        url.setHost(serverAddress);
        url.setPort(serverPort);
        if (!name.isEmpty()) url.setFragment(name);
        return url.toString(QUrl::FullyEncoded);
    }

    QString HysteriaBean::ToShareLink() {
        QUrl url;
        url.setScheme("hysteria");
        url.setHost(serverAddress);
        url.setPort(serverPort);
        QUrlQuery q;
        q.addQueryItem("upmbps", Int2String(uploadMbps));
        q.addQueryItem("downmbps", Int2String(downloadMbps));
        if (!obfsPassword.isEmpty()) {
            q.addQueryItem("obfs", "xplus");
            q.addQueryItem("obfsParam", obfsPassword);
        }
        if (authPayloadType == hysteria_auth_string) q.addQueryItem("auth", authPayload);
        if (protocol == hysteria_protocol_facktcp) q.addQueryItem("protocol", "faketcp");
        if (protocol == hysteria_protocol_wechat_video) q.addQueryItem("protocol", "wechat-video");
        if (!hopPort.trimmed().isEmpty()) q.addQueryItem("mport", hopPort);
        if (allowInsecure) q.addQueryItem("insecure", "1");
        if (!sni.isEmpty()) q.addQueryItem("peer", sni);
        if (!alpn.isEmpty()) q.addQueryItem("alpn", alpn);
        if (connectionReceiveWindow > 0) q.addQueryItem("recv_window", Int2String(connectionReceiveWindow));
        if (streamReceiveWindow > 0) q.addQueryItem("recv_window_conn", Int2String(streamReceiveWindow));
        if (!q.isEmpty()) url.setQuery(q);
        if (!name.isEmpty()) url.setFragment(name);
        return url.toString(QUrl::FullyEncoded);
    }

} // namespace NekoGui_fmt