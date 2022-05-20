#ifndef DIALOG_BASIC_SETTINGS_H
#define DIALOG_BASIC_SETTINGS_H

#include <QDialog>

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
        QString core_map;
        QString custom_inbound;
    } CACHE;
};

#endif // DIALOG_BASIC_SETTINGS_H
