#include "dialog_manage_groups.h"
#include "ui_dialog_manage_groups.h"

#include "db/Database.hpp"
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
wI->setSizeHint(w->sizeHint()); \
ui->listWidget->addItem(wI); \
ui->listWidget->setItemWidget(wI, w); \
}

DialogManageGroups::DialogManageGroups(QWidget *parent) :
        QDialog(parent), ui(new Ui::DialogManageGroups) {
    ui->setupUi(this);
    for (auto id: NekoRay::profileManager->_groups) {
        AddGroupToListIfExist(id)
    }
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
