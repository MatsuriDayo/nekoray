#include "ui_dialog_manage_groups.h"

#include "ui/dialog_manage_groups.h"
#include "ui/mainwindow_message.h"
#include "ui/edit/dialog_edit_group.h"

#include "db/Database.hpp"
#include "main/GuiUtils.hpp"

#include <QInputDialog>

DialogManageGroups::DialogManageGroups(QWidget *parent) :
        QDialog(parent), ui(new Ui::DialogManageGroups) {
    ui->setupUi(this);
    ui->groupListTable->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    this->refresh_group_list(-114514);
    this->create_right_click_menu();
}

DialogManageGroups::~DialogManageGroups() {
    delete ui;
}

void DialogManageGroups::on_groupListTable_customContextMenuRequested(const QPoint &pos) {
    menu->popup(ui->groupListTable->viewport()->mapToGlobal(pos)); //弹出菜单
}

void DialogManageGroups::refresh_group_list(int postMain_gid) {
    ui->groupListTable->setRowCount(0); //这样才能清空数据

    for (const auto &group: NekoRay::profileManager->groups) {
        int i = ui->groupListTable->rowCount();
        ui->groupListTable->insertRow(i);

        // C0: Name
        auto *f = new QTableWidgetItem(group->name);
        f->setData(114514, group->id);
        ui->groupListTable->setItem(i, 0, f);

        // C1: Profile Count
        f = f->clone();
        int count = 0;
        for (const auto &profile: NekoRay::profileManager->profiles) {
            if (profile->gid == group->id) count++;
        }
        f->setText(Int2String(count));
        ui->groupListTable->setItem(i, 1, f);

        // C2: Sub
        f = f->clone();
        f->setText(group->url);
        ui->groupListTable->setItem(i, 2, f);
    }

    if (postMain_gid > -114514) { // <= -114514 don't post to mainwindow
        emit GetMainWindow()->dialog_message(Dialog_DialogManageGroups, "refresh" + Int2String(postMain_gid));
    }
}

QMap<int, QSharedPointer<NekoRay::Group>> DialogManageGroups::GetNowSelected() {
    auto items = ui->groupListTable->selectedItems();
    QMap<int, QSharedPointer<NekoRay::Group>> map;
    for (auto item: items) {
        auto id = item->data(114514).toInt();
        auto ent = NekoRay::profileManager->GetGroup(id);
        if (ent != nullptr) {
            map[id] = ent;
        }
    }
    return map;
}

void DialogManageGroups::create_right_click_menu() {
    QList<QString> texts;
    texts.push_back(tr("Edit"));
    texts.push_back(tr("Add"));
    texts.push_back(tr("Delete"));
    texts.push_back(tr("Update Subscription"));

    menu = CreateMenu(this, texts, [=](QAction *action) {
        auto ents = GetNowSelected();
        switch (action->data().toInt()) {
            case 0: {//Edit
                on_groupListTable_itemDoubleClicked(nullptr);
                break;
            }
            case 1: {//Add
                auto ent = NekoRay::ProfileManager::NewGroup();
                auto dialog = new DialogEditGroup(ent, this);
                int ret = dialog->exec();
                dialog->deleteLater();

                if (ret == QDialog::Accepted) {
                    NekoRay::profileManager->AddGroup(ent);
                    refresh_group_list(-1);
                    //初次订阅
                    if (!ent->url.isEmpty() &&
                        QMessageBox::question(this, tr("Confirmation"), QString(tr("Update %1 item(s) ?")).arg(1)) ==
                        QMessageBox::StandardButton::Yes) {
                        NekoRay::profileManager->AsyncUpdateSubscription(ent->id);
                    }
                }
                break;
            }
            case 2: {//Delete
                ents.remove(0);
                if (ents.count() == 0) break;
                if (QMessageBox::question(this, tr("Confirmation"),
                                          QString(tr("Remove %1 item(s) ?")).arg(ents.count())) ==
                    QMessageBox::StandardButton::Yes) {
                    for (const auto &ent: ents) {
                        NekoRay::profileManager->DeleteGroup(ent->id);
                        refresh_group_list(-2);
                    }
                }
                break;
            }
            case 3: {// update sub
                QList<QSharedPointer<NekoRay::Group>> newS;
                for (const auto &s: ents) {
                    if (!s->IsSubscription()) continue;
                    newS += s;
                }
                if (newS.count() == 0) break;
                if (QMessageBox::question(this, tr("Confirmation"),
                                          QString(tr("Update %1 item(s) ?")).arg(newS.count())) ==
                    QMessageBox::StandardButton::Yes) {
                    for (const auto &ent: newS) {
                        NekoRay::profileManager->AsyncUpdateSubscription(ent->id);
                    }
                }
                break;
            }
        }
    });
}

//edit
void DialogManageGroups::on_groupListTable_itemDoubleClicked(QTableWidgetItem *item) {
    auto ents = GetNowSelected();
    if (ents.count() != 1) return;
    auto ent = ents.first();

    auto dialog = new DialogEditGroup(ent, this);
    int ret = dialog->exec();
    dialog->deleteLater();

    if (ret == QDialog::Accepted) {
        ent->Save();
        refresh_group_list(ent->id);
    }
}
