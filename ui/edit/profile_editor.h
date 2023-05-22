#pragma once

#include <QPushButton>

#include "db/ProxyEntity.hpp"
#include "main/GuiUtils.hpp"

class ProfileEditor {
public:
    virtual void onStart(std::shared_ptr<NekoGui::ProxyEntity> ent) = 0;

    virtual bool onEnd() = 0;

    std::function<QWidget *()> get_edit_dialog;
    std::function<QString()> get_edit_text_name;
    std::function<QString()> get_edit_text_serverAddress;
    std::function<QString()> get_edit_text_serverPort;

    // cached editor

    std::function<void()> editor_cache_updated;

    virtual QList<QPair<QPushButton *, QString>> get_editor_cached() { return {}; };
};
