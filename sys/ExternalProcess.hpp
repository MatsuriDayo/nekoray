#pragma once

#include "main/NekoRay.hpp"

#include <QProcess>

namespace NekoRay::sys {
    class ExternalProcess : public QProcess {
    public:
        QString tag;
        QString program;
        QStringList arguments;
        QStringList env;

        ExternalProcess(const QString &tag,
                        const QString &program,
                        const QStringList &arguments,
                        const QStringList &env);

        // start & kill is one time

        void Start();

        void Kill();

    private:
        bool started = false;
        bool killed = false;
        bool crashed = false;
    };

    // start & kill change this list
    inline QList<ExternalProcess *> running_ext;
}
