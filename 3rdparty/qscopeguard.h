/****************************************************************************
**
** Copyright (C) 2018 Klarälvdalens Datakonsult AB, a KDAB Group company, info@kdab.com, author Sérgio Martins <sergio.martins@kdab.com>
** Contact: https://www.qt.io/licensing/
**
** This file is part of the QtCore module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 3 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL3 included in the
** packaging of this file. Please review the following information to
** ensure the GNU Lesser General Public License version 3 requirements
** will be met: https://www.gnu.org/licenses/lgpl-3.0.html.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 2.0 or (at your option) the GNU General
** Public license version 3 or any later version approved by the KDE Free
** Qt Foundation. The licenses are as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL2 and LICENSE.GPL3
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-2.0.html and
** https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#ifndef QSCOPEGUARD_H
#define QSCOPEGUARD_H

#include <QtCore/qglobal.h>


QT_BEGIN_NAMESPACE


template <typename F> class QScopeGuard;
template <typename F> QScopeGuard<F> qScopeGuard(F f);

template <typename F>
class QScopeGuard
{
public:
    QScopeGuard(QScopeGuard &&other) Q_DECL_NOEXCEPT
        : m_func(std::move(other.m_func))
        , m_invoke(other.m_invoke)
    {
        other.dismiss();
    }

    ~QScopeGuard()
    {
        if (m_invoke)
            m_func();
    }

    void dismiss() Q_DECL_NOEXCEPT
    {
        m_invoke = false;
    }

private:
    explicit QScopeGuard(F f) Q_DECL_NOEXCEPT
        : m_func(std::move(f))
    {
    }

    Q_DISABLE_COPY(QScopeGuard)

    F m_func;
    bool m_invoke = true;
    friend QScopeGuard qScopeGuard<F>(F);
};


template <typename F>
QScopeGuard<F> qScopeGuard(F f)
{
    return QScopeGuard<F>(std::move(f));
}

QT_END_NAMESPACE

#endif // QSCOPEGUARD_H
