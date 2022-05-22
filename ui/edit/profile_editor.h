#pragma once

#include <QPushButton>

#include "db/ProxyEntity.hpp"
#include "main/GuiUtils.hpp"

class ProfileEditor {
public:
    virtual void onStart(QSharedPointer<NekoRay::ProxyEntity> ent) = 0;

    virtual bool onEnd() = 0;

    std::function<QWidget *()> get_edit_dialog;

    // cached editor

    std::function<void()> dialog_editor_cache_updated;

    virtual QList<QPair<QPushButton *, QString>> get_editor_cached() { return {}; };
};
