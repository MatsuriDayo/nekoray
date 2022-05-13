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
    QMenu *menu;

    void refresh_group_list(bool postMain);

    void create_right_click_menu();

    QMap<int, QSharedPointer<NekoRay::Group>> GetNowSelected();

private slots:

    void on_groupListTable_customContextMenuRequested(const QPoint &pos);

    void on_groupListTable_itemDoubleClicked(QTableWidgetItem *item);
};


