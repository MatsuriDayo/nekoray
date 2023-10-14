#include "LinuxCap.h"

#include <QDebug>
#include <QProcess>
#include <QStandardPaths>

#define EXIT_CODE(p) (p.exitStatus() == QProcess::NormalExit ? p.exitCode() : -1)

QString Linux_GetCapString(const QString &path) {
    QProcess p;
    p.setProgram(Linux_FindCapProgsExec("getcap"));
    p.setArguments({path});
    p.start();
    p.waitForFinished(500);
    return p.readAllStandardOutput();
}

int Linux_Pkexec_SetCapString(const QString &path, const QString &cap) {
    QProcess p;
    p.setProgram("pkexec");
    p.setArguments({Linux_FindCapProgsExec("setcap"), cap, path});
    p.start();
    p.waitForFinished(-1);
    return EXIT_CODE(p);
}

bool Linux_HavePkexec() {
    QProcess p;
    p.setProgram("pkexec");
    p.setArguments({"--help"});
    p.setProcessChannelMode(QProcess::SeparateChannels);
    p.start();
    p.waitForFinished(500);
    return EXIT_CODE(p) == 0;
}

QString Linux_FindCapProgsExec(const QString &name) {
    QString exec = QStandardPaths::findExecutable(name);
    if (exec.isEmpty())
        exec = QStandardPaths::findExecutable(name, {"/usr/sbin", "/sbin"});

    if (exec.isEmpty())
        qDebug() << "Executable" << name << "could not be resolved";
    else
        qDebug() << "Found exec" << name << "at" << exec;

    return exec.isEmpty() ? name : exec;
}
