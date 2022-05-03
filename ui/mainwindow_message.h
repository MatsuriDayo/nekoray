#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "db/Group.hpp"

class MainWindow : public QMainWindow {
Q_OBJECT

public:
    // 可以被外面调用的
    void dialog_message(const QString &dialog, const QString &info);

    void refresh_proxy_list(const int &id = -1, NekoRay::GroupSortAction groupSortAction = {});

    void refresh_groups();

    void refresh_status(const QString &traffic_update = "");

    void neko_start();

    void neko_stop();

    void writeLog_ui(const QString &log);
};

inline MainWindow *GetMainWindow() {
    return (MainWindow *) mainwindow;
}

#endif
