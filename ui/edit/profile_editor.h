#pragma once

#include <QPushButton>

#include "db/ProxyEntity.hpp"
#include "main/GuiUtils.hpp"

class ProfileEditor {
public:
    virtual void onStart(QSharedPointer<NekoRay::ProxyEntity> ent) = 0;

    virtual bool onEnd() = 0;


    // cached editor

    std::function<void()> dialog_editor_cache_updated;

    virtual QList<QPair<QPushButton *, QString>> get_editor_cached() { return {}; };
};

#define P_E_LOAD_STRING(a) ui->a->setText(bean->a);
#define P_E_SAVE_STRING(a) bean->a = ui->a->text();
#define P_E_SAVE_STRING_QTEXTEDIT(a) bean->a = ui->a->toPlainText();
#define P_E_LOAD_STRING_CACHE(a) CACHE.a = bean->a;
#define P_E_SAVE_STRING_CACHE(a) bean->a = CACHE.a;
#define P_E_LOAD_INT(a) ui->a->setText(Int2String(bean->a)); ui->a->setValidator(QRegExpValidator_Number, this));
#define P_E_SAVE_INT(a) bean->a = ui->a->text().toInt();
#define P_E_LOAD_COMBO(a) ui->a->setCurrentText(bean->a);
#define P_E_SAVE_COMBO(a) bean->a = ui->a->currentText();

#define P_E_LOAD_INT_ENABLE(i, e) \
if (NekoRay::dataStore->i > 0) { \
ui->e->setChecked(true); \
ui->i->setText(Int2String(NekoRay::dataStore->i)); \
} else { \
ui->e->setChecked(false); \
ui->i->setText(Int2String(-NekoRay::dataStore->i)); \
} \
ui->i->setValidator(QRegExpValidator_Number, this));
#define P_E_SAVE_INT_ENABLE(i, e) \
if (ui->e->isChecked()) { \
NekoRay::dataStore->i = ui->i->text().toInt(); \
} else { \
NekoRay::dataStore->i = -ui->i->text().toInt(); \
}
