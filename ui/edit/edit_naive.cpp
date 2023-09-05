#include "edit_naive.h"
#include "ui_edit_naive.h"

#include "fmt/NaiveBean.hpp"

#include <QInputDialog>

EditNaive::EditNaive(QWidget *parent) : QWidget(parent), ui(new Ui::EditNaive) {
    ui->setupUi(this);
}

EditNaive::~EditNaive() {
    delete ui;
}

void EditNaive::onStart(std::shared_ptr<NekoGui::ProxyEntity> _ent) {
    this->ent = _ent;
    auto bean = this->ent->NaiveBean();

    P_LOAD_STRING(username);
    P_LOAD_STRING(password);
    P_LOAD_COMBO_STRING(protocol);
    P_C_LOAD_STRING(extra_headers);
    P_LOAD_STRING(sni);
    P_C_LOAD_STRING(certificate);
    P_LOAD_INT(insecure_concurrency);
    P_LOAD_BOOL(disable_log);
}

bool EditNaive::onEnd() {
    auto bean = this->ent->NaiveBean();

    P_SAVE_STRING(username);
    P_SAVE_STRING(password);
    P_SAVE_COMBO_STRING(protocol);
    P_C_SAVE_STRING(extra_headers);
    P_SAVE_STRING(sni);
    P_C_SAVE_STRING(certificate);
    P_SAVE_INT(insecure_concurrency);
    P_SAVE_BOOL(disable_log);

    return true;
}

QList<QPair<QPushButton *, QString>> EditNaive::get_editor_cached() {
    return {
        {ui->certificate, CACHE.certificate},
        {ui->extra_headers, CACHE.extra_headers},
    };
}

void EditNaive::on_certificate_clicked() {
    bool ok;
    auto txt = QInputDialog::getMultiLineText(this, tr("Certificate"), "", CACHE.certificate, &ok);
    if (ok) {
        CACHE.certificate = txt;
        editor_cache_updated();
    }
}

void EditNaive::on_extra_headers_clicked() {
    bool ok;
    auto txt = QInputDialog::getMultiLineText(this, tr("Extra headers"), "", CACHE.extra_headers, &ok);
    if (ok) {
        CACHE.extra_headers = txt;
        editor_cache_updated();
    }
}
