#pragma once

#include <QWidget>
#include <QGridLayout>
#include <QHBoxLayout>
#include "profile_editor.h"

QT_BEGIN_NAMESPACE
namespace Ui {
    class EditQUIC;
}
QT_END_NAMESPACE

class EditQUIC : public QWidget, public ProfileEditor {
    Q_OBJECT

public:
    explicit EditQUIC(QWidget *parent = nullptr);

    ~EditQUIC() override;

    void onStart(std::shared_ptr<NekoGui::ProxyEntity> _ent) override;

    bool onEnd() override;

    QList<QPair<QPushButton *, QString>> get_editor_cached() override;

private:
    Ui::EditQUIC *ui;
    std::shared_ptr<NekoGui::ProxyEntity> ent;

    struct {
        QString caText;
    } CACHE;

private slots:

    void on_certificate_clicked();
};
