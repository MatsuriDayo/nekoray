#include "QtExtKeySequenceEdit.h"

QtExtKeySequenceEdit::QtExtKeySequenceEdit(QWidget *parent)
        : QKeySequenceEdit(parent) {
}

QtExtKeySequenceEdit::~QtExtKeySequenceEdit() {
}

void QtExtKeySequenceEdit::keyPressEvent(QKeyEvent *pEvent) {
    QKeySequenceEdit::keyPressEvent(pEvent);

    QKeySequence keySeq = keySequence();
    if (keySeq.count() <= 0) {
        return;
    }
    int key = keySeq[0];
    if (key == Qt::Key_Backspace || key == Qt::Key_Delete) {
        key = 0;
    }
    setKeySequence(key);
}
