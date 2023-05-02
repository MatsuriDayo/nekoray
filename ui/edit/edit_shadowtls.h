#ifndef EDIT_SHADOWTLS_H
#define EDIT_SHADOWTLS_H

#include <QWidget>
#include "profile_editor.h"

namespace Ui {
    class EditShadowTLS;
}

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

#endif // EDIT_SHADOWTLS_H
