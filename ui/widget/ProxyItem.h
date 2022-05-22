#pragma once

#include <QWidget>
#include "db/ProxyEntity.hpp"

QT_BEGIN_NAMESPACE
namespace Ui { class ProxyItem; }
QT_END_NAMESPACE

class ProxyItem : public QWidget {
Q_OBJECT

public:
    explicit ProxyItem(QWidget *parent, const QSharedPointer<NekoRay::ProxyEntity> &ent);

    ~ProxyItem() override;

private:
    Ui::ProxyItem *ui;
};
