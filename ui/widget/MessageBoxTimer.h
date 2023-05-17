#pragma once

#include <QMessageBox>
#include <QTimer>

class MessageBoxTimer : public QTimer {
public:
    QMessageBox *msgbox = nullptr;
    bool showed = false;

    explicit MessageBoxTimer(QObject *parent, QMessageBox *msgbox, int delayMs) : QTimer(parent) {
        connect(this, &QTimer::timeout, this, &MessageBoxTimer::timeoutFunc, Qt::ConnectionType::QueuedConnection);
        this->msgbox = msgbox;
        setSingleShot(true);
        setInterval(delayMs);
        start();
    };

    void cancel() {
        QTimer::stop();
        if (msgbox != nullptr && showed) {
            msgbox->reject(); // return the timeoutFunc
        }
    };

private:
    void timeoutFunc() {
        if (msgbox == nullptr) return;
        showed = true;
        msgbox->exec();
        msgbox = nullptr;
    }
};
