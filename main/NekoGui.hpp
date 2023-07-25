#pragma once

#include "Const.hpp"
#include "NekoGui_Utils.hpp"
#include "NekoGui_ConfigItem.hpp"
#include "NekoGui_DataStore.hpp"

// Switch core support

namespace NekoGui {
    inline int coreType = CoreType::V2RAY;

    QString FindCoreAsset(const QString &name);

    QString FindNekoBoxCoreRealPath();

    bool IsAdmin();
} // namespace NekoGui

#define IS_NEKO_BOX (NekoGui::coreType == NekoGui::CoreType::SING_BOX)
#define IS_NEKO_BOX_INTERNAL_TUN (IS_NEKO_BOX && NekoGui::dataStore->vpn_internal_tun)
#define ROUTES_PREFIX_NAME QString(IS_NEKO_BOX ? "routes_box" : "routes")
#define ROUTES_PREFIX QString(ROUTES_PREFIX_NAME + "/")
