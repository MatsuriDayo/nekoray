#include "dialog_manage_groups.h"
#include "ui_dialog_manage_groups.h"

#include "db/Database.hpp"
#include "sub/GroupUpdater.hpp"
#include "main/GuiUtils.hpp"
#include "ui/widget/GroupItem.h"
#include "ui/edit/dialog_edit_group.h"

#include <QInputDialog>
#include <QListWidgetItem>
#include <QMessageBox>

#define AddGroupToListIfExist(_id)                       \
    auto __ent = NekoGui::profileManager->GetGroup(_id); \
    if (__ent != nullptr) {                              \
        auto wI = new QListWidgetItem();                 \
        auto w = new GroupItem(this, __ent, wI);         \
        wI->setData(114514, _id);                        \
        ui->listWidget->addItem(wI);                     \
        ui->listWidget->setItemWidget(wI, w);            \
    }

DialogManageGroups::DialogManageGroups(QWidget *parent) : QDialog(parent), ui(new Ui::DialogManageGroups) {
    ui->setupUi(this);

    for (auto id: NekoGui::profileManager->groupsTabOrder) {
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
    auto ent = NekoGui::ProfileManager::NewGroup();
    auto dialog = new DialogEditGroup(ent, this);
    int ret = dialog->exec();
    dialog->deleteLater();

    if (ret == QDialog::Accepted) {
        NekoGui::profileManager->AddGroup(ent);
        AddGroupToListIfExist(ent->id);
        MW_dialog_message(Dialog_DialogManageGroups, "refresh-1");
    }
}

void DialogManageGroups::on_update_all_clicked() {
    if (QMessageBox::question(this, tr("Confirmation"), tr("Update all subscriptions?")) == QMessageBox::StandardButton::Yes) {
        for (const auto &gid: NekoGui::profileManager->groupsTabOrder) {
            auto group = NekoGui::profileManager->GetGroup(gid);
            if (group == nullptr || group->url.isEmpty()) continue;
            UI_update_one_group(NekoGui::profileManager->groupsTabOrder.indexOf(gid));
            return;
        }
    }
}

void UI_update_one_group(int _order) {
    // calculate next group
    int nextOrder = _order;
    std::shared_ptr<NekoGui::Group> nextGroup;
    forever {
        nextOrder += 1;
        if (nextOrder >= NekoGui::profileManager->groupsTabOrder.size()) break;
        auto nextGid = NekoGui::profileManager->groupsTabOrder[nextOrder];
        nextGroup = NekoGui::profileManager->GetGroup(nextGid);
        if (nextGroup == nullptr || nextGroup->url.isEmpty()) continue;
        break;
    }

    // calculate this group
    auto group = NekoGui::profileManager->GetGroup(NekoGui::profileManager->groupsTabOrder[_order]);
    if (group == nullptr) return;

    // v2.2: listener is moved to GroupItem, no refresh here.
    NekoGui_sub::groupUpdater->AsyncUpdate(group->url, group->id, [=] {
        if (nextGroup != nullptr) UI_update_one_group(nextOrder);
    });
}
