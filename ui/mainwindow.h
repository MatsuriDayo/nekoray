#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTableWidgetItem>
#include <QKeyEvent>
#include <QSystemTrayIcon>
#include <QProcess>
#include <QTextDocument>
#include <QShortcut>

#include "db/ProxyEntity.hpp"
#include "db/Group.hpp"
#include "rpc/gRPC.h"
#include "qv2ray/ui/LogHighlighter.hpp"
#include "GroupSort.hpp"

QT_BEGIN_NAMESPACE
namespace Ui {
    class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow {
Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);

    ~MainWindow();

    void dialog_message_impl(const QString &sender, const QString &info);

    void refresh_proxy_list_impl(const int &id = -1, NekoRay::GroupSortAction groupSortAction = {});

    void refresh_proxy_list(const int &id = -1) { refresh_proxy_list_impl(id, {}); };

    void refresh_groups();

    void refresh_status(const QString &traffic_update = "");

    void neko_start(int _id = -1);

    void neko_stop(bool crash = false);

    void neko_set_system_proxy(bool enable);

    void show_log_impl(const QString &log);

    void start_select_mode(QObject *context, const std::function<void(int)> &callback);

    void refresh_connection_list(const QJsonArray &arr);

    bool exit_update = false;

signals:

    void profile_selected(int id);

public slots:

    void on_menu_exit_triggered();

private slots:

    void on_masterLogBrowser_customContextMenuRequested(const QPoint &pos);

    void on_menu_basic_settings_triggered();

    void on_menu_routing_settings_triggered();

    void on_menu_add_from_input_triggered();

    void on_menu_add_from_clipboard_triggered();

    void on_menu_move_triggered();

    void on_menu_delete_triggered();

    void on_menu_reset_traffic_triggered();

    void on_menu_profile_debug_info_triggered();

    void on_menu_export_config_triggered();

    void on_menu_copy_link_triggered();

    void on_menu_qr_triggered();

    void on_menu_scan_qr_triggered();

    void on_menu_tcp_ping_triggered();

    void on_menu_url_test_triggered();

    void on_menu_clear_test_result_triggered();

    void on_menu_manage_groups_triggered();

    void on_menu_delete_repeat_triggered();

    void on_proxyListTable_itemDoubleClicked(QTableWidgetItem *item);

    void on_proxyListTable_customContextMenuRequested(const QPoint &pos);

    void on_tabWidget_currentChanged(int index);

private:
    Ui::MainWindow *ui;
    QSystemTrayIcon *tray;
    QShortcut *shortcut_ctrl_f = new QShortcut(QKeySequence("Ctrl+F"), this);
    QShortcut *shortcut_esc = new QShortcut(QKeySequence("Esc"), this);
    //
    bool core_process_killed = false;
    bool core_process_show_stderr = false;
    QProcess *core_process;
    //
    bool qvLogAutoScoll = true;
    QTextDocument *qvLogDocument = new QTextDocument(this);
    SyntaxHighlighter *qvLogHighlighter;
    //
    QString title_base;
    QString title_error;
    QString title_system_proxy;
    QSharedPointer<NekoRay::ProxyEntity> running;
    QString traffic_update_cache;
    QTime last_test_time;
    //
    int proxy_last_order = -1;
    bool select_mode = false;

    QMap<int, QSharedPointer<NekoRay::ProxyEntity>> GetNowSelected();

    void keyPressEvent(QKeyEvent *event) override;

    void closeEvent(QCloseEvent *event) override;

#ifndef NKR_NO_GRPC

    void speedtest_current_group(libcore::TestMode mode);

#endif

    void show_group(int gid);

protected:
    bool eventFilter(QObject *obj, QEvent *event) override;
};

inline MainWindow *GetMainWindow() {
    return (MainWindow *) mainwindow;
}

#endif // MAINWINDOW_H
