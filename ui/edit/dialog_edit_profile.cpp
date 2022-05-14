#include "ui_dialog_edit_profile.h"

#include "ui/mainwindow_message.h"

#include "ui/edit/dialog_edit_profile.h"
#include "ui/edit/edit_socks.h"
#include "ui/edit/edit_shadowsocks.h"
#include "ui/edit/edit_chain.h"
#include "ui/edit/edit_vmess.h"
#include "ui/edit/edit_trojan.h"

#include "qv2ray/ui/widgets/editors/w_JsonEditor.hpp"

#include <QInputDialog>

DialogEditProfile::DialogEditProfile(const QString &_type, int profileOrGroupId, QWidget *parent)
        : QDialog(parent),
          ui(new Ui::DialogEditProfile) {
    // setup UI
    ui->setupUi(this);
    ui->dialog_layout->setAlignment(ui->left, Qt::AlignTop);
    ui->dialog_layout->setAlignment(ui->right_all, Qt::AlignTop);

    // type changed
    connect(ui->type, &QComboBox::currentTextChanged, this, &DialogEditProfile::typeSelected);

    // network changed
    network_title_base = ui->network_box->title();
    connect(ui->network, &QComboBox::currentTextChanged, this, [=](const QString &txt) {
        if (txt == "tcp") {
            ui->network_box->setVisible(false);
        } else {
            ui->network_box->setVisible(true);
            ui->network_box->setTitle(network_title_base.arg(txt));
            if (txt == "grpc") {
                ui->host->setVisible(false);
                ui->host_l->setVisible(false);
            } else {
                ui->host->setVisible(true);
                ui->host_l->setVisible(true);
            }
        }
        //?
        adjustSize();
        adjustSize();
    });
    ui->network->removeItem(0);

    // security changed
    connect(ui->security, &QComboBox::currentTextChanged, this, [=](const QString &txt) {
        if (txt == "tls") {
            ui->security_box->setVisible(true);
        } else {
            ui->security_box->setVisible(false);
        }
        //?
        adjustSize();
        adjustSize();
    });
    ui->security->removeItem(0);

    // 确定模式和 ent
    newEnt = _type != "";
    if (newEnt) {
        this->groupId = profileOrGroupId;
        this->type = _type;
    } else {
        this->ent = NekoRay::profileManager->GetProfile(profileOrGroupId);
        this->type = ent->type;
        ui->type->setVisible(false);
        ui->type_l->setVisible(false);
    }

    typeSelected(this->type);
}

DialogEditProfile::~DialogEditProfile() {
    delete ui;
}

void DialogEditProfile::typeSelected(const QString &newType) {
    type = newType;
    bool validType = true;

    if (type == "socks") {
        auto _innerWidget = new EditSocks(this);
        innerWidget = _innerWidget;
        innerEditor = _innerWidget;
    } else if (type == "shadowsocks") {
        auto _innerWidget = new EditShadowSocks(this);
        innerWidget = _innerWidget;
        innerEditor = _innerWidget;
    } else if (type == "chain") {
        auto _innerWidget = new EditChain(this);
        innerWidget = _innerWidget;
        innerEditor = _innerWidget;
    } else if (type == "vmess") {
        auto _innerWidget = new EditVMess(this);
        innerWidget = _innerWidget;
        innerEditor = _innerWidget;
    } else if (type == "trojan") {
        auto _innerWidget = new EditTrojan(this);
        innerWidget = _innerWidget;
        innerEditor = _innerWidget;
    } else {
        validType = false;
    }

    if (!validType) {
        MessageBoxWarning(newType, "Wrong type");
        close();
    }

    if (newEnt) {
        this->ent = NekoRay::ProfileManager::NewProxyEntity(type);
        this->ent->group_id = groupId;
    }

    // hide some widget
    auto notChain = type != "chain";
    ui->address->setVisible(notChain);
    ui->address_l->setVisible(notChain);
    ui->port->setVisible(notChain);
    ui->port_l->setVisible(notChain);

    // 右边 Outbound: settings
    auto stream_item = ent->bean->_get("stream");
    if (stream_item != nullptr) {
        ui->right->setVisible(true);
        auto stream_store = (NekoRay::JsonStore *) stream_item->ptr;
        auto stream = (NekoRay::fmt::V2rayStreamSettings *) stream_store;
        ui->network->setCurrentText(stream->network);
        ui->security->setCurrentText(stream->security);
        ui->path->setText(stream->path);
        ui->host->setText(stream->host);
        ui->sni->setText(stream->sni);
        ui->insecure->setChecked(stream->allow_insecure);
        certificate_edit_cache = stream->certificate;
    } else {
        ui->right->setVisible(false);
    }
    auto custom_item = ent->bean->_get("custom");
    if (custom_item != nullptr) {
        ui->custom_box->setVisible(true);
        custom_edit_cache = *((QString *) custom_item->ptr);
    } else {
        ui->custom_box->setVisible(false);
    }

    // 左边 bean
    auto old = ui->bean->layout()->itemAt(0)->widget();
    ui->bean->layout()->removeWidget(old);
    old->deleteLater();
    ui->bean->layout()->addWidget(innerWidget);
    ui->bean->setTitle(ent->bean->DisplayType());
    innerEditor->onStart(ent);

    // 左边 common
    ui->name->setText(ent->bean->name);
    ui->address->setText(ent->bean->serverAddress);
    ui->port->setText(Int2String(ent->bean->serverPort));
    ui->port->setValidator(QRegExpValidator_Number, this));

    dialog_editor_cache_updated();

    //?
    adjustSize();
    adjustSize();
}

void DialogEditProfile::accept() {
    // 左边
    ent->bean->name = ui->name->text();
    ent->bean->serverAddress = ui->address->text();
    ent->bean->serverPort = ui->port->text().toInt();

    // bean
    if (!innerEditor->onEnd()) {
        return;
    }

    // 右边
    auto stream_item = ent->bean->_get("stream");
    if (stream_item != nullptr) {
        auto stream_store = (NekoRay::JsonStore *) stream_item->ptr;
        auto stream = (NekoRay::fmt::V2rayStreamSettings *) stream_store;
        stream->network = ui->network->currentText();
        stream->security = ui->security->currentText();
        stream->path = ui->path->text();
        stream->host = ui->host->text();
        stream->sni = ui->sni->text();
        stream->allow_insecure = ui->insecure->isChecked();
        stream->certificate = certificate_edit_cache;
    }
    auto custom_item = ent->bean->_get("custom");
    if (custom_item != nullptr) {
        *((QString *) custom_item->ptr) = custom_edit_cache;
    }

    if (newEnt) {
        auto ok = NekoRay::profileManager->AddProfile(ent);
        if (!ok) {
            MessageBoxWarning("???", "id exists");
        }
    } else {
        ent->Save();
    }

    emit GetMainWindow()->dialog_message(Dialog_DialogEditProfile, "accept");
    QDialog::accept();
}

// cached editor (dialog)

void DialogEditProfile::dialog_editor_cache_updated() {
    if (certificate_edit_cache.isEmpty()) {
        ui->certificate_edit->setText(tr("Not set"));
    } else {
        ui->certificate_edit->setText(tr("Already set"));
    }
    if (custom_edit_cache.isEmpty()) {
        ui->custom_edit->setText(tr("Not set"));
    } else {
        ui->custom_edit->setText(tr("Already set"));
    }
}

void DialogEditProfile::on_custom_edit_clicked() {
    auto editor = new JsonEditor(QString2QJsonObject(custom_edit_cache), this);
    auto result = editor->OpenEditor();
    custom_edit_cache = QJsonObject2QString(result, true);
    if (result.isEmpty()) custom_edit_cache = "";
    dialog_editor_cache_updated();
}

void DialogEditProfile::on_certificate_edit_clicked() {
    bool ok;
    auto txt = QInputDialog::getMultiLineText(this, tr("Certificate"), "", "", &ok);
    if (ok) {
        certificate_edit_cache = txt;
        dialog_editor_cache_updated();
    }
}
