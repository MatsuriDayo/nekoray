#include <QUrlQuery>
#include "SocksBean.hpp"
#include "ShadowSocksBean.hpp"
#include "VMessBean.hpp"

namespace NekoRay::fmt {

    bool SocksBean::ParseStdLink(const QString &link) {
        return false;
    }

    bool ShadowSocksBean::ParseStdLink(const QString &link) {
        if (SubStrBefore(link, "#").contains("@")) {
            // SS
            auto url = QUrl(link);
            if (!url.isValid()) return false;
            name = url.fragment();
            serverAddress = url.host();
            serverPort = url.port();
            auto method_password = DecodeB64IfValid(url.userName(), QByteArray::Base64Option::Base64UrlEncoding);
            if (method_password.isEmpty()) return false;
            method = SubStrBefore(method_password, ":");
            password = SubStrAfter(method_password, ":");
            auto query = GetQuery(url);
            plugin = query.queryItemValue("plugin");
        } else {
            // v2rayN
            auto hasRemarks = link.contains("#");
            auto linkN = DecodeB64IfValid(SubStrBefore(SubStrAfter(link, "ss://"), "#"),
                                          QByteArray::Base64Option::Base64UrlEncoding);
            if (linkN.isEmpty()) return false;
            if (hasRemarks) linkN += "#" + SubStrAfter(link, "#");
            auto url = QUrl("https://" + linkN);
            serverAddress = url.host();
            serverPort = url.port();
            method = url.userName();
            password = url.password();
            plugin = "";
            if (hasRemarks) {
                // TODO "+"
                name = url.fragment();
            }
        }
        return true;
    }

    bool VMessBean::TryParseLink(const QString &link) {
        // V2RayN Format
        auto linkN = DecodeB64IfValid(SubStrAfter(link, "vmess://"));
        if (!linkN.isEmpty()) {
            auto objN = QString2QJsonObject(linkN);
            if (objN.isEmpty()) return false;
            // REQUIRED
            uuid = objN["id"].toString();
            serverAddress = objN["add"].toString();
            serverPort = objN["port"].toInt();
            // OPTIONAL
            name = objN["ps"].toString();
            aid = objN["aid"].toInt();
            stream->host = objN["host"].toString();
            stream->path = objN["path"].toString();
            stream->sni = objN["sni"].toString();
            auto net = objN["net"].toString().replace("http", "h2");
            if (!net.isEmpty()) stream->network = net;
            auto scy = objN["scy"].toString();
            if (!scy.isEmpty()) security = scy;
            // TLS (XTLS?)
            if (!objN["tls"].toString().isEmpty()) stream->security = "tls";
            // TODO quic & kcp
            return true;
        }

        // Std Format
        return false;
    }

}