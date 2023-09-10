#include "dialog_manage_routes.h"
#include "ui_dialog_manage_routes.h"

#include "3rdparty/qv2ray/v2/ui/widgets/editors/w_JsonEditor.hpp"
#include "3rdparty/qv2ray/v3/components/GeositeReader/GeositeReader.hpp"
#include "main/GuiUtils.hpp"
#include "fmt/Preset.hpp"

#include <QFile>
#include <QMessageBox>
#include <QListWidget>
#include <QLineEdit>

#define REFRESH_ACTIVE_ROUTING(name, obj)           \
    this->active_routing = name;                    \
    setWindowTitle(title_base + " [" + name + "]"); \
    UpdateDisplayRouting(obj, false);

DialogManageRoutes::DialogManageRoutes(QWidget *parent) : QDialog(parent), ui(new Ui::DialogManageRoutes) {
    ui->setupUi(this);
    title_base = windowTitle();

    QStringList qsValue = {""};
    QString dnsHelpDocumentUrl;
    if (IS_NEKO_BOX) {
        ui->outbound_domain_strategy->addItems(Preset::SingBox::DomainStrategy);
        ui->domainStrategyCombo->addItems(Preset::SingBox::DomainStrategy);
        qsValue += QString("prefer_ipv4 prefer_ipv6 ipv4_only ipv6_only").split(" ");
        ui->dns_object->setPlaceholderText(DecodeB64IfValid("ewogICJzZXJ2ZXJzIjogW10sCiAgInJ1bGVzIjogW10sCiAgImZpbmFsIjogIiIsCiAgInN0cmF0ZWd5IjogIiIsCiAgImRpc2FibGVfY2FjaGUiOiBmYWxzZSwKICAiZGlzYWJsZV9leHBpcmUiOiBmYWxzZSwKICAiaW5kZXBlbmRlbnRfY2FjaGUiOiBmYWxzZSwKICAicmV2ZXJzZV9tYXBwaW5nIjogZmFsc2UsCiAgImZha2VpcCI6IHt9Cn0="));
        dnsHelpDocumentUrl = "https://sing-box.sagernet.org/configuration/dns/";
    } else {
        ui->outbound_domain_strategy->addItems({"AsIs", "UseIPv4", "UseIPv6", "PreferIPv4", "PreferIPv6"});
        ui->domainStrategyCombo->addItems({"AsIs", "IPIfNonMatch", "IPOnDemand"});
        qsValue += QString("use_ip use_ip4 use_ip6").split(" ");
        ui->dns_object->setPlaceholderText(DecodeB64IfValid("ewogICJzZXJ2ZXJzIjogW10KfQ=="));
        dnsHelpDocumentUrl = "https://www.v2fly.org/config/dns.html";
    }
    ui->direct_dns_strategy->addItems(qsValue);
    ui->remote_dns_strategy->addItems(qsValue);
    //
    D_C_LOAD_STRING(custom_route_global)
    //
    connect(ui->use_dns_object, &QCheckBox::stateChanged, this, [=](int state) {
        auto useDNSObject = state == Qt::Checked;
        ui->simple_dns_box->setDisabled(useDNSObject);
        ui->dns_object->setDisabled(!useDNSObject);
    });
    ui->use_dns_object->stateChanged(Qt::Unchecked); // uncheck to uncheck
    connect(ui->dns_document, &QPushButton::clicked, this, [=] {
        MessageBoxInfo("DNS", dnsHelpDocumentUrl);
    });
    connect(ui->format_dns_object, &QPushButton::clicked, this, [=] {
        auto obj = QString2QJsonObject(ui->dns_object->toPlainText());
        if (obj.isEmpty()) {
            MessageBoxInfo("DNS", "invaild json");
        } else {
            ui->dns_object->setPlainText(QJsonObject2QString(obj, false));
        }
    });
    //
    connect(ui->custom_route_edit, &QPushButton::clicked, this, [=] {
        C_EDIT_JSON_ALLOW_EMPTY(custom_route)
    });
    connect(ui->custom_route_global_edit, &QPushButton::clicked, this, [=] {
        C_EDIT_JSON_ALLOW_EMPTY(custom_route_global)
    });
    //
    builtInSchemesMenu = new QMenu(this);
    builtInSchemesMenu->addActions(this->getBuiltInSchemes());
    ui->preset->setMenu(builtInSchemesMenu);

    QString geoipFn = NekoGui::FindCoreAsset("geoip.dat");
    QString geositeFn = NekoGui::FindCoreAsset("geosite.dat");
    //
    const auto sourceStringsDomain = Qv2ray::components::GeositeReader::ReadGeoSiteFromFile(geositeFn);
    directDomainTxt = new AutoCompleteTextEdit("geosite", sourceStringsDomain, this);
    proxyDomainTxt = new AutoCompleteTextEdit("geosite", sourceStringsDomain, this);
    blockDomainTxt = new AutoCompleteTextEdit("geosite", sourceStringsDomain, this);
    //
    const auto sourceStringsIP = Qv2ray::components::GeositeReader::ReadGeoSiteFromFile(geoipFn);
    directIPTxt = new AutoCompleteTextEdit("geoip", sourceStringsIP, this);
    proxyIPTxt = new AutoCompleteTextEdit("geoip", sourceStringsIP, this);
    blockIPTxt = new AutoCompleteTextEdit("geoip", sourceStringsIP, this);
    //
    ui->directTxtLayout->addWidget(directDomainTxt, 0, 0);
    ui->proxyTxtLayout->addWidget(proxyDomainTxt, 0, 0);
    ui->blockTxtLayout->addWidget(blockDomainTxt, 0, 0);
    //
    ui->directIPLayout->addWidget(directIPTxt, 0, 0);
    ui->proxyIPLayout->addWidget(proxyIPTxt, 0, 0);
    ui->blockIPLayout->addWidget(blockIPTxt, 0, 0);
    //
    REFRESH_ACTIVE_ROUTING(NekoGui::dataStore->active_routing, NekoGui::dataStore->routing.get())

    ADD_ASTERISK(this)
}

DialogManageRoutes::~DialogManageRoutes() {
    delete ui;
}

void DialogManageRoutes::accept() {
    D_C_SAVE_STRING(custom_route_global)
    bool routeChanged = false;
    if (NekoGui::dataStore->active_routing != active_routing) routeChanged = true;
    SaveDisplayRouting(NekoGui::dataStore->routing.get());
    NekoGui::dataStore->active_routing = active_routing;
    NekoGui::dataStore->routing->fn = ROUTES_PREFIX + NekoGui::dataStore->active_routing;
    if (NekoGui::dataStore->routing->Save()) routeChanged = true;
    //
    QString info = "UpdateDataStore";
    if (routeChanged) info += "RouteChanged";
    MW_dialog_message(Dialog_DialogManageRoutes, info);
    QDialog::accept();
}

// built in settings

QList<QAction *> DialogManageRoutes::getBuiltInSchemes() {
    QList<QAction *> list;
    list.append(this->schemeToAction(tr("Bypass LAN and China"), routing_cn_lan));
    list.append(this->schemeToAction(tr("Global"), routing_global));
    return list;
}

QAction *DialogManageRoutes::schemeToAction(const QString &name, const NekoGui::Routing &scheme) {
    auto *action = new QAction(name, this);
    connect(action, &QAction::triggered, [this, &scheme] { this->UpdateDisplayRouting((NekoGui::Routing *) &scheme, true); });
    return action;
}

void DialogManageRoutes::UpdateDisplayRouting(NekoGui::Routing *conf, bool qv) {
    //
    directDomainTxt->setPlainText(conf->direct_domain);
    proxyDomainTxt->setPlainText(conf->proxy_domain);
    blockDomainTxt->setPlainText(conf->block_domain);
    //
    blockIPTxt->setPlainText(conf->block_ip);
    directIPTxt->setPlainText(conf->direct_ip);
    proxyIPTxt->setPlainText(conf->proxy_ip);
    //
    CACHE.custom_route = conf->custom;
    ui->def_outbound->setCurrentText(conf->def_outbound);
    //
    if (qv) return;
    //
    ui->sniffing_mode->setCurrentIndex(conf->sniffing_mode);
    ui->outbound_domain_strategy->setCurrentText(conf->outbound_domain_strategy);
    ui->domainStrategyCombo->setCurrentText(conf->domain_strategy);
    ui->use_dns_object->setChecked(conf->use_dns_object);
    ui->dns_object->setPlainText(conf->dns_object);
    ui->dns_routing->setChecked(conf->dns_routing);
    ui->remote_dns->setCurrentText(conf->remote_dns);
    ui->remote_dns_strategy->setCurrentText(conf->remote_dns_strategy);
    ui->direct_dns->setCurrentText(conf->direct_dns);
    ui->direct_dns_strategy->setCurrentText(conf->direct_dns_strategy);
    ui->dns_final_out->setCurrentText(conf->dns_final_out);
}

void DialogManageRoutes::SaveDisplayRouting(NekoGui::Routing *conf) {
    conf->direct_ip = directIPTxt->toPlainText();
    conf->direct_domain = directDomainTxt->toPlainText();
    conf->proxy_ip = proxyIPTxt->toPlainText();
    conf->proxy_domain = proxyDomainTxt->toPlainText();
    conf->block_ip = blockIPTxt->toPlainText();
    conf->block_domain = blockDomainTxt->toPlainText();
    conf->def_outbound = ui->def_outbound->currentText();
    conf->custom = CACHE.custom_route;
    //
    conf->sniffing_mode = ui->sniffing_mode->currentIndex();
    conf->domain_strategy = ui->domainStrategyCombo->currentText();
    conf->outbound_domain_strategy = ui->outbound_domain_strategy->currentText();
    conf->use_dns_object = ui->use_dns_object->isChecked();
    conf->dns_object = ui->dns_object->toPlainText();
    conf->dns_routing = ui->dns_routing->isChecked();
    conf->remote_dns = ui->remote_dns->currentText();
    conf->remote_dns_strategy = ui->remote_dns_strategy->currentText();
    conf->direct_dns = ui->direct_dns->currentText();
    conf->direct_dns_strategy = ui->direct_dns_strategy->currentText();
    conf->dns_final_out = ui->dns_final_out->currentText();
}

void DialogManageRoutes::on_load_save_clicked() {
    auto w = new QDialog;
    auto layout = new QVBoxLayout;
    w->setLayout(layout);
    auto lineEdit = new QLineEdit;
    layout->addWidget(lineEdit);
    auto list = new QListWidget;
    layout->addWidget(list);
    for (const auto &name: NekoGui::Routing::List()) {
        list->addItem(name);
    }
    connect(list, &QListWidget::currentTextChanged, lineEdit, &QLineEdit::setText);
    auto bottom = new QHBoxLayout;
    layout->addLayout(bottom);
    auto load = new QPushButton;
    load->setText(tr("Load"));
    bottom->addWidget(load);
    auto save = new QPushButton;
    save->setText(tr("Save"));
    bottom->addWidget(save);
    auto remove = new QPushButton;
    remove->setText(tr("Remove"));
    bottom->addWidget(remove);
    auto cancel = new QPushButton;
    cancel->setText(tr("Cancel"));
    bottom->addWidget(cancel);
    connect(load, &QPushButton::clicked, w, [=] {
        auto fn = lineEdit->text();
        if (!fn.isEmpty()) {
            auto r = std::make_unique<NekoGui::Routing>();
            r->load_control_must = true;
            r->fn = ROUTES_PREFIX + fn;
            if (r->Load()) {
                if (QMessageBox::question(nullptr, software_name, tr("Load routing: %1").arg(fn) + "\n" + r->DisplayRouting()) == QMessageBox::Yes) {
                    REFRESH_ACTIVE_ROUTING(fn, r.get()) // temp save to the window
                    w->accept();
                }
            }
        }
    });
    connect(save, &QPushButton::clicked, w, [=] {
        auto fn = lineEdit->text();
        if (!fn.isEmpty()) {
            auto r = std::make_unique<NekoGui::Routing>();
            SaveDisplayRouting(r.get());
            r->fn = ROUTES_PREFIX + fn;
            if (QMessageBox::question(nullptr, software_name, tr("Save routing: %1").arg(fn) + "\n" + r->DisplayRouting()) == QMessageBox::Yes) {
                r->Save();
                REFRESH_ACTIVE_ROUTING(fn, r.get())
                w->accept();
            }
        }
    });
    connect(remove, &QPushButton::clicked, w, [=] {
        auto fn = lineEdit->text();
        if (!fn.isEmpty() && NekoGui::Routing::List().length() > 1) {
            if (QMessageBox::question(nullptr, software_name, tr("Remove routing: %1").arg(fn)) == QMessageBox::Yes) {
                QFile f(ROUTES_PREFIX + fn);
                f.remove();
                if (NekoGui::dataStore->active_routing == fn) {
                    NekoGui::Routing::SetToActive(NekoGui::Routing::List().first());
                    REFRESH_ACTIVE_ROUTING(NekoGui::dataStore->active_routing, NekoGui::dataStore->routing.get())
                }
                w->accept();
            }
        }
    });
    connect(cancel, &QPushButton::clicked, w, &QDialog::accept);
    connect(list, &QListWidget::itemDoubleClicked, this, [=](QListWidgetItem *item) {
        lineEdit->setText(item->text());
        emit load->clicked();
    });
    w->exec();
    w->deleteLater();
}
