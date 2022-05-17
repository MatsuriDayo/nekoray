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

    QString core_editCache;

private slots:

    void on_core_edit_clicked();
};

#endif // DIALOG_BASIC_SETTINGS_H
