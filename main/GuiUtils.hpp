#pragma once

#ifdef Q_OS_WIN
#include "sys/windows/guihelper.h"
#endif

// Dialogs

#define Dialog_DialogBasicSettings "DialogBasicSettings"
#define Dialog_DialogEditProfile "DialogEditProfile"
#define Dialog_DialogManageGroups "DialogManageGroups"
#define Dialog_DialogManageRoutes "DialogManageRoutes"

// Utils

#define QRegExpValidator_Number new QRegularExpressionValidator(QRegularExpression("^[0-9]+$"), this)

// NekoRay Save&Load

#define P_C_LOAD_STRING(a) CACHE.a = bean->a;
#define P_C_SAVE_STRING(a) bean->a = CACHE.a;
#define D_C_LOAD_STRING(a) CACHE.a = NekoRay::dataStore->a;
#define D_C_SAVE_STRING(a) NekoRay::dataStore->a = CACHE.a;

#define P_LOAD_STRING(a) ui->a->setText(bean->a);
#define P_SAVE_STRING(a) bean->a = ui->a->text();
#define P_SAVE_STRING_QTEXTEDIT(a) bean->a = ui->a->toPlainText();
#define D_LOAD_STRING(a) ui->a->setText(NekoRay::dataStore->a);
#define D_SAVE_STRING(a) NekoRay::dataStore->a = ui->a->text();
#define D_SAVE_STRING_QTEXTEDIT(a) NekoRay::dataStore->a = ui->a->toPlainText();
#define P_LOAD_INT(a)                    \
    ui->a->setText(Int2String(bean->a)); \
    ui->a->setValidator(QRegExpValidator_Number);
#define P_SAVE_INT(a) bean->a = ui->a->text().toInt();
#define D_LOAD_INT(a)                                  \
    ui->a->setText(Int2String(NekoRay::dataStore->a)); \
    ui->a->setValidator(QRegExpValidator_Number);
#define D_SAVE_INT(a) NekoRay::dataStore->a = ui->a->text().toInt();
#define P_LOAD_COMBO_STR(a) ui->a->setCurrentText(bean->a);
#define P_SAVE_COMBO_STR(a) bean->a = ui->a->currentText();
#define P_LOAD_COMBO_INT(a) ui->a->setCurrentIndex(bean->a);
#define P_SAVE_COMBO_INT(a) bean->a = ui->a->currentIndex();
#define D_LOAD_BOOL(a) ui->a->setChecked(NekoRay::dataStore->a);
#define D_SAVE_BOOL(a) NekoRay::dataStore->a = ui->a->isChecked();
#define P_LOAD_BOOL(a) ui->a->setChecked(bean->a);
#define P_SAVE_BOOL(a) bean->a = ui->a->isChecked();

#define D_LOAD_INT_ENABLE(i, e)                             \
    if (NekoRay::dataStore->i > 0) {                        \
        ui->e->setChecked(true);                            \
        ui->i->setText(Int2String(NekoRay::dataStore->i));  \
    } else {                                                \
        ui->e->setChecked(false);                           \
        ui->i->setText(Int2String(-NekoRay::dataStore->i)); \
    }                                                       \
    ui->i->setValidator(QRegExpValidator_Number);
#define D_SAVE_INT_ENABLE(i, e)                         \
    if (ui->e->isChecked()) {                           \
        NekoRay::dataStore->i = ui->i->text().toInt();  \
    } else {                                            \
        NekoRay::dataStore->i = -ui->i->text().toInt(); \
    }

#define C_EDIT_JSON_ALLOW_EMPTY(a)                                    \
    auto editor = new JsonEditor(QString2QJsonObject(CACHE.a), this); \
    auto result = editor->OpenEditor();                               \
    CACHE.a = QJsonObject2QString(result, true);                      \
    if (result.isEmpty()) CACHE.a = "";                               \
    editor->deleteLater();

// System

#define _ACTIVE_THIS_WINDOW_COMMON                                             \
    hide();                                                                    \
    showMinimized();                                                           \
    showNormal();                                                              \
    activateWindow();                                                          \
    setWindowState((windowState() & ~Qt::WindowMinimized) | Qt::WindowActive); \
    raise();

#ifdef Q_OS_WIN
#define ACTIVE_THIS_WINDOW     \
    _ACTIVE_THIS_WINDOW_COMMON \
    Windows_QWidget_SetForegroundWindow(this);
#else
#define ACTIVE_THIS_WINDOW _ACTIVE_THIS_WINDOW_COMMON
#endif

#define ADD_ASTERISK(parent)                                         \
    for (auto label: parent->findChildren<QLabel *>()) {             \
        auto text = label->text();                                   \
        if (!label->toolTip().isEmpty() && !text.endsWith("*")) {    \
            label->setText(text + "*");                              \
        }                                                            \
    }                                                                \
    for (auto checkBox: parent->findChildren<QCheckBox *>()) {       \
        auto text = checkBox->text();                                \
        if (!checkBox->toolTip().isEmpty() && !text.endsWith("*")) { \
            checkBox->setText(text + "*");                           \
        }                                                            \
    }
