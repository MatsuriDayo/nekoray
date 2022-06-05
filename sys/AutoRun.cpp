#include "AutoRun.hpp"

#include <QApplication>

#ifdef Q_OS_WIN

#include <QDir>
#include <QSettings>

//设置程序自启动 appPath程序路径
void SetProcessAutoRunSelf(bool enable) {
    auto appPath = QApplication::applicationFilePath();

    QSettings settings("HKEY_CURRENT_USER\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run",
                       QSettings::NativeFormat);

    //以程序名称作为注册表中的键
    //根据键获取对应的值（程序路径）
    QFileInfo fInfo(appPath);
    QString name = fInfo.baseName();
    QString path = settings.value(name).toString();

    //如果注册表中的路径和当前程序路径不一样，
    //则表示没有设置自启动或自启动程序已经更换了路径
    //toNativeSeparators的意思是将"/"替换为"\"
    QString newPath = QDir::toNativeSeparators(appPath);

    if (enable) {
        if (path != newPath) {
            settings.setValue(name, newPath);
        }
    } else {
        settings.remove(name);
    }
}

bool GetProcessAutoRunSelf() {
    auto appPath = QApplication::applicationFilePath();

    QSettings settings("HKEY_CURRENT_USER\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run",
                       QSettings::NativeFormat);

    //以程序名称作为注册表中的键
    //根据键获取对应的值（程序路径）
    QFileInfo fInfo(appPath);
    QString name = fInfo.baseName();
    QString path = settings.value(name).toString();

    //如果注册表中的路径和当前程序路径不一样，
    //则表示没有设置自启动或自启动程序已经更换了路径
    //toNativeSeparators的意思是将"/"替换为"\"
    QString newPath = QDir::toNativeSeparators(appPath);
    return path == newPath;
}


#else

#include <QMessageBox>

void SetProcessAutoRunSelf(bool enable) {
    QMessageBox::warning(nullptr, "Error", "Autorun is not yet implemented on your platform.");
}

bool GetProcessAutoRunSelf() {
    return false;
}

#endif
