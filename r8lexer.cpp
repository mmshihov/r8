#include "r8lexer.h"

#include "r8charstream.h"

R8Lexer::R8Lexer() {
    mCharStream = 0;
}

bool R8Lexer::IsConstantStart(QChar ch) {
    return     ((QChar('0') <= ch) && (ch <= QChar('9')))
            || (ch == QChar('-'))
            || (ch == QChar('+'))
            ;
}

bool R8Lexer::IsIdentifierStart(QChar ch) {
    return     ((QChar('a') <= ch) && (ch <= QChar('z')))
            || ((QChar('A') <= ch) && (ch <= QChar('Z')))
            || (ch == QChar('_'))
            ;
}

QChar R8Lexer::CurrentChar() {Q_ASSERT(mCharStream != 0); return mCharStream->CurrentChar(); }
bool  R8Lexer::IsValidCurrentChar() const {Q_ASSERT(mCharStream != 0); return mCharStream->IsValidCurrentChar(); }
void  R8Lexer::GoToNextChar() { Q_ASSERT(mCharStream != 0); mCharStream->GoToNextChar(); }
int   R8Lexer::CurrentLine() const {Q_ASSERT(mCharStream != 0); return mCharStream->CurrentLine();}

void R8Lexer::SkipSpaces() {
    while (IsValidCurrentChar()) {
        QChar ch = CurrentChar();
        if ((ch == QChar(' ')) || (ch == QChar('\t')) || (ch == QChar('\r')) || (ch == QChar('\n'))) {
            GoToNextChar();
            continue;
        }
        break;
    }
}

void R8Lexer::SkipComment() {
    while (IsValidCurrentChar()) {
        QChar ch = CurrentChar();
        GoToNextChar();
        if (ch == QChar('\n'))
            return;
    }
}

void R8Lexer::NextToken() {
    QChar ch;
    while (1) {
        SkipSpaces();
        if (!IsValidCurrentChar()) {
            mCurrentToken = R8Token(R8Token::END_OF_SOURCE, "eos", 0);
            return;
        }

        ch = CurrentChar();
        if (ch == QChar(',')) {
            mCurrentToken = R8Token(R8Token::COMMA, QString(","), 0);
            GoToNextChar();
            return;
        } else if (ch == QChar(':')) {
            mCurrentToken = R8Token(R8Token::COLON, QString(":"), 0);
            GoToNextChar();
            return;
        } else if (ch == QChar('[')) {
            mCurrentToken = R8Token(R8Token::LEFT_SBRACE, QString("["), 0);
            GoToNextChar();
            return;
        } else if (ch == QChar(']')) {
            mCurrentToken = R8Token(R8Token::RIGHT_SBRACE, QString("]"), 0);
            GoToNextChar();
            return;
        } else if (ch == QChar(';')) {
            SkipComment();
        } else if (IsConstantStart(ch)) {
            mCurrentToken = GetConstantToken();
            return;
        } else if (IsIdentifierStart(ch)) {
            mCurrentToken = GetIdentifierToken();
            return;
        } else
            throw R8LexerException(R8LexerException::UNKNOWN_TOKEN, CurrentLine(), QString(ch));
    }
}

R8Token R8Lexer::ReadNumberToken(bool isNegative, const QString& prefix, int base) {
    QString str(prefix);
    int value = 0;

    while (IsValidCurrentChar()) {
        QChar ch = CurrentChar();

        if (base <= 10) {
            if ((QChar('0') <= ch) && (ch <= QChar('0' + base - 1))) {
                int digit = (int)(ch.unicode() - QChar('0').unicode());
                value = value * base + digit;

                GoToNextChar();
                str += ch;
            } else
                break;
        } else {
            if ((QChar('0') <= ch) && (ch <= QChar('0' + 10 - 1))) {
                int digit = (int)(ch.unicode() - QChar('0').unicode());
                value = value * base + digit;

                GoToNextChar();
                str += ch;
            } else if ((QChar('A') <= ch.toUpper()) && (ch.toUpper() <= QChar('A' + (base - 10) - 1))) {
                int digit = (int)(10 + ch.toUpper().unicode() - QChar('A').unicode());
                value = value * base + digit;

                GoToNextChar();
                str += ch;
            } else
                break;
        }
    }
    return R8Token(R8Token::NUMBER, str, (unsigned char)((isNegative)?(-value):value));
}

//число {+12, -12, 0xAC, 0123, 0o123, 0b01010}
R8Token R8Lexer::GetConstantToken() {
    QString str;
    bool    isNegative = false;

    QChar   ch = CurrentChar();
    if (ch == QChar('-')) {
        isNegative = true;

        GoToNextChar();
        str += ch;
    } else if (ch == QChar('+')) {
        GoToNextChar();
        str += ch;
    }

    if (!IsValidCurrentChar())
        return R8Token(R8Token::END_OF_SOURCE, "eos", 0);

    ch = CurrentChar();
    if (ch == QChar('0')) { //start of prefix
        str += ch;
        GoToNextChar();
        if (!IsValidCurrentChar())
            return R8Token(R8Token::NUMBER, str, 0);

        ch = CurrentChar(); //prefix char
        if (ch == QChar('b')) { //binary
            str += ch;
            GoToNextChar();
            return ReadNumberToken(isNegative, str, 2);
        } else if (ch == QChar('o')) { //octal
            str += ch;
            GoToNextChar();
            return ReadNumberToken(isNegative, str, 8);
        } else if (ch == QChar('x')) { //hex
            str += ch;
            GoToNextChar();
            return ReadNumberToken(isNegative, str, 16);
        } else if ((QChar('0') <= ch) && (ch <= QChar('7'))) { //octal
            return ReadNumberToken(isNegative, str, 8);
        } else {
            return ReadNumberToken(isNegative, str, 10);
        }
    } else {
        return ReadNumberToken(isNegative, str, 10);
    }
}

R8Token R8Lexer::GetIdentifierToken() {
    QString str;

    while (IsValidCurrentChar()) {
        QChar ch = CurrentChar();

        if (
                   IsIdentifierStart(ch)
                || ((QChar('0') <= ch) && (ch <= QChar('9')))
        ) {
            GoToNextChar();
            str += ch;
        } else
            break;
    }

    return R8Token(R8Token::IDENTIFIER, str, 0);
}
