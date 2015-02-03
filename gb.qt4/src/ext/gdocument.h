/***************************************************************************

  gdocument.h

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

#ifndef __GDOCUMENT_H
#define __GDOCUMENT_H

#include "garray.h"
#include "gstring.h"

struct GHighlight
{
	unsigned state : 5;
	unsigned alternate : 1;
	unsigned len : 10;
};

#define HIGHLIGHT_LEN_MAX 1023

typedef
	GHighlight *GHighlightArray;

class GLine
{
public:

	enum
	{
		Background = 0, Normal, Keyword, Subr,
		Operator, Symbol, Number, String,
		Comment, Breakpoint, Current, Datatype,
		Selection, Highlight, Line, Error, 
		Help, Preprocessor,
		NUM_STATE
	};

	enum
	{
		BookmarkFlag = 0,
		BreakpointFlag = 1
	};

	GString s;
	GHighlightArray highlight;
	unsigned state : 5;
	unsigned alternate : 1;
	unsigned modified : 1;
	unsigned changed : 1;
	unsigned saved : 1;
	unsigned flag : 2;
	unsigned proc : 1;
	unsigned unicode : 1;
	unsigned tab : 1;
	unsigned baptized : 1;
	unsigned nobreak : 1;
	signed tag : 16;

	GLine();
	~GLine();
	void insert(uint pos, const GString &text);
	bool isEmpty() const;
};

class GCommand;
class GEditor;

typedef
	void (*GHighlightCallback)(GEditor *master, int line, uint &state, bool &alternate, int &tag, GString &s, GHighlightArray *highlight, bool &proc);


class GDocument
{
private:

	static int convState(int state);

	GArray<GCommand> undoList;
	GArray<GCommand> redoList;
	int undoLevel;
	int highlightMode;
	GHighlightCallback highlightCallback;
	uint oldCount;
	GEditor *selector;
	GEditor *_currentView;
	GString _eol;
	int xs, ys, xs2, ys2;
	int tabWidth;
	int colorizeFrom;
	int _disableColorize;
	int _disableColorizeStart;
	int _currentLine;
	unsigned _eol_mode : 2;
	unsigned readOnly : 1;
	unsigned blockUndo : 1;
	unsigned keywordsUseUpperCase : 1;
	unsigned textHasChanged : 1;

	void init();
	void clearUndo();
	void addUndo(GCommand *);
	void addRedo(GCommand *);
	void updateViews(int row = -1, int count = 1);
	void updateMargin();
	void updateContents();

public:

	enum GHighlightMode
	{
		None = 0,
		Gambas = 1,
		Custom = 2
	};

	static void highlightGambas(GEditor *master, int line, uint &state, bool &alternate, int &tag, GString &s, GHighlightArray *data, bool &proc);

	GArray<GLine> lines;
	GArray<GEditor> views;
	int xAfter, yAfter;

	GDocument();
	~GDocument();

	void clear();
	void reset(bool saved);
	
	void setCurrentView(GEditor *view) { _currentView = view; }
	GEditor *currentView() const { return _currentView; }
	
	GString getText();
	void setText(const GString & text);
	int getLength() const;

	int getEndOfLine() const { return _eol_mode; }
	void setEndOfLine(int mode);
	GString getEndOfLineText() const { return _eol; }

	void subscribe(GEditor *view);
	void unsubscribe(GEditor *view);

	void setReadOnly(bool ro) { readOnly = ro; };
	bool isReadOnly() const { return readOnly; }

	void setTabWidth(int tw) { tabWidth = tw; }
	int getTabWidth() const { return tabWidth; }

	GString getLine(int y) const;
	void setLine(int y, GString & str);
	
	int currentLine() const { return _currentLine; }
	void setCurrentLine(int y);

	bool getLineFlag(int y, int f) const;
	void setLineFlag(int y, int f, bool b);
	
	bool isLineEditedSomewhere(int y);

	int lineLength(int y) const { return lines.at(y)->s.length(); }
	int numLines() const { return lines.count(); }
	
	bool hasLimit(int y) { colorize(y); return lines.at(y)->proc; }
	int getLimitIndex(int y);
	
	void getState(int y, bool colorize, uint &state, int &tag, bool &alternate);
	int getCharState(int y, int x);
	
	int getNextLimit(int y);
	int getPreviousLimit(int y);

	void insert(int y, int x, const GString &str, bool doNotMove = false);
	void remove(int y, int x, int y2, int x2);

	bool undo();
	bool redo();
	void begin(bool linked = false);
	void end(bool linked = false);

	void setHighlightMode(int mode, GHighlightCallback cb = 0);
	int getHighlightMode() const { return highlightMode; }
	
	void setKeywordsUseUpperCase(bool v);
	bool isKeywordsUseUpperCase() const { return keywordsUseUpperCase; }
	
	void colorize(int y, bool force = false);
	void colorizeAll();
	void invalidate(int y);

	int getIndent(int y, bool *empty = NULL);
	int wordLeft(int y, int x, bool word = false);
	int wordRight(int y, int x, bool word = false);

	bool hasSelection() { return selector != NULL && (ys != ys2 || xs != xs2); }
	bool hasSelection(GEditor *view) { return hasSelection() && selector == view; }
	void getSelection(int *y1, int *x1, int *y2, int *x2, bool insertMode);
	GString getSelectedText(bool insertMode);

	void startSelection(GEditor *view, int y, int x);
	void endSelection(int y, int x);
	void hideSelection();
	void eraseSelection(bool insertMode);

	bool insideUndo() const { return undoLevel; }

	void emitTextChanged();

private:

	void checkSelection();
	void updateLineWidth(int y);
	void insertLine(int y);
	void removeLine(int y);
	void modifyLine(GLine *l, int y);
	void disableColorize();
	void enableColorize();
};

#endif
