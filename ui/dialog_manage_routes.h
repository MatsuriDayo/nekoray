#pragma once

#include <QDialog>
#include <QMenu>

#include "qv2ray/ui/QvAutoCompleteTextEdit.hpp"
#include "main/NekoRay.hpp"

QT_BEGIN_NAMESPACE
namespace Ui { class DialogManageRoutes; }
QT_END_NAMESPACE

class DialogManageRoutes : public QDialog {
Q_OBJECT

public:
    explicit DialogManageRoutes(QWidget *parent = nullptr);

    ~DialogManageRoutes() override;

private:
    Ui::DialogManageRoutes *ui;

    struct {
        QString custom_route;
    } CACHE;


    QMenu *builtInSchemesMenu;
    Qv2ray::ui::widgets::AutoCompleteTextEdit *directDomainTxt;
    Qv2ray::ui::widgets::AutoCompleteTextEdit *proxyDomainTxt;
    Qv2ray::ui::widgets::AutoCompleteTextEdit *blockDomainTxt;
    //
    Qv2ray::ui::widgets::AutoCompleteTextEdit *directIPTxt;
    Qv2ray::ui::widgets::AutoCompleteTextEdit *blockIPTxt;
    Qv2ray::ui::widgets::AutoCompleteTextEdit *proxyIPTxt;
    //
    NekoRay::Routing routing_cn_lan = NekoRay::Routing();
    NekoRay::Routing routing_global = NekoRay::Routing(-1);
public slots:

    void accept() override;

    QList<QAction *> getBuiltInSchemes();

    QAction *schemeToAction(const QString &name, const NekoRay::Routing &scheme);

    void SetRouteConfig(const NekoRay::Routing &conf);
};


