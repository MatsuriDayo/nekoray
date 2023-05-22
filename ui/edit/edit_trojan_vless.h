#pragma once

#include <QWidget>
#include "profile_editor.h"

QT_BEGIN_NAMESPACE
namespace Ui {
    class EditTrojanVLESS;
}
QT_END_NAMESPACE

class EditTrojanVLESS : public QWidget, public ProfileEditor {
    Q_OBJECT

public:
    explicit EditTrojanVLESS(QWidget *parent = nullptr);

    ~EditTrojanVLESS() override;

    void onStart(std::shared_ptr<NekoGui::ProxyEntity> _ent) override;

    bool onEnd() override;

private:
    Ui::EditTrojanVLESS *ui;
    std::shared_ptr<NekoGui::ProxyEntity> ent;
};
