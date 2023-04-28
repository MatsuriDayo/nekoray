#pragma once

#include <QWidget>
#include "profile_editor.h"

QT_BEGIN_NAMESPACE
namespace Ui {
    class EditHysteria;
}
QT_END_NAMESPACE

class EditHysteria : public QWidget, public ProfileEditor {
    Q_OBJECT

public:
    explicit EditHysteria(QWidget *parent = nullptr);

    ~EditHysteria() override;

    void onStart(QSharedPointer<NekoRay::ProxyEntity> _ent) override;

    bool onEnd() override;

    QList<QPair<QPushButton *, QString>> get_editor_cached() override;

private:
    Ui::EditHysteria *ui;
    QSharedPointer<NekoRay::ProxyEntity> ent;

    struct {
        QString caText;
    } CACHE;

private slots:

    void on_certificate_clicked();
};
