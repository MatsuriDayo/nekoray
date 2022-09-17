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

#ifndef WINCOMMANDER_H
#define WINCOMMANDER_H

#include <QString>
#include <QStringList>

class WinCommander {
public:
    static uint runProcessElevated(const QString &path,
                                   const QStringList &parameters = QStringList(),
                                   const QString &workingDir = QString(),
                                   bool hide = false, bool aWait = true);

    static uint runProcessElevated(const QString &path,
                                   const QString &parameters = QString(),
                                   const QString &workingDir = QString(),
                                   bool hide = false, bool aWait = true);
};

#endif // WINCOMMANDER_H