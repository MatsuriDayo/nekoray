#ifndef NEKORAY_DIALOG_EDIT_GROUP_HPP
#define NEKORAY_DIALOG_EDIT_GROUP_HPP

#include <QDialog>
#include "db/Group.hpp"

QT_BEGIN_NAMESPACE
namespace Ui { class DialogEditGroup; }
QT_END_NAMESPACE

class DialogEditGroup : public QDialog {
Q_OBJECT

public:
    explicit DialogEditGroup(const QSharedPointer<NekoRay::Group>& ent, QWidget *parent = nullptr);

    ~DialogEditGroup() override;

private:
    Ui::DialogEditGroup *ui;
};


#endif //NEKORAY_DIALOG_EDIT_GROUP_HPP
