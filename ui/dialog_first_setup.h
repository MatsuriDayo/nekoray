#pragma once

#include <QDialog>

QT_BEGIN_NAMESPACE
namespace Ui {
    class DialogFirstSetup;
}
QT_END_NAMESPACE

class DialogFirstSetup : public QDialog {
    Q_OBJECT

public:
    explicit DialogFirstSetup(QWidget *parent = nullptr);
    ~DialogFirstSetup() override;

private:
    Ui::DialogFirstSetup *ui;

private slots:
    void onButtonClicked();
};
