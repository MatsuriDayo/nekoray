#include "edit_naive.h"
#include "ui_edit_naive.h"

#include <QInputDialog>

EditNaive::EditNaive(QWidget *parent) :
        QWidget(parent), ui(new Ui::EditNaive) {
    ui->setupUi(this);
}

EditNaive::~EditNaive() {
    delete ui;
}

void EditNaive::onStart(QSharedPointer<NekoRay::ProxyEntity> _ent) {
    this->ent = _ent;
    auto bean = this->ent->NaiveBean();

    P_E_LOAD_STRING(username);
    P_E_LOAD_STRING(password);
    P_E_LOAD_COMBO(protocol);
    P_E_LOAD_STRING_CACHE(extra_headers);
    P_E_LOAD_STRING(sni);
    P_E_LOAD_STRING_CACHE(certificate);
    P_E_LOAD_INT(insecure_concurrency);
}

bool EditNaive::onEnd() {
    auto bean = this->ent->NaiveBean();

    P_E_SAVE_STRING(username);
    P_E_SAVE_STRING(password);
    P_E_SAVE_COMBO(protocol);
    P_E_SAVE_STRING_CACHE(extra_headers);
    P_E_SAVE_STRING(sni);
    P_E_SAVE_STRING_CACHE(certificate);
    P_E_SAVE_INT(insecure_concurrency);

    return true;
}

QList<QPair<QPushButton *, QString>> EditNaive::get_editor_cached() {
    return {
            {ui->certificate,   editCache.certificate},
            {ui->extra_headers, editCache.extra_headers},
    };
}

void EditNaive::on_certificate_clicked() {
    bool ok;
    auto txt = QInputDialog::getMultiLineText(this, tr("Certificate"), "", editCache.certificate, &ok);
    if (ok) {
        editCache.certificate = txt;
        dialog_editor_cache_updated();
    }
}

void EditNaive::on_extra_headers_clicked() {
    bool ok;
    auto txt = QInputDialog::getMultiLineText(this, tr("Extra headers"), "", editCache.extra_headers, &ok);
    if (ok) {
        editCache.extra_headers = txt;
        dialog_editor_cache_updated();
    }
}
