#include "GroupItem.h"
#include "ui_GroupItem.h"

#include "ui/edit/dialog_edit_group.h"
#include "main/GuiUtils.hpp"
#include "sub/GroupUpdater.hpp"

#include <QMessageBox>

QString ParseSubInfo(const QString &info) {
    if (info.trimmed().isEmpty()) return "";

    QString result;

    long long used = 0;
    long long total = 0;
    long long expire = 0;

    auto re0m = QRegularExpression("total=([0-9]+)").match(info);
    if (re0m.lastCapturedIndex() >= 1) {
        total = re0m.captured(1).toLongLong();
    } else {
        return "";
    }
    auto re1m = QRegularExpression("upload=([0-9]+)").match(info);
    if (re1m.lastCapturedIndex() >= 1) {
        used += re1m.captured(1).toLongLong();
    }
    auto re2m = QRegularExpression("download=([0-9]+)").match(info);
    if (re2m.lastCapturedIndex() >= 1) {
        used += re2m.captured(1).toLongLong();
    }
    auto re3m = QRegularExpression("expire=([0-9]+)").match(info);
    if (re3m.lastCapturedIndex() >= 1) {
        expire = re3m.captured(1).toLongLong();
    }

    result = QObject::tr("Used: %1 Remain: %2 Expire: %3")
                 .arg(ReadableSize(used), ReadableSize(total - used), DisplayTime(expire, QLocale::ShortFormat));

    return result;
}

GroupItem::GroupItem(QWidget *parent, const std::shared_ptr<NekoGui::Group> &ent, QListWidgetItem *item) : QWidget(parent), ui(new Ui::GroupItem) {
    ui->setupUi(this);
    this->setLayoutDirection(Qt::LeftToRight);

    this->parentWindow = parent;
    this->ent = ent;
    this->item = item;
    if (ent == nullptr) return;

    connect(this, &GroupItem::edit_clicked, this, &GroupItem::on_edit_clicked);
    connect(NekoGui_sub::groupUpdater, &NekoGui_sub::GroupUpdater::asyncUpdateCallback, this, [=](int gid) { if (gid == this->ent->id) refresh_data(); });

    refresh_data();
}

GroupItem::~GroupItem() {
    delete ui;
}

void GroupItem::refresh_data() {
    ui->name->setText(ent->name);

    auto type = ent->url.isEmpty() ? tr("Basic") : tr("Subscription");
    if (ent->archive) type = tr("Archive") + " " + type;
    type += " (" + Int2String(ent->Profiles().length()) + ")";
    ui->type->setText(type);

    if (ent->url.isEmpty()) {
        ui->url->hide();
        ui->subinfo->hide();
        ui->update_sub->hide();
    } else {
        ui->url->setText(ent->url);
        QStringList info;
        if (ent->sub_last_update != 0) {
            info << tr("Last update: %1").arg(DisplayTime(ent->sub_last_update, QLocale::ShortFormat));
        }
        auto subinfo = ParseSubInfo(ent->info);
        if (!ent->info.isEmpty()) {
            info << subinfo;
        }
        if (info.isEmpty()) {
            ui->subinfo->hide();
        } else {
            ui->subinfo->show();
            ui->subinfo->setText(info.join(" | "));
        }
    }
    runOnUiThread(
        [=] {
            adjustSize();
            item->setSizeHint(sizeHint());
            dynamic_cast<QWidget *>(parent())->adjustSize();
        },
        this);
}

void GroupItem::on_update_sub_clicked() {
    NekoGui_sub::groupUpdater->AsyncUpdate(ent->url, ent->id);
}

void GroupItem::on_edit_clicked() {
    auto dialog = new DialogEditGroup(ent, parentWindow);
    connect(dialog, &QDialog::finished, this, [=] {
        if (dialog->result() == QDialog::Accepted) {
            ent->Save();
            refresh_data();
            MW_dialog_message(Dialog_DialogManageGroups, "refresh" + Int2String(ent->id));
        }
        dialog->deleteLater();
    });
    dialog->show();
}

void GroupItem::on_remove_clicked() {
    if (NekoGui::profileManager->groups.size() <= 1) return;
    if (QMessageBox::question(this, tr("Confirmation"), tr("Remove %1?").arg(ent->name)) ==
        QMessageBox::StandardButton::Yes) {
        NekoGui::profileManager->DeleteGroup(ent->id);
        MW_dialog_message(Dialog_DialogManageGroups, "refresh-1");
        delete item;
    }
}
