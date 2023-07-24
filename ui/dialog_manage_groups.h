#pragma once

#include <QWidget>
#include <QDialog>
#include <QMenu>
#include <QTableWidgetItem>

#include "db/Group.hpp"

QT_BEGIN_NAMESPACE
namespace Ui {
    class DialogManageGroups;
}
QT_END_NAMESPACE

class DialogManageGroups : public QDialog {
    Q_OBJECT

public:
    explicit DialogManageGroups(QWidget *parent = nullptr);

    ~DialogManageGroups() override;

private:
    Ui::DialogManageGroups *ui;

private slots:

    void on_add_clicked();

    void on_update_all_clicked();
};
