#pragma once

#include <QEvent>
#include <QCheckBox>

class FloatCheckBox : public QCheckBox {
public:
    QWidget *parent;
    QWidget *window;

    void refresh() {
        setFixedSize(24, 24);
        auto pos = parent->rect().topRight();
        pos = parent->mapTo(window, pos);
        pos.setX(pos.x() - 48); // ?
        move(pos);
        raise();
        setVisible(parent->isVisible());
    };

    bool eventFilter(QObject *obj, QEvent *e) override {
        if (obj != window || e->type() != QEvent::Resize) return false;
        refresh();
        return false;
    };

    explicit FloatCheckBox(QWidget *parent, QWidget *window) : QCheckBox(window) {
        this->parent = parent;
        this->window = window;
        window->installEventFilter(this);
        refresh();
    };
};
