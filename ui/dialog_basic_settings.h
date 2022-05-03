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
};

#endif // DIALOG_BASIC_SETTINGS_H
