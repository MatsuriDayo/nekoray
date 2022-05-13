#pragma once

#include <QWidget>
#include "profile_editor.h"


QT_BEGIN_NAMESPACE
namespace Ui { class EditTrojan; }
QT_END_NAMESPACE

class EditTrojan : public QWidget, public ProfileEditor {
Q_OBJECT

public:
    explicit EditTrojan(QWidget *parent = nullptr);

    ~EditTrojan() override;

    void onStart(QSharedPointer<NekoRay::ProxyEntity> _ent) override;

    bool onEnd() override;

private:
    Ui::EditTrojan *ui;
    QSharedPointer<NekoRay::ProxyEntity> ent;
};


