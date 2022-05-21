#include "CheckUpdate.hpp"
#include "ui/mainwindow.h"
#include "main/NekoRay.hpp"

#include <QInputDialog>
#include <QAbstractButton>
#include <QPushButton>
#include <QDesktopServices>

void CheckUpdate() {
    bool ok;
    libcore::UpdateReq request;
    request.set_action(libcore::UpdateAction::Check);
    auto response = NekoRay::rpc::defaultClient->Update(&ok, request);
    if (!ok) return;

    auto err = response.error();
    if (!err.empty()) {
        runOnUiThread([=] {
            MessageBoxWarning(QObject::tr("Update"), err.c_str());
        });
        return;
    }

    if (response.release_download_url() == nullptr) {
        runOnUiThread([=] {
            MessageBoxInfo(QObject::tr("Update"), QObject::tr("No update"));
        });
        return;
    }

    runOnUiThread([=] {
        QMessageBox box(QMessageBox::Question, QObject::tr("Update"),
                        QObject::tr("Update found: %1\nRelease note:\n%2")
                                .arg(response.assets_name().c_str(), response.release_note().c_str()));
        QAbstractButton *btn1 = box.addButton(QObject::tr("Update"), QMessageBox::AcceptRole);
        QAbstractButton *btn2 = box.addButton(QObject::tr("Open in browser"), QMessageBox::AcceptRole);
        box.addButton(QObject::tr("Close"), QMessageBox::RejectRole);
        box.exec();

        if (btn1 == box.clickedButton()) {
            // Download Update
            runOnNewThread([=] {
                bool ok2;
                libcore::UpdateReq request2;
                request2.set_action(libcore::UpdateAction::Download);
                auto response2 = NekoRay::rpc::defaultClient->Update(&ok2, request2);
                runOnUiThread([=] {
                    if (response2.error().empty()) {
                        auto q = QMessageBox::question(nullptr, QObject::tr("Update"),
                                                       QObject::tr("Update is ready, restart to install?"));
                        if (q == QMessageBox::StandardButton::Yes) {
                            GetMainWindow()->exit_update = true;
                            GetMainWindow()->on_menu_exit_triggered();
                        }
                    } else {
                        MessageBoxWarning(QObject::tr("Update"), response2.error().c_str());
                    }
                });
            });
        } else if (btn2 == box.clickedButton()) {
            QDesktopServices::openUrl(QUrl(response.release_url().c_str()));
        }
    });
}
