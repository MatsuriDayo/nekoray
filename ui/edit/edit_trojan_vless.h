#pragma once

#include <QWidget>
#include "profile_editor.h"


QT_BEGIN_NAMESPACE
namespace Ui { class EditTrojanVLESS; }
QT_END_NAMESPACE

class EditTrojanVLESS : public QWidget, public ProfileEditor {
Q_OBJECT

public:
    explicit EditTrojanVLESS(QWidget *parent = nullptr);

    ~EditTrojanVLESS() override;

    void onStart(QSharedPointer<NekoRay::ProxyEntity> _ent) override;

    bool onEnd() override;

private:
    Ui::EditTrojanVLESS *ui;
    QSharedPointer<NekoRay::ProxyEntity> ent;
};


