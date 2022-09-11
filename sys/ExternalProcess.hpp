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

        bool managed = true; // running_ext & stateChanged
        bool show_log = true;

        ExternalProcess();

        // start & kill is one time

        virtual void Start();

        void Kill();

    protected:
        bool started = false;
        bool killed = false;
        bool crashed = false;
    };

    class CoreProcess : public ExternalProcess {
    public:
        CoreProcess(const QString &core_path, const QStringList &args);

        void Start() override;

    private:
        bool show_stderr = false;
        bool failed_to_start = false;
    };

    // start & kill change this list
    inline QList<ExternalProcess *> running_ext;
}
