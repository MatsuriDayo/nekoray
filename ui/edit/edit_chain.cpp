#include "edit_chain.h"
#include "ui_edit_chain.h"

#include "ui/mainwindow.h"
#include "ui/widget/ProxyItem.h"
#include "db/Database.hpp"

#define AddProfileToListIfExist(_id) auto __ent = NekoRay::profileManager->GetProfile(_id); \
if (__ent != nullptr && __ent->type != "chain") { \
auto w = new ProxyItem(this, __ent); \
auto wI = new QListWidgetItem(); \
wI->setData(114514, _id); \
wI->setSizeHint(w->sizeHint()); \
ui->listWidget->addItem(wI); \
ui->listWidget->setItemWidget(wI, w); \
}

#define DeleteSelected auto items = ui->listWidget->selectedItems(); \
for (auto item: items) { \
delete item; \
}


EditChain::EditChain(QWidget *parent) : QWidget(parent), ui(new Ui::EditChain) {
    ui->setupUi(this);
    ui->listWidget->addActions(CreateActions(this, QStringList{tr("Delete")}, [=](QAction *action) {
        DeleteSelected
    }));
}

EditChain::~EditChain() {
    delete ui;
}

void EditChain::onStart(QSharedPointer<NekoRay::ProxyEntity> _ent) {
    this->ent = _ent;
    auto bean = this->ent->ChainBean();

    for (auto id: bean->list) {
        AddProfileToListIfExist(id)
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
        AddProfileToListIfExist(id)
    });
}

void EditChain::keyPressEvent(QKeyEvent *event) {
    switch (event->key()) {
        case Qt::Key_Delete: {
            DeleteSelected
            break;
        }
        default:
            QWidget::keyPressEvent(event);
    }
}
