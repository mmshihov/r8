#ifndef R8INPUTDIALOG_H
#define R8INPUTDIALOG_H

#include <QDialog>

#include "r8engine.h"
#include "r8lexer.h"
#include "r8charstream.h"

namespace Ui {
class R8InputDialog;
}

class R8InputDialog : public QDialog {
    Q_OBJECT
    
public:
    explicit R8InputDialog(QWidget *parent = 0);
    ~R8InputDialog();
    
    unsigned char Input();
    void SetValue(const QString& value);

private slots:
    void on_inputButton_clicked();
    void on_breakPushButton_clicked();

private:
    Ui::R8InputDialog *ui;
    R8Lexer            mLexer;
    R8StringCharStream mCharStream;

    bool IsValidInput();
    void InitLexer();
};

class R8UiInputPort : public R8InputPort {
public:
    R8UiInputPort();
    virtual ~R8UiInputPort() {delete mInputDialog;}
private:
    R8InputDialog *mInputDialog;
protected:
    virtual unsigned char DoInput();
};

#endif // R8INPUTDIALOG_H
