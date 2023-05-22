#include "ProxyItem.h"
#include "ui_ProxyItem.h"

#include <QMessageBox>

ProxyItem::ProxyItem(QWidget *parent, const std::shared_ptr<NekoGui::ProxyEntity> &ent, QListWidgetItem *item)
    : QWidget(parent), ui(new Ui::ProxyItem) {
    ui->setupUi(this);
    this->setLayoutDirection(Qt::LeftToRight);

    this->item = item;
    this->ent = ent;
    if (ent == nullptr) return;

    refresh_data();
}

ProxyItem::~ProxyItem() {
    delete ui;
}

void ProxyItem::refresh_data() {
    ui->type->setText(ent->bean->DisplayType());
    ui->name->setText(ent->bean->DisplayName());
    ui->address->setText(ent->bean->DisplayAddress());
    ui->traffic->setText(ent->traffic_data->DisplayTraffic());
    ui->test_result->setText(ent->DisplayLatency());

    runOnUiThread(
        [=] {
            adjustSize();
            item->setSizeHint(sizeHint());
            dynamic_cast<QWidget *>(parent())->adjustSize();
        },
        this);
}

void ProxyItem::on_remove_clicked() {
    if (!this->remove_confirm ||
        QMessageBox::question(this, tr("Confirmation"), tr("Remove %1?").arg(ent->bean->DisplayName())) == QMessageBox::StandardButton::Yes) {
        // TODO do remove (or not) -> callback
        delete item;
    }
}

QPushButton *ProxyItem::get_change_button() {
    return ui->change;
}
