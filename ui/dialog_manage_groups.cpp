#include "dialog_manage_groups.h"
#include "ui_dialog_manage_groups.h"

#include "db/Database.hpp"
#include "sub/RawUpdater.hpp"
#include "main/GuiUtils.hpp"
#include "ui/widget/GroupItem.h"
#include "ui/edit/dialog_edit_group.h"

#include <QInputDialog>
#include <QListWidgetItem>

#define AddGroupToListIfExist(_id) auto __ent = NekoRay::profileManager->GetGroup(_id); \
if (__ent != nullptr) { \
auto wI = new QListWidgetItem(); \
auto w = new GroupItem(this, __ent, wI); \
wI->setData(114514, _id); \
ui->listWidget->addItem(wI); \
ui->listWidget->setItemWidget(wI, w); \
}

DialogManageGroups::DialogManageGroups(QWidget *parent) :
        QDialog(parent), ui(new Ui::DialogManageGroups) {
    ui->setupUi(this);

    for (auto id: NekoRay::profileManager->_groups) {
        AddGroupToListIfExist(id)
    }

    connect(ui->listWidget, &QListWidget::itemDoubleClicked, this, [=](QListWidgetItem *wI) {
        auto w = dynamic_cast<GroupItem *>(ui->listWidget->itemWidget(wI));
        emit w->edit_clicked();
    });
}

DialogManageGroups::~DialogManageGroups() {
    delete ui;
}

void DialogManageGroups::on_add_clicked() {
    auto ent = NekoRay::ProfileManager::NewGroup();
    auto dialog = new DialogEditGroup(ent, this);
    int ret = dialog->exec();
    dialog->deleteLater();

    if (ret == QDialog::Accepted) {
        NekoRay::profileManager->AddGroup(ent);
        AddGroupToListIfExist(ent->id)
        dialog_message(Dialog_DialogManageGroups, "refresh-1");
    }
}

void DialogManageGroups::on_update_all_clicked() {
    if (QMessageBox::question(this, tr("Confirmation"), tr("Update all subscriptions?"))
        == QMessageBox::StandardButton::Yes) {
        for (const auto &gid: NekoRay::profileManager->_groups) {
            auto group = NekoRay::profileManager->GetGroup(gid);
            if (group == nullptr || group->url.isEmpty()) continue;
            _update_one_group(group);
            return;
        }
    }
}

void DialogManageGroups::_update_one_group(const QSharedPointer<NekoRay::Group> &group) {
    NekoRay::sub::rawUpdater->AsyncUpdate(group->url, group->id, this, [=] {
        // refresh ui
        for (int i = 0; i < ui->listWidget->count(); i++) {
            auto w = ui->listWidget->itemWidget(ui->listWidget->item(i));
            if (w == nullptr) return;
            auto item = dynamic_cast<GroupItem *>(w);
            if (item->ent->id == group->id) {
                item->refresh_data();
            }
        }
        // update next group
        auto nextOrder = NekoRay::profileManager->_groups.indexOf(group->id);
        forever {
            nextOrder += 1;
            if (nextOrder >= NekoRay::profileManager->_groups.length()) return;
            auto nextGid = NekoRay::profileManager->_groups[nextOrder];
            auto nextGroup = NekoRay::profileManager->GetGroup(nextGid);
            if (nextGroup == nullptr || nextGroup->url.isEmpty()) continue;
            _update_one_group(nextGroup);
        }
    });
}
