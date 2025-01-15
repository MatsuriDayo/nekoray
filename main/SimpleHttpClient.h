#ifndef SIMPLEHTTPCLIENT_H
#define SIMPLEHTTPCLIENT_H

#include <QNetworkRequest>
#include <QUrl>
#include <QByteArray>
#include <QList>
#include <QPair>
#include <QString>
#include <QNetworkAccessManager>


#ifdef __GNUC__
namespace QNetworkAccessManagerAlternative{
    struct HttpResult {
        bool success;                                  // Success flag
        QString body;                                  // Response body
        QList<QPair<QByteArray, QByteArray>> headers;  // Raw headers as key-value pairs
        QString error;                                 // Error message if not successful
        int statusCode;                                // HTTP status code
    };

    class HttpClient {
    public:
        static HttpResult executeGetRequest(const QNetworkRequest &request, QNetworkAccessManager &manager);
    };

}
#endif

#endif // SIMPLEHTTPCLIENT_H
