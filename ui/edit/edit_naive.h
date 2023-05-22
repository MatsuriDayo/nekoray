#pragma once

#include <QWidget>
#include "profile_editor.h"

QT_BEGIN_NAMESPACE
namespace Ui {
    class EditNaive;
}
QT_END_NAMESPACE

class EditNaive : public QWidget, public ProfileEditor {
    Q_OBJECT

public:
    explicit EditNaive(QWidget *parent = nullptr);

    ~EditNaive() override;

    void onStart(std::shared_ptr<NekoGui::ProxyEntity> _ent) override;

    bool onEnd() override;

    QList<QPair<QPushButton *, QString>> get_editor_cached() override;

private:
    Ui::EditNaive *ui;
    std::shared_ptr<NekoGui::ProxyEntity> ent;

    struct {
        QString certificate;
        QString extra_headers;
    } CACHE;

private slots:

    void on_certificate_clicked();

    void on_extra_headers_clicked();
};
