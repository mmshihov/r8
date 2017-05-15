#include "r8charstream.h"

R8CharStream::R8CharStream() {}

void R8StringCharStream::SetSourceString(const QString &source) {
    mSource = source;
    mCurrentLine = 0;
    mCurrentCharIndex = 0;
}

QChar R8StringCharStream::CurrentChar() const {
    if (IsValidCurrentChar())
        return mSource[mCurrentCharIndex];
    return QChar(' '); //dont want throw...
}

void R8StringCharStream::GoToNextChar() {
    if (IsValidCurrentChar()) {
        QChar ch = mSource[mCurrentCharIndex];
        if (ch == QChar('\n'))
            mCurrentLine++;
        mCurrentCharIndex++;
    }
}

bool R8StringCharStream::IsValidCurrentChar() const {
    return (mCurrentCharIndex < mSource.length());
}
