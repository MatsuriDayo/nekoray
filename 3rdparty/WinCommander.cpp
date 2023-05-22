/****************************************************************************
**
** Copyright (C) 2014 UpdateNode UG (haftungsbeschränkt)
** Contact: code@updatenode.com
**
** This file is part of the UpdateNode Client.
**
** Commercial License Usage
** Licensees holding valid commercial UpdateNode license may use this file
** under the terms of the the Apache License, Version 2.0
** Full license description file: LICENSE.COM
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3.0 as published by the Free Software
** Foundation. Please review the following information to ensure the
** GNU General Public License version 3.0 requirements will be met:
** http://www.gnu.org/copyleft/gpl.html.
** Full license description file: LICENSE.GPL
**
****************************************************************************/

#include "WinCommander.hpp"

#include <QSysInfo>
#include <QDir>

#ifdef Q_OS_WIN
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#include <shellapi.h>
#include <sddl.h>
#define MAX_KEY_LENGTH 255
#define MAX_VALUE_NAME 16383
#endif


/*!
Executes a command elevated specified by \apath , using paramters \aparameters.
\n
Parameter /aaWait decides if the function should return immediatelly after it's\n
execution or wait for the exit of the launched process
\n
Returns the return value of the executed command
*/
uint WinCommander::runProcessElevated(const QString &path,
                                      const QStringList &parameters,
                                      const QString &workingDir,
                                      int nShow, bool aWait) {
    uint result = 0;

#ifdef Q_OS_WIN
    QString params;
    HWND hwnd = NULL;
    LPCTSTR pszPath = (LPCTSTR)path.utf16();
    foreach(QString item, parameters)
        params += "\"" + item + "\" ";

    LPCTSTR pszParameters = (LPCTSTR)params.utf16();
    QString dir;
    if (workingDir.count() == 0)
        dir = QDir::toNativeSeparators(QDir::currentPath());
    else
        dir = QDir::toNativeSeparators(workingDir);
    LPCTSTR pszDirectory = (LPCTSTR)dir.utf16();

    SHELLEXECUTEINFO shex;
    DWORD dwCode  =   0;

    ZeroMemory(&shex, sizeof(shex));

    shex.cbSize       = sizeof(shex);
    shex.fMask        = SEE_MASK_NOCLOSEPROCESS;
    shex.hwnd         = hwnd;
    shex.lpVerb       = TEXT("runas");
    shex.lpFile       = pszPath;
    shex.lpParameters = pszParameters;
    shex.lpDirectory  = pszDirectory;
    // https://docs.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-showwindow
    shex.nShow        = nShow;

    ShellExecuteEx(&shex);
    if (shex.hProcess)
    {
        if(aWait)
        {
            WaitForSingleObject(shex.hProcess, INFINITE );
            GetExitCodeProcess(shex.hProcess, &dwCode);
        }
        CloseHandle (shex.hProcess) ;
    }
    else
        return -1;

    result = (uint)dwCode;
#else
    Q_UNUSED(path);
    Q_UNUSED(parameters);
    Q_UNUSED(workingDir);
    Q_UNUSED(aWait);
#endif
    return result;
}
