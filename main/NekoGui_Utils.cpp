#include "NekoGui_Utils.hpp"

#include "3rdparty/base64.h"
#include "3rdparty/QThreadCreateThread.hpp"

#include <random>

#include <QApplication>
#include <QUrlQuery>
#include <QTcpServer>
#include <QTimer>
#include <QMessageBox>
#include <QFile>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>
#include <QRegularExpression>
#include <QDateTime>
#include <QLocale>

#ifdef Q_OS_WIN
#include "sys/windows/guihelper.h"
#endif

QStringList SplitLines(const QString &_string) {
#if QT_VERSION >= QT_VERSION_CHECK(5, 14, 0)
    return _string.split(QRegularExpression("[\r\n]"), Qt::SplitBehaviorFlags::SkipEmptyParts);
#else
    return _string.split(QRegularExpression("[\r\n]"), QString::SkipEmptyParts);
#endif
}

QStringList SplitLinesSkipSharp(const QString &_string, int maxLine) {
    auto lines = SplitLines(_string);
    QStringList newLines;
    int i = 0;
    for (const auto &line: lines) {
        if (line.trimmed().startsWith("#")) continue;
        newLines << line;
        if (maxLine > 0 && ++i >= maxLine) break;
    }
    return newLines;
}

QByteArray DecodeB64IfValid(const QString &input, QByteArray::Base64Options options) {
    Qt515Base64::Base64Options newOptions = Qt515Base64::Base64Option::AbortOnBase64DecodingErrors;
    if (options.testFlag(QByteArray::Base64UrlEncoding)) newOptions |= Qt515Base64::Base64Option::Base64UrlEncoding;
    if (options.testFlag(QByteArray::OmitTrailingEquals)) newOptions |= Qt515Base64::Base64Option::OmitTrailingEquals;
    auto result = Qt515Base64::QByteArray_fromBase64Encoding(input.toUtf8(), newOptions);
    if (result) {
        return result.decoded;
    }
    return {};
}

QString QStringList2Command(const QStringList &list) {
    QStringList new_list;
    for (auto str: list) {
        auto q = "\"" + str.replace("\"", "\\\"") + "\"";
        new_list << q;
    }
    return new_list.join(" ");
}

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

    std::uniform_int_distribution<int> dist(0, possibleCharacters.length() - 1);

    QString randomString;
    for (int i = 0; i < randomStringLength; ++i) {
        QChar nextChar = possibleCharacters.at(dist(mt));
        randomString.append(nextChar);
    }
    return randomString;
}

quint64 GetRandomUint64() {
    std::random_device rd;
    std::mt19937 mt(rd());
    std::uniform_int_distribution<quint64> dist;
    return dist(mt);
}

// QString >> QJson
QJsonObject QString2QJsonObject(const QString &jsonString) {
    QJsonDocument jsonDocument = QJsonDocument::fromJson(jsonString.toUtf8());
    QJsonObject jsonObject = jsonDocument.object();
    return jsonObject;
}

// QJson >> QString
QString QJsonObject2QString(const QJsonObject &jsonObject, bool compact) {
    return QJsonDocument(jsonObject).toJson(compact ? QJsonDocument::Compact : QJsonDocument::Indented);
}

template<typename T>
QJsonArray QList2QJsonArray(const QList<T> &list) {
    QVariantList list2;
    for (auto &item: list)
        list2.append(item);
    return QJsonArray::fromVariantList(list2);
}

template QJsonArray QList2QJsonArray<int>(const QList<int> &list);
template QJsonArray QList2QJsonArray<QString>(const QList<QString> &list);

QList<int> QJsonArray2QListInt(const QJsonArray &arr) {
    QList<int> list2;
    for (auto item: arr)
        list2.append(item.toInt());
    return list2;
}

QList<QString> QJsonArray2QListString(const QJsonArray &arr) {
    QList<QString> list2;
    for (auto item: arr)
        list2.append(item.toString());
    return list2;
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

QString ReadableSize(const qint64 &size) {
    double sizeAsDouble = size;
    static QStringList measures;
    if (measures.isEmpty())
        measures << "B"
                 << "KiB"
                 << "MiB"
                 << "GiB"
                 << "TiB"
                 << "PiB"
                 << "EiB"
                 << "ZiB"
                 << "YiB";
    QStringListIterator it(measures);
    QString measure(it.next());
    while (sizeAsDouble >= 1024.0 && it.hasNext()) {
        measure = it.next();
        sizeAsDouble /= 1024.0;
    }
    return QString::fromLatin1("%1 %2").arg(sizeAsDouble, 0, 'f', 2).arg(measure);
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

QString DisplayTime(long long time, int formatType) {
    QDateTime t;
    t.setMSecsSinceEpoch(time * 1000);
    return QLocale().toString(t, QLocale::FormatType(formatType));
}

QWidget *GetMessageBoxParent() {
    auto activeWindow = QApplication::activeWindow();
    if (activeWindow == nullptr && mainwindow != nullptr) {
        if (mainwindow->isVisible()) return mainwindow;
        return nullptr;
    }
    return activeWindow;
}

int MessageBoxWarning(const QString &title, const QString &text) {
    return QMessageBox::warning(GetMessageBoxParent(), title, text);
}

int MessageBoxInfo(const QString &title, const QString &text) {
    return QMessageBox::information(GetMessageBoxParent(), title, text);
}

void ActivateWindow(QWidget *w) {
    w->setWindowState(w->windowState() & ~Qt::WindowMinimized);
    w->setVisible(true);
#ifdef Q_OS_WIN
    Windows_QWidget_SetForegroundWindow(w);
#endif
    w->raise();
    w->activateWindow();
}

void runOnUiThread(const std::function<void()> &callback, QObject *parent) {
    // any thread
    auto *timer = new QTimer();
    auto thread = dynamic_cast<QThread *>(parent);
    if (thread == nullptr) {
        timer->moveToThread(parent == nullptr ? mainwindow->thread() : parent->thread());
    } else {
        timer->moveToThread(thread);
    }
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
