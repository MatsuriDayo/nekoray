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

    void accept();

private slots:

    void on_custom_edit_clicked();

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
    QString custom_edit_cache;
    QString certificate_edit_cache;

    void typeSelected(const QString &newType);

    void dialog_editor_cache_updated();
};

#endif // DIALOG_EDIT_PROFILE_H
