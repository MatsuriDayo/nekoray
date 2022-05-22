#pragma once

#include <QWidget>
#include <QListWidgetItem>

#include "db/Database.hpp"

QT_BEGIN_NAMESPACE
namespace Ui { class GroupItem; }
QT_END_NAMESPACE

class GroupItem : public QWidget {
Q_OBJECT

public:
    explicit GroupItem(QWidget *parent, const QSharedPointer<NekoRay::Group> &ent, QListWidgetItem *item);

    ~GroupItem() override;

private:
    Ui::GroupItem *ui;
    QListWidgetItem *item;
    QSharedPointer<NekoRay::Group> ent;

    void refresh_data();

private slots:

    void on_update_sub_clicked();

    void on_edit_clicked();

    void on_remove_clicked();
};
