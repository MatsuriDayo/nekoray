#pragma once

#include <QWidget>
#include "profile_editor.h"

namespace Ui {
    class EditSocksHttp;
}

class EditSocksHttp : public QWidget, public ProfileEditor {
Q_OBJECT

public:
    explicit EditSocksHttp(QWidget *parent = nullptr);

    ~EditSocksHttp() override;

    void onStart(QSharedPointer<NekoRay::ProxyEntity> _ent) override;

    bool onEnd() override;

private:
    Ui::EditSocksHttp *ui;
    QSharedPointer<NekoRay::ProxyEntity> ent;
};
