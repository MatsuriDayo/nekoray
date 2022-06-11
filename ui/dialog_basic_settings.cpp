#include "dialog_basic_settings.h"
#include "ui_dialog_basic_settings.h"

#include "qv2ray/ui/widgets/editors/w_JsonEditor.hpp"
#include "ui/ThemeManager.hpp"
#include "main/GuiUtils.hpp"
#include "main/NekoRay.hpp"

#include <QStyleFactory>
#include <QFileDialog>

DialogBasicSettings::DialogBasicSettings(QWidget *parent)
        : QDialog(parent), ui(new Ui::DialogBasicSettings) {
    ui->setupUi(this);

    // Common

    ui->socks_ip->setText(NekoRay::dataStore->inbound_address);
    ui->log_level->setCurrentText(NekoRay::dataStore->log_level);
    CACHE.custom_inbound = NekoRay::dataStore->custom_inbound;

    D_LOAD_INT(inbound_socks_port)
    D_LOAD_INT_ENABLE(inbound_http_port, http_enable)
    D_LOAD_INT_ENABLE(mux_cool, mux_cool_enable)
    D_LOAD_INT(test_concurrent)
    D_LOAD_STRING(test_url)

    connect(ui->custom_inbound_edit, &QPushButton::clicked, this, [=] {
        C_EDIT_JSON_ALLOW_EMPTY(custom_inbound)
    });

    // Style

    ui->language->setCurrentIndex(NekoRay::dataStore->language);
    connect(ui->language, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [=](int index) {
        CACHE.needRestart = true;
    });

    int built_in_len = ui->theme->count();
    ui->theme->addItems(QStyleFactory::keys());
    //
    bool ok;
    auto themeId = NekoRay::dataStore->theme.toInt(&ok);
    if (ok) {
        ui->theme->setCurrentIndex(themeId);
    } else {
        ui->theme->setCurrentText(NekoRay::dataStore->theme);
    }
    //
    connect(ui->theme, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [=](int index) {
        if (index + 1 <= built_in_len) {
            themeManager->ApplyTheme(Int2String(index));
            NekoRay::dataStore->theme = Int2String(index);
        } else {
            themeManager->ApplyTheme(ui->theme->currentText());
            NekoRay::dataStore->theme = ui->theme->currentText();
        }
        repaint();
        mainwindow->repaint();
        NekoRay::dataStore->Save();
    });

    // Subscription

    ui->user_agent->setText(NekoRay::dataStore->user_agent);
    ui->sub_use_proxy->setChecked(NekoRay::dataStore->sub_use_proxy);

    // Core

    ui->core_v2ray_asset->setText(NekoRay::dataStore->v2ray_asset_dir);
    CACHE.core_map = NekoRay::dataStore->extraCore->core_map;
    ui->core_naive->setText(NekoRay::dataStore->extraCore->Get("naive"));
    ui->core_hysteria->setText(NekoRay::dataStore->extraCore->Get("hysteria"));

    connect(ui->core_v2ray_asset, &QLineEdit::textChanged, this, [=] {
        CACHE.needRestart = true;
    });
    connect(ui->core_v2ray_asset_pick, &QPushButton::clicked, this, [=] {
        auto fn = QFileDialog::getExistingDirectory(this, tr("Select"), QDir::currentPath(),
                                                    QFileDialog::Option::ShowDirsOnly | QFileDialog::Option::ReadOnly);
        if (!fn.isEmpty()) {
            ui->core_v2ray_asset->setText(fn);
        }
    });
    connect(ui->core_naive_pick, &QPushButton::clicked, this, [=] {
        auto fn = QFileDialog::getOpenFileName(this, tr("Select"), QDir::currentPath(),
                                               "", nullptr, QFileDialog::Option::ReadOnly);
        if (!fn.isEmpty()) {
            ui->core_naive->setText(fn);
        }
    });
    connect(ui->core_hysteria_pick, &QPushButton::clicked, this, [=] {
        auto fn = QFileDialog::getOpenFileName(this, tr("Select"), QDir::currentPath(),
                                               "", nullptr, QFileDialog::Option::ReadOnly);
        if (!fn.isEmpty()) {
            ui->core_hysteria->setText(fn);
        }
    });
    connect(ui->core_edit, &QPushButton::clicked, this, [=] {
        C_EDIT_JSON_ALLOW_EMPTY(core_map)
    });

    // Security

    ui->insecure_hint->setChecked(NekoRay::dataStore->insecure_hint);
    ui->skip_cert->setChecked(NekoRay::dataStore->skip_cert);
}

DialogBasicSettings::~DialogBasicSettings() {
    delete ui;
}

void DialogBasicSettings::accept() {
    if (CACHE.needRestart) MessageBoxWarning(tr("Settings changed"), tr("Restart nekoray to take effect."));

    // Common

    NekoRay::dataStore->inbound_address = ui->socks_ip->text();
    NekoRay::dataStore->log_level = ui->log_level->currentText();
    NekoRay::dataStore->custom_inbound = CACHE.custom_inbound;

    D_SAVE_INT(inbound_socks_port)
    D_SAVE_INT_ENABLE(inbound_http_port, http_enable)
    D_SAVE_INT_ENABLE(mux_cool, mux_cool_enable)
    D_SAVE_INT(test_concurrent)
    D_SAVE_STRING(test_url)

    // Style

    NekoRay::dataStore->language = ui->language->currentIndex();

    // Subscription

    NekoRay::dataStore->user_agent = ui->user_agent->text();
    NekoRay::dataStore->sub_use_proxy = ui->sub_use_proxy->isChecked();

    // Core

    NekoRay::dataStore->v2ray_asset_dir = ui->core_v2ray_asset->text();
    NekoRay::dataStore->extraCore->core_map = CACHE.core_map;
    NekoRay::dataStore->extraCore->Set("naive", ui->core_naive->text());
    NekoRay::dataStore->extraCore->Set("hysteria", ui->core_hysteria->text());

    // Security

    NekoRay::dataStore->insecure_hint = ui->insecure_hint->isChecked();
    NekoRay::dataStore->skip_cert = ui->skip_cert->isChecked();

    dialog_message(Dialog_DialogBasicSettings, "SaveDataStore");
    QDialog::accept();
}
