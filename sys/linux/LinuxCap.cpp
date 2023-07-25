#include "LinuxCap.h"

#include <QProcess>

#define EXIT_CODE(p) (p.exitStatus() == QProcess::NormalExit ? p.exitCode() : -1)

QString Linux_GetCapString(const QString &path) {
    QProcess p;
    p.setProgram("getcap");
    p.setArguments({path});
    p.start();
    p.waitForFinished(500);
    return p.readAllStandardOutput();
}

int Linux_Pkexec_SetCapString(const QString &path, const QString &cap) {
    QProcess p;
    p.setProgram("pkexec");
    p.setArguments({"setcap", cap, path});
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
