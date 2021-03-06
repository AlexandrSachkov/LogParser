/*
 * This file is part of the Line Catcher distribution (https://github.com/AlexandrSachkov/LineCatcher).
 * Copyright (c) 2019 Alexandr Sachkov.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 3.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "pagedfileviewwidget.h"
#include "signalingscrollbar.h"
#include "linenumberarea.h"
#include "common.h"
#include "fileview.h"
#include "highlightsdialog.h"
#include <QDebug>
#include <QPainter>
#include <QTextBlock>
#include <QtMath>
#include <QScrollBar>
#include <QMessageBox>
#include <QMenu>
#include <QAction>
#include <QApplication>
#include <QClipboard>

#include "ReturnType.h"

PagedFileViewWidget::PagedFileViewWidget(
        CoreObjPtr<PLP::FileReaderI> fileReader,
        ULLSpinBox* lineNavBox,
        std::function<QList<const HighlightItem*>()> getHighlightedItems,
        std::function<void(const QString&)> highlight,
        std::function<void()> clearHighlights,
        QWidget *parent) : QPlainTextEdit (parent) {

    _fileReader = std::move(fileReader);
    _lineNavBox = lineNavBox;
    _getHighlightedItems = getHighlightedItems;
    _highlight = highlight;
    _clearHighlights = clearHighlights;

    _lineNavBox->setValue(0);
    connect(_lineNavBox, SIGNAL(valueUpdated(unsigned long long)), this, SLOT(gotoLine(unsigned long long)));

    _lineNumberArea = new LineNumberArea(this);
    connect(_lineNumberArea, SIGNAL(paintEventOccurred(QPaintEvent*)), this, SLOT(lineNumberAreaPaintEvent(QPaintEvent*)));
    connect(_lineNumberArea, SIGNAL(sizeHintRequested(void)), this, SLOT(lineNumberAreaWidth(void)));

    connect(this, SIGNAL(textChanged(void)), this, SLOT(textChangedImpl(void)));
    connect(this, SIGNAL(blockCountChanged(int)), this, SLOT(updateLineNumberAreaWidth(int)));
    connect(this, SIGNAL(updateRequest(QRect,int)), this, SLOT(updateLineNumberArea(QRect,int)));

    this->setLineWrapMode(LineWrapMode::NoWrap);
    this->setWordWrapMode(QTextOption::WrapMode::NoWrap);
    this->setMouseTracking(true);
    this->setMaximumBlockCount(MAX_NUM_BLOCKS + 1); //there is always going to be a last empty block

    SignalingScrollBar* scrollBar = new SignalingScrollBar();
    connect(scrollBar, SIGNAL(mouseReleased(void)), this, SLOT(readBlockIfRequired(void)));
    connect(scrollBar, SIGNAL(wheelMoved(void)), this, SLOT(readBlockIfRequired(void)));
    connect(scrollBar, SIGNAL(valueChanged(int)), this, SLOT(scrollBarMoved(int)));
    this->setVerticalScrollBar(scrollBar);

    this->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(this, SIGNAL(customContextMenuRequested(const QPoint &)),
            this, SLOT(showRightClickMenu(const QPoint &)));

    QFont f("Courier New", 14);
    f.setStyleHint(QFont::Monospace);
    this->setFont(f);

    calcNumVisibleLines();

    readNextBlock();
    this->moveCursor(QTextCursor::Start);
    this->ensureCursorVisible();

    updateLineNumberAreaWidth(0);
}

void PagedFileViewWidget::resizeEvent(QResizeEvent *e){
    QPlainTextEdit::resizeEvent(e);
    calcNumVisibleLines();

    QRect cr = contentsRect();
    _lineNumberArea->setGeometry(QRect(cr.left(), cr.top(), lineNumberAreaWidth(), cr.height()));
}

void PagedFileViewWidget::textChangedImpl() {
    calcNumVisibleLines();
}

void PagedFileViewWidget::mouseMoveEvent(QMouseEvent *e) {
    QPlainTextEdit::mouseMoveEvent(e);

    _mouseOverSelection.format.setBackground(Common::LineHighlightBGColor);
    _mouseOverSelection.format.setForeground(Common::LineHighlightTextColor);
    _mouseOverSelection.format.setProperty(QTextFormat::FullWidthSelection, true);
    _mouseOverSelection.cursor = this->cursorForPosition(e->pos());
    _mouseOverSelection.cursor.clearSelection();
    _mouseOverSelection.lineNumber = _startLineNum + _mouseOverSelection.cursor.blockNumber();

    doHighlighting();
}

void PagedFileViewWidget::calcNumVisibleLines() {
    _numVisibleLines = this->height() / this->fontMetrics().height();
}

int PagedFileViewWidget::lineNumberAreaWidth() const
{
    int digits = 1;
    unsigned long long max = _endLineNum > 1 ? _endLineNum : 1;
    while (max >= 10) {
        max /= 10;
        ++digits;
    }

    int space = 3 + fontMetrics().horizontalAdvance(QLatin1Char('9')) * digits;
    return space;
}

void PagedFileViewWidget::updateLineNumberAreaWidth(int /* newBlockCount */)
{
    setViewportMargins(lineNumberAreaWidth(), 0, 0, 0);
}

void PagedFileViewWidget::updateLineNumberArea(const QRect &rect, int dy)
{
    if (dy){
        _lineNumberArea->scroll(0, dy);
    } else {
        _lineNumberArea->update(0, rect.y(), _lineNumberArea->width(), rect.height());
    }

    if (rect.contains(viewport()->rect())) {
        updateLineNumberAreaWidth(0);
    }
}

void PagedFileViewWidget::lineNumberAreaPaintEvent(QPaintEvent *event)
{
    QPainter painter(_lineNumberArea);
    painter.fillRect(event->rect(), Common::LineNumberAreaBGColor);


    QTextBlock block = firstVisibleBlock();
    int blockNumber = block.blockNumber();
    int top = (int) blockBoundingGeometry(block).translated(contentOffset()).top();
    int bottom = top + (int) blockBoundingRect(block).height();

    while (block.isValid() && top <= event->rect().bottom()) {
        if (block.isVisible() && bottom >= event->rect().top()) {
            QString number = QString::number(_startLineNum + blockNumber);
            painter.setPen(Common::LineNumberAreaTextColor);
            painter.drawText(0, top, _lineNumberArea->width(), fontMetrics().height(), Qt::AlignLeft, number);
        }

        block = block.next();
        top = bottom;
        bottom = top + (int) blockBoundingRect(block).height();
        ++blockNumber;
    }
}

void PagedFileViewWidget::readNextBlock() {
    const int currScrollbarValue = this->verticalScrollBar()->value();
    const unsigned long long currStartLineNum = _startLineNum;

    char* lineStart = nullptr;
    unsigned int length;

    QTextCursor cursor = this->textCursor();
    PLP::LineReaderResult result = _fileReader->getLine(_endLineNum, lineStart, length);
    if(result == PLP::LineReaderResult::SUCCESS){
        cursor.movePosition(QTextCursor::End);
        cursor.insertText(QString::fromUtf8(lineStart, static_cast<int>(length)).replace("\r",""));
        _endLineNum++;

        for(unsigned int i = 0; i < NUM_LINES_PER_READ - 1; i++){
            result = _fileReader->nextLine(lineStart, length);
            if(result != PLP::LineReaderResult::SUCCESS){
                break;
            }
            cursor.movePosition(QTextCursor::End);
            cursor.insertText(QString::fromUtf8(lineStart, static_cast<int>(length)).replace("\r",""));
            _endLineNum++;
        }
    }

    if(result == PLP::LineReaderResult::ERROR){
        QMessageBox::critical(this,"Error","Failed to read block.",QMessageBox::Ok);
        return;
    }

    if(_endLineNum - _startLineNum > MAX_NUM_BLOCKS){
        _startLineNum = _endLineNum - MAX_NUM_BLOCKS;
    }

    buildHighlightList();
    doHighlighting();

    const int newScrollbarValue = currScrollbarValue - (_startLineNum - currStartLineNum);
    this->verticalScrollBar()->setValue(newScrollbarValue);
}

void PagedFileViewWidget::readPreviousBlock() {
    if(_startLineNum == 0){
        return;
    }

    const int currScrollbarValue = this->verticalScrollBar()->value();
    const unsigned long long currStartLineNum = _startLineNum;
    _startLineNum = _startLineNum >= NUM_LINES_PER_READ ? _startLineNum - NUM_LINES_PER_READ : 0;

    const unsigned long long numLinesToRead = currStartLineNum - _startLineNum;
    if(numLinesToRead == 0){
        return;
    }

    unsigned long long numLinesToDelete = 0;
    const unsigned long long totalFutureNumLines = _endLineNum - _startLineNum + numLinesToRead;
    if(totalFutureNumLines > MAX_NUM_BLOCKS){
        numLinesToDelete = totalFutureNumLines - MAX_NUM_BLOCKS;
    }

    char* lineStart = nullptr;
    unsigned int length;

    //delete lines from end of document
    QTextCursor cursor = this->textCursor();
    cursor.movePosition(QTextCursor::End);

    //delete last empty block
    cursor.movePosition(QTextCursor::StartOfLine, QTextCursor::KeepAnchor);
    cursor.removeSelectedText();
    cursor.deletePreviousChar();

    for(int i = 0; i < numLinesToDelete; i++){
        cursor.movePosition(QTextCursor::StartOfLine, QTextCursor::KeepAnchor);
        cursor.removeSelectedText();
        cursor.deletePreviousChar();
        _endLineNum--;
    }

    //insert new lines
    cursor.movePosition(QTextCursor::Start);
    PLP::LineReaderResult result = _fileReader->getLine(_startLineNum, lineStart, length);
    if(result == PLP::LineReaderResult::SUCCESS){
        cursor.insertText(QString::fromUtf8(lineStart, length).replace("\r",""));

        for(unsigned int i = 0; i < numLinesToRead - 1; i++){
            result = _fileReader->nextLine(lineStart, length);
            if(result != PLP::LineReaderResult::SUCCESS){
                break;
            }
            cursor.insertText(QString::fromUtf8(lineStart, length).replace("\r",""));
        }
    }

    if(result == PLP::LineReaderResult::ERROR){
        QMessageBox::critical(this,"Error","Failed to read block.",QMessageBox::Ok);
        return;
    }

    //insert last empty block
    cursor.movePosition(QTextCursor::End);
    cursor.insertBlock();

    buildHighlightList();
    doHighlighting();

    int newScrollbarValue = currScrollbarValue + (currStartLineNum - _startLineNum);
    this->verticalScrollBar()->setValue(newScrollbarValue);
}

void PagedFileViewWidget::readBlockIfRequired() {
    const int currScrollbarValue = this->verticalScrollBar()->value();
    const unsigned long long currStartLineNum = _startLineNum;

    if(_startLineNum + currScrollbarValue + _numVisibleLines > _endLineNum - (NUM_LINES_PER_READ / 2)){
        readNextBlock();
    } else if(currScrollbarValue < (NUM_LINES_PER_READ / 2)){
        readPreviousBlock();
    }
}

bool PagedFileViewWidget::getLineFromIndex(
        CoreObjPtr<PLP::IndexReaderI>& indexReader,
        QString& data
) {
    char* lineStart = nullptr;
    unsigned int length;
    if(PLP::LineReaderResult::SUCCESS != _fileReader->getLineFromResult(indexReader.get(), lineStart, length)){
        return false;
    }

    data = QString::fromUtf8(lineStart, length).replace("\r","");
    return true;
}

void PagedFileViewWidget::gotoLine(unsigned long long lineNum, bool highlight){
    if(lineNum >= _startLineNum && lineNum < _endLineNum){
        this->verticalScrollBar()->setValue(lineNum - _startLineNum);
        if(highlight){
            setHighlightedLine(lineNum);
        }
        doHighlighting();
        return;
    }

    const unsigned int halfLinesPerRead = NUM_LINES_PER_READ / 2;
    unsigned long long startLine;
    if(lineNum < halfLinesPerRead){
        startLine = 0;
    }else if(lineNum >= _fileReader->getNumberOfLines()) {
        startLine = _fileReader->getNumberOfLines() - NUM_LINES_PER_READ;
    }else if(_fileReader->getNumberOfLines() - lineNum < halfLinesPerRead){
        startLine = _fileReader->getNumberOfLines() - NUM_LINES_PER_READ;
    }else{
        startLine = lineNum - halfLinesPerRead;
    }
    _startLineNum = startLine;
    _endLineNum = startLine;

    this->clear();

    char* lineStart = nullptr;
    unsigned int length;

    QTextCursor cursor = this->textCursor();
    PLP::LineReaderResult result = _fileReader->getLine(_endLineNum, lineStart, length);
    if(result == PLP::LineReaderResult::SUCCESS){
        cursor.movePosition(QTextCursor::End);
        cursor.insertText(QString::fromUtf8(lineStart, static_cast<int>(length)).replace("\r",""));
        _endLineNum++;

        for(unsigned int i = 0; i < NUM_LINES_PER_READ - 1; i++){
            result = _fileReader->nextLine(lineStart, length);
            if(result != PLP::LineReaderResult::SUCCESS){
                break;
            }
            cursor.movePosition(QTextCursor::End);
            cursor.insertText(QString::fromUtf8(lineStart, static_cast<int>(length)).replace("\r",""));
            _endLineNum++;
        }
    }

    if(result == PLP::LineReaderResult::ERROR){
        QMessageBox::critical(this,"Error","Failed to read block.",QMessageBox::Ok);
        return;
    }

    if(_endLineNum - _startLineNum > MAX_NUM_BLOCKS){
        _startLineNum = _endLineNum - MAX_NUM_BLOCKS;
    }
    doHighlighting();
    this->verticalScrollBar()->setValue(lineNum - _startLineNum);

    if(highlight){
        setHighlightedLine(lineNum);
    }
    buildHighlightList();
    doHighlighting();
}

void PagedFileViewWidget::scrollBarMoved(int val) {
    _lineNavBox->setValue(_startLineNum + val);
}

void PagedFileViewWidget::setFontSize(int pointSize) {
    QFont font = this->font();
    font.setPointSize(pointSize);
    this->setFont(font);
}

void PagedFileViewWidget::setHighlightedLine(unsigned long long lineNum) {
    int lineToHighlight = (int)(lineNum - _startLineNum);
    QTextCursor cursor(document()->findBlockByLineNumber(lineToHighlight));

    _indexSelection.lineNumber = lineNum;
    _indexSelection.format.setBackground(Common::LineHighlightBGColor);
    _indexSelection.format.setForeground(Common::LineHighlightTextColor);
    _indexSelection.format.setProperty(QTextFormat::FullWidthSelection, true);
    _indexSelection.cursor = cursor;
    _indexSelection.cursor.clearSelection();
}

void PagedFileViewWidget::showRightClickMenu(const QPoint& pos) {
    QMenu menu(tr("Context menu"), this);

    QAction copy("Copy", this);
    copy.setShortcut(QKeySequence(Qt::CTRL + Qt::Key_C));
    copy.setShortcutVisibleInContextMenu(true);
    connect(&copy, SIGNAL(triggered()), this, SLOT(copySelection()));
    menu.addAction(&copy);

    QAction copyLineNum("Copy Line #", this);
    connect(&copyLineNum, &QAction::triggered, [this, pos](){
        copyLineNumber(pos);
    });
    menu.addAction(&copyLineNum);

    QAction highlight("Highlight", this);
    connect(&highlight, SIGNAL(triggered()), this, SLOT(highlightSelection()));
    menu.addAction(&highlight);

    QAction clearHighlights("Clear highlights", this);
    connect(&clearHighlights, &QAction::triggered, [this](){
        _clearHighlights();
    });
    menu.addAction(&clearHighlights);

    QString selection = this->textCursor().selectedText();
    if(selection.isEmpty()) {
        copy.setEnabled(false);
        highlight.setEnabled(false);
    }

    // disable if multiple lines are selected. Multi-line selection does not work
    if(selection.contains(u'\u2029')){
        highlight.setEnabled(false);
    }

    menu.exec(mapToGlobal(pos));
}

void PagedFileViewWidget::copySelection() {
    QClipboard* clipboard = QApplication::clipboard();
    clipboard->setText(this->textCursor().selectedText());
}

void PagedFileViewWidget::copyLineNumber(const QPoint& pos) {
    QTextCursor cursor = this->cursorForPosition(pos);
    unsigned long long lineNum = _startLineNum + cursor.blockNumber();

    QClipboard* clipboard = QApplication::clipboard();
    clipboard->setText(QString::number(lineNum));
}

void PagedFileViewWidget::highlightSelection() {
    _highlight(this->textCursor().selectedText());
}

void PagedFileViewWidget::buildHighlightList(){
    _highlights.clear();

    int initialPos = firstVisibleBlock().position();
    QList<const HighlightItem*> items = _getHighlightedItems();
    for(const HighlightItem* item : items){
        QTextCursor cursor = this->textCursor();
        cursor.movePosition(QTextCursor::Start);
        this->setTextCursor(cursor);

        if(item->getRegex()){
            QRegExp regex(item->getPattern());
            while(this->find(regex)){
                QTextEdit::ExtraSelection selection;
                selection.format.setBackground(item->getColor());
                selection.format.setForeground(Qt::black);
                selection.cursor = this->textCursor();
                _highlights.push_back(selection);
            }
        }else{
            while(this->find(item->getPattern(), QTextDocument::FindCaseSensitively)){
                QTextEdit::ExtraSelection selection;
                selection.format.setBackground(item->getColor());
                selection.format.setForeground(Qt::black);
                selection.cursor = this->textCursor();
                _highlights.push_back(selection);
            }
        }
    }

    QTextCursor cursor = this->textCursor();
    cursor.setPosition(initialPos);
    this->setTextCursor(cursor);
}

void PagedFileViewWidget::doHighlighting() {
    QList<QTextEdit::ExtraSelection> allHighlights;
    if(_indexSelection.lineNumber >= _startLineNum && _indexSelection.lineNumber <= _endLineNum){
        allHighlights.push_back(_indexSelection);
    }
    if(_mouseOverSelection.lineNumber >= _startLineNum && _mouseOverSelection.lineNumber <= _endLineNum){
        allHighlights.push_back(_mouseOverSelection);
    }
    allHighlights.append(_highlights);

    this->setExtraSelections(allHighlights);
}

void PagedFileViewWidget::onHighlightListUpdated() {
    buildHighlightList();
    doHighlighting();
}
