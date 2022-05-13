#pragma once

#include <QLineEdit>

class MyLineEdit : public QLineEdit {
public slots:

    explicit MyLineEdit(QWidget *parent = nullptr) : QLineEdit(parent) {
    }

    void setText(const QString &s) {
        QLineEdit::setText(s);
        QLineEdit::home(false);
    }
};
