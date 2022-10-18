#include "NekoRay_Utils.hpp"

#include "3rdparty/QThreadCreateThread.hpp"
#include "main/GuiUtils.hpp"

#include <random>

#include <QUrlQuery>
#include <QTcpServer>
#include <QTimer>
#include <QMessageBox>
#include <QFile>

QString GetQueryValue(const QUrlQuery &q, const QString &key, const QString &def) {
    auto a = q.queryItemValue(key);
    if (a.isEmpty()) {
        return def;
    }
    return a;
}

QString GetRandomString(int randomStringLength) {
    std::random_device rd;
    std::mt19937 mt(rd());

    const QString possibleCharacters("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789");

    std::uniform_int_distribution<int> dist(0, possibleCharacters.count() - 1);

    QString randomString;
    for (int i = 0; i < randomStringLength; ++i) {
        QChar nextChar = possibleCharacters.at(dist(mt));
        randomString.append(nextChar);
    }
    return randomString;
}

QByteArray ReadFile(const QString &path) {
    QFile file(path);
    file.open(QFile::ReadOnly);
    return file.readAll();
}

QString ReadFileText(const QString &path) {
    QFile file(path);
    file.open(QFile::ReadOnly | QFile::Text);
    QTextStream stream(&file);
    return stream.readAll();
}

int MkPort() {
    QTcpServer s;
    s.listen();
    auto port = s.serverPort();
    s.close();
    return port;
}

bool IsIpAddress(const QString &str) {
    auto address = QHostAddress(str);
    if (address.protocol() == QAbstractSocket::IPv4Protocol || address.protocol() == QAbstractSocket::IPv6Protocol)
        return true;
    return false;
}

bool IsIpAddressV4(const QString &str) {
    auto address = QHostAddress(str);
    if (address.protocol() == QAbstractSocket::IPv4Protocol)
        return true;
    return false;
}

bool IsIpAddressV6(const QString &str) {
    auto address = QHostAddress(str);
    if (address.protocol() == QAbstractSocket::IPv6Protocol)
        return true;
    return false;
}

QWidget *GetMessageBoxParent() {
    if (mainwindow == nullptr) return nullptr;
    if (mainwindow->isVisible()) return mainwindow;
    return nullptr;
}

int MessageBoxWarning(const QString &title, const QString &text) {
    return QMessageBox::warning(GetMessageBoxParent(), title, text);
}

int MessageBoxInfo(const QString &title, const QString &text) {
    return QMessageBox::information(GetMessageBoxParent(), title, text);
}

void runOnUiThread(const std::function<void()> &callback, QObject *parent) {
    // any thread
    auto *timer = new QTimer();
    timer->moveToThread(parent == nullptr ? mainwindow->thread() : parent->thread());
    timer->setSingleShot(true);
    QObject::connect(timer, &QTimer::timeout, [=]() {
        // main thread
        callback();
        timer->deleteLater();
    });
    QMetaObject::invokeMethod(timer, "start", Qt::QueuedConnection, Q_ARG(int, 0));
}

void runOnNewThread(const std::function<void()> &callback) {
    createQThread(callback)->start();
}

void setTimeout(const std::function<void()> &callback, QObject *obj, int timeout) {
    auto t = new QTimer;
    QObject::connect(t, &QTimer::timeout, obj, [=] {
        callback();
        t->deleteLater();
    });
    t->setSingleShot(true);
    t->setInterval(timeout);
    t->start();
}
