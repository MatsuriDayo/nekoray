#ifndef DIALOG_EDIT_PROFILE_H
#define DIALOG_EDIT_PROFILE_H

#include <QDialog>
#include "db/Database.hpp"
#include "profile_editor.h"

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

private:
    Ui::DialogEditProfile *ui;
    QWidget *innerWidget{};
    ProfileEditor *innerEditor{};

    QString type;
    int groupId;
    bool newEnt = false;
    QSharedPointer<NekoRay::ProxyEntity> ent;

    QString network_title_base;

    struct {
        QString custom_outbound;
        QString custom_config;
        QString certificate;
    } CACHE;

    void typeSelected(const QString &newType);

    void editor_cache_updated_impl();
};

#endif // DIALOG_EDIT_PROFILE_H
