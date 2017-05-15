#ifndef R8LEXER_H
#define R8LEXER_H

#include<QString>

class R8CharStream;

class R8LexerException {
public:
    enum EType {
        UNKNOWN_TOKEN
    };

    R8LexerException(EType type, unsigned int lineNumber, const QString& info = QString()) :
        mType(type),mLineNumber(lineNumber),mInfo(info) {}

    EType Type() const {return mType;}
    unsigned int LineNumber() const {return mLineNumber;}
    QString Info() const {return mInfo;}
private:
    EType mType;
    unsigned int mLineNumber;
    QString      mInfo;
};


class R8Token {
public:
    enum EType {
        IDENTIFIER, //r1, mov, label_name (i.e. register, opcode, label)
        LEFT_SBRACE,
        RIGHT_SBRACE,
        COMMA,
        COLON,
        NUMBER,
        END_OF_SOURCE,
        MISPRINT
    };

    R8Token() : mType(END_OF_SOURCE), mTokenString(), mValue(0) {}
    R8Token(EType Type, QString TokenString, unsigned char Value) :
        mType(Type),mTokenString(TokenString),mValue(Value) {}

    EType   Type() const { return mType; }
    const QString TokenString() const { return mTokenString; }
    unsigned char Value() const { return mValue; }

private:
    EType           mType;
    QString         mTokenString;
    unsigned char   mValue;   //register index, memory index, constant
};


class R8Lexer {
public:
    R8Lexer();
    void SetSource(R8CharStream *charStream) {mCharStream = charStream;}
    const R8Token& CurrentToken() {return mCurrentToken;}
    void NextToken();

    int  CurrentLine() const;

private:
    R8CharStream *mCharStream;
    R8Token       mCurrentToken;

    static bool IsConstantStart(QChar ch);
    static bool IsIdentifierStart(QChar ch);

    QChar CurrentChar();
    bool  IsValidCurrentChar() const;
    void  GoToNextChar();

    void SkipSpaces();
    void SkipComment();

    R8Token ReadNumberToken(bool isNegative, const QString& prefix, int base);
    R8Token GetConstantToken();
    R8Token GetIdentifierToken();
};

#endif // R8LEXER_H
