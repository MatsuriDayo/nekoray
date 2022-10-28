#pragma once

#include <QWidget>
#include <QDialog>
#include <QMenu>
#include <QTableWidgetItem>

#include "db/Group.hpp"

QT_BEGIN_NAMESPACE
namespace Ui { class DialogManageGroups; }
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

// 更新所有订阅 关闭分组窗口时 更新动作继续执行

void UI_update_one_group(int _order);
