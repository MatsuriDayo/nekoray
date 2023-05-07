#pragma once

#include <QProcess>

namespace NekoRay::sys {
    class ExternalProcess : public QProcess {
    public:
        QString tag;
        QString program;
        QStringList arguments;
        QStringList env;

        bool managed = true; // MW_dialog_message

        ExternalProcess();
        ~ExternalProcess();

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
        int restart_id = -1;
    };

    // 手动管理
    inline QList<QSharedPointer<ExternalProcess>> running_ext;
} // namespace NekoRay::sys
