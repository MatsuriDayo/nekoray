#pragma once

#include <QMenu>
#include <QWidget>
#include <QRegExpValidator>
#include <QJsonDocument>

inline QMenu *CreateMenu(QWidget *parent, const QList<QString> &texts, std::function<void(QAction *)> slot) {
    auto menu = new QMenu(parent);
    QList<QAction *> acts;

    for (const auto &text: texts) {
        acts.push_back(new QAction(text, parent)); //按顺序来
    }

    for (int i = 0; i < acts.size(); i++) {
        if (acts[i]->text() == "[Separator]") {
            acts[i]->setSeparator(true);
            acts[i]->setText("");
            acts[i]->setDisabled(true);
            acts[i]->setData(-1);
        } else {
            acts[i]->setData(i);
        }

        menu->addAction(acts[i]);
    }

    QWidget::connect(menu, &QMenu::triggered, parent, std::move(slot));
    return menu;
}

inline QString VerifyJsonString(const QString &source) {
    QJsonParseError error{};
    QJsonDocument doc = QJsonDocument::fromJson(source.toUtf8(), &error);
    Q_UNUSED(doc)

    if (error.error == QJsonParseError::NoError) {
        return "";
    } else {
        //LOG("WARNING: Json parse returns: " + error.errorString());
        return error.errorString();
    }
}

#define RED(obj)                                                                                                                                     \
    {                                                                                                                                                \
        auto _temp = obj->palette();                                                                                                                 \
        _temp.setColor(QPalette::Text, Qt::red);                                                                                                     \
        obj->setPalette(_temp);                                                                                                                      \
    }

#define BLACK(obj) obj->setPalette(QWidget::palette());

#define QRegExpValidator_Number new QRegExpValidator(QRegExp("^[0-9]+$")
