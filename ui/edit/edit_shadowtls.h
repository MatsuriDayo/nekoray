#pragma once
// #ifndef EDIT_SHADOWTLS_H
// #define EDIT_SHADOWTLS_H

#include <QWidget>
#include "profile_editor.h"

QT_BEGIN_NAMESPACE
namespace Ui {
    class EditShadowTLS;
}
QT_END_NAMESPACE

class EditShadowTLS : public QWidget, public ProfileEditor {
    Q_OBJECT

public:
    explicit EditShadowTLS(QWidget *parent = nullptr);

    ~EditShadowTLS() override;

    void onStart(QSharedPointer<NekoRay::ProxyEntity> _ent) override;

    bool onEnd() override;

private:
    Ui::EditShadowTLS *ui;
    QSharedPointer<NekoRay::ProxyEntity> ent;
};

//#endif // EDIT_SHADOWTLS_H
