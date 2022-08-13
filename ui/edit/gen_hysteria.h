#pragma once

#include <QWidget>
#include "db/ProxyEntity.hpp"

QT_BEGIN_NAMESPACE
namespace Ui { class GenHysteria; }
QT_END_NAMESPACE

class GenHysteria : public QWidget {
Q_OBJECT

public:
    explicit GenHysteria(const QSharedPointer<NekoRay::ProxyEntity> &ent, QWidget *parent = nullptr);

    ~GenHysteria() override;

private:
    Ui::GenHysteria *ui;
    QSharedPointer<NekoRay::ProxyEntity> ent;

signals:

    void config_generated(const QString &result);

private slots:

    void on_gen_clicked();
};
