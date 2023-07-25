#pragma once

#include <QString>

QString Linux_GetCapString(const QString &path);

int Linux_Pkexec_SetCapString(const QString &path, const QString &cap);

bool Linux_HavePkexec();
