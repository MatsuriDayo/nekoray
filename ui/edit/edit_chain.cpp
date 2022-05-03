#include "edit_chain.h"
#include "ui_edit_chain.h"

#include "ui/mainwindow.h"
#include "ui/widget/ProxyItem.h"

#include "db/Database.hpp"
#include "fmt/ChainBean.hpp"

EditChain::EditChain(QWidget *parent) : QWidget(parent), ui(new Ui::EditChain) {
    ui->setupUi(this);
}

EditChain::~EditChain() {
    delete ui;
}

void EditChain::onStart(QSharedPointer<NekoRay::ProxyEntity> _ent) {
    this->ent = _ent;
    auto bean = this->ent->ChainBean();

    for (auto id: bean->list) {
        AddProfileToListIfExist(id);
    }
}

bool EditChain::onEnd() {
    auto bean = this->ent->ChainBean();

    QList<int> idList;
    for (int i = 0; i < ui->listWidget->count(); i++) {
        idList << ui->listWidget->item(i)->data(114514).toInt();
    }
    bean->list = idList;

    return true;
}

void EditChain::on_select_profile_clicked() {
    get_edit_dialog()->hide();
    GetMainWindow()->start_select_mode(this, [=](int id) {
        get_edit_dialog()->show();
        AddProfileToListIfExist(id);
    });
}

void EditChain::AddProfileToListIfExist(int id) {
    auto _ent = NekoRay::profileManager->GetProfile(id);
    if (_ent != nullptr && _ent->type != "chain") {
        auto wI = new QListWidgetItem();
        auto w = new ProxyItem(this, _ent, wI);
        wI->setData(114514, id);
        ui->listWidget->addItem(wI);
        ui->listWidget->setItemWidget(wI, w);
    }
}
