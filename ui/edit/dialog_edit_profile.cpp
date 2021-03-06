#include "dialog_edit_profile.h"
#include "ui_dialog_edit_profile.h"

#include "ui/edit/edit_socks_http.h"
#include "ui/edit/edit_shadowsocks.h"
#include "ui/edit/edit_chain.h"
#include "ui/edit/edit_vmess.h"
#include "ui/edit/edit_trojan_vless.h"
#include "ui/edit/edit_naive.h"
#include "ui/edit/edit_custom.h"

#include "fmt/includes.h"
#include "qv2ray/ui/widgets/editors/w_JsonEditor.hpp"
#include "main/GuiUtils.hpp"

#include <QInputDialog>

#define ADJUST_SIZE runOnUiThread([=] { adjustSize(); adjustPosition(mainwindow); }, this);
#define LOAD_TYPE(a) ui->type->addItem(NekoRay::ProfileManager::NewProxyEntity(a)->bean->DisplayType(), a);

DialogEditProfile::DialogEditProfile(const QString &_type, int profileOrGroupId, QWidget *parent)
        : QDialog(parent),
          ui(new Ui::DialogEditProfile) {
    // setup UI
    ui->setupUi(this);
    ui->dialog_layout->setAlignment(ui->left, Qt::AlignTop);
    ui->dialog_layout->setAlignment(ui->right_all, Qt::AlignTop);

    // network changed
    network_title_base = ui->network_box->title();
    connect(ui->network, &QComboBox::currentTextChanged, this, [=](const QString &txt) {
        if (txt == "tcp" || txt == "quic") {
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
        ADJUST_SIZE
    });
    ui->network->removeItem(0);

    // security changed
    connect(ui->security, &QComboBox::currentTextChanged, this, [=](const QString &txt) {
        if (txt == "tls") {
            ui->security_box->setVisible(true);
        } else {
            ui->security_box->setVisible(false);
        }
        ADJUST_SIZE
    });
    ui->security->removeItem(0);

    // 确定模式和 ent
    newEnt = _type != "";
    if (newEnt) {
        this->groupId = profileOrGroupId;
        this->type = _type;

        // load type to combo box
        LOAD_TYPE("socks")
        LOAD_TYPE("http")
        LOAD_TYPE("shadowsocks");
        LOAD_TYPE("trojan");
        LOAD_TYPE("vmess");
        LOAD_TYPE("vless");
        LOAD_TYPE("naive");
        ui->type->addItem("Hysteria", "hysteria");
        ui->type->addItem(tr("Custom"), "custom");
        LOAD_TYPE("chain");

        // type changed
        connect(ui->type, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [=](int index) {
            typeSelected(ui->type->itemData(index).toString());
        });
    } else {
        this->ent = NekoRay::profileManager->GetProfile(profileOrGroupId);
        if (this->ent == nullptr) return;
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

    if (type == "socks" || type == "http") {
        auto _innerWidget = new EditSocksHttp(this);
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
    } else if (type == "trojan" || type == "vless") {
        auto _innerWidget = new EditTrojanVLESS(this);
        innerWidget = _innerWidget;
        innerEditor = _innerWidget;
    } else if (type == "naive") {
        auto _innerWidget = new EditNaive(this);
        innerWidget = _innerWidget;
        innerEditor = _innerWidget;
    } else if (type == "custom" || type == "hysteria") {
        auto _innerWidget = new EditCustom(this);
        innerWidget = _innerWidget;
        innerEditor = _innerWidget;
        // I don't want to write a settings
        if (type == "hysteria") {
            _innerWidget->preset_core = type;
            _innerWidget->preset_command = "-c %config%";
            _innerWidget->preset_config = "{\n"
                                          "  \"server\": \"127.0.0.1:%mapping_port%\",\n"
                                          "  \"obfs\": \"fuck me till the daylight\",\n"
                                          "  \"up_mbps\": 10,\n"
                                          "  \"down_mbps\": 50,\n"
                                          "  \"server_name\": \"real.name.com\",\n"
                                          "  \"socks5\": {\n"
                                          "    \"listen\": \"127.0.0.1:%socks_port%\"\n"
                                          "  }\n"
                                          "}";
            type = "custom";
        }
    } else {
        validType = false;
    }

    if (!validType) {
        MessageBoxWarning(newType, "Wrong type");
        return;
    }

    if (newEnt) {
        this->ent = NekoRay::ProfileManager::NewProxyEntity(type);
        this->ent->gid = groupId;
    }

    // hide some widget
    auto notChain = type != "chain";
    ui->address->setVisible(notChain);
    ui->address_l->setVisible(notChain);
    ui->port->setVisible(notChain);
    ui->port_l->setVisible(notChain);

    // 右边 Outbound: settings
    auto stream = GetStreamSettings(ent->bean);
    if (stream != nullptr) {
        ui->right_all_w->setVisible(true);
        ui->network->setCurrentText(stream->network);
        ui->security->setCurrentText(stream->security);
        ui->packet_encoding->setCurrentText(stream->packet_encoding);
        ui->path->setText(stream->path);
        ui->host->setText(stream->host);
        ui->sni->setText(stream->sni);
        ui->insecure->setChecked(stream->allow_insecure);
        CACHE.certificate = stream->certificate;
    } else {
        ui->right_all_w->setVisible(false);
    }
    auto custom_item = ent->bean->_get("custom");
    if (custom_item != nullptr) {
        ui->custom_box->setVisible(true);
        CACHE.custom = *((QString *) custom_item->ptr);
    } else {
        ui->custom_box->setVisible(false);
    }

    // 左边 bean
    auto old = ui->bean->layout()->itemAt(0)->widget();
    ui->bean->layout()->removeWidget(old);
    innerWidget->layout()->setContentsMargins(0, 0, 0, 0);
    ui->bean->layout()->addWidget(innerWidget);
    ui->bean->setTitle(ent->bean->DisplayType());
    delete old;

    // 左边 bean inner editor
    innerEditor->get_edit_dialog = [&]() {
        return (QWidget *) this;
    };
    innerEditor->editor_cache_updated = [=] {
        editor_cache_updated_impl();
    };
    innerEditor->onStart(ent);

    // 左边 common
    ui->name->setText(ent->bean->name);
    ui->address->setText(ent->bean->serverAddress);
    ui->port->setText(Int2String(ent->bean->serverPort));
    ui->port->setValidator(QRegExpValidator_Number, this));

    editor_cache_updated_impl();
    ADJUST_SIZE

    // 第一次显示
    if (isHidden()) {
        show();
    }
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
    auto stream = GetStreamSettings(ent->bean);
    if (stream != nullptr) {
        stream->network = ui->network->currentText();
        stream->security = ui->security->currentText();
        stream->packet_encoding = ui->packet_encoding->currentText();
        stream->path = ui->path->text();
        stream->host = ui->host->text();
        stream->sni = ui->sni->text();
        stream->allow_insecure = ui->insecure->isChecked();
        stream->certificate = CACHE.certificate;
    }
    auto custom_item = ent->bean->_get("custom");
    if (custom_item != nullptr) {
        *((QString *) custom_item->ptr) = CACHE.custom;
    }

    if (newEnt) {
        auto ok = NekoRay::profileManager->AddProfile(ent);
        if (!ok) {
            MessageBoxWarning("???", "id exists");
        }
    } else {
        ent->Save();
    }

    dialog_message(Dialog_DialogEditProfile, "accept");
    QDialog::accept();
}

// cached editor (dialog)

void DialogEditProfile::editor_cache_updated_impl() {
    if (CACHE.certificate.isEmpty()) {
        ui->certificate_edit->setText(tr("Not set"));
    } else {
        ui->certificate_edit->setText(tr("Already set"));
    }
    if (CACHE.custom.isEmpty()) {
        ui->custom_edit->setText(tr("Not set"));
    } else {
        ui->custom_edit->setText(tr("Already set"));
    }

    // CACHE macro
    for (auto a: innerEditor->get_editor_cached()) {
        if (a.second.isEmpty()) {
            a.first->setText(tr("Not set"));
        } else {
            a.first->setText(tr("Already set"));
        }
    }
}

void DialogEditProfile::on_custom_edit_clicked() {
    C_EDIT_JSON_ALLOW_EMPTY(custom)
    editor_cache_updated_impl();
}

void DialogEditProfile::on_certificate_edit_clicked() {
    bool ok;
    auto txt = QInputDialog::getMultiLineText(this, tr("Certificate"), "", CACHE.certificate, &ok);
    if (ok) {
        CACHE.certificate = txt;
        editor_cache_updated_impl();
    }
}
