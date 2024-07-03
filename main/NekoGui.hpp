#pragma once

#include "Const.hpp"
#include "NekoGui_Utils.hpp"
#include "NekoGui_ConfigItem.hpp"
#include "NekoGui_DataStore.hpp"

// Switch core support

namespace NekoGui {
    inline int coreType = CoreType::SING_BOX;

    QString FindCoreAsset(const QString &name);

    QString FindNekoBoxCoreRealPath();

    bool IsAdmin();
} // namespace NekoGui

#define ROUTES_PREFIX_NAME QString("routes_box")
#define ROUTES_PREFIX QString(ROUTES_PREFIX_NAME + "/")
