#pragma once

#include <QCheckBox>

class FloatCheckBox : public QCheckBox {
public:
    explicit FloatCheckBox(QWidget *parent, QWidget *window) : QCheckBox(window) {
        setFixedSize(24, 24);
        auto pos = parent->rect().topRight();
        pos = parent->mapTo(window, pos);
        pos.setX(pos.x() - 48); // ?
        move(pos);
        raise();
        if (parent->isVisible()) show();
    };
};
