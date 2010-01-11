/***************************************************************************

  gdocument.h

  (c) 2000-2009 Benoît Minisini <gambas@users.sourceforge.net>

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
  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

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
		Alternate, Help,
		NUM_STATE
	};

	enum
	{
		CurrentFlag = 0,
		BreakpointFlag = 1
	};

	GString s;
	GHighlightArray highlight;
	unsigned state : 5;
	unsigned alternate : 1;
	unsigned modified : 1;
	unsigned changed : 1;
	unsigned flag : 2;
	unsigned proc : 1;
	unsigned baptized : 1;
	unsigned _reserved : 4;
	signed tag : 16;

	GLine();
	~GLine();
};

class GCommand;
class GEditor;

typedef
	void (*GHighlightCallback)(GEditor *master, uint &state, bool &alternate, int &tag, GString &s, GHighlightArray *highlight, bool &proc);


class GDocument
{
private:

	static int convState(int state);

	GArray<GCommand> undoList;
	GArray<GCommand> redoList;
	int undoLevel;
	bool textHasChanged;
	int highlightMode;
	bool keywordsUseUpperCase;
	GHighlightCallback highlightCallback;
	uint oldCount;
	bool readOnly;
	bool blockUndo;
	GEditor *selector;
	int xs, ys, xs2, ys2;
	int tabWidth;
	int baptismLimit;

	void init();
	void clearUndo();
	void addUndo(GCommand *);
	void addRedo(GCommand *);
	void updateViews(int row = -1, int count = 1);

public:

	enum GHighlightMode
	{
		None = 0,
		Gambas = 1,
		Custom = 2
	};

	static void highlightGambas(GEditor *master, uint &state, bool &alternate, int &tag, GString &s, GHighlightArray *data, bool &proc);

	GArray<GLine> lines;
	GArray<GEditor> views;
	int xAfter, yAfter;

	GDocument();
	~GDocument();

	void clear();
	void reset();
	GString getText();
	void setText(const GString & text);
	int getLength() const;

	void subscribe(GEditor *view);
	void unsubscribe(GEditor *view);

	void setReadOnly(bool ro) { readOnly = ro; };
	bool isReadOnly() const { return readOnly; }

	void setTabWidth(int tw) { tabWidth = tw; }
	int getTabWidth() const { return tabWidth; }

	GString getLine(int y) const;
	void setLine(int y, GString & str);

	bool getLineFlag(int y, int f) const;
	void setLineFlag(int y, int f, bool b);

	int lineLength(int y) const { return lines.at(y)->s.length(); }
	int numLines() const { return lines.count(); }
	
	bool hasLimit(int y) { colorize(y); return lines.at(y)->proc; }
	
	void getState(int y, bool colorize, uint &state, int &tag, bool &alternate);
	
	int getNextLimit(int y);
	int getPreviousLimit(int y);

	void insert(int y, int x, const GString & str);
	void remove(int y, int x, int y2, int x2);

	bool undo();
	bool redo();
	void begin(bool linked = false);
	void end(bool linked = false);

	void setHighlightMode(int mode, GHighlightCallback cb = 0);
	int getHighlightMode() const { return highlightMode; }
	
	void setKeywordsUseUpperCase(bool v);
	bool isKeywordsUseUpperCase() const { return keywordsUseUpperCase; }
	
	void colorize(int y);
	void baptizeUntil(int y);
	void invalidate(int y);

	int getIndent(int y, bool *empty = NULL);
	int wordLeft(int y, int x, bool word = false);
	int wordRight(int y, int x, bool word = false);

	GString getSelectedText() const;

	bool hasSelection() const { return selector != NULL; }
	bool hasSelection(GEditor *view) const { return selector == view; }
	void getSelection(int *y1, int *x1, int *y2, int *x2) const;
	void startSelection(GEditor *view, int y, int x);
	void endSelection(int y, int x);
	void hideSelection();
	void eraseSelection();

	void emitTextChanged();

private:

	void updateLineWidth(int y);
	void insertLine(int y);
	void removeLine(int y);
};

#endif
