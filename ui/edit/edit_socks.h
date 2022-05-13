#ifndef EDIT_SOCKS_H
#define EDIT_SOCKS_H

#include <QWidget>
#include "profile_editor.h"

namespace Ui {
    class EditSocks;
}

class EditSocks : public QWidget, public ProfileEditor {
Q_OBJECT

public:
    explicit EditSocks(QWidget *parent = nullptr);

    ~EditSocks() override;

    void onStart(QSharedPointer<NekoRay::ProxyEntity> _ent) override;

    bool onEnd() override;

private:
    Ui::EditSocks *ui;
    QSharedPointer<NekoRay::ProxyEntity> ent;
};

#endif // EDIT_SOCKS_H
