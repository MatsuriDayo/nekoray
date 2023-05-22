#include "dialog_hotkey.h"
#include "ui_dialog_hotkey.h"

#include "ui/mainwindow_interface.h"

DialogHotkey::DialogHotkey(QWidget *parent) : QDialog(parent), ui(new Ui::DialogHotkey) {
    ui->setupUi(this);
    ui->show_mainwindow->setKeySequence(NekoGui::dataStore->hotkey_mainwindow);
    ui->show_groups->setKeySequence(NekoGui::dataStore->hotkey_group);
    ui->show_routes->setKeySequence(NekoGui::dataStore->hotkey_route);
    ui->system_proxy->setKeySequence(NekoGui::dataStore->hotkey_system_proxy_menu);
    GetMainWindow()->RegisterHotkey(true);
}

DialogHotkey::~DialogHotkey() {
    if (result() == QDialog::Accepted) {
        NekoGui::dataStore->hotkey_mainwindow = ui->show_mainwindow->keySequence().toString();
        NekoGui::dataStore->hotkey_group = ui->show_groups->keySequence().toString();
        NekoGui::dataStore->hotkey_route = ui->show_routes->keySequence().toString();
        NekoGui::dataStore->hotkey_system_proxy_menu = ui->system_proxy->keySequence().toString();
        NekoGui::dataStore->Save();
    }
    GetMainWindow()->RegisterHotkey(false);
    delete ui;
}
