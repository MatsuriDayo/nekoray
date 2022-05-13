#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTableWidgetItem>
#include <QKeyEvent>
#include <QSystemTrayIcon>
#include <QDesktopServices>
#include <QProcess>
#include <QTextDocument>

#include "db/ProxyEntity.hpp"
#include "db/Group.hpp"
#include "rpc/gRPC.h"
#include "qv2ray/ui/LogHighlighter.hpp"

QT_BEGIN_NAMESPACE
namespace Ui {
    class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow {
Q_OBJECT

public:
#ifndef NO_GRPC
    NekoRay::rpc::Client *defaultClient;
#endif

    MainWindow(QWidget *parent = nullptr);

    ~MainWindow();

    // 可以被外面调用的
    void dialog_message(const QString &dialog, const QString &info);

    void refresh_proxy_list(const int &id = -1, NekoRay::GroupSortAction groupSortAction = {});

    void refresh_groups();

    void refresh_status(const QString &traffic_update = "");

    void neko_start(int id = -1);

    void neko_stop();

    void neko_set_system_proxy(bool enable);

    void writeLog_ui(const QString &log);

private slots:

    void on_masterLogBrowser_customContextMenuRequested(const QPoint &pos);

    void on_menu_basic_settings_triggered();

    void on_menu_routing_settings_triggered();

    void on_menu_exit_triggered();

    void on_menu_add_from_input_triggered();

    void on_menu_add_from_clipboard_triggered();

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
    //
    bool core_process_killed = false;
    QProcess *core_process;
    //
    bool qvLogAutoScoll = true;
    QTextDocument *qvLogDocument = new QTextDocument(this);
    SyntaxHighlighter *qvLogHighlighter;
    //
    QString title_base;
    QString title_status;
    QSharedPointer<NekoRay::ProxyEntity> running;
    QString traffic_update_cache;
    //
    int proxy_last_order = -1;

    QMap<int, QSharedPointer<NekoRay::ProxyEntity>> GetNowSelected();

    void keyPressEvent(QKeyEvent *event) override;

    void closeEvent(QCloseEvent *event) override;

    void hideEvent(QHideEvent *event) override;

#ifndef NO_GRPC

    void speedtest_current_group(libcore::TestMode mode);

#endif
};

#endif // MAINWINDOW_H
