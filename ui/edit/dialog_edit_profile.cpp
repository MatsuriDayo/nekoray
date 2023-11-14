#include "dialog_edit_profile.h"
#include "ui_dialog_edit_profile.h"

#include "ui/edit/edit_socks_http.h"
#include "ui/edit/edit_shadowsocks.h"
#include "ui/edit/edit_chain.h"
#include "ui/edit/edit_vmess.h"
#include "ui/edit/edit_trojan_vless.h"
#include "ui/edit/edit_naive.h"
#include "ui/edit/edit_quic.h"
#include "ui/edit/edit_custom.h"

#include "fmt/includes.h"
#include "fmt/Preset.hpp"

#include "3rdparty/qv2ray/v2/ui/widgets/editors/w_JsonEditor.hpp"
#include "main/GuiUtils.hpp"

#include <QInputDialog>

#define ADJUST_SIZE runOnUiThread([=] { adjustSize(); adjustPosition(mainwindow); }, this);
#define LOAD_TYPE(a) ui->type->addItem(NekoGui::ProfileManager::NewProxyEntity(a)->bean->DisplayType(), a);

DialogEditProfile::DialogEditProfile(const QString &_type, int profileOrGroupId, QWidget *parent)
    : QDialog(parent), ui(new Ui::DialogEditProfile) {
    // setup UI
    ui->setupUi(this);
    ui->dialog_layout->setAlignment(ui->left, Qt::AlignTop);

    // network changed
    network_title_base = ui->network_box->title();
    connect(ui->network, &QComboBox::currentTextChanged, this, [=](const QString &txt) {
        ui->network_box->setTitle(network_title_base.arg(txt));
        // 传输设置
        if (txt == "tcp") {
            ui->header_type->setVisible(true);
            ui->header_type_l->setVisible(true);
            ui->path->setVisible(true);
            ui->path_l->setVisible(true);
            ui->host->setVisible(true);
            ui->host_l->setVisible(true);
        } else if (txt == "grpc") {
            ui->header_type->setVisible(false);
            ui->header_type_l->setVisible(false);
            ui->path->setVisible(true);
            ui->path_l->setVisible(true);
            ui->host->setVisible(false);
            ui->host_l->setVisible(false);
        } else if (txt == "ws" || txt == "http" || txt == "httpupgrade") {
            ui->header_type->setVisible(false);
            ui->header_type_l->setVisible(false);
            ui->path->setVisible(true);
            ui->path_l->setVisible(true);
            ui->host->setVisible(true);
            ui->host_l->setVisible(true);
        } else {
            ui->header_type->setVisible(false);
            ui->header_type_l->setVisible(false);
            ui->path->setVisible(false);
            ui->path_l->setVisible(false);
            ui->host->setVisible(false);
            ui->host_l->setVisible(false);
        }
        // 传输设置 ED
        if (txt == "ws" && IS_NEKO_BOX) {
            ui->ws_early_data_length->setVisible(true);
            ui->ws_early_data_length_l->setVisible(true);
            ui->ws_early_data_name->setVisible(true);
            ui->ws_early_data_name_l->setVisible(true);
        } else {
            ui->ws_early_data_length->setVisible(false);
            ui->ws_early_data_length_l->setVisible(false);
            ui->ws_early_data_name->setVisible(false);
            ui->ws_early_data_name_l->setVisible(false);
        }
        // 传输设置 for NekoBox
        if (IS_NEKO_BOX) {
            if (!ui->utlsFingerprint->count()) ui->utlsFingerprint->addItems(Preset::SingBox::UtlsFingerPrint);
        } else {
            if (!ui->utlsFingerprint->count()) ui->utlsFingerprint->addItems(Preset::Xray::UtlsFingerPrint);
        }
        // 传输设置 是否可见
        int networkBoxVisible = 0;
        for (auto label: ui->network_box->findChildren<QLabel *>()) {
            if (!label->isHidden()) networkBoxVisible++;
        }
        ui->network_box->setVisible(networkBoxVisible);
        ADJUST_SIZE
    });
    ui->network->removeItem(0);

//    if (IS_NEKO_BOX) {
//        ui->network->addItem("httpupgrade");
//    }

    // security changed
    connect(ui->security, &QComboBox::currentTextChanged, this, [=](const QString &txt) {
        if (txt == "tls") {
            ui->security_box->setVisible(true);
            ui->tls_camouflage_box->setVisible(true);
            if (IS_NEKO_BOX) {
                ui->reality_spx->hide();
                ui->reality_spx_l->hide();
            }
        } else {
            ui->security_box->setVisible(false);
            ui->tls_camouflage_box->setVisible(false);
        }
        ADJUST_SIZE
    });
    emit ui->security->currentTextChanged(ui->security->currentText());

    // 确定模式和 ent
    newEnt = _type != "";
    if (newEnt) {
        this->groupId = profileOrGroupId;
        this->type = _type;

        // load type to combo box
        LOAD_TYPE("socks")
        LOAD_TYPE("http")
        LOAD_TYPE("shadowsocks")
        LOAD_TYPE("trojan")
        LOAD_TYPE("vmess")
        LOAD_TYPE("vless")
        LOAD_TYPE("naive")
        LOAD_TYPE("hysteria")
        LOAD_TYPE("hysteria2")
        LOAD_TYPE("tuic")
        ui->type->addItem(tr("Custom (%1 outbound)").arg(software_core_name), "internal");
        ui->type->addItem(tr("Custom (%1 config)").arg(software_core_name), "internal-full");
        ui->type->addItem(tr("Custom (Extra Core)"), "custom");
        LOAD_TYPE("chain")

        // type changed
        connect(ui->type, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, [=](int index) {
            typeSelected(ui->type->itemData(index).toString());
        });

        ui->apply_to_group->hide();
    } else {
        this->ent = NekoGui::profileManager->GetProfile(profileOrGroupId);
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
    QString customType;
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
    } else if (type == "hysteria" || type == "hysteria2" || type == "tuic") {
        auto _innerWidget = new EditQUIC(this);
        innerWidget = _innerWidget;
        innerEditor = _innerWidget;
    } else if (type == "custom" || type == "internal" || type == "internal-full") {
        auto _innerWidget = new EditCustom(this);
        innerWidget = _innerWidget;
        innerEditor = _innerWidget;
        customType = newEnt ? type : ent->CustomBean()->core;
        if (customType != "custom") _innerWidget->preset_core = customType;
        type = "custom";
    } else {
        validType = false;
    }

    if (!validType) {
        MessageBoxWarning(newType, "Wrong type");
        return;
    }

    if (newEnt) {
        this->ent = NekoGui::ProfileManager::NewProxyEntity(type);
        this->ent->gid = groupId;
    }

    // hide some widget
    auto showAddressPort = type != "chain" && customType != "internal" && customType != "internal-full";
    ui->address->setVisible(showAddressPort);
    ui->address_l->setVisible(showAddressPort);
    ui->port->setVisible(showAddressPort);
    ui->port_l->setVisible(showAddressPort);

    // 右边 stream
    auto stream = GetStreamSettings(ent->bean.get());
    if (stream != nullptr) {
        ui->right_all_w->setVisible(true);
        ui->network->setCurrentText(stream->network);
        ui->security->setCurrentText(stream->security);
        ui->packet_encoding->setCurrentText(stream->packet_encoding);
        ui->path->setText(stream->path);
        ui->host->setText(stream->host);
        ui->sni->setText(stream->sni);
        ui->alpn->setText(stream->alpn);
        if (newEnt) {
            ui->utlsFingerprint->setCurrentText(NekoGui::dataStore->utlsFingerprint);
        } else {
            ui->utlsFingerprint->setCurrentText(stream->utlsFingerprint);
        }
        ui->insecure->setChecked(stream->allow_insecure);
        ui->header_type->setCurrentText(stream->header_type);
        ui->ws_early_data_name->setText(stream->ws_early_data_name);
        ui->ws_early_data_length->setText(Int2String(stream->ws_early_data_length));
        ui->reality_pbk->setText(stream->reality_pbk);
        ui->reality_sid->setText(stream->reality_sid);
        ui->multiplex->setCurrentIndex(stream->multiplex_status);
        CACHE.certificate = stream->certificate;
    } else {
        ui->right_all_w->setVisible(false);
    }

    // left: custom
    CACHE.custom_config = ent->bean->custom_config;
    CACHE.custom_outbound = ent->bean->custom_outbound;
    bool show_custom_config = true;
    bool show_custom_outbound = true;
    if (type == "chain") {
        show_custom_outbound = false;
    } else if (type == "custom") {
        if (customType == "internal") {
            show_custom_outbound = false;
        } else if (customType == "internal-full") {
            show_custom_outbound = false;
            show_custom_config = false;
        }
    }
    ui->custom_box->setVisible(show_custom_outbound);
    ui->custom_global_box->setVisible(show_custom_config);

    // 左边 bean
    auto old = ui->bean->layout()->itemAt(0)->widget();
    ui->bean->layout()->removeWidget(old);
    innerWidget->layout()->setContentsMargins(0, 0, 0, 0);
    ui->bean->layout()->addWidget(innerWidget);
    ui->bean->setTitle(ent->bean->DisplayType());
    delete old;

    // 左边 bean inner editor
    innerEditor->get_edit_dialog = [&]() { return (QWidget *) this; };
    innerEditor->get_edit_text_name = [&]() { return ui->name->text(); };
    innerEditor->get_edit_text_serverAddress = [&]() { return ui->address->text(); };
    innerEditor->get_edit_text_serverPort = [&]() { return ui->port->text(); };
    innerEditor->editor_cache_updated = [=] { editor_cache_updated_impl(); };
    innerEditor->onStart(ent);

    // 左边 common
    ui->name->setText(ent->bean->name);
    ui->address->setText(ent->bean->serverAddress);
    ui->port->setText(Int2String(ent->bean->serverPort));
    ui->port->setValidator(QRegExpValidator_Number);

    // 星号
    ADD_ASTERISK(this)

    // 设置 for NekoBox
    if (IS_NEKO_BOX) {
        if (type == "vmess" || type == "vless") {
            ui->packet_encoding->setVisible(true);
            ui->packet_encoding_l->setVisible(true);
        } else {
            ui->packet_encoding->setVisible(false);
            ui->packet_encoding_l->setVisible(false);
        }
        if (type == "vmess" || type == "vless" || type == "trojan") {
            ui->network_l->setVisible(true);
            ui->network->setVisible(true);
            ui->network_box->setVisible(true);
        } else {
            ui->network_l->setVisible(false);
            ui->network->setVisible(false);
            ui->network_box->setVisible(false);
        }
        if (type == "vmess" || type == "vless" || type == "trojan" || type == "http") {
            ui->security->setVisible(true);
            ui->security_l->setVisible(true);
        } else {
            ui->security->setVisible(false);
            ui->security_l->setVisible(false);
        }
        if (type == "vmess" || type == "vless" || type == "trojan" || type == "shadowsocks") {
            ui->multiplex->setVisible(true);
            ui->multiplex_l->setVisible(true);
        } else {
            ui->multiplex->setVisible(false);
            ui->multiplex_l->setVisible(false);
        }
        // 设置 是否可见
        int streamBoxVisible = 0;
        for (auto label: ui->stream_box->findChildren<QLabel *>()) {
            if (!label->isHidden()) streamBoxVisible++;
        }
        ui->stream_box->setVisible(streamBoxVisible);
    } else {
        ui->packet_encoding->setVisible(false);
        ui->packet_encoding_l->setVisible(false);
    }

    // 载入 type 之后，有些类型没有右边的设置
    auto rightNoBox = (ui->stream_box->isHidden() && ui->network_box->isHidden() && ui->security_box->isHidden());
    if (rightNoBox && !ui->right_all_w->isHidden()) {
        ui->right_all_w->setVisible(false);
    }

    editor_cache_updated_impl();
    ADJUST_SIZE

    // 第一次显示
    if (isHidden()) {
        runOnUiThread([=] { show(); }, this);
    }
}

bool DialogEditProfile::onEnd() {
    // bean
    if (!innerEditor->onEnd()) {
        return false;
    }

    // 左边
    ent->bean->name = ui->name->text();
    ent->bean->serverAddress = ui->address->text().remove(' ');
    ent->bean->serverPort = ui->port->text().toInt();

    // 右边 stream
    auto stream = GetStreamSettings(ent->bean.get());
    if (stream != nullptr) {
        stream->network = ui->network->currentText();
        stream->security = ui->security->currentText();
        stream->packet_encoding = ui->packet_encoding->currentText();
        stream->path = ui->path->text();
        stream->host = ui->host->text();
        stream->sni = ui->sni->text();
        stream->alpn = ui->alpn->text();
        stream->utlsFingerprint = ui->utlsFingerprint->currentText();
        stream->allow_insecure = ui->insecure->isChecked();
        stream->header_type = ui->header_type->currentText();
        stream->ws_early_data_name = ui->ws_early_data_name->text();
        stream->ws_early_data_length = ui->ws_early_data_length->text().toInt();
        stream->reality_pbk = ui->reality_pbk->text();
        stream->reality_sid = ui->reality_sid->text();
        stream->multiplex_status = ui->multiplex->currentIndex();
        stream->certificate = CACHE.certificate;
    }

    // cached custom
    ent->bean->custom_outbound = CACHE.custom_outbound;
    ent->bean->custom_config = CACHE.custom_config;

    return true;
}

void DialogEditProfile::accept() {
    // save to ent
    if (!onEnd()) {
        return;
    }

    // finish
    QStringList msg = {"accept"};

    if (newEnt) {
        auto ok = NekoGui::profileManager->AddProfile(ent);
        if (!ok) {
            MessageBoxWarning("???", "id exists");
        }
    } else {
        auto changed = ent->Save();
        if (changed && NekoGui::dataStore->started_id == ent->id) msg << "restart";
    }

    MW_dialog_message(Dialog_DialogEditProfile, msg.join(","));
    QDialog::accept();
}

// cached editor (dialog)

void DialogEditProfile::editor_cache_updated_impl() {
    if (CACHE.certificate.isEmpty()) {
        ui->certificate_edit->setText(tr("Not set"));
    } else {
        ui->certificate_edit->setText(tr("Already set"));
    }
    if (CACHE.custom_outbound.isEmpty()) {
        ui->custom_outbound_edit->setText(tr("Not set"));
    } else {
        ui->custom_outbound_edit->setText(tr("Already set"));
    }
    if (CACHE.custom_config.isEmpty()) {
        ui->custom_config_edit->setText(tr("Not set"));
    } else {
        ui->custom_config_edit->setText(tr("Already set"));
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

void DialogEditProfile::on_custom_outbound_edit_clicked() {
    C_EDIT_JSON_ALLOW_EMPTY(custom_outbound)
    editor_cache_updated_impl();
}

void DialogEditProfile::on_custom_config_edit_clicked() {
    C_EDIT_JSON_ALLOW_EMPTY(custom_config)
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

void DialogEditProfile::on_apply_to_group_clicked() {
    if (apply_to_group_ui.empty()) {
        apply_to_group_ui[ui->multiplex] = new FloatCheckBox(ui->multiplex, this);
        apply_to_group_ui[ui->sni] = new FloatCheckBox(ui->sni, this);
        apply_to_group_ui[ui->alpn] = new FloatCheckBox(ui->alpn, this);
        apply_to_group_ui[ui->host] = new FloatCheckBox(ui->host, this);
        apply_to_group_ui[ui->path] = new FloatCheckBox(ui->path, this);
        apply_to_group_ui[ui->utlsFingerprint] = new FloatCheckBox(ui->utlsFingerprint, this);
        apply_to_group_ui[ui->insecure] = new FloatCheckBox(ui->insecure, this);
        apply_to_group_ui[ui->certificate_edit] = new FloatCheckBox(ui->certificate_edit, this);
        apply_to_group_ui[ui->custom_config_edit] = new FloatCheckBox(ui->custom_config_edit, this);
        apply_to_group_ui[ui->custom_outbound_edit] = new FloatCheckBox(ui->custom_outbound_edit, this);
        ui->apply_to_group->setText(tr("Confirm"));
    } else {
        auto group = NekoGui::profileManager->GetGroup(ent->gid);
        if (group == nullptr) {
            MessageBoxWarning("failed", "unknown group");
            return;
        }
        // save this
        if (onEnd()) {
            ent->Save();
        } else {
            MessageBoxWarning("failed", "failed to save");
            return;
        }
        // copy keys
        for (const auto &pair: apply_to_group_ui) {
            if (pair.second->isChecked()) {
                do_apply_to_group(group, pair.first);
            }
            delete pair.second;
        }
        apply_to_group_ui.clear();
        ui->apply_to_group->setText(tr("Apply settings to this group"));
    }
}

void DialogEditProfile::do_apply_to_group(const std::shared_ptr<NekoGui::Group> &group, QWidget *key) {
    auto stream = GetStreamSettings(ent->bean.get());

    auto copyStream = [=](void *p) {
        for (const auto &profile: group->Profiles()) {
            auto newStream = GetStreamSettings(profile->bean.get());
            if (newStream == nullptr) continue;
            if (stream == newStream) continue;
            newStream->_setValue(stream->_name(p), p);
            // qDebug() << newStream->ToJsonBytes();
            profile->Save();
        }
    };

    auto copyBean = [=](void *p) {
        for (const auto &profile: group->Profiles()) {
            if (profile == ent) continue;
            profile->bean->_setValue(ent->bean->_name(p), p);
            // qDebug() << profile->bean->ToJsonBytes();
            profile->Save();
        }
    };

    if (key == ui->multiplex) {
        copyStream(&stream->multiplex_status);
    } else if (key == ui->sni) {
        copyStream(&stream->sni);
    } else if (key == ui->alpn) {
        copyStream(&stream->alpn);
    } else if (key == ui->host) {
        copyStream(&stream->host);
    } else if (key == ui->path) {
        copyStream(&stream->path);
    } else if (key == ui->utlsFingerprint) {
        copyStream(&stream->utlsFingerprint);
    } else if (key == ui->insecure) {
        copyStream(&stream->allow_insecure);
    } else if (key == ui->certificate_edit) {
        copyStream(&stream->certificate);
    } else if (key == ui->custom_config_edit) {
        copyBean(&ent->bean->custom_config);
    } else if (key == ui->custom_outbound_edit) {
        copyBean(&ent->bean->custom_outbound);
    }
}
