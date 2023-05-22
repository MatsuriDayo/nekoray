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

    void onStart(std::shared_ptr<NekoGui::ProxyEntity> _ent) override;

    bool onEnd() override;

private:
    Ui::EditSocksHttp *ui;
    std::shared_ptr<NekoGui::ProxyEntity> ent;
};
