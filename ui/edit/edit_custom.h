#pragma once

#include <QWidget>
#include "profile_editor.h"

QT_BEGIN_NAMESPACE
namespace Ui {
    class EditCustom;
}
QT_END_NAMESPACE

class EditCustom : public QWidget, public ProfileEditor {
    Q_OBJECT

public:
    QString preset_core;
    QString preset_command;
    QString preset_config;

    explicit EditCustom(QWidget *parent = nullptr);

    ~EditCustom() override;

    void onStart(std::shared_ptr<NekoGui::ProxyEntity> _ent) override;

    bool onEnd() override;

private:
    Ui::EditCustom *ui;
    std::shared_ptr<NekoGui::ProxyEntity> ent;

private slots:

    void on_as_json_clicked();
};
