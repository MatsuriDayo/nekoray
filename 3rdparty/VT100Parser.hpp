#pragma once

#include <QString>

inline QString cleanVT100String(const QString &in) {
    QString out;
    bool in_033 = false;
    for (auto &&chr: in) {
        if (chr == '\033') {
            in_033 = true;
            continue;
        }
        if (in_033) {
            if (chr == 'm') {
                in_033 = false;
            }
            continue;
        }
        out += chr;
    }
    return out;
}
