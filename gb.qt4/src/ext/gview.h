/***************************************************************************

  gview.h

  (c) 2000-2013 Benoît Minisini <gambas@users.sourceforge.net>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2, or (at your option)
  any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
  MA 02110-1301, USA.

***************************************************************************/

#ifndef __GVIEW_H
#define __GVIEW_H

#include <QString>
#include <QColor>
#include <QPixmap>
#include <Q3ScrollView>
#include <QTimer>
#include <QResizeEvent>
#include <QFocusEvent>
#include <QMouseEvent>
#include <QKeyEvent>
#include <QEvent>
#include <QHash>
#include <QStyle>

#include "gdocument.h"
#include "../gb.qt.h"

struct GHighlightStyle
{
	QColor color;
	QColor backgroundColor;
	bool bold;
	bool italic;
	bool underline;
	bool background;
};

struct GFoldedProc
{
	int start;
	int end;
};

class GEditor;

class GEditor : public Q3ScrollView
{
	Q_OBJECT

	friend class GDocument;

private:

	static QPixmap *_cache;
	static QPixmap *_breakpoint;
	static QPixmap *_bookmark;
	static QStyle *_style;
	static int count;

	GDocument *doc;
	QFontMetrics fm;
	int _ytext;
	int largestLine;
	int x, y, xx;
	int nx, ny;
	bool _cursor;
	QTimer *blinkTimer;
	QTimer *scrollTimer;
	int x1m, x2m, y1m, y2m;
	int margin;
	int lineNumberLength;
	bool center;
	bool flashed;
	bool painting;
	GString _showString;
	bool _showStringIgnoreCase;
	int _showRow, _showCol, _showLen;
	bool _posOutside;
	int _cellw, _cellh;
	int _nrows;
	bool _insertMode;
	double *_charWidth;
	double _sameWidth;
	int _tabWidth;
	bool _oddLine;
	QColor _altBackground;
	QColor _oddBackground;
	bool _checkCache;
	bool _border;
	bool _ensureCursorVisibleLater;
	int _firstLineNumber;
	QCursor _saveCursor;
	GString _cutBuffer;
	
	int lastx;
	bool left;
	bool _dblclick;
	GArray<GFoldedProc> fold;
		
	QFont normalFont;
	QFont italicFont;
	GHighlightStyle styles[GLine::NUM_STATE];
	int flags;
	
	int lineLength(int y) const { return doc->lineLength(y); }
	int numLines() const { return doc->numLines(); }
	int visibleLines() const;
	void startBlink();
	void stopBlink();
	bool updateCursor();

	int lineWidth(int y) const;
	int lineWidth(int y, int len);
	void updateWidth(int y = -1);
	void updateMargin();
	void updateHeight();
	void updateCache();
	
	void lineInserted(int y);
	void lineRemoved(int y);
	int findLargestLine();

	void drawTextWithTab(QPainter &p, int sx, int x, int y, const QString &s);
	void paintText(QPainter &p, GLine *l, int x, int y, int xmin, int lmax, int h, int xs1, int xs2, int row, QColor &);
	void paintShowString(QPainter &p, GLine *l, int x, int y, int xmin, int lmax, int h, int row);
	void paintDottedSpaces(QPainter &p, int row, int ps, int ls);
	//void paintEmptyArea(QPainter *p, int cx, int cy, int cw, int ch);
	
	void docTextChanged();
	void redrawContents();
	
	int viewToReal(int row) const;
	int realToView(int row) const;
	int checkCursor(int y);	
	bool isCursorVisible();

	void updateViewport();
	void updateFont();

	int getStringWidth(const QString &s, int len, bool unicode) const;
	
	void updateViewportAttributes();

private slots:

	void blinkTimerTimeout();
	void scrollTimerTimeout();
	//void baptizeVisible();
	//void baptizeVisible(int x, int y);
	void unflash();
	void docTextChangedLater();
	void ensureCursorVisible();

protected:

	virtual void paintCell(QPainter &, int row, int);
	virtual void changeEvent(QEvent *e);
	virtual void keyPressEvent(QKeyEvent *e);
	virtual void mousePressEvent(QMouseEvent *e);
	virtual void mouseReleaseEvent(QMouseEvent *e);
	virtual void mouseMoveEvent(QMouseEvent *e);
	virtual void mouseDoubleClickEvent(QMouseEvent *e);
	virtual void resizeEvent(QResizeEvent *e);
	virtual void focusInEvent(QFocusEvent *);
	virtual void focusOutEvent(QFocusEvent *);
	virtual bool focusNextPrevChild(bool);
	virtual void inputMethodEvent(QInputMethodEvent *e);
	virtual void drawContents(QPainter *p, int cx, int cy, int cw, int ch);
	virtual void viewportResizeEvent(QResizeEvent *e);

public:

	enum Flag
	{
		ShowProcedureLimits = 1,
		DrawWithRelief = 2,
		ShowModifiedLines = 3,
		ShowCurrentLine = 4,
		ShowLineNumbers = 5,
		HighlightBraces = 6,
		HighlightImmediately = 7,
		BlendedProcedureLimits = 8,
		ShowDots = 9,
		ShowCursorPosition = 10,
		ChangeBackgroundAtLimit = 11,
		HideMargin = 12,
		BlinkCursor = 13,
		NoFolding = 14
	};

	static void setBreakpointPixmap(QPixmap *p);
	static void setBookmarkPixmap(QPixmap *p);
	
	GEditor(QWidget *parent);
	~GEditor();
	void reset();

	virtual QVariant inputMethodQuery(Qt::InputMethodQuery property) const;

	void setDocument(GDocument *doc);
	GDocument *getDocument() const { return doc; }

	void getCursor(int *yc, int *xc) const { *yc = y; *xc = x; }
	void insert(QString text);
	bool cursorGoto(int ny, int nx, bool mark);
	void cursorCenter() { center = true; }
	void cursorLeft(bool shift, bool ctrl);
	void cursorRight(bool shift, bool ctrl);
	void cursorUp(bool shift, bool ctrl, bool alt);
	void cursorDown(bool shift, bool ctrl, bool alt);
	void cursorPageUp(bool shift, bool alt);
	void cursorPageDown(bool shift, bool alt);
	void cursorHome(bool shift, bool ctrl, bool alt);
	void cursorEnd(bool shift, bool ctrl, bool alt);
	void newLine();
	void backspace(bool ctrl);
	void del(bool ctrl);
	void copy(bool mouse);
	void copy() { copy(false); }
	void cut();
	void paste(bool mouse);
	void paste() { paste(false); }
	void undo();
	void redo();
	void tab(bool back);
	void selectAll();
	void movePreviousSameIndent(bool shift);
	void moveNextSameIndent(bool shift);
	void expand(bool shift);
	void selectCurrentLine();
	void deleteCurrentLine();

	bool getInsertMode() const { return _insertMode; }
	void setInsertMode(bool mode);
	
	void setStyle(int index, GHighlightStyle *style);
	void getStyle(int index, GHighlightStyle *style) const;
	bool getFlag(int f) const { return flags & (1 << f); }
	void setFlag(int f, bool v);
	bool hasBorder() const { return _border; }
	void setBorder(bool b);
	void setLineOffset(int l);

	int rowAt(int y) const { return y / _cellh; }
	int getLineHeight() const { return _cellh; }
	int getCharWidth(unsigned char c) const { return _charWidth[c]; }
	void cursorToPos(int y, int x, int *px, int *py);
	bool isPosOutside() const { return _posOutside; }
	int posToLine(int py);
	int posToColumn(int y, int px);
	bool posToCursor(int px, int py, int *y, int *x);
	int lastVisibleRow(int y) const { return rowAt(y + visibleHeight() - 1); }
	int lastVisibleRow() const { return lastVisibleRow(contentsY()); }
	void updateLine(int y);
	void setNumRows(int);
	void leaveCurrentLine();
	int lineOffset() const { return _firstLineNumber; }
	
	virtual void resizeContents(int w, int h);
	
	void checkMatching();
	void flash();
	void showString(GString s, bool ignoreCase);
	void showWord(int y, int x, int len);
	
	void foldClear() { fold.clear(); }
	void foldLine(int row, bool no_refresh = false);
	void foldAll();
	void unfoldAll();
	void unfoldLine(int row);
	bool isFolded(int row);
	//bool insideFolded(int row);
	int checkFolded(int row);
	void foldRemove(int y1, int y2 = -1);
	void foldInsert(int y, int n);
	
	bool hasSelection() { return doc->hasSelection(); }
	void getSelection(int *y1, int *x1, int *y2, int *x2) { return doc->getSelection(y1, x1, y2, x2, _insertMode); }
	GString getSelectedText() { return doc->getSelectedText(_insertMode); }
	void hideSelection() { doc->hideSelection(); }

	void saveCursor();

signals:

	void cursorMoved();
	void textChanged();
	void marginClicked(int);
	void marginDoubleClicked(int);
};

#endif
