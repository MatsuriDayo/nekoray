#include <QKeySequenceEdit>

class QtExtKeySequenceEdit : public QKeySequenceEdit {
public:
    QtExtKeySequenceEdit(QWidget *parent);

    ~QtExtKeySequenceEdit();

protected:
    virtual void keyPressEvent(QKeyEvent *pEvent);
};
