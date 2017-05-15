#include "r8inputdialog.h"

#include<QMessageBox>

#include "ui_r8inputdialog.h"


R8InputDialog::R8InputDialog(QWidget *parent) :  QDialog(parent), ui(new Ui::R8InputDialog) {
    ui->setupUi(this);

    InitLexer();
}

R8InputDialog::~R8InputDialog() {delete ui;}

unsigned char R8InputDialog::Input() { return mLexer.CurrentToken().Value(); }

void R8InputDialog::SetValue(const QString &value) {
     ui->inputLineEdit->setText(value);
}

void R8InputDialog::on_inputButton_clicked() {
    if (IsValidInput()) {
        accept();
    } else {
        QMessageBox::information(
                    this,
                    tr("Digit format error"),
                    tr("You can input number in\n"
                       " - decimal (10)\n"
                       " - hex (0xA)\n"
                       " - oct (012)\n"
                       " - bin (0b1010)"));
    }
}

void R8InputDialog::on_breakPushButton_clicked() { reject(); }

bool R8InputDialog::IsValidInput() {
    mCharStream.SetSourceString(ui->inputLineEdit->text());
    mLexer.SetSource(&mCharStream);
    mLexer.NextToken();

    return (mLexer.CurrentToken().Type() == R8Token::NUMBER);
}

void R8InputDialog::InitLexer() {
    mLexer.SetSource(&mCharStream);
}


R8UiInputPort::R8UiInputPort() {
    mInputDialog = new R8InputDialog();
}

unsigned char R8UiInputPort::DoInput() {
    SetFailure(false);
    mInputDialog->SetValue(QString(""));
    if (mInputDialog->exec() == QDialog::Accepted) {
        return mInputDialog->Input();
    } else {
        SetFailure(true);
        return 0;
    }
}


