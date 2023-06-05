#include "edit_custom.h"
#include "ui_edit_custom.h"

#include "3rdparty/qv2ray/v2/ui/widgets/editors/w_JsonEditor.hpp"
#include "fmt/CustomBean.hpp"
#include "fmt/Preset.hpp"
#include "db/ConfigBuilder.hpp"
#include "db/Database.hpp"

#include <QMessageBox>
#include <QClipboard>

EditCustom::EditCustom(QWidget *parent) : QWidget(parent), ui(new Ui::EditCustom) {
    ui->setupUi(this);
    ui->config_simple->setPlaceholderText(
        "example:\n"
        "  server-address: \"127.0.0.1:%mapping_port%\"\n"
        "  listen-address: \"127.0.0.1\"\n"
        "  listen-port: %socks_port%\n"
        "  host: your-domain.com\n"
        "  sni: your-domain.com\n");
}

EditCustom::~EditCustom() {
    delete ui;
}

#define SAVE_CUSTOM_BEAN                            \
    P_SAVE_COMBO_STRING(core)                       \
    bean->command = ui->command->text().split(" "); \
    P_SAVE_STRING_PLAIN(config_simple)              \
    P_SAVE_COMBO_STRING(config_suffix)              \
    P_SAVE_INT(mapping_port)                        \
    P_SAVE_INT(socks_port)

void EditCustom::onStart(std::shared_ptr<NekoGui::ProxyEntity> _ent) {
    this->ent = _ent;
    auto bean = this->ent->CustomBean();

    // load known core
    auto core_map = QString2QJsonObject(NekoGui::dataStore->extraCore->core_map);
    for (const auto &key: core_map.keys()) {
        if (key == "naive" || key == "hysteria") continue;
        ui->core->addItem(key);
    }
    if (preset_core == "internal") {
        preset_command = preset_config = "";
        ui->config_simple->setPlaceholderText(
            "{\n"
            "    \"type\": \"socks\",\n"
            "    // ...\n"
            "}");
    } else if (preset_core == "internal-full") {
        preset_command = preset_config = "";
        ui->config_simple->setPlaceholderText(
            "{\n"
            "    \"inbounds\": [],\n"
            "    \"outbounds\": []\n"
            "}");
    }

    // load core ui
    P_LOAD_COMBO_STRING(core)
    ui->command->setText(bean->command.join(" "));
    ui->config_simple->setPlainText(bean->config_simple);
    P_LOAD_COMBO_STRING(config_suffix)
    P_LOAD_INT(mapping_port)
    P_LOAD_INT(socks_port)

    // custom external
    if (!bean->core.isEmpty()) {
        ui->core->setDisabled(true);
    } else if (!preset_core.isEmpty()) {
        bean->core = preset_core;
        ui->core->setDisabled(true);
        ui->core->setCurrentText(preset_core);
        ui->command->setText(preset_command);
        ui->config_simple->setPlainText(preset_config);
    }

    // custom internal
    if (preset_core == "internal" || preset_core == "internal-full") {
        ui->core->hide();
        if (preset_core == "internal") {
            ui->core_l->setText(tr("Outbound JSON, please read the documentation."));
        } else {
            ui->core_l->setText(tr("Please fill the complete config."));
        }
        ui->w_ext1->hide();
        ui->w_ext2->hide();
    }

    // Preview
    connect(ui->preview, &QPushButton::clicked, this, [=] {
        // CustomBean::BuildExternal
        QStringList th;
        auto mapping_port = ui->mapping_port->text().toInt();
        auto socks_port = ui->socks_port->text().toInt();
        th << "%mapping_port% => " + (mapping_port <= 0 ? "Random" : Int2String(mapping_port));
        th << "%socks_port% => " + (socks_port <= 0 ? "Random" : Int2String(socks_port));
        th << "%server_addr% => " + get_edit_text_serverAddress();
        th << "%server_port% => " + get_edit_text_serverPort();
        MessageBoxInfo(tr("Preview replace"), th.join("\n"));
        // EditCustom::onEnd
        auto tmpEnt = NekoGui::ProfileManager::NewProxyEntity("custom");
        auto bean = tmpEnt->CustomBean();
        SAVE_CUSTOM_BEAN
        // 补充
        bean->serverAddress = get_edit_text_serverAddress();
        bean->serverPort = get_edit_text_serverPort().toInt();
        if (bean->core.isEmpty()) return;
        //
        auto result = NekoGui::BuildConfig(tmpEnt, false, false);
        if (!result->error.isEmpty()) {
            MessageBoxInfo(software_name, result->error);
            return;
        }
        for (const auto &extR: result->extRs) {
            auto command = QStringList{extR->program};
            command += extR->arguments;
            auto btn = QMessageBox::information(this, tr("Preview config"),
                                                QString("Command: %1\n\n%2").arg(QStringList2Command(command), extR->config_export),
                                                "OK", "Copy", "", 0, 0);
            if (btn == 1) {
                QApplication::clipboard()->setText(extR->config_export);
            }
        }
    });
}

bool EditCustom::onEnd() {
    if (get_edit_text_name().isEmpty()) {
        MessageBoxWarning(software_name, tr("Name cannot be empty."));
        return false;
    }
    if (ui->core->currentText().isEmpty()) {
        MessageBoxWarning(software_name, tr("Please pick a core."));
        return false;
    }

    auto bean = this->ent->CustomBean();

    SAVE_CUSTOM_BEAN

    return true;
}

void EditCustom::on_as_json_clicked() {
    auto editor = new JsonEditor(QString2QJsonObject(ui->config_simple->toPlainText()), this);
    auto result = editor->OpenEditor();
    if (!result.isEmpty()) {
        ui->config_simple->setPlainText(QJsonObject2QString(result, false));
    }
}
