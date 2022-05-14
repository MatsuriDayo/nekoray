#include "dialog_edit_group.h"
#include "ui_dialog_edit_group.h"

#include "db/Database.hpp"

DialogEditGroup::DialogEditGroup(const QSharedPointer<NekoRay::Group> &ent, QWidget *parent) :
        QDialog(parent), ui(new Ui::DialogEditGroup) {
    ui->setupUi(this);

    connect(ui->type, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [=](int index) {
        ui->cat_sub->setHidden(index == 0);
    });

    ui->name->setText(ent->name);
    ui->archive->setChecked(ent->archive);
    ui->url->setText(ent->url);
    ui->type->setCurrentIndex(ent->url.isEmpty() ? 0 : 1);
    ui->type->currentIndexChanged(ui->type->currentIndex());
    if (ent->id >= 0) { // already a group
        ui->type->setDisabled(true);
    } else {
        ui->update_sub->setDisabled(true);
    }

    connect(ui->update_sub, &QAbstractButton::pressed, this, [=] {
        if (QMessageBox::question(this, tr("Confirmation"), QString(tr("update?")))
            == QMessageBox::StandardButton::Yes) {
            NekoRay::profileManager->AsyncUpdateSubscription(ent->id);
        }
    });

    connect(ui->buttonBox, &QDialogButtonBox::accepted, this, [=] {
        ent->name = ui->name->text();
        ent->url = ui->url->text();
        ent->archive = ui->archive->isChecked();
        QDialog::accept();
    });
}

DialogEditGroup::~DialogEditGroup() {
    delete ui;
}

