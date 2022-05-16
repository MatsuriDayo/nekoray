#include "ExternalProcess.hpp"

namespace NekoRay::sys {
    ExternalProcess::ExternalProcess(const QString &tag,
                                     const QString &program,
                                     const QStringList &arguments,
                                     const QStringList &env)
            : QProcess() {
        this->tag = tag;
        this->program = program;
        this->arguments = arguments;
        this->env = env;
    }

    void ExternalProcess::Start() {
        connect(this, &QProcess::readyReadStandardOutput, this,
                [=]() {
                    // TODO read output sometimes crash
                    showLog_ext(tag, readAllStandardOutput().trimmed());
                });
        connect(this, &QProcess::readyReadStandardError, this,
                [=]() {
                    // TODO read output sometimes crash
                    showLog_ext(tag, readAllStandardError().trimmed());
                });
        connect(this, &QProcess::errorOccurred, this,
                [=](QProcess::ProcessError error) {
                    if (!killed) showLog_ext(tag, "[ERROR] " + QProcess::errorString());
                });
        connect(this, &QProcess::stateChanged, this,
                [=](QProcess::ProcessState state) {
                    if (state == QProcess::NotRunning) {
                        showLog_ext(tag, "Stopped");
                    }
                });

        showLog_ext(tag, "[Starting] " + env.join(" ") + " " + program + " " + arguments.join(" "));

        QProcess::setEnvironment(env);
        QProcess::start(program, arguments);
    }

    void ExternalProcess::Kill() {
        killed = true;
        QProcess::kill();
        QProcess::waitForFinished(500);
    }

}
