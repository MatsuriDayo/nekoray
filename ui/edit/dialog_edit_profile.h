#ifndef DIALOG_EDIT_PROFILE_H
#define DIALOG_EDIT_PROFILE_H

#include <QDialog>
#include "db/Database.hpp"
#include "profile_editor.h"

#include "ui/widget/FloatCheckBox.h"

namespace Ui {
    class DialogEditProfile;
}

class DialogEditProfile : public QDialog {
    Q_OBJECT

public:
    explicit DialogEditProfile(const QString &_type, int profileOrGroupId, QWidget *parent = nullptr);

    ~DialogEditProfile() override;

public slots:

    void accept() override;

private slots:

    void on_custom_outbound_edit_clicked();

    void on_custom_config_edit_clicked();

    void on_certificate_edit_clicked();

    void on_apply_to_group_clicked();

private:
    Ui::DialogEditProfile *ui;

    std::map<QWidget *, FloatCheckBox *> apply_to_group_ui;

    QWidget *innerWidget{};
    ProfileEditor *innerEditor{};

    QString type;
    int groupId;
    bool newEnt = false;
    std::shared_ptr<NekoGui::ProxyEntity> ent;

    QString network_title_base;

    struct {
        QString custom_outbound;
        QString custom_config;
        QString certificate;
    } CACHE;

    void typeSelected(const QString &newType);

    bool onEnd();

    void editor_cache_updated_impl();

    void do_apply_to_group(const std::shared_ptr<NekoGui::Group> &group, QWidget *key);
};

#endif // DIALOG_EDIT_PROFILE_H
