#include "AutoRun.hpp"

#include <QApplication>
#include <QDir>

#include "3rdparty/fix_old_qt.h"
#include "main/NekoGui.hpp"

// macOS headers (possibly OBJ-c)
#if defined(Q_OS_MACOS)
#include <CoreFoundation/CoreFoundation.h>
#include <CoreServices/CoreServices.h>
#endif

#ifdef Q_OS_WIN

#include <QSettings>

QString Windows_GenAutoRunString() {
    auto appPath = QApplication::applicationFilePath();
    appPath = "\"" + QDir::toNativeSeparators(appPath) + "\"";
    appPath += " -tray";
    return appPath;
}

void AutoRun_SetEnabled(bool enable) {
    // 以程序名称作为注册表中的键
    // 根据键获取对应的值（程序路径）
    auto appPath = QApplication::applicationFilePath();
    QFileInfo fInfo(appPath);
    QString name = fInfo.baseName();

    QSettings settings("HKEY_CURRENT_USER\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run", QSettings::NativeFormat);

    if (enable) {
        settings.setValue(name, Windows_GenAutoRunString());
    } else {
        settings.remove(name);
    }
}

bool AutoRun_IsEnabled() {
    QSettings settings("HKEY_CURRENT_USER\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run", QSettings::NativeFormat);

    // 以程序名称作为注册表中的键
    // 根据键获取对应的值（程序路径）
    auto appPath = QApplication::applicationFilePath();
    QFileInfo fInfo(appPath);
    QString name = fInfo.baseName();

    return settings.value(name).toString() == Windows_GenAutoRunString();
}

#endif

#ifdef Q_OS_MACOS

void AutoRun_SetEnabled(bool enable) {
    // From
    // https://github.com/nextcloud/desktop/blob/master/src/common/utility_mac.cpp
    QString filePath = QDir(QCoreApplication::applicationDirPath() + QLatin1String("/../..")).absolutePath();
    CFStringRef folderCFStr = CFStringCreateWithCString(0, filePath.toUtf8().data(), kCFStringEncodingUTF8);
    CFURLRef urlRef = CFURLCreateWithFileSystemPath(0, folderCFStr, kCFURLPOSIXPathStyle, true);
    LSSharedFileListRef loginItems = LSSharedFileListCreate(0, kLSSharedFileListSessionLoginItems, 0);

    if (loginItems && enable) {
        // Insert an item to the list.
        LSSharedFileListItemRef item =
            LSSharedFileListInsertItemURL(loginItems, kLSSharedFileListItemLast, 0, 0, urlRef, 0, 0);

        if (item) CFRelease(item);

        CFRelease(loginItems);
    } else if (loginItems && !enable) {
        // We need to iterate over the items and check which one is "ours".
        UInt32 seedValue;
        CFArrayRef itemsArray = LSSharedFileListCopySnapshot(loginItems, &seedValue);
        CFStringRef appUrlRefString = CFURLGetString(urlRef);

        for (int i = 0; i < CFArrayGetCount(itemsArray); i++) {
            LSSharedFileListItemRef item = (LSSharedFileListItemRef) CFArrayGetValueAtIndex(itemsArray, i);
            CFURLRef itemUrlRef = NULL;

            if (LSSharedFileListItemResolve(item, 0, &itemUrlRef, NULL) == noErr && itemUrlRef) {
                CFStringRef itemUrlString = CFURLGetString(itemUrlRef);

                if (CFStringCompare(itemUrlString, appUrlRefString, 0) == kCFCompareEqualTo) {
                    LSSharedFileListItemRemove(loginItems, item); // remove it!
                }

                CFRelease(itemUrlRef);
            }
        }

        CFRelease(itemsArray);
        CFRelease(loginItems);
    }

    CFRelease(folderCFStr);
    CFRelease(urlRef);
}

bool AutoRun_IsEnabled() {
    // From
    // https://github.com/nextcloud/desktop/blob/master/src/common/utility_mac.cpp
    // this is quite some duplicate code with setLaunchOnStartup, at some
    // point we should fix this FIXME.
    bool returnValue = false;
    QString filePath = QDir(QCoreApplication::applicationDirPath() + QLatin1String("/../..")).absolutePath();
    CFStringRef folderCFStr = CFStringCreateWithCString(0, filePath.toUtf8().data(), kCFStringEncodingUTF8);
    CFURLRef urlRef = CFURLCreateWithFileSystemPath(0, folderCFStr, kCFURLPOSIXPathStyle, true);
    LSSharedFileListRef loginItems = LSSharedFileListCreate(0, kLSSharedFileListSessionLoginItems, 0);

    if (loginItems) {
        // We need to iterate over the items and check which one is "ours".
        UInt32 seedValue;
        CFArrayRef itemsArray = LSSharedFileListCopySnapshot(loginItems, &seedValue);
        CFStringRef appUrlRefString = CFURLGetString(urlRef); // no need for release

        for (int i = 0; i < CFArrayGetCount(itemsArray); i++) {
            LSSharedFileListItemRef item = (LSSharedFileListItemRef) CFArrayGetValueAtIndex(itemsArray, i);
            CFURLRef itemUrlRef = NULL;

            if (LSSharedFileListItemResolve(item, 0, &itemUrlRef, NULL) == noErr && itemUrlRef) {
                CFStringRef itemUrlString = CFURLGetString(itemUrlRef);

                if (CFStringCompare(itemUrlString, appUrlRefString, 0) == kCFCompareEqualTo) {
                    returnValue = true;
                }

                CFRelease(itemUrlRef);
            }
        }

        CFRelease(itemsArray);
    }

    CFRelease(loginItems);
    CFRelease(folderCFStr);
    CFRelease(urlRef);
    return returnValue;
}

#endif

#ifdef Q_OS_LINUX

#include <QStandardPaths>
#include <QProcessEnvironment>
#include <QTextStream>

#define NEWLINE "\r\n"

//  launchatlogin.cpp
//  ShadowClash
//
//  Created by TheWanderingCoel on 2018/6/12.
//  Copyright © 2019 Coel Wu. All rights reserved.
//
QString getUserAutostartDir_private() {
    QString config = QStandardPaths::writableLocation(QStandardPaths::ConfigLocation);
    config += QLatin1String("/autostart/");
    return config;
}

void AutoRun_SetEnabled(bool enable) {
    // From https://github.com/nextcloud/desktop/blob/master/src/common/utility_unix.cpp
    QString appName = QCoreApplication::applicationName();
    QString userAutoStartPath = getUserAutostartDir_private();
    QString desktopFileLocation = userAutoStartPath + appName + QLatin1String(".desktop");
    QStringList appCmdList;

    // nekoray: launcher
    if (qEnvironmentVariable("NKR_FROM_LAUNCHER") == "1") {
        appCmdList << QApplication::applicationDirPath() + "/launcher"
                   << "--";
    } else {
        if (QProcessEnvironment::systemEnvironment().contains("APPIMAGE")) {
            appCmdList << QProcessEnvironment::systemEnvironment().value("APPIMAGE");
        } else {
            appCmdList << QApplication::applicationFilePath();
        }
    }

    appCmdList << "-tray";

    if (NekoGui::dataStore->flag_use_appdata) {
        appCmdList << "-appdata";
    }

    if (enable) {
        if (!QDir().exists(userAutoStartPath) && !QDir().mkpath(userAutoStartPath)) {
            // qCWarning(lcUtility) << "Could not create autostart folder"
            // << userAutoStartPath;
            return;
        }

        QFile iniFile(desktopFileLocation);

        if (!iniFile.open(QIODevice::WriteOnly)) {
            // qCWarning(lcUtility) << "Could not write auto start entry" <<
            // desktopFileLocation;
            return;
        }

        QTextStream ts(&iniFile);
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
        ts.setCodec("UTF-8");
#endif
        ts << QLatin1String("[Desktop Entry]") << NEWLINE
           << QLatin1String("Name=") << appName << NEWLINE
           << QLatin1String("Exec=") << appCmdList.join(" ") << NEWLINE
           << QLatin1String("Terminal=") << "false" << NEWLINE
           << QLatin1String("Categories=") << "Network" << NEWLINE
           << QLatin1String("Type=") << "Application" << NEWLINE
           << QLatin1String("StartupNotify=") << "false" << NEWLINE
           << QLatin1String("X-GNOME-Autostart-enabled=") << "true" << NEWLINE;
        ts.flush();
        iniFile.close();
    } else {
        QFile::remove(desktopFileLocation);
    }
}

bool AutoRun_IsEnabled() {
    QString appName = QCoreApplication::applicationName();
    QString desktopFileLocation = getUserAutostartDir_private() + appName + QLatin1String(".desktop");
    return QFile::exists(desktopFileLocation);
}

#endif
