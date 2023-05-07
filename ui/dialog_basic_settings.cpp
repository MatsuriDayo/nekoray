#include "dialog_basic_settings.h"
#include "ui_dialog_basic_settings.h"

#include "3rdparty/qv2ray/v2/ui/widgets/editors/w_JsonEditor.hpp"
#include "fmt/Preset.hpp"
#include "ui/ThemeManager.hpp"
#include "ui/Icon.hpp"
#include "main/GuiUtils.hpp"
#include "main/NekoRay.hpp"

#include <QStyleFactory>
#include <QFileDialog>
#include <QInputDialog>
#include <QMessageBox>

class ExtraCoreWidget : public QWidget {
public:
    QString coreName;

    QLabel *label_name;
    MyLineEdit *lineEdit_path;
    QPushButton *pushButton_pick;

    explicit ExtraCoreWidget(QJsonObject *extraCore, const QString &coreName_,
                             QWidget *parent = nullptr)
        : QWidget(parent) {
        coreName = coreName_;
        label_name = new QLabel;
        label_name->setText(coreName);
        lineEdit_path = new MyLineEdit;
        lineEdit_path->setText(extraCore->value(coreName).toString());
        pushButton_pick = new QPushButton;
        pushButton_pick->setText(QObject::tr("Select"));
        auto layout = new QHBoxLayout;
        layout->addWidget(label_name);
        layout->addWidget(lineEdit_path);
        layout->addWidget(pushButton_pick);
        setLayout(layout);
        setContentsMargins(0, 0, 0, 0);
        //
        connect(pushButton_pick, &QPushButton::clicked, this, [=] {
            auto fn = QFileDialog::getOpenFileName(this, QObject::tr("Select"), QDir::currentPath(),
                                                   "", nullptr, QFileDialog::Option::ReadOnly);
            if (!fn.isEmpty()) {
                lineEdit_path->setText(fn);
            }
        });
        connect(lineEdit_path, &QLineEdit::textChanged, this, [=](const QString &newTxt) {
            extraCore->insert(coreName, newTxt);
        });
    }
};

DialogBasicSettings::DialogBasicSettings(QWidget *parent)
    : QDialog(parent), ui(new Ui::DialogBasicSettings) {
    ui->setupUi(this);
    ADD_ASTERISK(this);

    // Common

    if (IS_NEKO_BOX) {
        ui->groupBox_http->hide();
        ui->inbound_socks_port_l->setText(ui->inbound_socks_port_l->text().replace("Socks", "Mixed"));
        ui->hlayout_l2->addWidget(ui->groupbox_custom_inbound);
        ui->log_level->addItems(QString("trace debug info warn error fatal panic").split(" "));
        ui->mux_protocol->addItems({"", "smux", "yamux"});
    } else {
        ui->log_level->addItems({"debug", "info", "warning", "none"});
        ui->mux_protocol->addItems({"", "mux.cool"});
    }

    refresh_auth();

    D_LOAD_STRING(inbound_address)
    D_LOAD_COMBO_STRING(log_level)
    CACHE.custom_inbound = NekoRay::dataStore->custom_inbound;
    D_LOAD_INT(inbound_socks_port)
    D_LOAD_INT_ENABLE(inbound_http_port, http_enable)
    D_LOAD_INT(mux_concurrency)
    D_LOAD_COMBO_STRING(mux_protocol)
    D_LOAD_INT(test_concurrent)
    D_LOAD_STRING(test_url)

    connect(ui->custom_inbound_edit, &QPushButton::clicked, this, [=] {
        C_EDIT_JSON_ALLOW_EMPTY(custom_inbound)
    });

#ifdef Q_OS_WIN
    connect(ui->sys_proxy_format, &QPushButton::clicked, this, [=] {
        bool ok;
        auto str = QInputDialog::getItem(this, ui->sys_proxy_format->text() + " (Windows)",
                                         tr("Advanced system proxy settings. Please select a format."),
                                         Preset::Windows::system_proxy_format,
                                         Preset::Windows::system_proxy_format.indexOf(NekoRay::dataStore->system_proxy_format),
                                         false, &ok);
        if (ok) NekoRay::dataStore->system_proxy_format = str;
    });
#else
    ui->sys_proxy_format->hide();
#endif

    // Style
    if (IS_NEKO_BOX) {
        ui->connection_statistics_box->setDisabled(true);
    }
    //
    D_LOAD_BOOL(check_include_pre)
    D_LOAD_BOOL(connection_statistics)
    D_LOAD_BOOL(start_minimal)
    D_LOAD_INT(max_log_line)
    //
    if (NekoRay::dataStore->traffic_loop_interval == 500) {
        ui->rfsh_r->setCurrentIndex(0);
    } else if (NekoRay::dataStore->traffic_loop_interval == 1000) {
        ui->rfsh_r->setCurrentIndex(1);
    } else {
        ui->rfsh_r->setCurrentIndex(2);
    }
    //
    ui->language->setCurrentIndex(NekoRay::dataStore->language);
    connect(ui->language, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [=](int index) {
        CACHE.needRestart = true;
    });
    //
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
    //
    ui->AA_EnableHighDpiScaling->setChecked(ReadFileText("groups/HiDPI").toInt() == 1);
    connect(ui->AA_EnableHighDpiScaling, &QCheckBox::clicked, this, [=](bool checked) {
        QFile file;
        file.setFileName("groups/HiDPI");
        file.open(QIODevice::ReadWrite | QIODevice::Truncate);
        if (checked) {
            file.write("1");
        } else {
            file.write("0");
        }
        file.close();
        CACHE.needRestart = true;
    });

    // Subscription

    ui->user_agent->setText(NekoRay::dataStore->user_agent);
    D_LOAD_BOOL(sub_use_proxy)
    D_LOAD_BOOL(sub_clear)
    D_LOAD_BOOL(sub_insecure)

    // Core

    ui->groupBox_core->setTitle(software_core_name);
    ui->core_v2ray_asset->setText(NekoRay::dataStore->v2ray_asset_dir);
    //
    CACHE.extraCore = QString2QJsonObject(NekoRay::dataStore->extraCore->core_map);
    if (!CACHE.extraCore.contains("naive")) CACHE.extraCore.insert("naive", "");
    if (!CACHE.extraCore.contains("hysteria")) CACHE.extraCore.insert("hysteria", "");
    //
    auto extra_core_layout = ui->extra_core_box->layout();
    for (const auto &s: CACHE.extraCore.keys()) {
        extra_core_layout->addWidget(new ExtraCoreWidget(&CACHE.extraCore, s));
    }
    //
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
    connect(ui->extra_core_add, &QPushButton::clicked, this, [=] {
        bool ok;
        auto s = QInputDialog::getText(nullptr, tr("Add"),
                                       tr("Please input the core name."),
                                       QLineEdit::Normal, "", &ok)
                     .trimmed();
        if (s.isEmpty() || !ok) return;
        if (CACHE.extraCore.contains(s)) return;
        extra_core_layout->addWidget(new ExtraCoreWidget(&CACHE.extraCore, s));
        CACHE.extraCore.insert(s, "");
    });
    connect(ui->extra_core_del, &QPushButton::clicked, this, [=] {
        bool ok;
        auto s = QInputDialog::getItem(nullptr, tr("Delete"),
                                       tr("Please select the core name."),
                                       CACHE.extraCore.keys(), 0, false, &ok);
        if (s.isEmpty() || !ok) return;
        for (int i = 0; i < extra_core_layout->count(); i++) {
            auto item = extra_core_layout->itemAt(i);
            auto ecw = dynamic_cast<ExtraCoreWidget *>(item->widget());
            if (ecw != nullptr && ecw->coreName == s) {
                ecw->deleteLater();
                CACHE.extraCore.remove(s);
                return;
            }
        }
    });

    // switch core
    ui->switch_core_v2ray->setChecked(!IS_NEKO_BOX);
    ui->switch_core_sing_box->setChecked(IS_NEKO_BOX);
    auto switch_core_on_click = [=] {
        int neko_core_new;
        if (sender() == ui->switch_core_sing_box) {
            if (IS_NEKO_BOX) return;
            neko_core_new = NekoRay::CoreType::SING_BOX;
        } else {
            if (!IS_NEKO_BOX) return;
            neko_core_new = NekoRay::CoreType::V2RAY;
        }
        QString core_name_new = dynamic_cast<QRadioButton *>(sender())->text();
        if (QMessageBox::question(this, tr("Confirmation"),
                                  tr("Switching the core to %1, click \"Yes\" to complete the switch and the program will restart. This feature may be unstable, please do not switch frequently.")
                                      .arg(core_name_new)) == QMessageBox::StandardButton::Yes) {
            QFile file;
            file.setFileName("groups/coreType");
            file.open(QIODevice::ReadWrite | QIODevice::Truncate);
            file.write(Int2String(neko_core_new).toUtf8());
            file.close();
            MW_dialog_message("", "RestartProgram");
        }
    };
    connect(ui->switch_core_v2ray, &QRadioButton::clicked, this, switch_core_on_click);
    connect(ui->switch_core_sing_box, &QRadioButton::clicked, this, switch_core_on_click);

    // Security

    ui->utlsFingerprint->addItems(IS_NEKO_BOX ? Preset::SingBox::UtlsFingerPrint : Preset::V2Ray::UtlsFingerPrint);

    D_LOAD_BOOL(skip_cert)
    ui->enable_js_hook->setCurrentIndex(NekoRay::dataStore->enable_js_hook);
    ui->utlsFingerprint->setCurrentText(NekoRay::dataStore->utlsFingerprint);
}

DialogBasicSettings::~DialogBasicSettings() {
    delete ui;
}

void DialogBasicSettings::accept() {
    // Common

    D_SAVE_STRING(inbound_address)
    D_SAVE_COMBO_STRING(log_level)
    NekoRay::dataStore->custom_inbound = CACHE.custom_inbound;
    D_SAVE_INT(inbound_socks_port)
    D_SAVE_INT_ENABLE(inbound_http_port, http_enable)
    D_SAVE_INT(mux_concurrency)
    D_SAVE_COMBO_STRING(mux_protocol)
    D_SAVE_INT(test_concurrent)
    D_SAVE_STRING(test_url)

    // Style

    NekoRay::dataStore->language = ui->language->currentIndex();
    D_SAVE_BOOL(connection_statistics)
    D_SAVE_BOOL(check_include_pre)
    D_SAVE_BOOL(start_minimal)
    D_SAVE_INT(max_log_line)

    if (NekoRay::dataStore->max_log_line <= 0) {
        NekoRay::dataStore->max_log_line = 200;
    }

    if (ui->rfsh_r->currentIndex() == 0) {
        NekoRay::dataStore->traffic_loop_interval = 500;
    } else if (ui->rfsh_r->currentIndex() == 1) {
        NekoRay::dataStore->traffic_loop_interval = 1000;
    } else {
        NekoRay::dataStore->traffic_loop_interval = 0;
    }

    // Subscription

    NekoRay::dataStore->user_agent = ui->user_agent->text();
    D_SAVE_BOOL(sub_use_proxy)
    D_SAVE_BOOL(sub_clear)
    D_SAVE_BOOL(sub_insecure)

    // Core

    NekoRay::dataStore->v2ray_asset_dir = ui->core_v2ray_asset->text();
    NekoRay::dataStore->extraCore->core_map = QJsonObject2QString(CACHE.extraCore, true);

    // Security

    D_SAVE_BOOL(skip_cert)
    NekoRay::dataStore->enable_js_hook = ui->enable_js_hook->currentIndex();
    NekoRay::dataStore->utlsFingerprint = ui->utlsFingerprint->currentText();

    // 关闭连接统计，停止刷新前清空记录。
    if (NekoRay::dataStore->traffic_loop_interval == 0 || NekoRay::dataStore->connection_statistics == false) {
        MW_dialog_message("", "ClearConnectionList");
    }

    QStringList str{"UpdateDataStore"};
    if (CACHE.needRestart) str << "NeedRestart";
    MW_dialog_message(Dialog_DialogBasicSettings, str.join(","));
    QDialog::accept();
}

// slots

void DialogBasicSettings::refresh_auth() {
    ui->inbound_auth->setText({});
    if (NekoRay::dataStore->inbound_auth->NeedAuth()) {
        ui->inbound_auth->setIcon(Icon::GetMaterialIcon("lock-outline"));
    } else {
        ui->inbound_auth->setIcon(Icon::GetMaterialIcon("lock-open-outline"));
    }
}

void DialogBasicSettings::on_set_custom_icon_clicked() {
    auto title = ui->set_custom_icon->text();
    QString user_icon_path = "./" + software_name.toLower() + ".png";
    auto c = QMessageBox::question(this, title, tr("Please select a PNG file."),
                                   tr("Select"), tr("Reset"), tr("Cancel"), 2, 2);
    if (c == 0) {
        auto fn = QFileDialog::getOpenFileName(this, QObject::tr("Select"), QDir::currentPath(),
                                               "*.png", nullptr, QFileDialog::Option::ReadOnly);
        QImage img(fn);
        if (img.isNull() || img.height() != img.width()) {
            MessageBoxWarning(title, tr("Please select a valid square image."));
            return;
        }
        QFile::copy(fn, user_icon_path);
    } else if (c == 1) {
        QFile::remove(user_icon_path);
    } else {
        return;
    }
    MW_dialog_message(Dialog_DialogBasicSettings, "UpdateIcon");
}

void DialogBasicSettings::on_inbound_auth_clicked() {
    auto w = new QDialog(this);
    w->setWindowTitle(tr("Inbound Auth"));
    auto layout = new QGridLayout;
    w->setLayout(layout);
    //
    auto user_l = new QLabel(tr("Username"));
    auto pass_l = new QLabel(tr("Password"));
    auto user = new MyLineEdit;
    auto pass = new MyLineEdit;
    user->setText(NekoRay::dataStore->inbound_auth->username);
    pass->setText(NekoRay::dataStore->inbound_auth->password);
    //
    layout->addWidget(user_l, 0, 0);
    layout->addWidget(user, 0, 1);
    layout->addWidget(pass_l, 1, 0);
    layout->addWidget(pass, 1, 1);
    auto box = new QDialogButtonBox;
    box->setOrientation(Qt::Horizontal);
    box->setStandardButtons(QDialogButtonBox::Cancel | QDialogButtonBox::Ok);
    connect(box, &QDialogButtonBox::accepted, w, [=] {
        NekoRay::dataStore->inbound_auth->username = user->text();
        NekoRay::dataStore->inbound_auth->password = pass->text();
        MW_dialog_message(Dialog_DialogBasicSettings, "UpdateDataStore");
        w->accept();
    });
    connect(box, &QDialogButtonBox::rejected, w, &QDialog::reject);
    layout->addWidget(box, 2, 1);
    //
    w->exec();
    w->deleteLater();
    refresh_auth();
}

void DialogBasicSettings::on_core_settings_clicked() {
    auto w = new QDialog(this);
    w->setWindowTitle(software_core_name + " Core Options");
    auto layout = new QGridLayout;
    w->setLayout(layout);
    //
    auto line = -1;
    QCheckBox *core_box_enable_clash_api;
    MyLineEdit *core_box_clash_api;
    MyLineEdit *core_box_clash_api_secret;
    MyLineEdit *core_box_underlying_dns;
    QCheckBox *core_ray_direct_dns;
    QCheckBox *core_ray_windows_disable_auto_interface;
    //
    auto core_box_underlying_dns_l = new QLabel(tr("Override underlying DNS"));
    core_box_underlying_dns_l->setToolTip(tr(
        "It is recommended to leave it blank, but it sometimes does not work, at this time you can set this option.\n"
        "For NekoRay, this rewrites the underlying(localhost) DNS in VPN mode.\n"
        "For NekoBox, this rewrites the underlying(localhost) DNS in VPN mode, normal mode, and also URL Test."));
    core_box_underlying_dns = new MyLineEdit;
    core_box_underlying_dns->setText(NekoRay::dataStore->core_box_underlying_dns);
    core_box_underlying_dns->setMinimumWidth(300);
    layout->addWidget(core_box_underlying_dns_l, ++line, 0);
    layout->addWidget(core_box_underlying_dns, line, 1);
    //
    if (IS_NEKO_BOX) {
        auto core_box_enable_clash_api_l = new QLabel("Enable Clash API");
        core_box_enable_clash_api = new QCheckBox;
        core_box_enable_clash_api->setChecked(NekoRay::dataStore->core_box_clash_api > 0);
        layout->addWidget(core_box_enable_clash_api_l, ++line, 0);
        layout->addWidget(core_box_enable_clash_api, line, 1);
        //
        auto core_box_clash_api_l = new QLabel("Clash API Listen Port");
        core_box_clash_api = new MyLineEdit;
        core_box_clash_api->setText(Int2String(std::abs(NekoRay::dataStore->core_box_clash_api)));
        layout->addWidget(core_box_clash_api_l, ++line, 0);
        layout->addWidget(core_box_clash_api, line, 1);
        //
        auto core_box_clash_api_secret_l = new QLabel("Clash API Secret");
        core_box_clash_api_secret = new MyLineEdit;
        core_box_clash_api_secret->setText(NekoRay::dataStore->core_box_clash_api_secret);
        layout->addWidget(core_box_clash_api_secret_l, ++line, 0);
        layout->addWidget(core_box_clash_api_secret, line, 1);
    } else {
        auto core_ray_direct_dns_l = new QLabel("NKR_CORE_RAY_DIRECT_DNS");
        core_ray_direct_dns_l->setToolTip(tr("If you VPN mode is not working, try to change this option."));
        core_ray_direct_dns = new QCheckBox;
        core_ray_direct_dns->setChecked(NekoRay::dataStore->core_ray_direct_dns);
        connect(core_ray_direct_dns, &QCheckBox::clicked, this, [&] { CACHE.needRestart = true; });
        layout->addWidget(core_ray_direct_dns_l, ++line, 0);
        layout->addWidget(core_ray_direct_dns, line, 1);
#ifdef Q_OS_WIN
        auto core_ray_windows_disable_auto_interface_l = new QLabel("NKR_CORE_RAY_WINDOWS_DISABLE_AUTO_INTERFACE");
        core_ray_windows_disable_auto_interface_l->setToolTip(tr("If you VPN mode is not working, try to change this option."));
        core_ray_windows_disable_auto_interface = new QCheckBox;
        core_ray_windows_disable_auto_interface->setChecked(NekoRay::dataStore->core_ray_windows_disable_auto_interface);
        connect(core_ray_windows_disable_auto_interface, &QCheckBox::clicked, this, [&] { CACHE.needRestart = true; });
        layout->addWidget(core_ray_windows_disable_auto_interface_l, ++line, 0);
        layout->addWidget(core_ray_windows_disable_auto_interface, line, 1);
#endif
    }
    //
    auto box = new QDialogButtonBox;
    box->setOrientation(Qt::Horizontal);
    box->setStandardButtons(QDialogButtonBox::Cancel | QDialogButtonBox::Ok);
    connect(box, &QDialogButtonBox::accepted, w, [=] {
        NekoRay::dataStore->core_box_underlying_dns = core_box_underlying_dns->text();
        if (IS_NEKO_BOX) {
            NekoRay::dataStore->core_box_clash_api = core_box_clash_api->text().toInt() * (core_box_enable_clash_api->isChecked() ? 1 : -1);
            NekoRay::dataStore->core_box_clash_api_secret = core_box_clash_api_secret->text();
        } else {
            NekoRay::dataStore->core_ray_direct_dns = core_ray_direct_dns->isChecked();
#ifdef Q_OS_WIN
            NekoRay::dataStore->core_ray_windows_disable_auto_interface = core_ray_windows_disable_auto_interface->isChecked();
#endif
        }
        MW_dialog_message(Dialog_DialogBasicSettings, "UpdateDataStore");
        w->accept();
    });
    connect(box, &QDialogButtonBox::rejected, w, &QDialog::reject);
    layout->addWidget(box, ++line, 1);
    //
    ADD_ASTERISK(w)
    w->exec();
    w->deleteLater();
    refresh_auth();
}
