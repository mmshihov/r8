#ifndef R8SOURCEEDITOR_H
#define R8SOURCEEDITOR_H

#include <QPlainTextEdit>
#include <QObject>
#include <QList>
#include <QIcon>

#include "r8syntaxhighlighter.h"

#include "r8charstream.h"

class QPaintEvent;
class QResizeEvent;
class QSize;
class QWidget;

class R8LineNumberArea;


class R8SourceEditor : public QPlainTextEdit {
    Q_OBJECT
public:
    enum EState {
        EDIT_STATE,
        DEBUG_STATE
    };

    explicit R8SourceEditor(QWidget *parent = 0);

    void lineNumberAreaPaintEvent(QPaintEvent *event);
    int lineNumberAreaWidth();

    void SetIpAtLine(int lineNumber);

    void AddBreakpointAt(int line);
    void AddOrRemoveBreakpointAt(int line);
    void AddBreakpointByMouseEvent(QMouseEvent *pe);
    void ClearBreakpointAt(int line);
    void ClearAllBreakpoints();

    bool IsBreakedLine(int line) const {return mBreakpoints.contains(line); }
    bool IsBreakointBetweenLines(int start, int end) const;

    void ShiftStateTo(EState state);
    bool IsStateIs(EState state) const {return (State() == state);}
    EState State() const {return mState;}

protected:
    void resizeEvent(QResizeEvent *event);

private:
    QWidget             *mLineNumberArea;

    typedef QList<int> TBreakpoints;

    EState          mState;
    int             mIpLine;      //current execution pointer in debug mode
    TBreakpoints    mBreakpoints;
    QIcon           mStopIcon,
                    mPlayIcon;
    int             mOldBlockCount;

    bool IsValidIp() const {return (mIpLine >= 0);}

    void CorrectBreakpoints(int blockDelta);

private slots:
    void updateLineNumberAreaWidth(int newBlockCount);
    void highlightCurrentLine();
    void updateLineNumberArea(const QRect &, int);
};

class R8LineNumberArea : public QWidget {
public:
    R8LineNumberArea(R8SourceEditor *editor) : QWidget(editor) { mSourceEditor = editor; }

    QSize sizeHint() const { return QSize(mSourceEditor->lineNumberAreaWidth(), 0); }

protected:
    virtual void paintEvent(QPaintEvent *event) { mSourceEditor->lineNumberAreaPaintEvent(event); }
    virtual void mouseDoubleClickEvent(QMouseEvent *pe) {mSourceEditor->AddBreakpointByMouseEvent(pe);}

private:
    R8SourceEditor *mSourceEditor;
};

class R8SourceEditorCharStream : public R8CharStream {
public:
    R8SourceEditorCharStream() {mCurrentCharIndex = 1;}

    void SetSourceTextBlock(const QTextBlock& block);

    virtual QChar CurrentChar() const;
    virtual void  GoToNextChar();
    virtual bool  IsValidCurrentChar() const;
    virtual int   CurrentLine() const;

private:
    QTextBlock mTextBlock;
    QString    mCurrentLine;
    int        mCurrentCharIndex;
};

#endif // R8SOURCEEDITOR_H
