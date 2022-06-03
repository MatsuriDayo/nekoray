#include "GroupItem.h"
#include "ui_GroupItem.h"

#include "ui/edit/dialog_edit_group.h"
#include "main/GuiUtils.hpp"

QString ParseSubInfo(const QString &info) {
    if (info.trimmed().isEmpty()) return "";

    QString result;

    long long used = 0;
    long long total = 0;
    long long expire = 0;

    QRegExp re0("total=([0-9]+)");
    if (re0.indexIn(info) != -1) {
        total = re0.cap(1).toLongLong();
    } else {
        return "";
    }
    QRegExp re1("upload=([0-9]+)");
    if (re1.indexIn(info) != -1) {
        used += re1.cap(1).toLongLong();
    }
    QRegExp re2("download=([0-9]+)");
    if (re2.indexIn(info) != -1) {
        used += re2.cap(1).toLongLong();
    }
    QRegExp re3("expire=([0-9]+)");
    if (re3.indexIn(info) != -1) {
        expire = re3.cap(1).toLongLong();
    }

    result = QObject::tr("Used: %1 Remain: %2 Expire: %3")
            .arg(ReadableSize(used), ReadableSize(total - used), DisplayTime(expire, QLocale::ShortFormat));

    return result;
}

GroupItem::GroupItem(QWidget *parent, const QSharedPointer<NekoRay::Group> &ent, QListWidgetItem *item) :
        QWidget(parent), ui(new Ui::GroupItem) {
    ui->setupUi(this);
    this->ent = ent;
    this->item = item;
    if (ent == nullptr) return;

    refresh_data();
}

GroupItem::~GroupItem() {
    delete ui;
}

void GroupItem::refresh_data() {
    ui->name->setText(ent->name);
    auto type = ent->url.isEmpty() ? tr("Basic") : tr("Subscription");
    if (ent->archive) type = tr("Archive") + " " + type;
    ui->type->setText(type);
    if (ent->url.isEmpty()) {
        ui->url->hide();
        ui->subinfo->hide();
        ui->update_sub->hide();
    } else {
        ui->url->setText(ent->url);
        auto subinfo = ParseSubInfo(ent->info);
        if (subinfo.isEmpty()) {
            ui->subinfo->hide();
        } else {
            ui->subinfo->show();
            ui->subinfo->setText(subinfo);
        }
    }

    item->setSizeHint(sizeHint());
}

void GroupItem::on_update_sub_clicked() {
    if (QMessageBox::question(this, tr("Confirmation"), tr("Update %1?").arg(ent->name))
        == QMessageBox::StandardButton::Yes) {
        NekoRay::profileManager->AsyncUpdateSubscription(ent->id, [=] {
            refresh_data();
        });
    }
}

void GroupItem::on_edit_clicked() {
    auto dialog = new DialogEditGroup(ent, this);
    int ret = dialog->exec();
    dialog->deleteLater();

    if (ret == QDialog::Accepted) {
        ent->Save();
        refresh_data();
        dialog_message(Dialog_DialogManageGroups, "refresh" + Int2String(ent->id));
    }
}

void GroupItem::on_remove_clicked() {
    if (QMessageBox::question(this, tr("Confirmation"), tr("Remove %1?").arg(ent->name)) ==
        QMessageBox::StandardButton::Yes) {
        NekoRay::profileManager->DeleteGroup(ent->id);
        dialog_message(Dialog_DialogManageGroups, "refresh-1");
        delete item;
    }
}
