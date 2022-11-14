#pragma once

class ThemeManager {
public:
    QString system_style_name = "";
    QString current_theme = "0"; // int: 0:system 1+:builtin string: QStyleFactory

    void ApplyTheme(const QString &theme);
};

extern ThemeManager *themeManager;
