#include "edit_hysteria.h"
#include "ui_edit_hysteria.h"

#include "fmt/HysteriaBean.hpp"

#include <QInputDialog>

EditHysteria::EditHysteria(QWidget *parent) : QWidget(parent), ui(new Ui::EditHysteria) {
    ui->setupUi(this);
}

EditHysteria::~EditHysteria() {
    delete ui;
}

void EditHysteria::onStart(std::shared_ptr<NekoGui::ProxyEntity> _ent) {
    this->ent = _ent;
    auto bean = this->ent->HysteriaBean();

    P_LOAD_STRING(hopPort);
    P_LOAD_INT(hopInterval);
    P_LOAD_INT(uploadMbps);
    P_LOAD_INT(downloadMbps);
    P_LOAD_COMBO_INT(protocol);
    P_LOAD_BOOL(disableMtuDiscovery)
    P_LOAD_STRING(obfsPassword);
    P_LOAD_COMBO_INT(authPayloadType);
    P_LOAD_STRING(authPayload);
    P_LOAD_STRING(sni);
    P_LOAD_STRING(alpn);
    P_LOAD_BOOL(allowInsecure)
    P_C_LOAD_STRING(caText);
    P_LOAD_INT(streamReceiveWindow);
    P_LOAD_INT(connectionReceiveWindow);
}

bool EditHysteria::onEnd() {
    auto bean = this->ent->HysteriaBean();

    P_SAVE_STRING(hopPort);
    P_SAVE_INT(hopInterval);
    P_SAVE_INT(uploadMbps);
    P_SAVE_INT(downloadMbps);
    P_SAVE_COMBO_INT(protocol);
    P_SAVE_BOOL(disableMtuDiscovery)
    P_SAVE_STRING(obfsPassword);
    P_SAVE_COMBO_INT(authPayloadType);
    P_SAVE_STRING(authPayload);
    P_SAVE_STRING(sni);
    P_SAVE_STRING(alpn);
    P_SAVE_BOOL(allowInsecure)
    P_C_SAVE_STRING(caText);
    P_SAVE_INT(streamReceiveWindow);
    P_SAVE_INT(connectionReceiveWindow);

    return true;
}

QList<QPair<QPushButton *, QString>> EditHysteria::get_editor_cached() {
    return {
        {ui->certificate, CACHE.caText},
    };
}

void EditHysteria::on_certificate_clicked() {
    bool ok;
    auto txt = QInputDialog::getMultiLineText(this, tr("Certificate"), "", CACHE.caText, &ok);
    if (ok) {
        CACHE.caText = txt;
        editor_cache_updated();
    }
}
