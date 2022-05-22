#pragma once

#include <QMenu>
#include <QWidget>
#include <QRegExpValidator>

// Dialogs

#define Dialog_DialogBasicSettings "DialogBasicSettings"
#define Dialog_DialogEditProfile "DialogEditProfile"
#define Dialog_DialogManageGroups "DialogManageGroups"
#define Dialog_DialogManageRoutes "DialogManageRoutes"

// Utils

inline QList<QAction *>
CreateActions(QWidget *parent, const QList<QString> &texts, const std::function<void(QAction *)> &slot) {
    QList<QAction *> acts;

    for (const auto &text: texts) {
        acts.push_back(new QAction(text, parent)); //按顺序来
    }

    for (int i = 0; i < acts.size(); i++) {
        if (acts[i]->text() == "[Separator]") {
            acts[i]->setSeparator(true);
            acts[i]->setText("");
            acts[i]->setDisabled(true);
            acts[i]->setData(-1);
        } else {
            acts[i]->setData(i);
            QObject::connect(acts[i], &QAction::triggered, parent, [=] {
                slot(acts[i]);
            });
        }
    }

    return acts;
}

inline QMenu *CreateMenu(QWidget *parent, const QList<QString> &texts, const std::function<void(QAction *)> &slot) {
    auto menu = new QMenu(parent);
    menu->addActions(CreateActions(parent, texts, slot));
    return menu;
}

#define QRegExpValidator_Number new QRegExpValidator(QRegExp("^[0-9]+$")

// NekoRay Save&Load

#define P_LOAD_STRING(a) ui->a->setText(bean->a);
#define P_SAVE_STRING(a) bean->a = ui->a->text();
#define P_SAVE_STRING_QTEXTEDIT(a) bean->a = ui->a->toPlainText();
#define P_C_LOAD_STRING(a) CACHE.a = bean->a;
#define P_C_SAVE_STRING(a) bean->a = CACHE.a;
#define D_C_LOAD_STRING(a) CACHE.a = NekoRay::dataStore->a;
#define D_C_SAVE_STRING(a) NekoRay::dataStore->a = CACHE.a;
#define P_LOAD_INT(a) ui->a->setText(Int2String(bean->a)); ui->a->setValidator(QRegExpValidator_Number, this));
#define P_SAVE_INT(a) bean->a = ui->a->text().toInt();
#define D_LOAD_INT(a) ui->a->setText(Int2String(NekoRay::dataStore->a)); ui->a->setValidator(QRegExpValidator_Number, this));
#define D_SAVE_INT(a) NekoRay::dataStore->a = ui->a->text().toInt();
#define P_LOAD_COMBO(a) ui->a->setCurrentText(bean->a);
#define P_SAVE_COMBO(a) bean->a = ui->a->currentText();

#define D_LOAD_INT_ENABLE(i, e) \
if (NekoRay::dataStore->i > 0) { \
ui->e->setChecked(true); \
ui->i->setText(Int2String(NekoRay::dataStore->i)); \
} else { \
ui->e->setChecked(false); \
ui->i->setText(Int2String(-NekoRay::dataStore->i)); \
} \
ui->i->setValidator(QRegExpValidator_Number, this));
#define D_SAVE_INT_ENABLE(i, e) \
if (ui->e->isChecked()) { \
NekoRay::dataStore->i = ui->i->text().toInt(); \
} else { \
NekoRay::dataStore->i = -ui->i->text().toInt(); \
}

#define C_EDIT_JSON_ALLOW_EMPTY(a) auto editor = new JsonEditor(QString2QJsonObject(CACHE.a), this); \
auto result = editor->OpenEditor(); \
CACHE.a = QJsonObject2QString(result, true); \
if (result.isEmpty()) CACHE.a = ""; \
editor->deleteLater();
