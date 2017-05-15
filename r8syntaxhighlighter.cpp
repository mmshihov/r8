#include "r8syntaxhighlighter.h"

R8SyntaxHighlighter::R8SyntaxHighlighter(QTextDocument *parent) : QSyntaxHighlighter(parent) {}

void R8SyntaxHighlighter::highlightBlock(const QString &text) {
    SetText(text);
    GotoNextToken();

    SetState(previousBlockState());

    while (CurrentTokenType() != R8Token::END_OF_SOURCE)
        HandleCurrentState();

    setCurrentBlockState(State());
}

void R8SyntaxHighlighter::SetAvailableCommand(const QString &name, const R8CommandDescriptor &descriptor) {
    mCommands[name] = descriptor;
}

bool R8SyntaxHighlighter::IsRegisterString(const QString &str) {
    return
            (str.length() == 2)
         && ((str[0] == 'r') || (str[0] == 'R'))
         && (('0' <= str[1]) && (str[1] <= '7'));
}

QString R8SyntaxHighlighter::CurrentTokenString() const {
    Q_ASSERT((mCurrentCharIndex - mStartCharIndex) >= 0);

    return mText.mid(mStartCharIndex, (mCurrentCharIndex - mStartCharIndex));
}

void R8SyntaxHighlighter::HandleCurrentState() {
    switch (mState) {
    case START_STATE:                   HandleStart(); break;
    case SRC_SRC_DST_STATE:             HandleSrcSrcDst(); break;
    case VALUE_RBRACE_SRC_DST_STATE:    HandleValueRbraceSrcDst(); break;
    case RBRACE_SRC_DST_STATE:          HandleRbraceSrcDst(); break;
    case COMMA_SRC_DST_STATE:           HandleCommaSrcDst(); break;
    case SRC_DST_STATE:                 HandleSrcDst(); break;
    case VALUE_RBRACE_DST_STATE:        HandleValueRbraceDst(); break;
    case RBRACE_DST_STATE:              HandleRbraceDst(); break;
    case COMMA_DST_STATE:               HandleCommaDst(); break;
    case DST_STATE:                     HandleDst(); break;
    case VALUE_RBRACE_STATE:            HandleValueRbrace(); break;
    case RBRACE_STATE:                  HandleRbrace(); break;
    case SRC_STATE:                     HandleSrc(); break;
    case SRC_LABEL_STATE:               HandleSrcLabel(); break;
    case VALUE_RBRACE_LABEL_STATE:      HandleValueRbraceLabel(); break;
    case RBRACE_LABEL_STATE:            HandleRbraceLabel(); break;
    case COMMA_LABEL_STATE:             HandleCommaLabel(); break;
    case LABEL_STATE:                   HandleLabel(); break;
    case ERROR_STATE:
    default:
        HandleError();
    }
}

void R8SyntaxHighlighter::HandleStart() {
    if (CurrentTokenType() == R8Token::IDENTIFIER) {
        QString idString = CurrentTokenString();
        int     idStart  = mStartCharIndex;
        int     idEnd    = mCurrentCharIndex;

        GotoNextToken();
        if (CurrentTokenType() == R8Token::COLON) {
            HighlightLabelDefinition(idStart, (mCurrentCharIndex - idStart));
            GotoNextToken();
            SetState(START_STATE);
        } else {
            HandleOpcode(idString, idStart, (idEnd - idStart));
            if (State() == ERROR_STATE)
                mStartCharIndex = mCurrentCharIndex = idStart;
        }
    } else
        SetState(ERROR_STATE);
}

void R8SyntaxHighlighter::HandleOpcode(const QString& opcodeString, int idStart, int idCount) {
    Q_ASSERT(idCount >= 0);

    if (!mCommands.contains(opcodeString.toUpper())) {
        SetState(ERROR_STATE); //undefined command
        return;
    }

    R8CommandDescriptor descriptor = mCommands[opcodeString.toUpper()];
    switch (descriptor.Type()) {
    case R8CommandDescriptor::ARGS_NO:          SetState(START_STATE);      break;
    case R8CommandDescriptor::ARGS_SRC:         SetState(SRC_STATE);        break;
    case R8CommandDescriptor::ARGS_DST:         SetState(DST_STATE);        break;
    case R8CommandDescriptor::ARGS_SRC_DST:     SetState(SRC_DST_STATE);    break;
    case R8CommandDescriptor::ARGS_SRC_SRC_DST: SetState(SRC_SRC_DST_STATE);break;
    case R8CommandDescriptor::ARGS_SRC_LABEL:   SetState(SRC_LABEL_STATE);  break;
    default:
        Q_ASSERT(false);
    }

    HighlightOpcode(idStart, idCount);
}

void R8SyntaxHighlighter::HandleSrcSrcDst() {
    if (CurrentTokenType() == R8Token::LEFT_SBRACE) {
        HighlightSBrace();
        GotoNextToken();
        SetState(VALUE_RBRACE_SRC_DST_STATE);
    } else if (CurrentTokenType() == R8Token::IDENTIFIER) {
        HighlightRegister();
        GotoNextToken();
        SetState(COMMA_SRC_DST_STATE);
    } else if (CurrentTokenType() == R8Token::NUMBER) {
        HighlightNumber();
        GotoNextToken();
        SetState(COMMA_SRC_DST_STATE);
    } else if (CurrentTokenType() != R8Token::END_OF_SOURCE)
        SetState(ERROR_STATE);
}

void R8SyntaxHighlighter::HandleValueRbraceSrcDst() {
    if (CurrentTokenType() == R8Token::IDENTIFIER) {
        HighlightRegister();
        GotoNextToken();
        SetState(RBRACE_SRC_DST_STATE);
    } else if (CurrentTokenType() == R8Token::NUMBER) {
        HighlightNumber();
        GotoNextToken();
        SetState(RBRACE_SRC_DST_STATE);
    } else if (CurrentTokenType() != R8Token::END_OF_SOURCE)
        SetState(ERROR_STATE);
}

void R8SyntaxHighlighter::HandleRbraceSrcDst() {
    if (CurrentTokenType() == R8Token::RIGHT_SBRACE) {
        HighlightSBrace();
        GotoNextToken();
        SetState(COMMA_SRC_DST_STATE);
    } else if (CurrentTokenType() != R8Token::END_OF_SOURCE)
        SetState(ERROR_STATE);
}

void R8SyntaxHighlighter::HandleCommaSrcDst() {
    if (CurrentTokenType() == R8Token::COMMA) {
        HighlightComma();
        GotoNextToken();
        SetState(SRC_DST_STATE);
    } else if (CurrentTokenType() != R8Token::END_OF_SOURCE)
        SetState(ERROR_STATE);
}

void R8SyntaxHighlighter::HandleSrcDst() {
    if (CurrentTokenType() == R8Token::LEFT_SBRACE) {
        HighlightSBrace();
        GotoNextToken();
        SetState(VALUE_RBRACE_DST_STATE);
    } else if (CurrentTokenType() == R8Token::IDENTIFIER) {
        HighlightRegister();
        GotoNextToken();
        SetState(COMMA_DST_STATE);
    } else if (CurrentTokenType() == R8Token::NUMBER) {
        HighlightNumber();
        GotoNextToken();
        SetState(COMMA_DST_STATE);
    } else if (CurrentTokenType() != R8Token::END_OF_SOURCE)
        SetState(ERROR_STATE);
}

void R8SyntaxHighlighter::HandleValueRbraceDst() {
    if (CurrentTokenType() == R8Token::IDENTIFIER) {
        HighlightRegister();
        GotoNextToken();
        SetState(RBRACE_DST_STATE);
    } else if (CurrentTokenType() == R8Token::NUMBER) {
        HighlightNumber();
        GotoNextToken();
        SetState(RBRACE_DST_STATE);
    } else if (CurrentTokenType() != R8Token::END_OF_SOURCE)
        SetState(ERROR_STATE);
}

void R8SyntaxHighlighter::HandleRbraceDst() {
    if (CurrentTokenType() == R8Token::RIGHT_SBRACE) {
        HighlightSBrace();
        GotoNextToken();
        SetState(COMMA_DST_STATE);
    } else if (CurrentTokenType() != R8Token::END_OF_SOURCE)
        SetState(ERROR_STATE);
}

void R8SyntaxHighlighter::HandleCommaDst() {
    if (CurrentTokenType() == R8Token::COMMA) {
        HighlightComma();
        GotoNextToken();
        SetState(DST_STATE);
    } else if (CurrentTokenType() != R8Token::END_OF_SOURCE)
        SetState(ERROR_STATE);
}

void R8SyntaxHighlighter::HandleDst() {
    if (CurrentTokenType() == R8Token::LEFT_SBRACE) {
        HighlightSBrace();
        GotoNextToken();
        SetState(VALUE_RBRACE_STATE);
    } else if (CurrentTokenType() == R8Token::IDENTIFIER) {
        HighlightRegister();
        GotoNextToken();
        SetState(START_STATE);
    } else if (CurrentTokenType() != R8Token::END_OF_SOURCE)
        SetState(ERROR_STATE);
}

void R8SyntaxHighlighter::HandleValueRbrace() {
    if (CurrentTokenType() == R8Token::IDENTIFIER) {
        HighlightRegister();
        GotoNextToken();
        SetState(RBRACE_STATE);
    } else if (CurrentTokenType() == R8Token::NUMBER) {
        HighlightNumber();
        GotoNextToken();
        SetState(RBRACE_STATE);
    } else if (CurrentTokenType() != R8Token::END_OF_SOURCE)
        SetState(ERROR_STATE);
}

void R8SyntaxHighlighter::HandleRbrace() {
    if (CurrentTokenType() == R8Token::RIGHT_SBRACE) {
        HighlightSBrace();
        GotoNextToken();
        SetState(START_STATE);
    } else if (CurrentTokenType() != R8Token::END_OF_SOURCE)
        SetState(ERROR_STATE);
}

void R8SyntaxHighlighter::HandleSrc() {
    if (CurrentTokenType() == R8Token::LEFT_SBRACE) {
        HighlightSBrace();
        GotoNextToken();
        SetState(VALUE_RBRACE_STATE);
    } else if (CurrentTokenType() == R8Token::IDENTIFIER) {
        HighlightRegister();
        GotoNextToken();
        SetState(START_STATE);
    } else if (CurrentTokenType() == R8Token::NUMBER) {
        HighlightNumber();
        GotoNextToken();
        SetState(START_STATE);
    } else if (CurrentTokenType() != R8Token::END_OF_SOURCE)
        SetState(ERROR_STATE);
}

void R8SyntaxHighlighter::HandleSrcLabel() {
    if (CurrentTokenType() == R8Token::LEFT_SBRACE) {
        HighlightSBrace();
        GotoNextToken();
        SetState(VALUE_RBRACE_LABEL_STATE);
    } else if (CurrentTokenType() == R8Token::IDENTIFIER) {
        HighlightRegister();
        GotoNextToken();
        SetState(COMMA_LABEL_STATE);
    } else if (CurrentTokenType() == R8Token::NUMBER) {
        HighlightNumber();
        GotoNextToken();
        SetState(COMMA_LABEL_STATE);
    } else if (CurrentTokenType() != R8Token::END_OF_SOURCE)
        SetState(ERROR_STATE);
}

void R8SyntaxHighlighter::HandleValueRbraceLabel() {
    if (CurrentTokenType() == R8Token::IDENTIFIER) {
        HighlightRegister();
        GotoNextToken();
        SetState(RBRACE_LABEL_STATE);
    } else if (CurrentTokenType() == R8Token::NUMBER) {
        HighlightNumber();
        GotoNextToken();
        SetState(RBRACE_LABEL_STATE);
    } else if (CurrentTokenType() != R8Token::END_OF_SOURCE)
        SetState(ERROR_STATE);
}

void R8SyntaxHighlighter::HandleRbraceLabel() {
    if (CurrentTokenType() == R8Token::RIGHT_SBRACE) {
        HighlightSBrace();
        GotoNextToken();
        SetState(COMMA_LABEL_STATE);
    } else if (CurrentTokenType() != R8Token::END_OF_SOURCE)
        SetState(ERROR_STATE);
}

void R8SyntaxHighlighter::HandleCommaLabel() {
    if (CurrentTokenType() == R8Token::COMMA) {
        HighlightComma();
        GotoNextToken();
        SetState(LABEL_STATE);
    } else if (CurrentTokenType() != R8Token::END_OF_SOURCE)
        SetState(ERROR_STATE);
}

void R8SyntaxHighlighter::HandleLabel() {
    if (CurrentTokenType() == R8Token::IDENTIFIER) {
        HighlightLabel();
        GotoNextToken();
        SetState(START_STATE);
    } else if (CurrentTokenType() != R8Token::END_OF_SOURCE)
        SetState(ERROR_STATE);
}

void R8SyntaxHighlighter::HandleError() {
    GotoEndSource();
    GotoNextToken();

    HighlightError();

    SetState(START_STATE);
}

void R8SyntaxHighlighter::GotoNextChar() {
    if (IsCurrentCharValid())
        ++mCurrentCharIndex;
}

void R8SyntaxHighlighter::SkipSpace() {
    while (IsCurrentCharValid()) {
        QChar ch = CurrentChar();
        if ((ch == QChar(' ')) || (ch == QChar('\t')) || (ch == QChar('\r')) || (ch == QChar('\n'))) {
            GotoNextChar();
        } else {
            MoveStartToCurrentIndex();
            return;
        }
    }
}

void R8SyntaxHighlighter::GotoNextToken() {
    while (1) {
        SkipSpace();
        if (!IsCurrentCharValid()) {
            SetCurrentTokenType(R8Token::END_OF_SOURCE);
            return;
        }

        QChar ch = CurrentChar();
        if (ch == QChar(',')) {
            SetCurrentTokenType(R8Token::COMMA);
            GotoNextChar();
        } else if (ch == QChar(':')) {
            SetCurrentTokenType(R8Token::COLON);
            GotoNextChar();
        } else if (ch == QChar('[')) {
            SetCurrentTokenType(R8Token::LEFT_SBRACE);
            GotoNextChar();
        } else if (ch == QChar(']')) {
            SetCurrentTokenType(R8Token::RIGHT_SBRACE);
            GotoNextChar();
        } else if (ch == QChar(';')) {
            SetCurrentTokenType(R8Token::END_OF_SOURCE);
            HighlightComment();
        } else if (IsConstantStart(ch)) {
            GetConstantToken();
        } else if (IsIdentifierStart(ch)) {
            GetIdentifierToken();
        } else
            SetCurrentTokenType(R8Token::MISPRINT);

        return;
    }
}

void R8SyntaxHighlighter::GetConstantToken() {
    QChar ch = CurrentChar();

    if ((ch == QChar('-')) || (ch == QChar('+')))
        GotoNextChar();

    if (!IsCurrentCharValid()) {
        SetCurrentTokenType(R8Token::END_OF_SOURCE);
        return;
    }

    ch = CurrentChar();
    if (ch == QChar('0')) { //start of prefix
        GotoNextChar();
        if (!IsCurrentCharValid()) {
            SetCurrentTokenType(R8Token::NUMBER);
            return;
        }

        ch = CurrentChar(); //prefix char
        if (ch == QChar('b')) { //binary
            GotoNextChar();
            ReadNumberToken(2);
            return;
        } else if (ch == QChar('o')) { //octal
            GotoNextChar();
            ReadNumberToken(8);
            return;
        } else if (ch == QChar('x')) { //hex
            GotoNextChar();
            ReadNumberToken(16);
            return;
        } else if ((QChar('0') <= ch) && (ch <= QChar('7'))) { //octal
            ReadNumberToken(8);
            return;
        } else {
            ReadNumberToken(10);
            return;
        }
    } else {
        ReadNumberToken(10);
        return;
    }
}

void R8SyntaxHighlighter::ReadNumberToken(int base) {
    while (IsCurrentCharValid()) {
        QChar ch = CurrentChar();

        if (base <= 10) {
            if ((QChar('0') <= ch) && (ch <= QChar('0' + base - 1))) {
                GotoNextChar();
            } else
                break;
        } else {
            if ((QChar('0') <= ch) && (ch <= QChar('0' + 10 - 1))) {
                GotoNextChar();
            } else if ((QChar('A') <= ch.toUpper()) && (ch.toUpper() <= QChar('A' + (base - 10) - 1))) {
                GotoNextChar();
            } else
                break;
        }
    }

    SetCurrentTokenType(R8Token::NUMBER);
}

void R8SyntaxHighlighter::GetIdentifierToken() {
    while (IsCurrentCharValid()) {
        QChar ch = CurrentChar();

        if (IsIdentifierStart(ch) || ((QChar('0') <= ch) && (ch <= QChar('9')))) {
            GotoNextChar();
        } else
            break;
    }

    SetCurrentTokenType(R8Token::IDENTIFIER);
}

void R8SyntaxHighlighter::HighlightComment() {
    mCurrentCharIndex = mText.length();

    Q_ASSERT((mCurrentCharIndex - mStartCharIndex) >= 0);

    QTextCharFormat commentFormat;
    commentFormat.setForeground(Qt::green);

    setFormat(mStartCharIndex, (mCurrentCharIndex - mStartCharIndex), commentFormat);
}

void R8SyntaxHighlighter::HighlightNumber() {
    Q_ASSERT((mCurrentCharIndex - mStartCharIndex) >= 0);

    QTextCharFormat numberFormat;
    numberFormat.setForeground(Qt::magenta);

    setFormat(mStartCharIndex, (mCurrentCharIndex - mStartCharIndex), numberFormat);
}

void R8SyntaxHighlighter::HighlightComma() {
    Q_ASSERT((mCurrentCharIndex - mStartCharIndex) >= 0);

    QTextCharFormat commaFormat;
    commaFormat.setForeground(Qt::black);

    setFormat(mStartCharIndex, (mCurrentCharIndex - mStartCharIndex), commaFormat);
}

void R8SyntaxHighlighter::HighlightError() {
    Q_ASSERT((mCurrentCharIndex - mStartCharIndex) >= 0);

    QTextCharFormat errorFormat;
    errorFormat.setFontWeight(QFont::Bold);
    errorFormat.setFontUnderline(true);
    errorFormat.setUnderlineColor(Qt::red);
    errorFormat.setUnderlineStyle(QTextCharFormat::WaveUnderline);
    errorFormat.setForeground(Qt::red);

    setFormat(mStartCharIndex, (mCurrentCharIndex - mStartCharIndex), errorFormat);
}

void R8SyntaxHighlighter::HighlightRegister() {
    Q_ASSERT((mCurrentCharIndex - mStartCharIndex) >= 0);


    if (IsRegisterString(CurrentTokenString())) {
        QTextCharFormat registerFormat;
        registerFormat.setForeground(Qt::darkRed);

        setFormat(mStartCharIndex, (mCurrentCharIndex - mStartCharIndex), registerFormat);
    } else
        HighlightError();
}

void R8SyntaxHighlighter::HighlightSBrace() {
    Q_ASSERT((mCurrentCharIndex - mStartCharIndex) >= 0);

    QTextCharFormat sbraceFormat;
    sbraceFormat.setForeground(Qt::darkGray);

    setFormat(mStartCharIndex, (mCurrentCharIndex - mStartCharIndex), sbraceFormat);
}

void R8SyntaxHighlighter::HighlightLabel() {
    HighlightLabelDefinition(mStartCharIndex, (mCurrentCharIndex - mStartCharIndex));
}

void R8SyntaxHighlighter::HighlightLabelDefinition(int start, int count) {
    Q_ASSERT(count >= 0);

    QTextCharFormat labelFormat;
    labelFormat.setFontWeight(QFont::Bold);
    labelFormat.setForeground(QColor(0xff,0xA5,0x00));

    setFormat(start, count, labelFormat);
}

void R8SyntaxHighlighter::HighlightOpcode(int start, int count) {
    Q_ASSERT(count >= 0);

    QTextCharFormat commentFormat;
    commentFormat.setFontWeight(QFont::Bold);
    commentFormat.setForeground(Qt::blue);

    setFormat(start, count, commentFormat);
}


bool R8SyntaxHighlighter::IsConstantStart(QChar ch) {
    return     ((QChar('0') <= ch) && (ch <= QChar('9')))
            || (ch == QChar('-'))
            || (ch == QChar('+'))
            ;
}

bool R8SyntaxHighlighter::IsIdentifierStart(QChar ch) {
    return     ((QChar('a') <= ch) && (ch <= QChar('z')))
            || ((QChar('A') <= ch) && (ch <= QChar('Z')))
            || (ch == QChar('_'))
            ;
}

