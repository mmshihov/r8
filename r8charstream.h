#ifndef R8CHARSTREAM_H
#define R8CHARSTREAM_H

#include <QChar>
#include <QString>

class R8CharStream {
public:
    R8CharStream();
    virtual ~R8CharStream() {}

    virtual QChar CurrentChar() const           = 0;
    virtual void  GoToNextChar()                = 0;
    virtual bool  IsValidCurrentChar() const    = 0;
    virtual int   CurrentLine() const           = 0;
};

class R8StringCharStream : public R8CharStream {
public:
    R8StringCharStream() {}
    R8StringCharStream(const QString& source) {SetSourceString(source);}

    void SetSourceString(const QString& source);

    virtual QChar CurrentChar() const;
    virtual void  GoToNextChar();
    virtual bool  IsValidCurrentChar() const;
    virtual int   CurrentLine() const {return mCurrentLine;}
private:
    int     mCurrentCharIndex;
    int     mCurrentLine;
    QString mSource;
};

#endif // R8CHARSTREAM_H
