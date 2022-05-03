// DO NOT INCLUDE THIS

#include <QString>
#include <QWidget>
#include <QVariant>
#include <QDebug>
#include <QMessageBox>
#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonArray>
#include <QMenu>
#include <QTimer>
#include <QMetaObject>
#include <QThread>
#include <QUrlQuery>
#include <random>

#include <utility>
#include <functional>
#include <QHostAddress>

// Dialogs

inline QString Dialog_DialogBasicSettings = "DialogBasicSettings";
inline QString Dialog_DialogEditProfile = "DialogEditProfile";
inline QString Dialog_DialogManageGroups = "DialogManageGroups";
inline QString Dialog_DialogManageRoutes = "DialogManageRoutes";
inline QWidget *mainwindow;
inline std::function<void(QString)> showLog;

// Utils

inline QString SubStrBefore(QString str, const QString &sub) {
    if (!str.contains(sub)) return str;
    return str.left(str.indexOf(sub));
}

inline QString SubStrAfter(QString str, const QString &sub) {
    if (!str.contains(sub)) return str;
    return str.right(str.length() - str.indexOf(sub) - sub.length());
}

inline QString
DecodeB64IfValid(const QString &input, QByteArray::Base64Option options = QByteArray::Base64Option::Base64Encoding) {
    auto result = QByteArray::fromBase64Encoding(input.toUtf8(),
                                                 options | QByteArray::Base64Option::AbortOnBase64DecodingErrors);
    if (result) {
        return result.decoded;
    }
    return "";
}

inline QUrlQuery GetQuery(const QUrl &url) {
    return QUrlQuery(url.query(QUrl::ComponentFormattingOption::FullyDecoded));
}

inline QString Int2String(int i) {
    return QVariant(i).toString();
}

inline QString GetRandomString(int randomStringLength) {
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

// QString >> QJson
inline QJsonObject QString2QJsonObject(const QString &jsonString) {
    QJsonDocument jsonDocument = QJsonDocument::fromJson(jsonString.toLocal8Bit().data());
    QJsonObject jsonObject = jsonDocument.object();
    return jsonObject;
}

// QJson >> QString
inline QString QJsonObject2QString(const QJsonObject &jsonObject, bool compact) {
    return QString(QJsonDocument(jsonObject).toJson(compact ? QJsonDocument::Compact : QJsonDocument::Indented));
}

template<typename T>
inline QJsonArray QList2QJsonArray(const QList<T> &list) {
    QVariantList list2;
    for (auto &item: list)
        list2.append(item);
    return QJsonArray::fromVariantList(list2);
}

inline QList<int> QJsonArray2QListInt(const QJsonArray &arr) {
    QList<int> list2;
    for (auto item: arr)
        list2.append(item.toInt());
    return list2;
}

inline QList<QString> QJsonArray2QListString(const QJsonArray &arr) {
    QList<QString> list2;
    for (auto item: arr)
        list2.append(item.toString());
    return list2;
}

inline bool InRange(unsigned x, unsigned low, unsigned high) {
    return (low <= x && x <= high);
}

inline QString ReadableSize(const qint64 &size) {
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

// Validators

inline bool IsIpAddress(const QString &str) {
    auto address = QHostAddress(str);
    if (address.protocol() == QAbstractSocket::IPv4Protocol || address.protocol() == QAbstractSocket::IPv6Protocol)
        return true;
    return false;
}

inline bool IsIpAddressV4(const QString &str) {
    auto address = QHostAddress(str);
    if (address.protocol() == QAbstractSocket::IPv4Protocol)
        return true;
    return false;
}

inline bool IsIpAddressV6(const QString &str) {
    auto address = QHostAddress(str);
    if (address.protocol() == QAbstractSocket::IPv6Protocol)
        return true;
    return false;
}

// [2001:4860:4860::8888] -> 2001:4860:4860::8888
inline QString UnwrapIPV6Host(QString &str) {
    return str.replace("[", "").replace("]", "");
}

// [2001:4860:4860::8888] or 2001:4860:4860::8888 -> [2001:4860:4860::8888]
inline QString WrapIPV6Host(QString &str) {
    if (!IsIpAddressV6(str)) return str;
    return "[" + UnwrapIPV6Host(str) + "]";
}

inline QString DisplayAddress(QString serverAddress, int serverPort) {
    return WrapIPV6Host(serverAddress) + ":" + Int2String(serverPort);
};

// UI

inline int MessageBoxWarning(const QString &title, const QString &text) {
    return QMessageBox::warning(mainwindow, title, text);
}

inline int MessageBoxWarningStdString(const std::string &title, const std::string &text) {
    return QMessageBox::warning(mainwindow, QString(title.c_str()), QString(text.c_str()));
}

inline QMenu *CreateMenu(QWidget *parent, const QList<QString> &texts, std::function<void(QAction *)> slot) {
    auto menu = new QMenu(parent);
    QList<QAction *> acts;

    for (const auto &text: texts) {
        acts.push_back(new QAction(text, parent)); //按顺序来
    }

    for (int i = 0; i < acts.size(); i++) {
        if (acts[i]->text() == "[Separator]") {
            acts[i]->setSeparator(true);
            acts[i]->setText("");
            acts[i]->setDisabled(true);
            acts[i]->setData(-1);
        } else {
            acts[i]->setData(i);
        }

        menu->addAction(acts[i]);
    }

    QWidget::connect(menu, &QMenu::triggered, parent, std::move(slot));
    return menu;
}

inline void runOnUiThread(const std::function<void()> &callback) {
    // any thread
    auto *timer = new QTimer();
    timer->moveToThread(mainwindow->thread());
    timer->setSingleShot(true);
    QObject::connect(timer, &QTimer::timeout, [=]() {
        // main thread
        callback();
        timer->deleteLater();
    });
    QMetaObject::invokeMethod(timer, "start", Qt::QueuedConnection, Q_ARG(int, 0));
}

inline void runOnNewThread(const std::function<void()> &callback) {
    QThread::create(callback)->start();
}

// Copy from Qv2ray

inline QStringList SplitLines(const QString &_string) {
#if QT_VERSION >= QT_VERSION_CHECK(5, 14, 0)
    return _string.split(QRegularExpression("[\r\n]"), Qt::SplitBehaviorFlags::SkipEmptyParts);
#else
    return _string.split(QRegularExpression("[\r\n]"), QString::SkipEmptyParts);
#endif
}

inline QString VerifyJsonString(const QString &source) {
    QJsonParseError error{};
    QJsonDocument doc = QJsonDocument::fromJson(source.toUtf8(), &error);
    Q_UNUSED(doc)

    if (error.error == QJsonParseError::NoError) {
        return "";
    } else {
        //LOG("WARNING: Json parse returns: " + error.errorString());
        return error.errorString();
    }
}

// GUI TOOLS
#define RED(obj)                                                                                                                                     \
    {                                                                                                                                                \
        auto _temp = obj->palette();                                                                                                                 \
        _temp.setColor(QPalette::Text, Qt::red);                                                                                                     \
        obj->setPalette(_temp);                                                                                                                      \
    }

#define BLACK(obj) obj->setPalette(QWidget::palette());
