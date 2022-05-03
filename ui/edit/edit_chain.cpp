#include "edit_chain.h"
#include "ui_edit_chain.h"

#include "db/Database.hpp"

EditChain::EditChain(QWidget *parent) :
        QWidget(parent), ui(new Ui::EditChain) {
    ui->setupUi(this);
}

EditChain::~EditChain() {
    delete ui;
}

void EditChain::onStart(QSharedPointer<NekoRay::ProxyEntity> _ent) {
    this->ent = _ent;
    auto bean = this->ent->ChainBean();

    QStringList list2;
    for (auto id: bean->list) {
        list2 += Int2String(id);
    }
    ui->plainTextEdit->setPlainText(list2.join("\n"));
}

bool EditChain::onEnd() {
    auto bean = this->ent->ChainBean();
    QList<int> idList;
    auto idStrList = SplitLines(ui->plainTextEdit->toPlainText());
    for (const auto &idStr: idStrList) {
        bool ok;
        auto id = idStr.toInt(&ok);
        if (!ok || !NekoRay::profileManager->profiles.contains(id)) {
            MessageBoxWarning("Error", tr("Invalid ID: %1").arg((idStr)));
            return false;
        }
        idList += id;
    }
    bean->list = idList;

    return true;
}

