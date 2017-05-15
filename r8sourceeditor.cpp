#include <QtWidgets>

#include "r8sourceeditor.h"

R8SourceEditor::R8SourceEditor(QWidget *parent) :
        QPlainTextEdit(parent)
      , mStopIcon(":/images/breakpoint")
      , mPlayIcon(":/images/step")
{
    mLineNumberArea = new R8LineNumberArea(this);

    connect(this, SIGNAL(blockCountChanged(int)),   this, SLOT(updateLineNumberAreaWidth(int)));
    connect(this, SIGNAL(updateRequest(QRect,int)), this, SLOT(updateLineNumberArea(QRect,int)));
    connect(this, SIGNAL(cursorPositionChanged()),  this, SLOT(highlightCurrentLine()));

    mOldBlockCount = blockCount();
    updateLineNumberAreaWidth(blockCount());

    highlightCurrentLine();
    mIpLine = 0;
}

int R8SourceEditor::lineNumberAreaWidth() {
    int digits = 1;
    int max = qMax(1, blockCount());

    while (max >= 10) {
        max /= 10;
        ++digits;
    }

    int space = 3 + fontMetrics().height() + fontMetrics().width(QLatin1Char('9')) * digits;
    return space;
}

void R8SourceEditor::SetIpAtLine(int lineNumber) {
    mIpLine = lineNumber;

    if (lineNumber < 0) {
        mLineNumberArea->repaint();
        return;
    }

    QTextBlock textBlock = firstVisibleBlock();
    if (!textBlock.isValid())
        return;

    while (true) {
        if (textBlock.blockNumber() < 0) {
            return; //bad block or ip?
        } if (textBlock.blockNumber() < lineNumber) {
            textBlock = textBlock.next();
        } else if (textBlock.blockNumber() > lineNumber) {
            textBlock = textBlock.previous();
        } else { //equal
            QTextCursor cursor(textBlock);
            setTextCursor(cursor);
            ensureCursorVisible();
            mLineNumberArea->repaint();
            return;
        }
    }
}

void R8SourceEditor::AddBreakpointAt(int lineNumber) {
    if (!mBreakpoints.contains(lineNumber)) {
        mBreakpoints.append(lineNumber);
        mLineNumberArea->repaint();
    }
}

void R8SourceEditor::ClearBreakpointAt(int lineNumber) {
    if (mBreakpoints.contains(lineNumber)) {
        mBreakpoints.removeOne(lineNumber);
        mLineNumberArea->repaint();
    }
}

void R8SourceEditor::AddOrRemoveBreakpointAt(int lineNumber) {
    if (mBreakpoints.contains(lineNumber)) {
        mBreakpoints.removeOne(lineNumber);
    } else
        mBreakpoints.append(lineNumber);
    mLineNumberArea->repaint();
}

void R8SourceEditor::AddBreakpointByMouseEvent(QMouseEvent *pe) {
    QTextBlock block = firstVisibleBlock();
    int blockNumber = block.blockNumber();
    int top = (int) blockBoundingGeometry(block).translated(contentOffset()).top();
    int bottom = top + (int) blockBoundingRect(block).height();
    int mouseY = pe->y();

    while (block.isValid()) {
        if ((top <= mouseY) && (mouseY < bottom)) {
            AddOrRemoveBreakpointAt(blockNumber);
            return;
        }

        block = block.next();
        top = bottom;
        bottom = top + (int) blockBoundingRect(block).height();
        ++blockNumber;
    }
}

void R8SourceEditor::ClearAllBreakpoints() {
    if (!mBreakpoints.isEmpty()) {
        mBreakpoints.clear();
        mLineNumberArea->repaint();
    }
}

bool R8SourceEditor::IsBreakointBetweenLines(int startLine, int endLine) const {
    QListIterator<int> it(mBreakpoints);
    while (it.hasNext()) {
        int line = it.next();
        if ((startLine <= line) && (line <= endLine))
            return true;
    }
    return false;
}

void R8SourceEditor::ShiftStateTo(R8SourceEditor::EState state) {
     mState = state;
     mLineNumberArea->repaint();
}

void R8SourceEditor::CorrectBreakpoints(int blockDelta)
{
    QTextCursor cursor = textCursor();
    int currBlockIndex = cursor.blockNumber();

    if (blockDelta > 0) {
        QMutableListIterator<int> breakIt(mBreakpoints);
        while (breakIt.hasNext()) {
            breakIt.next();

            if (breakIt.value() > (currBlockIndex - blockDelta))
                breakIt.setValue(breakIt.value() + blockDelta);
        }
    } else {
        QMutableListIterator<int> breakIt(mBreakpoints);
        while (breakIt.hasNext()) {
            breakIt.next();

            if ((currBlockIndex < breakIt.value()) && (breakIt.value() < (currBlockIndex - blockDelta))) {
                breakIt.remove();
            } else if ((currBlockIndex - blockDelta) < breakIt.value()) {
                breakIt.setValue(breakIt.value() + blockDelta);
            } else if ((currBlockIndex - blockDelta) == breakIt.value()) {
                if (cursor.atBlockStart())
                    breakIt.setValue(breakIt.value() + blockDelta);
                else
                    breakIt.remove();
            }
        }
    }
}

void R8SourceEditor::updateLineNumberAreaWidth(int newBlockCount) {
    if (mOldBlockCount != newBlockCount) {
        CorrectBreakpoints(newBlockCount - mOldBlockCount);
        mOldBlockCount = newBlockCount;
    }

    setViewportMargins(lineNumberAreaWidth(), 0, 0, 0);
}

void R8SourceEditor::updateLineNumberArea(const QRect &rect, int dy) {
    if (dy)
        mLineNumberArea->scroll(0, dy);
    else
        mLineNumberArea->update(0, rect.y(), mLineNumberArea->width(), rect.height());

    if (rect.contains(viewport()->rect()))
        updateLineNumberAreaWidth(blockCount());
}

void R8SourceEditor::resizeEvent(QResizeEvent *e) {
    QPlainTextEdit::resizeEvent(e);

    QRect cr = contentsRect();
    mLineNumberArea->setGeometry(QRect(cr.left(), cr.top(), lineNumberAreaWidth(), cr.height()));
}

void R8SourceEditor::highlightCurrentLine() {
    QList<QTextEdit::ExtraSelection> extraSelections;

    if (!isReadOnly()) {
        QTextEdit::ExtraSelection selection;

        QColor lineColor = QColor(Qt::yellow).lighter(160);

        selection.format.setBackground(lineColor);
        selection.format.setProperty(QTextFormat::FullWidthSelection, true);
        selection.cursor = textCursor();
        selection.cursor.clearSelection();
        extraSelections.append(selection);
    }

    setExtraSelections(extraSelections);
}

void R8SourceEditor::lineNumberAreaPaintEvent(QPaintEvent *event) {
    QPainter painter(mLineNumberArea);
    painter.fillRect(event->rect(), Qt::lightGray);

    QTextBlock block = firstVisibleBlock();
    int blockNumber = block.blockNumber();
    int top = (int) blockBoundingGeometry(block).translated(contentOffset()).top();
    int bottom = top + (int) blockBoundingRect(block).height();

    while (block.isValid() && top <= event->rect().bottom()) {
        if (block.isVisible() && bottom >= event->rect().top()) {
            QString number = QString::number(blockNumber + 1);

            painter.setPen(Qt::black);
            painter.drawText(0, top,
                             mLineNumberArea->width(), fontMetrics().height(),
                             Qt::AlignRight,
                             number);

            if (mBreakpoints.contains(blockNumber))
                mStopIcon.paint(&painter, 1, top, fontMetrics().height(), fontMetrics().height());

            if (IsStateIs(DEBUG_STATE) && (mIpLine == blockNumber))
                mPlayIcon.paint(&painter, 2, top, fontMetrics().height(), fontMetrics().height());
        }

        block = block.next();
        top = bottom;
        bottom = top + (int) blockBoundingRect(block).height();
        ++blockNumber;
    }
}

void R8SourceEditorCharStream::SetSourceTextBlock(const QTextBlock& block){
    mTextBlock = block;

    while (mTextBlock.isValid()) {
        if (mTextBlock.blockNumber() == 0)
            break;
        mTextBlock = mTextBlock.previous();
    }

    if (mTextBlock.isValid()) {
        mCurrentCharIndex = 0;
        mCurrentLine = mTextBlock.text();
    } else {
        mCurrentCharIndex = 1;
        mCurrentLine.clear();
    }
}

QChar R8SourceEditorCharStream::CurrentChar() const {
    if (IsValidCurrentChar()) {
        if (mCurrentCharIndex >= mCurrentLine.length())
            return QChar('\n');
        return mCurrentLine[mCurrentCharIndex];
    }
    return QChar(' ');
}

void R8SourceEditorCharStream::GoToNextChar() {
    if (IsValidCurrentChar())
        ++mCurrentCharIndex;

    if (!IsValidCurrentChar()) {
        if (mTextBlock.isValid()) {
            mTextBlock = mTextBlock.next();
            if (mTextBlock.isValid()) {
                mCurrentLine = mTextBlock.text();
                mCurrentCharIndex = 0;
            }
        }
    }
}

bool R8SourceEditorCharStream::IsValidCurrentChar() const {
    return (mCurrentCharIndex <= mCurrentLine.length());
}

int R8SourceEditorCharStream::CurrentLine() const {
    if (IsValidCurrentChar())
        return mTextBlock.blockNumber();
    return -1;
}

