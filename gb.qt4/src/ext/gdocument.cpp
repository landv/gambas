/***************************************************************************

  gdocument.cpp

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

#define __G_DOCUMENT_CPP

#include <ctype.h>

#include "main.h"
#include "gview.h"
#include "gdocument.h"

//#define DEBUG_UNDO 1

#define GMAX(x, y) ((x) > (y) ? (x) : (y))
#define GMIN(x, y) ((x) < (y) ? (x) : (y))

#define COLORIZE_FROM(_y) \
	if (colorizeFrom > (_y)) \
	{ \
		colorizeFrom = (_y); \
		/*fprintf(stderr, "colorizeFrom -> %d\n", (_y));*/ \
	}

/**---- GLine -----------------------------------------------------------*/

GLine::GLine()
{
	s = "";
	state = Normal;
	alternate = false;
	modified = false;
	changed = false;
	saved = false;
	flag = 0;
	tag = 0;
	proc = false;
	unicode = false;
	highlight = NULL;
	nobreak = false;
	baptized = false;
	tab = false;
}

GLine::~GLine()
{
	GB.FreeArray(&highlight);
}

void GLine::insert(uint pos, const GString &text)
{
	s.insert(pos, text);
	if (text.hasUnicode())
		unicode = true;
}

bool GLine::isEmpty() const
{
	uint i;
	
	for (i = 0; i < s.length(); i++)
	{
		if (!s.isSpace(i))
			return false;
	}
	
	return true;
}

/**---- GCommand -----------------------------------------------------------*/

struct GCommandDocument
{
public:
	GEditor *view;
	int cx;
	int cy;
	int sx;
	int sy;
	int sx2;
	int sy2;
	
	GCommandDocument() { }
	GCommandDocument(GDocument *doc);
	void process(GDocument *doc) const;
	void print() const;
	bool equals(GCommandDocument *o) const;
};

GCommandDocument::GCommandDocument(GDocument *doc)
{
	view = doc->currentView();
	
	view->getCursor(&cy, &cx);
	
	if (doc->hasSelection())
		doc->getSelection(&sy, &sx, &sy2, &sx2, false); // TODO: store insertMode
	else
		sx = sy = sx2 = sy2 = -1;
}

void GCommandDocument::process(GDocument *doc) const
{
	//qDebug("goto %d %d", cx, cy);
	view->cursorGoto(cy, cx, false);
	if (sx >= 0)
	{
		//qDebug("select %d %d %d %d", sx, sy, sx2, sy2);
		doc->startSelection(view, sy, sx);
		doc->endSelection(sy2, sx2);
	}
}

void GCommandDocument::print() const
{
	qDebug("- %d %d [%d %d %d %d]", cx, cy, sx, sy, sx2, sy2);
}

bool GCommandDocument::equals(GCommandDocument *o) const
{
	return view == o->view && cx == o->cx && cy == o->cy 
		&& sx == o->sx && sy == o->sy && sx2 == o->sx2 && sy2 == o->sy2;
}

class GCommand
{
public:
	enum Type
	{
		None, Begin, End, Move, Insert, Delete, Indent, Unindent
	};
	GCommandDocument info;

	virtual ~GCommand() { }
	virtual Type type() const { return None; }
	virtual int nest() const { return 0; }
	virtual void print() const { }
	virtual bool merge(GCommand *) const { return false; }
	virtual void process(GDocument *doc, bool undo) const { }
	virtual bool linked() const { return false; }
	virtual bool remove(GCommand *) const { return false; }
};

class GBeginCommand: public GCommand
{
public:
	bool _linked;
	
	GBeginCommand(GCommandDocument *info, bool linked = false) { _linked = linked; this->info = *info; }
	Type type() const { return Begin; }
	void print() const { qDebug("Begin"); info.print(); }
	int nest() const { return 1; }
	bool linked() const { return _linked; }
	void process(GDocument *doc, bool undo) const { info.process(doc); }
};

class GEndCommand: public GCommand
{
public:
	bool _linked;

	GEndCommand(bool linked = false) { _linked = linked; }
	Type type() const { return End; }
	void print() const { qDebug("End"); }
	int nest() const { return -1; }
	bool linked() const { return _linked; }
	bool remove(GCommand *o) const { return (o->type() == Begin); }
};

class GDeleteCommand: public GCommand
{
public:
	int x, y, x2, y2;
	GString str;

	GDeleteCommand(GCommandDocument *info, int y, int x, int y2, int x2, const GString &str)
	{
		this->x = x; this->y = y; this->x2 = x2; this->y2 = y2; this->str = str;
		this->info = *info;
	}
	
	Type type() const { return Delete; }
	void print() const { qDebug("Delete: (%d %d)-(%d %d): '%s'", x, y, x2, y2, str.utf8()); info.print(); }

	bool merge(GCommand *c) const
	{
		if (c->type() != type())
			return false;

		GDeleteCommand *o = (GDeleteCommand *)c;
		
		if (info.view != o->info.view)
			return false;

		if (x2 != o->x || y2 != o->y || o->y != o->y2)
			return false;

		o->str.prepend(str);
		o->x = x;
		o->y = y;
		//o->info = info;
		return true;
	}

	void process(GDocument *doc, bool undo) const 
	{
		if (undo)
			doc->insert(y, x, str);
		else
			doc->remove(y, x, y2, x2);
		
		info.process(doc);
	}
};

class GInsertCommand: public GDeleteCommand
{
public:
	GInsertCommand(GCommandDocument *info, int y, int x, int y2, int x2, const GString &str): 
		GDeleteCommand(info, y, x, y2, x2, str) {}
	Type type() const { return Insert; }
	void print() const { qDebug("Insert: (%d %d)-(%d %d): '%s'", x, y, x2, y2, str.utf8()); info.print(); }

	bool merge(GCommand *c) const
	{
		if (c->type() != type())
			return false;

		if (str.length() && str.isNewLine(0))
			return false;

		GInsertCommand *o = (GInsertCommand *)c;

		if (o->info.view != info.view)
			return false;
		
		if (o->str.length() && o->str.isNewLine(str.length() - 1))
			return false;

		if (x != o->x2 || y != o->y2)
			return false;

		o->str += str;
		o->x2 = x2;
		o->y2 = y2;
		return true;
	}

	void process(GDocument *doc, bool undo) const
	{
		GDeleteCommand::process(doc, !undo);
	}
};


/**---- GDocument -----------------------------------------------------------*/

#define FOR_EACH_VIEW(_view) \
	for (GEditor *_view = views.first(); _view ; _view = views.next())

void GDocument::init()
{
	selector = NULL;
	colorizeFrom = 0;
	_disableColorize = 0;
	_currentLine = -1;
}

GDocument::GDocument()
{
	oldCount = 0;
	blockUndo = false;
	tabWidth = 2;
	readOnly = false;
	highlightMode = None;
	keywordsUseUpperCase = false;
	_currentView = NULL;
	setEndOfLine(GB_EOL_UNIX);

	//lines = new GArray<GLine>;
	lines.setAutoDelete(true);

	undoList.setAutoDelete(true);
	redoList.setAutoDelete(true);

	clear();
	//setText(""); //GString("Gambas\nAlmost\nMeans\nBASIC!\n"));
}

GDocument::~GDocument()
{
}

void GDocument::clear()
{
	uint i;

	clearUndo();
	lines.clear();
	lines.append(new GLine);
	init();

	updateViews();

	for (i = 0; i < views.count(); i++)
		views.at(i)->reset();
}


void GDocument::reset(bool saved)
{
	if (saved)
	{
		for (uint i = 0; i < lines.count(); i++)
		{
			lines.at(i)->saved |= lines.at(i)->changed;
			lines.at(i)->changed = false;
		}
	}
	else
	{
		for (uint i = 0; i < lines.count(); i++)
			lines.at(i)->saved = lines.at(i)->changed = false;
	}

	updateViews();
}

GString GDocument::getText()
{
	GString tmp;

	if (lines.count())
	{
		for (uint i = 0; i < (lines.count() - 1); i++)
		{
			tmp += lines.at(i)->s;
			tmp += _eol;
		}

		tmp += lines.at(lines.count() - 1)->s;

		updateViews();
	}

	return tmp;
}

GString GDocument::getSelectedText(bool insertMode)
{
	GString tmp, line;
	int x1, y1, x2, y2, i;

	if (lines.count() && hasSelection())
	{
		getSelection(&y1, &x1, &y2, &x2, insertMode);
		
		if (insertMode)
		{
			for (i = y1; i <= y2; i++)
			{
				line = lines.at(i)->s.mid(x1, x2 - x1);
				while ((int)line.length() < (x2 - x1))
					line.append(' ');
				
				tmp += line;
				if (i < y2)
					tmp += _eol;
			}
		}
		else
		{
			if (y1 == y2)
				tmp = lines.at(y1)->s.mid(x1, x2 - x1);
			else
			{
				tmp = lines.at(y1)->s.mid(x1);
				tmp += _eol;
				for (i = y1 + 1; i < y2; i++)
				{
					tmp += lines.at(i)->s;
					tmp += _eol;
				}

				tmp += lines.at(y2)->s.left(x2);
			}
		}
	}

	return tmp;
}

int GDocument::getIndent(int y, bool *empty)
{
	int i;
	GString s = lines.at(y)->s;
	bool e = true;

	for (i = 0; i < (int)s.length(); i++)
	{
		if (!s.isSpace(i))
		{
			e = false;
			break;
		}
	}

	if (empty)
		*empty = e;

	return i;
}

void GDocument::modifyLine(GLine *l, int y)
{
	//if (!l->modified)
	//	fprintf(stderr, "modifyLine: %d\n", y);
	l->modified = l->changed = true;
	updateLineWidth(y);
	COLORIZE_FROM(y);
}

void GDocument::updateLineWidth(int y)
{
	FOR_EACH_VIEW(v)
	{
		v->updateWidth(y);
	}
}

void GDocument::insertLine(int y)
{
	lines.insert(y, new GLine());
	modifyLine(lines.at(y), y);
	FOR_EACH_VIEW(v) { v->lineInserted(y); }
}

void GDocument::insert(int y, int x, const GString &text, bool doNotMove)
{
	int pos;
	int len;
	int pos2;
	int xs, ys;
	GLine *l;
	int n = 1;
	int nl = 0;
	GString rest;
	int yy;
	int i, ns;
	GCommandDocument info(this);

	if (readOnly || text.length() == 0)
	{
		xAfter = x;
		yAfter = y;
		return;
	}

	FOR_EACH_VIEW(v)
	{
		v->nx = v->x;
		v->ny = v->y;
	}

	disableColorize();
	
	while (y >= (int)lines.count())
	{
		yy = (int)lines.count();
		insertLine(yy);
		nl++;
	}

	l = lines.at(y);
	
	ns = x - (int)l->s.length();
	if (ns > 0)
	{
		GString str;
		
		for (i = 0; i < ns; i++)
			str.append(' ');
		
		insert(y, x - ns, str, doNotMove);
	}

	xs = x;
	ys = y;
	pos = 0;

	for(;;)
	{
		pos2 = text.findNextLine(pos, len);

		l = lines.at(y);

		if (len > 0)
		{
			l->insert(x, text.mid(pos, len));
			modifyLine(l, y);

			//maxLength = GMAX(maxLength, (int)l->s.length());
			//updateLineWidth(y);

			FOR_EACH_VIEW(v)
			{
				if (v->ny == y && v->nx >= x)
					v->nx += len;
			}

			x += len;
		}

		pos = pos2;

		if (pos == 0)
			break;

		if (x < (int)l->s.length())
		{
			rest = l->s.mid(x);

			l->s.remove(x, rest.length());
			modifyLine(l, y);
		}

		FOR_EACH_VIEW(v)
		{
			if (v->ny >= y)
				v->ny++;
		}

		y++;

		insertLine(y);
		nl++;

		n = -1;
		x = 0;

	}

	if (n < 0 && rest.length())
	{
		l->insert(x, rest);
		modifyLine(l, y);
		//maxLength = GMAX(maxLength, (int)l->s.length());
		//updateLineWidth(y);
	}

	FOR_EACH_VIEW(v)
	{
		v->foldInsert(ys, nl);
	}
	
	begin();
	addUndo(new GInsertCommand(&info, ys, xs, y, x, text));
	enableColorize();
	end();

	updateViews(ys, n);

	yAfter = y;
	xAfter = x;

	emitTextChanged();

	if (!doNotMove)
	{
		FOR_EACH_VIEW(v)
		{
			v->cursorGoto(v->ny, v->nx, FALSE);
		}
	}
}

void GDocument::removeLine(int y)
{
	lines.remove(y);
	COLORIZE_FROM(y);
	FOR_EACH_VIEW(v) { v->lineRemoved(y); }
}

void GDocument::remove(int y1, int x1, int y2, int x2)
{
	GLine *l, *l2;
	GString text, rest;
	int y;
	GCommandDocument info(this);

	yAfter = y1;
	xAfter = x1;

	if (readOnly)
		return;
	
	FOR_EACH_VIEW(v)
	{
		v->nx = v->x;
		v->ny = v->y;
	}
	
	disableColorize();
	
	l = lines.at(y1);

	if (y1 == y2)
	{
		if (x2 >= x1)
		{
			text = l->s.mid(x1, x2 - x1);

			l->s.remove(x1, x2 - x1);
			if (text.hasUnicode())
				l->unicode = l->s.hasUnicode();

			modifyLine(l, y1);

			FOR_EACH_VIEW(v)
			{
				v->foldRemove(y1);
				if (v->ny == y1 && v->nx > x1)
					v->nx = GMAX(x1, v->nx - (x2 - x1));
			}

			updateViews(y1);
		}
	}
	else
	{
		l2 = lines.at(y2);
		text = l->s.mid(x1) + _eol;
		rest = l2->s.left(x2);

		if (x1 < (int)l->s.length() || x2 < (int)l2->s.length())
		{
			l->s = l->s.left(x1) + lines.at(y2)->s.mid(x2);
			l->state = 0; // force highlighting of next line.
			modifyLine(l, y1);
		}
		else
			updateLineWidth(y1);

		for (y = y1 + 1; y < y2; y++)
			text += lines.at(y)->s + _eol;
		text += rest;

		for (y = y1 + 1; y <= y2; y++)
		{
			removeLine(y1 + 1);
		}

		FOR_EACH_VIEW(v)
		{
			v->foldRemove(y1 + 1, y2);
			if (v->ny > y1)
			{
				v->ny = GMAX(y1, v->ny - (y2 - y1));
				if (v->ny == y1)
					v->nx = x1;
			}
			else if (v->ny == y1 && v->nx > x1)
				v->nx = x1;
		}

		updateViews(y1, -1);
	}

	begin();
	addUndo(new GDeleteCommand(&info, y1, x1, y2, x2, text));
	enableColorize();
	end();

	FOR_EACH_VIEW(v)
	{
		v->cursorGoto(v->ny, v->nx, false);
	}
	
	emitTextChanged();
}

void GDocument::setText(const GString &text)
{
	bool oldReadOnly = readOnly;
	int mode;
	uint l;

	readOnly = false;
	blockUndo = true;
	
	clear();
	clearUndo();

	mode = GB_EOL_UNIX;

	l = text.length();
	for (uint i = 0; i < l; i++)
	{
		char c = text.at(i);
		if (c == '\n')
		{
			break;
		}
		if (c == '\r')
		{
			if ((i < (l - 1)) && text.at(i + 1) == '\n')
				mode = GB_EOL_WINDOWS;
			else
				mode = GB_EOL_MAC;
			break;
		}
	}

	setEndOfLine(mode);

	undoLevel++; // Prevent textChanged emission
	insert(0, 0, text, true);
	reset(false);
	undoLevel--;

	blockUndo = false;
	readOnly = oldReadOnly;

	FOR_EACH_VIEW(v)
	{
		v->cursorGoto(0, 0, false);
	}
	
	emitTextChanged();
}

void GDocument::unsubscribe(GEditor *view)
{
	//qDebug("unsubscribe %p -> %p", view, this);
	views.removeRef(view);
	
	if (views.count() == 0)
		delete this;
	else
	{
		if (_currentView == view)
			_currentView = views.at(0);
	}
}

void GDocument::subscribe(GEditor *view)
{
	//qDebug("subscribe %p -> %p", view, this);
	views.removeRef(view);
	views.append(view);
	view->setNumRows(lines.count());
	view->updateContents();
	
	if (!_currentView)
		_currentView = view;
}

void GDocument::updateViews(int row, int count)
{
	uint i;

	if (lines.count() > oldCount)
	{
		oldCount = lines.count();
		FOR_EACH_VIEW(v)
		{
			v->setNumRows(oldCount);
			v->updateHeight();
		}
	}

	/*if (maxLength != oldMaxLength)
	{
		oldMaxLength = maxLength;
		FOR_EACH_VIEW(v)
			v->updateLength();
	}*/

	if (row < 0)
	{
		row = 0;
		count = oldCount;
	}
	else if (count < 0)
	{
		count = oldCount - row;
	}

	count = GMIN((int)oldCount - row, count);

	if (((row + count) < numLines()) && lines.at(row + count)->proc)
		count++;
	
	FOR_EACH_VIEW(v)
	{
		for (i = row; i < (uint)(row + count); i++)
		{
			v->updateLine(i);
		}
	}

	if (lines.count() < oldCount)
	{
		oldCount = lines.count();
		FOR_EACH_VIEW(v)
		{
			v->setNumRows(oldCount);
			v->updateHeight();
		}
	}

	FOR_EACH_VIEW(v)
		v->checkMatching();
}


void GDocument::getSelection(int *y1, int *x1, int *y2, int *x2, bool insertMode)
{
	if (!selector)
		return;

	if (ys2 >= numLines())
	{
		ys2 = numLines() - 1;
		if (!insertMode)
			xs2 = lineLength(ys2);
	}
	
	if (ys >= numLines())
	{
		ys = numLines() - 1;
		xs = lineLength(ys);
	}
	
	if (ys2 > ys || (ys2 == ys && xs2 > xs))
	{
		*y1 = ys;
		*y2 = ys2;
		if (x1) *x1 = xs;
		if (x2) *x2 = xs2;
	}
	else
	{
		*y1 = ys2;
		*y2 = ys;
		if (x1) *x1 = xs2;
		if (x2) *x2 = xs;
	}
	
	if (!insertMode)
	{
		*x1 = QMIN(*x1, lineLength(*y1));
		*x2 = QMIN(*x2, lineLength(*y2));
	}
}

void GDocument::startSelection(GEditor *view, int y, int x)
{
	hideSelection();
	ys = y;
	xs = x;
	ys2 = ys;
	xs2 = xs;
	selector = view;
	updateViews(y);
}

void GDocument::endSelection(int y, int x)
{
	int y1a, y2a, y1b, y2b;

	getSelection(&y1a, NULL, &y2a, NULL, true);
	ys2 = y;
	xs2 = x;
	getSelection(&y1b, NULL, &y2b, NULL, true);

	/*if (y1a == y1b)
		updateViews(GMIN(y2a, y2b), GMAX(y2a, y2b) - GMIN(y2a, y2b) + 1);
	else if (y2a == y2b)
		updateViews(GMIN(y1a, y1b), GMAX(y1a, y1b) - GMIN(y1a, y1b) + 1);
	else*/
		updateViews(GMIN(y1a, y1b), GMAX(y2a, y2b) - GMIN(y1a, y1b) + 1);

	updateViews(y);
}

void GDocument::hideSelection()
{
	int y1, y2;

	if (!selector)
		return;

	getSelection(&y1, NULL, &y2, NULL, true);
	selector = NULL;
	updateViews(y1, y2 - y1 + 1);
}

void GDocument::eraseSelection(bool insertMode)
{
	int y1, y2, x1, x2, i;

	if (!selector)
		return;

	begin();
	getSelection(&y1, &x1, &y2, &x2, false);
	selector = NULL;
	if (!insertMode)
		remove(y1, x1, y2, x2);
	else
	{
		for (i = y1; i <= y2; i++)
			remove(i, x1, i, x2);
	}
	end();
}

void GDocument::clearUndo()
{
	undoList.clear();
	redoList.clear();
	undoLevel = 0;
}

void GDocument::addUndo(GCommand *c)
{
	if (blockUndo)
		return;
	
	#if DEBUG_UNDO
		fprintf(stderr, "addUndo: ");
		c->print();
	#endif

	if (!undoList.isEmpty())
	{
		if (c->merge(undoList.last()))
		{
			#if DEBUG_UNDO
			qDebug("         MERGE");
			#endif
			delete c;
			return;
		}
		else if (c->remove(undoList.last()))
		{
			#if DEBUG_UNDO
			qDebug("         REMOVE");
			#endif
			delete c;
			delete undoList.take();
			return;
		}
	}
	
	if (c->type() == GCommand::End && undoList.count() >= 2 && undoList.at(undoList.count() - 2)->type() == GCommand::Begin)
	{
		GCommand *o = undoList.take();
		GCommand *begin = undoList.take();
		#if DEBUG_UNDO
		qDebug("         NO BEGIN / END");
		#endif
		o->info = begin->info;
		delete begin;
		delete c;
		addUndo(o);
		return;
		/*else
		{
			undoList.append(begin);
			undoList.append(o);
		}*/
	}

	undoList.append(c);

	if (!redoList.isEmpty())
	{
		redoList.clear();
		//emit redoAvailable(false);
	}
}

void GDocument::begin(bool linked)
{
	if (undoLevel == 0)
		textHasChanged = false;
	undoLevel++;  
	if (!blockUndo && (undoLevel == 1))
	{
		GCommandDocument info(this);
		addUndo(new GBeginCommand(&info, linked));
	}
}

void GDocument::end(bool linked)
{
	undoLevel--;
	if (!blockUndo && undoLevel == 0) addUndo(new GEndCommand(linked));
	if (undoLevel == 0 && textHasChanged)
		emitTextChanged();
}

void GDocument::addRedo(GCommand * c)
{
	if (blockUndo)
		return;

	redoList.append(c);
}

bool GDocument::undo()
{
	int nest;

	if (undoList.isEmpty() || isReadOnly() || blockUndo)
		return true;

	disableColorize();

	blockUndo = true;

	nest = 0;
	begin();

	#ifdef DEBUG_UNDO
	qDebug("BEGIN UNDO");
	#endif

	for(;;)
	{
		GCommand *c = undoList.take();
		if (!c)
			break;
		#ifdef DEBUG_UNDO
		c->print();
		#endif
		c->process(this, true);
		nest += c->nest();
		//if (d->undoList.isEmpty())
		//  emit undoAvailable(false);
		redoList.append(c);
		if (!nest && !c->linked())
			break;
	}

	end();
	
	#ifdef DEBUG_UNDO
	qDebug("END UNDO: %d", nest);
	#endif
	
	blockUndo = false;
	
	enableColorize();
	
	return false;
}

bool GDocument::redo()
{
	int nest;

	if (redoList.isEmpty() || isReadOnly() || blockUndo)
		return true;

	disableColorize();
	
	blockUndo = true;

	nest = 0;
	begin();

	#ifdef DEBUG_UNDO
	qDebug("BEGIN REDO");
	#endif
	
	for(;;)
	{
		GCommand *c = redoList.take();
		if (!c)
			break;
		#ifdef DEBUG_UNDO
		c->print();
		#endif
		c->process(this, false);
		nest += c->nest();

		/*if (d->redoList.isEmpty())
			emit redoAvailable(false);
		if (d->undoList.isEmpty())
			emit undoAvailable(true);*/
		undoList.append(c);
		if (!nest && !c->linked())
			break;
	}

	end();
	
	#ifdef DEBUG_UNDO
	qDebug("END REDO: %d", nest);
	#endif

	blockUndo = false;
	
	enableColorize();
	
	return false;
}

int GDocument::wordLeft(int y, int x, bool word)
{
	int xw = x;
	GString s = lines.at(y)->s;

	if (!word)
	{
		while (xw > 0 && s.isSpace(xw - 1))
			xw--;
	}
	
	if (xw > 0)
	{
		if (s.isWordChar(xw - 1))
		{
			for(;;)
			{
				xw--;
				if (xw <= 0 || !s.isWordChar(xw - 1))
					break;
			}
		}
		else if (!word)
		{
			for(;;)
			{
				xw--;
				if (xw <= 0 || s.isWordChar(xw - 1) || s.isSpace(xw - 1))
					break;
			}
		}
	}

	return xw;
}

int GDocument::wordRight(int y, int x, bool word)
{
	int xw = x;
	GString s = lines.at(y)->s;
	int len = s.length();

	if (xw < len)
	{
		if (s.isWordChar(xw))
		{
			for(;;)
			{
				xw++;
				if (xw >= len || !s.isWordChar(xw))
					break;
			}
		}
		else if (!word)
		{
			for(;;)
			{
				xw++;
				if (xw >= len || s.isWordChar(xw) || s.isSpace(xw))
					break;
			}
		}
	}

	if (!word)
	{
		while (xw < len && s.isSpace(xw))
			xw++;
	}
	
	return xw;
}

int GDocument::getLength() const
{
	uint i, len;

	if (lines.count())
		return 0;

	len = 0;

	for (i = 0; i < lines.count(); i++)
		len += lines.at(i)->s.length() + 1;

	return (len - 1);
}


GString GDocument::getLine(int y) const
{
	GString tmp;

	if (y >= 0 || y < (int)lines.count())
		tmp = lines.at(y)->s;

	return tmp;
}

void GDocument::setLine(int y, GString & str)
{
	if (y < 0 || y >= (int)lines.count())
		return;

	begin();
	if (lines.at(y)->s.length())
		remove(y, 0, y, lines.at(y)->s.length());
	if (str.length())
		insert(y, 0, str);
	end();
	updateViews(y);
}

bool GDocument::getLineFlag(int y, int f) const
{
	if (y >= 0 && y < (int)lines.count())
		return lines.at(y)->flag & (1 << f);
	else
		return false;
}

void GDocument::setLineFlag(int y, int f, bool b)
{
	if (y < 0 || y >= (int)lines.count())
		return;

	if (b)
		lines.at(y)->flag |= (1 << f);
	else
		lines.at(y)->flag &= ~(1 << f);

	updateViews(y);
}

int GDocument::convState(int state)
{
	switch(state)
	{
		case EVAL_TYPE_END: return GLine::Normal;
		case EVAL_TYPE_RESERVED: return GLine::Keyword;
		case EVAL_TYPE_IDENTIFIER: return GLine::Symbol;
		case EVAL_TYPE_CLASS: return GLine::Datatype;
		case EVAL_TYPE_NUMBER: return GLine::Number;
		case EVAL_TYPE_STRING: return GLine::String;
		case EVAL_TYPE_SUBR: return GLine::Subr;
		case EVAL_TYPE_COMMENT: return GLine::Comment;
		case EVAL_TYPE_OPERATOR: return GLine::Operator;
		case EVAL_TYPE_DATATYPE: return GLine::Datatype;
		case EVAL_TYPE_ERROR: return GLine::Error;
		case EVAL_TYPE_HELP: return GLine::Help;
		case EVAL_TYPE_PREPROCESSOR: return GLine::Preprocessor;
		default: return GLine::Normal;
	}
}

void GDocument::highlightGambas(GEditor *editor, int line, uint &state, bool &alternate, int &tag, GString &s, GHighlightArray *data, bool &proc)
{
	const char *src;
	EVAL_ANALYZE result;
	int i;

	src = (const char *)s.utf8();

	EVAL.Analyze(src, strlen(src), state == GLine::Comment ? EVAL_TYPE_COMMENT : EVAL_TYPE_END, &result, TRUE);

	GB.NewArray(data, sizeof(GHighlight), result.len);

	for (i = 0; i < result.len; i++)
	{
		//qDebug("state = %d -> %d  len = %d", result.color[i].state, QEditor::convState[result.color[i].state], result.color[i].len);
		(*data)[i].state = convState(result.color[i].state);
		(*data)[i].alternate = result.color[i].alternate;
		(*data)[i].len = result.color[i].len;
	}

	s = result.str;
	GB.FreeString(&result.str);

	proc = result.proc;
	state = convState(result.state);
}

#if 0
bool GDocument::highlightTest(GEditor *, uint &state, GString &s, GHighlightArray &data, bool &proc)
{
	QMemArray<uchar> d(s.length());
	uint i;
	int j;
	uint nstate = 0;

	if (s.length() == 0)
	{
		proc = false;
		return false;
	}

	for (i = 0; i < s.length(); i++)
	{
		nstate = state;

		if (state == GLine::Normal)
		{
			if (s[i] == '/' && i < (s.length() - 1) && s[i + 1] == '*')
				state = nstate = GLine::Comment;
		}
		else if (state == GLine::Comment)
		{
			if (s[i] == '/' && i > 0 && s[i - 1] == '*')
				nstate = GLine::Normal;
		}

		d[i] = state;
		state = nstate;
	}

	j = -1;
	data.resize(s.length());

	for (i = 0; i < s.length(); i++)
	{
		if (i == 0 || d[i] != data[j].state)
		{
			j++;
			data[j].state = d[i];
			data[j].len = 1;
		}
		else
			data[j].len++;
	}

	if (j >= 0)
		data.resize(j + 1);

	proc = false;

	return false; //qstrcmp((const char *)cs, result.str);
}
#endif

void GDocument::invalidate(int y)
{
	if (y >= 0 && y < numLines())
	{
		lines.at(y)->modified = true;
		COLORIZE_FROM(y);
	}
}

void GDocument::colorize(int y, bool force)
{
	GLine *l;
	//bool modif;
	bool proc;
	GString old;
	uint state;
	bool alternate;
	int tag;
	bool changed = false;
	bool updateAll = false;
	int yy;
	int updateFrom;
	bool undo;

	if (highlightMode == None || highlightCallback == NULL)
		return;

	if (y < 0)
		return;
	
	if (force)
	{
		if (colorizeFrom > y)
			colorizeFrom = y;
	}
	
	if (_disableColorize)
	{
		if (_disableColorizeStart < 0)
			_disableColorizeStart = y;
		else if (_disableColorizeStart > y)
			_disableColorizeStart = y;
		return;
	}
	
	if (y < colorizeFrom)
		return;
	
	yy = colorizeFrom;
	updateFrom = -1;
	
	undo = false;
	
	while (yy < numLines())
	{
		l = lines.at(yy);

		if (!l->modified) // || ( isLineEditedSomewhere(yy)))
		{
			if (yy < y)
			{
				yy++;
				continue;
			}
			else
				break;
		}
		
		if (updateFrom < 0)
			updateFrom = yy;
		//modif = false;

		if (!force && l->baptized && isLineEditedSomewhere(yy))
		{
			yy++;
			continue;
		}

		//fprintf(stderr, "colorize %d\n", yy);
		
		getState(yy, false, state, tag, alternate);

		if (l->s.length())
		{
			GCommandDocument info(this);
				
			old = l->s;
			GB.FreeArray(&l->highlight);
			proc = l->proc;
			(*highlightCallback)(views.first(), yy, state, alternate, tag, l->s, &l->highlight, proc);
			
			updateAll |= proc != l->proc;
			l->proc = proc;

			if (old != l->s)
			{
				if (!undo)
				{
					undo = true;
					begin();
				}
				addUndo(new GDeleteCommand(&info, yy, 0, yy, old.length(), old));
				if (l->s.length())
					addUndo(new GInsertCommand(&info, yy, 0, yy, l->s.length(), l->s));
				
				//maxLength = GMAX(maxLength, (int)l->s.length());
				updateLineWidth(yy);
				//qDebug("colorize: %d has changed: '%s' -> '%s'", y, old.utf8(), l->s.utf8());
				l->changed = true;
				changed = true;
			}
		}
		else
		{
			GB.FreeArray(&l->highlight);
			updateAll |= l->proc;
			l->proc = false;
		}

		if (yy == 0)
			l->proc = true;

		//fprintf(stderr, "unmodifyLine: %d\n", yy);
		l->modified = false;
		l->baptized = true;

		state &= 0x1F;
		tag &= 0xFFFF;

		if (l->state == state && l->tag == tag && l->alternate == alternate)
		{
			if (yy < y)
			{
				yy++;
				continue;
			}
			else
				break;
		}

		l->state = state;
		l->alternate = alternate;
		l->tag = tag;
		
		yy++;
		if (yy < numLines())
		{
			//if (!lines.at(yy)->modified) 
			//	fprintf(stderr, "modifyLine: %d\n", yy);
			lines.at(yy)->modified = true;
		}
	}

	if (undo)
		end();
	
	//if (yedit < 0 || yy < yedit)
		colorizeFrom = yy + 1;
	//else
	//	colorizeFrom = yedit;
		
	//fprintf(stderr, "colorizeFrom -> %d\n", colorizeFrom);
	
	if (changed)
		emitTextChanged();
	
	if (updateFrom >= 0)
		updateViews(updateFrom, yy - updateFrom + 1);
	
	if (updateAll)
		updateContents();
}

void GDocument::colorizeAll()
{
	int y;
	
	if (highlightMode == None)
		return;
	
	FOR_EACH_VIEW(v)
	{
		//if (v->viewport()->hasFocus())
		v->leaveCurrentLine();
	}

	for (y = colorizeFrom; y < numLines(); y++)
		colorize(y);
}

#if 0
void GDocument::baptizeUntil(int y)
{
	/*bool busy = (y - baptismLimit) > 256;

	if (busy)
		qApp->setOverrideCursor(Qt::waitCursor);*/

	while (baptismLimit <= y)
	{
		colorize(baptismLimit);
		baptismLimit++;
	}

	/*if (busy)
		qApp->restoreOverrideCursor();*/
}
#endif

void GDocument::setHighlightMode(int mode, GHighlightCallback cb)
{
	int i;

	highlightMode = mode;

	if (mode == Gambas)
		highlightCallback = GDocument::highlightGambas;
	else
		highlightCallback = cb;

	for (i = 0; i < numLines(); i++)
		lines.at(i)->modified = true;

	colorizeFrom = 0;

	updateMargin();
	updateViews();
}

void GDocument::setKeywordsUseUpperCase(bool v)
{ 
	if (v == keywordsUseUpperCase)
		return;
		
	keywordsUseUpperCase = v; 
	setHighlightMode(getHighlightMode());
}

void GDocument::emitTextChanged()
{
	if (undoLevel > 0)
	{
		textHasChanged = true;
		return;
	}
	
	FOR_EACH_VIEW(v)
		v->docTextChanged();
}


int GDocument::getNextLimit(int y)
{
	for(;;)
	{
		y++;
		if (y >= numLines())
			return (-1);
		if (hasLimit(y))
			return y;
	}
}

int GDocument::getPreviousLimit(int y)
{
	for(;;)
	{
		y--;
		if (y < 0)
			return (-1);
		if (y == 0 || hasLimit(y))
			return y;
	}
}

void GDocument::getState(int y, bool col, uint &state, int &tag, bool &alternate)
{
	if (y == 0)
	{
		state = GLine::Normal;
		alternate = false;
		tag = 0;
	}
	else
	{
		if (col)
			colorize(y - 1);
		GLine *l = lines.at(y - 1);
		state = l->state;
		alternate = l->alternate;
		tag = l->tag;
	}
}

int GDocument::getCharState(int y, int x)
{
	GLine *l;
	int i;
	GHighlight *hl;
	
	l = lines.at(y);
	if (l->modified)
	{
		if (x < 0 || x > (int)l->s.length())
			return GLine::Background;
		else
			return GLine::Normal;
	}

	for (i = 0; i < GB.Count(l->highlight); i++)
	{
		hl = &l->highlight[i];
		if (x < hl->len)
			return hl->state;
		x -= hl->len;
	}
	
	return GLine::Background;
}

bool GDocument::isLineEditedSomewhere(int y)
{
	//fprintf(stderr, "isLineEditedSomewhere %d ?\n", y);
	if (!lines.at(y)->modified)
	{
		//fprintf(stderr, "--> false (not modified)\n");
		return false;
	}
	
	FOR_EACH_VIEW(v)
	{
		//fprintf(stderr, "check with %d\n", v->y);
		if (v->y == y && !v->getFlag(GEditor::HighlightImmediately))
		{
			//fprintf(stderr, "--> true\n");
			return true;
		}
	}
	
	//fprintf(stderr, "--> false (no view matches)\n");
	return false;
}

int GDocument::getLimitIndex(int y)
{
	int i = 0;
	
	if (y >= numLines())
		y = numLines() - 1;
	
	if (y < 0)
		return -1;
	
	while (y > 0)
	{
		if (lines.at(y)->proc)
			i++;
		y--;
	}
	
	return i;
}

void GDocument::updateMargin()
{
	FOR_EACH_VIEW(v)
	{
		v->updateMargin();
	}
}

void GDocument::disableColorize()
{
	if (_disableColorize == 0)
		_disableColorizeStart = -1;
	_disableColorize++;
}

void GDocument::enableColorize()
{
	_disableColorize--;
	if (_disableColorize == 0 && _disableColorizeStart >= 0)
	{
		colorize(_disableColorizeStart);
		_disableColorizeStart = -1;
	}
}

void GDocument::updateContents()
{
	FOR_EACH_VIEW(v)
	{
		v->updateContents();
	}
}

void GDocument::setCurrentLine(int y)
{
	_currentLine = y;
	updateContents();
}

void GDocument::setEndOfLine(int mode)
{
	_eol_mode = mode;

	switch (mode)
	{
		case GB_EOL_WINDOWS: _eol = GString("\r\n"); break;
		case GB_EOL_MAC: _eol = GString("\r"); break;
		default: _eol = GString("\n"); break;
	}
}
