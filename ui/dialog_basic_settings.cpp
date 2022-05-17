#include <QStyleFactory>
#include <QFileDialog>

#include "qv2ray/ui/widgets/editors/w_JsonEditor.hpp"

#include "ui_dialog_basic_settings.h"

#include "ui/ThemeManager.hpp"
#include "ui/mainwindow_message.h"
#include "ui/dialog_basic_settings.h"

#include "main/NekoRay.hpp"
#include "main/GuiUtils.hpp"

DialogBasicSettings::DialogBasicSettings(QWidget *parent)
        : QDialog(parent), ui(new Ui::DialogBasicSettings) {
    ui->setupUi(this);

    // Common

    ui->socks_port->setValidator(QRegExpValidator_Number, this));
    ui->http_port->setValidator(QRegExpValidator_Number, this));

    ui->socks_ip->setText(NekoRay::dataStore->inbound_address);
    ui->socks_port->setText(Int2String(NekoRay::dataStore->inbound_socks_port));
    ui->log_level->setCurrentText(NekoRay::dataStore->log_level);

    auto http_port = NekoRay::dataStore->inbound_http_port;
    if (http_port > 0) {
        ui->http_enable->setChecked(true);
    } else {
        ui->http_enable->setChecked(false);
        http_port = -http_port;
    }
    ui->http_port->setText(Int2String(http_port));

    auto mux_cool = NekoRay::dataStore->mux_cool;
    if (mux_cool > 0) {
        ui->mux_cool_enable->setChecked(true);
    } else {
        ui->mux_cool_enable->setChecked(false);
        mux_cool = -mux_cool;
    }
    ui->mux_cool->setText(Int2String(mux_cool));

    // Style
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

    // Core

    core_editCache = NekoRay::dataStore->extraCore->core_map;
    ui->core_naive->setText(NekoRay::dataStore->extraCore->Get("naive"));
    ui->core_hysteria->setText(NekoRay::dataStore->extraCore->Get("hysteria"));

    connect(ui->core_naive_pick, &QPushButton::clicked, this, [=] {
        auto fn = QFileDialog::getOpenFileName(this, tr("Select"), QDir::currentPath());
        if (!fn.isEmpty()) {
            ui->core_naive->setText(fn);
        }
    });
    connect(ui->core_hysteria_pick, &QPushButton::clicked, this, [=] {
        auto fn = QFileDialog::getOpenFileName(this, tr("Select"), QDir::currentPath());
        if (!fn.isEmpty()) {
            ui->core_hysteria->setText(fn);
        }
    });

}

DialogBasicSettings::~DialogBasicSettings() {
    delete ui;
}

void DialogBasicSettings::accept() {
    // Common

    NekoRay::dataStore->inbound_address = ui->socks_ip->text();
    NekoRay::dataStore->inbound_socks_port = ui->socks_port->text().toInt();
    NekoRay::dataStore->log_level = ui->log_level->currentText();

    if (ui->http_enable->isChecked()) {
        NekoRay::dataStore->inbound_http_port = ui->http_port->text().toInt();
    } else {
        NekoRay::dataStore->inbound_http_port = -ui->http_port->text().toInt();
    }

    if (ui->mux_cool_enable->isChecked()) {
        NekoRay::dataStore->mux_cool = ui->mux_cool->text().toInt();
    } else {
        NekoRay::dataStore->mux_cool = -ui->mux_cool->text().toInt();
    }

    // Subscription

    NekoRay::dataStore->user_agent = ui->user_agent->text();

    // Core

    NekoRay::dataStore->extraCore->core_map = core_editCache;
    NekoRay::dataStore->extraCore->Set("naive", ui->core_naive->text());
    NekoRay::dataStore->extraCore->Set("hysteria", ui->core_hysteria->text());

    emit GetMainWindow()->dialog_message(Dialog_DialogBasicSettings, "SaveDataStore");
    QDialog::accept();
}

void DialogBasicSettings::on_core_edit_clicked() {
    auto editor = new JsonEditor(QString2QJsonObject(core_editCache), this);
    auto result = editor->OpenEditor();
    core_editCache = QJsonObject2QString(result, true);
    if (result.isEmpty()) core_editCache = "";
}
