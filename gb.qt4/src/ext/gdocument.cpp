/***************************************************************************

  gdocument.cpp

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

#define __G_DOCUMENT_CPP

#include <ctype.h>

#include "main.h"
#include "gview.h"
#include "gdocument.h"

#define GMAX(x, y) ((x) > (y) ? (x) : (y))
#define GMIN(x, y) ((x) < (y) ? (x) : (y))

/**---- GLine -----------------------------------------------------------*/

GLine::GLine()
{
	s = "";
	state = Normal;
	alternate = false;
	tag = 0;
	flag = 0;
	modified = false;
	changed = false;
	baptized = false;
	proc = false;
	highlight = NULL;
}

GLine::~GLine()
{
	GB.FreeArray(&highlight);
}

/**---- GCommand -----------------------------------------------------------*/

class GCommand
{
public:
  enum Type
  {
    None, Begin, End, Move, Insert, Delete, Indent, Unindent
  };

  virtual ~GCommand() { }
  virtual Type type() const { return None; }
  virtual int nest() const { return 0; }
  virtual void print() const { }
  virtual bool merge(GCommand *) const { return false; }
  virtual void process(GDocument *doc, bool undo) const { }
	virtual bool linked() const { return false; }
};

class GBeginCommand: public GCommand
{
public:
	bool _linked;
	
  GBeginCommand(bool linked = false) { _linked = linked; }
  Type type() const { return Begin; }
  void print() const { qDebug("Begin"); }
  int nest() const { return 1; }
	bool linked() const { return _linked; }
};

class GEndCommand: public GCommand
{
public:
	bool _linked;

  GEndCommand(bool linked = false) { _linked = linked; }
  Type type() const { return End; }
  void print() const { qDebug("End"); }
  int nest() const { return -1; }
  bool merge(GCommand *o) const { return (o->type() == Begin); }
	bool linked() const { return _linked; }
};

class GDeleteCommand: public GCommand
{
public:
  int x, y, x2, y2;
  GString str;

  GDeleteCommand(int y, int x, int y2, int x2, const GString & str)
  {
    this->x = x; this->y = y; this->x2 = x2; this->y2 = y2; this->str = str;
  }

  Type type() const { return Delete; }
  void print() const { qDebug("Delete: (%d %d)-(%d %d): '%s'", x, y, x2, y2, str.utf8()); }

  bool merge(GCommand *c) const
  {
    if (c->type() != type())
      return false;

    GDeleteCommand *o = (GDeleteCommand *)c;

    if (x2 != o->x || y2 != o->y)
      return false;

    o->str.prepend(str);
    o->x = x;
    o->y = y;
    return true;
  }

  void process(GDocument *doc, bool undo) const 
  {
    if (undo)
      doc->insert(y, x, str);
    else
      doc->remove(y, x, y2, x2);
  }
};

class GInsertCommand: public GDeleteCommand
{
public:
  GInsertCommand(int y, int x, int y2, int x2, const GString & str): GDeleteCommand(y, x, y2, x2, str) {}
  Type type() const { return Insert; }
  void print() const { qDebug("Insert: (%d %d)-(%d %d): '%s'", x, y, x2, y2, str.utf8()); }

  bool merge(GCommand *c) const
  {
    if (c->type() != type())
      return false;

    if (str.length() && str.isNewLine(0))
      return false;

    GInsertCommand *o = (GInsertCommand *)c;

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
  baptismLimit = 0;
}

GDocument::GDocument()
{
  oldCount = 0;
  blockUndo = false;
  tabWidth = 2;
  readOnly = false;
  highlightMode = None;
  keywordsUseUpperCase = false;

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
  {
    views.at(i)->cursorGoto(0, 0, false);
  	views.at(i)->foldClear();
  }
}


void GDocument::reset()
{
  for (uint i = 0; i < lines.count(); i++)
    lines.at(i)->changed = false;

  updateViews();
}


GString GDocument::getText()
{
  GString tmp;

  if (lines.count())
  {
    for (uint i = 0; i < lines.count(); i++)
      colorize(i);

    for (uint i = 0; i < (lines.count() - 1); i++)
    {
      tmp += lines.at(i)->s;
      tmp += '\n';
		}

    tmp += lines.at(lines.count() - 1)->s;

    updateViews();
  }

  return tmp;
}

GString GDocument::getSelectedText() const
{
  GString tmp;
  int x1, y1, x2, y2;

  if (lines.count() && hasSelection())
  {
    getSelection(&y1, &x1, &y2, &x2);
    if (y1 == y2)
      tmp = lines.at(y1)->s.mid(x1, x2 - x1);
    else
    {
      tmp = lines.at(y1)->s.mid(x1);
      tmp += '\n';
      for (int i = y1 + 1; i < y2; i++)
      {
        tmp += lines.at(i)->s;
        tmp += '\n';
			}

      tmp += lines.at(y2)->s.left(x2);
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
	lines.at(y)->modified = lines.at(y)->changed = true;
	if (baptismLimit > y)
		baptismLimit++;
	FOR_EACH_VIEW(v) { v->lineInserted(y); }
}

void GDocument::insert(int y, int x, const GString & text)
{
  int pos = 0;
  int pos2;
  int xs = x, ys = y;
  GLine *l;
  int n = 1;
  int nl = 0;
  GString rest;
  int yy;

  if (readOnly)
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

	while (y >= (int)lines.count())
	{
		yy = (int)lines.count();
		insertLine(yy);
    nl++;
	}

  for(;;)
  {
    pos2 = text.find('\n', pos);
    if (pos2 < 0)
      pos2 = text.length();

    l = lines.at(y);

    if (pos2 > pos)
    {
      l->s.insert(x, text.mid(pos, pos2 - pos));
      l->modified = l->changed = true;

      //maxLength = GMAX(maxLength, (int)l->s.length());
      updateLineWidth(y);

      FOR_EACH_VIEW(v)
      {
        if (v->ny == y && v->nx >= x)
          v->nx += pos2 - pos;
      }

      x += pos2 - pos;
    }

    pos = pos2 + 1;

    if (pos > (int)text.length())
      break;

    if (x < (int)l->s.length())
    {
      rest = l->s.mid(x);

      l->s.remove(x, rest.length());
      l->modified = l->changed = true;
      updateLineWidth(y);
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
    l->s.insert(x, rest);
    l->modified = l->changed = true;

    //maxLength = GMAX(maxLength, (int)l->s.length());
    updateLineWidth(y);
  }

	FOR_EACH_VIEW(v)
	{
		v->foldInsert(ys, nl);
	}
	
  addUndo(new GInsertCommand(ys, xs, y, x, text));

  updateViews(ys, n);

  yAfter = y;
  xAfter = x;

	emitTextChanged();

	FOR_EACH_VIEW(v)
	{
		v->cursorGoto(v->ny, v->nx, FALSE);
	}
}

void GDocument::removeLine(int y)
{
	lines.remove(y);
	if (baptismLimit > y)
		baptismLimit--;
	FOR_EACH_VIEW(v) { v->lineRemoved(y); }
}

void GDocument::remove(int y1, int x1, int y2, int x2)
{
  GLine *l;
  GString text, rest;
  int y;

  yAfter = y1;
  xAfter = x1;

  if (readOnly)
    return;
	
	FOR_EACH_VIEW(v)
  {
		v->nx = v->x;
		v->ny = v->y;
  }
  
  l = lines.at(y1);

  if (y1 == y2)
  {
    if (x2 >= x1)
    {
      text = l->s.mid(x1, x2 - x1);

      l->s.remove(x1, x2 - x1);
      l->modified = l->changed = true;
      updateLineWidth(y1);

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
		text = l->s.mid(x1) + '\n';
		rest = lines.at(y2)->s.left(x2);

		l->s = l->s.left(x1) + lines.at(y2)->s.mid(x2);
		l->modified = l->changed = true;
		l->state = 0; // force highlighting of next line.

    //maxLength = GMAX(maxLength, (int)l->s.length());
    updateLineWidth(y1);

    for (y = y1 + 1; y < y2; y++)
      text += lines.at(y)->s + '\n';
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

  addUndo(new GDeleteCommand(y1, x1, y2, x2, text));

	FOR_EACH_VIEW(v)
  {
  	v->cursorGoto(v->ny, v->nx, false);
  }
  
  emitTextChanged();
}

void GDocument::setText(const GString & text)
{
  bool oldReadOnly = readOnly;

  readOnly = false;
  blockUndo = true;

  clear();
  insert(0, 0, text);
  colorize(0);
  reset();

  blockUndo = false;
  readOnly = oldReadOnly;

  FOR_EACH_VIEW(v)
  {
    v->cursorGoto(0, 0, false);
  }
}

void GDocument::unsubscribe(GEditor *view)
{
  //qDebug("unsubscribe %p -> %p", view, this);
  views.removeRef(view);
	if (views.count() == 0)
		delete this;
}

void GDocument::subscribe(GEditor *view)
{
  //qDebug("subscribe %p -> %p", view, this);
  views.removeRef(view);
  views.append(view);
  view->setNumRows(lines.count());
  view->updateContents();
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


void GDocument::getSelection(int *y1, int *x1, int *y2, int *x2) const
{
  if (!selector)
    return;

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

  getSelection(&y1a, NULL, &y2a, NULL);
  ys2 = y;
  xs2 = x;
  getSelection(&y1b, NULL, &y2b, NULL);

  if (y1a == y1b)
    updateViews(GMIN(y2a, y2b), GMAX(y2a, y2b) - GMIN(y2a, y2b) + 1);
  else if (y2a == y2b)
    updateViews(GMIN(y1a, y1b), GMAX(y1a, y1b) - GMIN(y1a, y1b) + 1);
  else
    updateViews(GMIN(y1a, y1b), GMAX(y2a, y2b) - GMIN(y1a, y1b) + 1);

  updateViews(y);
}

void GDocument::hideSelection()
{
  int y1, y2;

  if (!selector)
    return;

  getSelection(&y1, NULL, &y2, NULL);
  selector = NULL;
  updateViews(y1, y2 - y1 + 1);
}

void GDocument::eraseSelection()
{
  int y1, y2, x1, x2;

  if (!selector)
    return;

  getSelection(&y1, &x1, &y2, &x2);
  selector = NULL;
  /*x2--;
  if (x2 < 0)
  {
    y2--;
    x2 = lines.at(y2)->s.length() - 1;
  }*/
  remove(y1, x1, y2, x2);
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

  //c->print();

  if (!undoList.isEmpty())
  {
    if (c->merge(undoList.last()))
    {
      //qDebug("merge");
      delete c;
      return;
    }
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
	addUndo(new GBeginCommand(linked));
}

void GDocument::end(bool linked)
{
	undoLevel--;
  addUndo(new GEndCommand(linked));
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

  blockUndo = true;
  nest = 0;
  begin();

  //qDebug("BEGIN UNDO");

  for(;;)
  {
    GCommand *c = undoList.take();
    if (!c)
      break;
    //c->print();
    c->process(this, true);
    nest += c->nest();
    //if (d->undoList.isEmpty())
    //  emit undoAvailable(false);
    redoList.append(c);
		if (!nest && !c->linked())
			break;
  }

  //qDebug("END UNDO");

	end();
  blockUndo = false;
  return false;
}

bool GDocument::redo()
{
  int nest;

  if (redoList.isEmpty() || isReadOnly() || blockUndo)
    return true;

  blockUndo = true;
  nest = 0;
  begin();

  for(;;)
  {
    GCommand *c = redoList.take();
    if (!c)
      break;
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
  blockUndo = false;
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
  if (y >= 0 || y < (int)lines.count())
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
		case EVAL_TYPE_ALTERNATE: return GLine::Alternate;
    case EVAL_TYPE_HELP: return GLine::Help;
    default: return GLine::Normal;
  }
}

void GDocument::highlightGambas(GEditor *editor, uint &state, bool &alternate, int &tag, GString &s, GHighlightArray *data, bool &proc)
{
  const char *src;
  EVAL_ANALYZE result;
  int i;
  uint ls;

  ls = s.length();
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
		lines.at(y)->modified = true;
}

void GDocument::colorize(int y)
{
  GLine *l;
  //bool modif;
  bool proc;
  GString old;
  uint state;
	bool alternate;
  int tag;
  int nupd = 0;
  bool changed = false;

  if (highlightMode == None)
    return;

  if (y < 0)
    return;

	while (y < numLines())
	{
		l = lines.at(y);

		if (!l->modified)
			break;

		//qDebug("colorize: %d: %s", y, l->s.utf8());

		nupd++;
		//modif = false;

		getState(y, false, state, tag, alternate);

		if (l->s.length())
		{
			old = l->s;
			GB.FreeArray(&l->highlight);
			proc = l->proc;
			(*highlightCallback)(views.first(), state, alternate, tag, l->s, &l->highlight, proc);
			l->proc = proc;

			if (old != l->s)
			{
				begin(true);
				addUndo(new GDeleteCommand(y, 0, y, old.length(), old));
				if (l->s.length())
					addUndo(new GInsertCommand(y, 0, y, l->s.length(), l->s));
				end(true);
				
				//maxLength = GMAX(maxLength, (int)l->s.length());
				updateLineWidth(y);
			//qDebug("colorize: %d has changed: '%s' -> '%s'", y, old.utf8(), l->s.utf8());
			l->changed = true;
			changed = true;
			}
		}
		else
		{
			GB.FreeArray(&l->highlight);
			l->proc = false;
		}

		if (y == 0)
			l->proc = true;

		l->modified = false;

		state &= 0x1F;
		tag &= 0xFFFF;

		if (l->state == state && l->tag == tag)
			break;

		l->state = state;
		l->alternate = alternate;
		l->tag = tag;
		
		y++;
		if (y < numLines())
			lines.at(y)->modified = true;
	}

	if (changed)
		emitTextChanged();
	
	if (nupd >= 1)
		updateViews(y - nupd + 1, nupd);
}


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

void GDocument::setHighlightMode(int mode, GHighlightCallback cb)
{
	int i;

	highlightMode = mode;

	if (mode == Gambas)
		highlightCallback = GDocument::highlightGambas;
	else
		highlightCallback = cb;

	for (i = 0; i < numLines(); i++)
	{
		lines.at(i)->modified = true;
		lines.at(i)->baptized = false;
	}

	baptismLimit = 0;

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
	
	colorize(y);
	l = lines.at(y);

	for (i = 0; i < GB.Count(l->highlight); i++)
	{
		hl = &l->highlight[i];
		if (x < hl->len)
			return hl->state;
		x -= hl->len;
	}
	
	return GLine::Background;
}
