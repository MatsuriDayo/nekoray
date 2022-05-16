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


        void Start();

        void Kill();

    private:
        bool killed = false;
    };

    inline QList<ExternalProcess *> running_ext;
}
