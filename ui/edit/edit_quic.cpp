#include "edit_quic.h"
#include "ui_edit_quic.h"

#include "fmt/QUICBean.hpp"

#include <QInputDialog>
#include <QUuid>

EditQUIC::EditQUIC(QWidget *parent) : QWidget(parent), ui(new Ui::EditQUIC) {
    ui->setupUi(this);
    connect(ui->uuidgen, &QPushButton::clicked, this, [=] { ui->uuid->setText(QUuid::createUuid().toString().remove("{").remove("}")); });
}

EditQUIC::~EditQUIC() {
    delete ui;
}

void EditQUIC::onStart(std::shared_ptr<NekoGui::ProxyEntity> _ent) {
    this->ent = _ent;
    auto bean = this->ent->QUICBean();

    P_LOAD_STRING(hopPort);
    P_LOAD_INT(hopInterval);
    P_LOAD_INT(uploadMbps);
    P_LOAD_INT(downloadMbps);
    P_LOAD_BOOL(disableMtuDiscovery)
    P_LOAD_STRING(obfsPassword);
    P_LOAD_STRING(authPayload);
    P_LOAD_INT(streamReceiveWindow);
    P_LOAD_INT(connectionReceiveWindow);

    P_LOAD_BOOL(forceExternal);
    P_LOAD_COMBO_INT(hyProtocol);
    P_LOAD_COMBO_INT(authPayloadType);
    P_LOAD_STRING(uuid);
    P_LOAD_STRING(password);

    P_LOAD_COMBO_STRING(congestionControl);
    P_LOAD_COMBO_STRING(udpRelayMode);
    P_LOAD_BOOL(zeroRttHandshake);
    P_LOAD_STRING(heartbeat);
    P_LOAD_BOOL(uos);

    // TLS
    P_LOAD_STRING(sni);
    P_LOAD_STRING(alpn);
    P_C_LOAD_STRING(caText);
    P_LOAD_BOOL(allowInsecure);
    P_LOAD_BOOL(disableSni);

    if (bean->proxy_type == NekoGui_fmt::QUICBean::proxy_Hysteria || bean->proxy_type == NekoGui_fmt::QUICBean::proxy_Hysteria2) {
        ui->uuid->hide();
        ui->uuid_l->hide();
        ui->uuidgen->hide();
        ui->congestionControl->hide();
        ui->congestionControl_l->hide();
        ui->udpRelayMode->hide();
        ui->udpRelayMode_l->hide();
        ui->zeroRttHandshake->hide();
        ui->heartbeat->hide();
        ui->heartbeat_l->hide();
        ui->uos->hide();
        if (!IS_NEKO_BOX) ui->forceExternal->hide();

        if (bean->proxy_type == NekoGui_fmt::QUICBean::proxy_Hysteria) { // hy1
            ui->password->hide();
            ui->password_l->hide();
        } else { // hy2
            ui->hyProtocol->hide();
            ui->hyProtocol_l->hide();
            ui->hyProtocol->hide();
            ui->hyProtocol_l->hide();
            ui->authPayload->hide();
            ui->authPayload_l->hide();
            ui->authPayloadType->hide();
            ui->authPayloadType_l->hide();
            ui->alpn->hide();
            ui->alpn_l->hide();
            ui->TLS->removeItem(ui->alpn_sp);
            if (IS_NEKO_BOX) {
                ui->disableMtuDiscovery->hide();
                ui->hopInterval->hide();
                ui->hopInterval_l->hide();
                ui->hopPort->hide();
                ui->hopPort_l->hide();
                ui->connectionReceiveWindow->hide();
                ui->connectionReceiveWindow_l->hide();
                ui->streamReceiveWindow->hide();
                ui->streamReceiveWindow_l->hide();
            }
        }
    } else if (bean->proxy_type == NekoGui_fmt::QUICBean::proxy_TUIC) {
        ui->hopPort->hide();
        ui->hopPort_l->hide();
        ui->hopInterval->hide();
        ui->hopInterval_l->hide();
        ui->uploadMbps->hide();
        ui->uploadMbps_l->hide();
        ui->downloadMbps->hide();
        ui->downloadMbps_l->hide();
        ui->hyProtocol->hide();
        ui->hyProtocol_l->hide();
        ui->disableMtuDiscovery->hide();
        ui->obfsPassword->hide();
        ui->obfsPassword_l->hide();
        ui->authPayload->hide();
        ui->authPayload_l->hide();
        ui->authPayloadType->hide();
        ui->authPayloadType_l->hide();
        ui->streamReceiveWindow->hide();
        ui->streamReceiveWindow_l->hide();
        ui->connectionReceiveWindow->hide();
        ui->connectionReceiveWindow_l->hide();
        if (!IS_NEKO_BOX) {
            ui->uos->hide();
        }
    }
}

bool EditQUIC::onEnd() {
    auto bean = this->ent->QUICBean();

    P_SAVE_BOOL(forceExternal);

    // Hysteria
    P_SAVE_STRING(hopPort);
    P_SAVE_INT(hopInterval);
    P_SAVE_INT(uploadMbps);
    P_SAVE_INT(downloadMbps);
    P_SAVE_COMBO_INT(hyProtocol);
    P_SAVE_BOOL(disableMtuDiscovery)
    P_SAVE_STRING(obfsPassword);
    P_SAVE_COMBO_INT(authPayloadType);
    P_SAVE_STRING(authPayload);
    P_SAVE_INT(streamReceiveWindow);
    P_SAVE_INT(connectionReceiveWindow);

    // TUIC
    P_SAVE_STRING(uuid);
    P_SAVE_STRING(password);
    P_SAVE_COMBO_STRING(congestionControl);
    P_SAVE_COMBO_STRING(udpRelayMode);
    P_SAVE_BOOL(zeroRttHandshake);
    P_SAVE_STRING(heartbeat);
    P_SAVE_BOOL(uos);

    // TLS
    P_SAVE_STRING(sni);
    P_SAVE_STRING(alpn);
    P_SAVE_BOOL(allowInsecure);
    P_C_SAVE_STRING(caText);
    P_SAVE_BOOL(disableSni);
    return true;
}

QList<QPair<QPushButton *, QString>> EditQUIC::get_editor_cached() {
    return {
        {ui->certificate, CACHE.caText},
    };
}

void EditQUIC::on_certificate_clicked() {
    bool ok;
    auto txt = QInputDialog::getMultiLineText(this, tr("Certificate"), "", CACHE.caText, &ok);
    if (ok) {
        CACHE.caText = txt;
        editor_cache_updated();
    }
}
