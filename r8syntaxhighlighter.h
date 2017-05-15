#ifndef R8SYNTAXHIGHLIGHTER_H
#define R8SYNTAXHIGHLIGHTER_H

#include <QSyntaxHighlighter>

#include "r8compiler.h"

class R8SyntaxHighlighter : public QSyntaxHighlighter {
public:
    R8SyntaxHighlighter(QTextDocument *parent);

    virtual void highlightBlock(const QString &text);

    void ClearAvailableCommands() {mCommands.clear();}
    void SetAvailableCommand(const QString& name, const R8CommandDescriptor& descriptor);

private:
    typedef QMap<QString, R8CommandDescriptor>  TCommandNameMapping;

    enum EState{
        START_STATE,
        ERROR_STATE,
        SRC_SRC_DST_STATE,
        VALUE_RBRACE_SRC_DST_STATE,
        RBRACE_SRC_DST_STATE,
        COMMA_SRC_DST_STATE,
        SRC_DST_STATE,
        VALUE_RBRACE_DST_STATE,
        RBRACE_DST_STATE,
        COMMA_DST_STATE,
        DST_STATE,
        SRC_STATE,
        VALUE_RBRACE_STATE,
        RBRACE_STATE,
        SRC_LABEL_STATE,
        VALUE_RBRACE_LABEL_STATE,
        RBRACE_LABEL_STATE,
        COMMA_LABEL_STATE,
        LABEL_STATE,
        END_STATE
    };

    EState               mState;

    int                  mStartCharIndex;
    int                  mCurrentCharIndex;
    R8Token::EType       mCurrentTokenType;

    QString              mText;

    TCommandNameMapping  mCommands;

    static bool IsRegisterString(const QString& str);

    EState State() const { return mState; }

    void SetState(int state) {mState = ((START_STATE<=state) && (state<END_STATE)) ? (EState)state : START_STATE;}
    void SetState(EState state) {mState = state;}
    void SetText(const QString& text) {mText = text; mStartCharIndex = mCurrentCharIndex = 0; }

    R8Token::EType CurrentTokenType() const { return mCurrentTokenType; }
    void SetCurrentTokenType(R8Token::EType type) { mCurrentTokenType = type; }

    QString CurrentTokenString() const;

    void HandleCurrentState();

    void HandleStart();
    void HandleOpcode(const QString& opcodeString, int idStart, int idEnd);
    void HandleSrcSrcDst();
    void HandleValueRbraceSrcDst();
    void HandleRbraceSrcDst();
    void HandleCommaSrcDst();
    void HandleSrcDst();
    void HandleValueRbraceDst();
    void HandleRbraceDst();
    void HandleCommaDst();
    void HandleDst();
    void HandleValueRbrace();
    void HandleRbrace();
    void HandleSrc();
    void HandleSrcLabel();
    void HandleValueRbraceLabel();
    void HandleRbraceLabel();
    void HandleCommaLabel();
    void HandleLabel();
    void HandleError();

    void HighlightComment();
    void HighlightNumber();
    void HighlightComma();
    void HighlightError();
    void HighlightRegister();
    void HighlightSBrace();
    void HighlightLabel();
    void HighlightLabelDefinition(int start, int count);
    void HighlightOpcode(int start, int count);

    void  GotoNextChar();
    void  MoveStartToCurrentIndex() {mStartCharIndex = mCurrentCharIndex;}
    QChar CurrentChar() const {Q_ASSERT(IsCurrentCharValid()); return mText[mCurrentCharIndex];}
    bool  IsCurrentCharValid() const {return (mCurrentCharIndex < mText.length());}
    void  GotoEndSource() {mCurrentCharIndex = mText.length();}

    void SkipSpace();
    void GotoNextToken();
    void GetConstantToken();
    void ReadNumberToken(int base);
    void GetIdentifierToken();

    static bool IsConstantStart(QChar ch);
    static bool IsIdentifierStart(QChar ch);
};

#endif // R8SYNTAXHIGHLIGHTER_H
