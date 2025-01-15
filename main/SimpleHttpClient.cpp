#include "SimpleHttpClient.h"
#include "3rdparty/cpp-httplib/httplib.h"
#include <QNetworkProxy>

#ifdef __GNUC__

QNetworkAccessManagerAlternative::HttpResult QNetworkAccessManagerAlternative::HttpClient::executeGetRequest(const QNetworkRequest &request, QNetworkAccessManager &manager) {
    HttpResult result;
    result.success = false;
    result.statusCode = -1;

    QUrl url = request.url();
    if (!url.isValid() || (url.scheme() != "http" && url.scheme() != "https")) {
        result.error = "Unsupported or invalid URL scheme";
        return result;
    }

    // bool isHttps = (url.scheme() == "https");

    QString host = url.host();
    // int port = (url.port() == -1) ? (isHttps ? 443 : 80) : url.port();

    // httplib::Client client(host.toStdString(), port);
    httplib::Client client(host.toStdString());

#ifdef CPPHTTPLIB_OPENSSL_SUPPORT
    if (isHttps) {
        client.enable_server_certificate_verification(false); // Skip SSL verification for demo
    }
#endif

    QNetworkProxy proxy = manager.proxy();
    if (proxy.type() != QNetworkProxy::NoProxy) {
        client.set_proxy(
            proxy.hostName().toStdString(),
            static_cast<int>(proxy.port())
            );
        if(!proxy.user().isEmpty())
            client.set_proxy_basic_auth(
                proxy.user().toStdString(),
                proxy.password().toStdString()
                );
    }

    httplib::Headers headers;
    QList<QByteArray> headerList = request.rawHeaderList();
    for (const QByteArray &header : headerList) {
        headers.insert(std::pair{header.toStdString(), request.rawHeader(header).toStdString()});
    }

    auto const pathURL = url.path();
    auto res = client.Get(pathURL.toStdString());

    if (res) {
        result.statusCode = res->status;
        for (const auto &header : res->headers) {
            result.headers.append(qMakePair(QByteArray(header.first.c_str()),
                                            QByteArray(header.second.c_str())));
        }
        if (res->status == 200) {
            result.success = true;
            result.body = QString::fromStdString(res->body);
        } else {
            result.error = QString("HTTP Error: %1").arg(res->status);
        }
    } else {
        result.error = QString::fromStdString(httplib::to_string(res.error()));
    }

    return result;
}

#endif
