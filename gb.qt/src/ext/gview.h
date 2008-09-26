/***************************************************************************

  gview.h

  (c) 2000-2007 Benoit Minisini <gambas@users.sourceforge.net>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 1, or (at your option)
  any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

***************************************************************************/

#ifndef __GVIEW_H
#define __GVIEW_H

#include <qstring.h>
#include <qcolor.h>
#include <qpixmap.h>
#include <qgridview.h>
#include <qtimer.h>

#include "gdocument.h"
#include "../gb.qt.h"

struct GHighlightStyle
{
  QColor color;
  bool bold;
  bool italic;
  bool underline;
};

class GEditor : public QGridView
{
  Q_OBJECT

  friend class GDocument;

private:

  static QPixmap *cache;
  static QPixmap *breakpoint;
  static int count;

  GDocument *doc;
  //int charWidth;
  QFontMetrics fm;
  int largestLine;
  int x, y, xx;
  int nx, ny;
  bool cursor;
  QTimer *blinkTimer;
  QTimer *scrollTimer;
  int x1m, x2m, ym;
  int margin;
  int lineNumberLength;
  bool center;
  bool flashed;
  bool painting;
  
  int lastx;
  bool left;
  
	QFont italicFont;
  GHighlightStyle styles[GLine::NUM_STATE];
  int flags;
  QPixmap pattern;

  int lineLength(int y) const { return doc->lineLength(y); }
  int numLines() const { return doc->numLines(); }
  void startBlink();
  void stopBlink();
  void updateLine(int y) { updateCell(y, 0); }
  void repaintLine(int y) { repaintCell(y, 0, FALSE); }
  bool updateCursor();
  //void updatePattern();

  int lineWidth(int y) const;
  int lineWidth(int y, int len) const;
  void updateWidth(int y);
  void updateMargin();
  void updateHeight();
  void updateCache();
  
  void lineInserted(int y);
  void lineRemoved(int y);
  int findLargestLine();

  void paintText(QPainter &p, GLine *l, int x, int y, int xmin, int lmax, int row);
  void paintDottedSpaces(QPainter &p, int row, int ps, int ls);

	void redrawContents();
	
	void docTextChanged();

	bool isCursorVisible() const;

private slots:

  void blinkTimerTimeout();
  void scrollTimerTimeout();
	void baptizeVisible();
	void baptizeVisible(int x, int y);
	void unflash();
	void docTextChangedLater();
	void ensureCursorVisible();

protected:

  virtual void paintCell(QPainter *, int row, int);
  virtual void fontChange(const QFont &oldFont);
  virtual void keyPressEvent(QKeyEvent *e);
  virtual void mousePressEvent(QMouseEvent *e);
  virtual void mouseReleaseEvent(QMouseEvent *e);
  virtual void mouseMoveEvent(QMouseEvent *e);
  virtual void mouseDoubleClickEvent(QMouseEvent *e);
  virtual void resizeEvent(QResizeEvent *e);
  virtual void focusInEvent(QFocusEvent *);
  virtual void focusOutEvent(QFocusEvent *);
	virtual void imStartEvent(QIMEvent *e);
	virtual void imComposeEvent(QIMEvent *e);
	virtual void imEndEvent(QIMEvent *e);
  virtual bool focusNextPrevChild(bool);

public:

  enum Flag
  {
    ShowProcedureLimits = 1,
    DrawWithRelief = 2,
    ShowModifiedLines = 3,
    ShowCurrentLine = 4,
    ShowLineNumbers = 5,
    HighlightBraces = 6,
    HighlightCurrent = 7,
    BlendedProcedureLimits = 8
  };

	static void setBreakpointPixmap(QPixmap *p);
	
  GEditor(QWidget *parent);
  ~GEditor();

  void setDocument(GDocument *doc);
  GDocument *getDocument() const { return doc; }

  void getCursor(int *yc, int *xc) const { *yc = y; *xc = x; }
  void insert(QString text);
  bool cursorGoto(int ny, int nx, bool mark);
  void cursorCenter() { center = true; }
  void cursorLeft(bool shift, bool ctrl);
  void cursorRight(bool shift, bool ctrl);
  void cursorUp(bool shift, bool ctrl);
  void cursorDown(bool shift, bool ctrl);
  void cursorPageUp(bool mark);
  void cursorPageDown(bool mark);
  void cursorHome(bool shift, bool ctrl);
  void cursorEnd(bool shift, bool ctrl);
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

  void setStyle(int index, GHighlightStyle *style);
  void getStyle(int index, GHighlightStyle *style);
  bool getFlag(int f) const { return flags & (1 << f); }
  void setFlag(int f, bool v);

  int getLineHeight() const { return cellHeight(); }
  int getCharWidth() const;
  void cursorToPos(int y, int x, int *px, int *py) const;
  int posToLine(int py) const;
  int posToColumn(int y, int px) const;
  void posToCursor(int px, int py, int *y, int *x) const;
	int lastVisibleRow(int y) const { return rowAt(y + visibleHeight() - 1); }
	int lastVisibleRow() const { return lastVisibleRow(contentsY()); }

	virtual void setNumRows(int);

  void checkMatching();
	void flash();
  
signals:

  void cursorMoved();
  void textChanged();
  void marginClicked(int);
  void marginDoubleClicked(int);
};

#endif
