#include "edit_socks_http.h"
#include "ui_edit_socks_http.h"

#include "fmt/SocksHttpBean.hpp"

EditSocksHttp::EditSocksHttp(QWidget *parent) : QWidget(parent),
                                                ui(new Ui::EditSocksHttp) {
    ui->setupUi(this);
}

EditSocksHttp::~EditSocksHttp() {
    delete ui;
}

void EditSocksHttp::onStart(std::shared_ptr<NekoGui::ProxyEntity> _ent) {
    this->ent = _ent;
    auto bean = this->ent->SocksHTTPBean();

    if (bean->socks_http_type == NekoGui_fmt::SocksHttpBean::type_Socks4) {
        ui->version->setCurrentIndex(1);
    } else {
        ui->version->setCurrentIndex(0);
    }
    if (bean->socks_http_type == NekoGui_fmt::SocksHttpBean::type_HTTP) {
        ui->version->setVisible(false);
        ui->version_l->setVisible(false);
    }

    ui->username->setText(bean->username);
    ui->password->setText(bean->password);
}

bool EditSocksHttp::onEnd() {
    auto bean = this->ent->SocksHTTPBean();

    if (ui->version->isVisible()) {
        if (ui->version->currentIndex() == 1) {
            bean->socks_http_type = NekoGui_fmt::SocksHttpBean::type_Socks4;
        } else {
            bean->socks_http_type = NekoGui_fmt::SocksHttpBean::type_Socks5;
        }
    }

    bean->username = ui->username->text();
    bean->password = ui->password->text();

    return true;
}
