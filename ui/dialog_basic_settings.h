#ifndef DIALOG_BASIC_SETTINGS_H
#define DIALOG_BASIC_SETTINGS_H

#include <QDialog>
#include <QJsonObject>

namespace Ui {
    class DialogBasicSettings;
}

class DialogBasicSettings : public QDialog {
Q_OBJECT

public:
    explicit DialogBasicSettings(QWidget *parent = nullptr);

    ~DialogBasicSettings();

public slots:

    void accept();

private:
    Ui::DialogBasicSettings *ui;

    struct {
        QJsonObject extraCore;
        QString custom_inbound;
        bool needRestart = false;
    } CACHE;
};

#endif // DIALOG_BASIC_SETTINGS_H
