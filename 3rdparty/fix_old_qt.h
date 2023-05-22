#pragma once

#include <QString>

#if QT_VERSION < QT_VERSION_CHECK(5, 10, 0)

inline QString qEnvironmentVariable(const char *varName) {
    return qgetenv(varName);
}

#endif
