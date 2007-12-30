/***************************************************************************

  CEditor.cpp

  (c) 2000-2007 Benoit Minisini <gambas@users.sourceforge.net>

  WIDGET program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 1, or (at your option)
  any later version.

  WIDGET program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with WIDGET program; if not, write to the Free Software
  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

***************************************************************************/

#define __CEDITOR_CPP

#include "main.h"

#include <qobject.h>
#include <qpalette.h>
#include "gdocument.h"

#include "CEditor.h"

DECLARE_EVENT(EVENT_Change);
DECLARE_EVENT(EVENT_Cursor);
DECLARE_EVENT(EVENT_Scroll);
DECLARE_EVENT(EVENT_Highlight);
DECLARE_EVENT(EVENT_Margin);

static int _x1, _y1, _x2, _y2;
static void *analyze_symbol = 0;
static void *analyze_type = 0;
static void *analyze_pos = 0;
static int _style;

static int _highlight_state = 0;
static int _highlight_tag = 0;
static bool _highlight_show_limit = false;
static QString _highlight_text = "";
static GHighlightArray *_highlight_data = NULL;

static QT_PICTURE _breakpoint_picture = 0;

static QString purge(const QString &s, bool comment, bool string)
{
  QString r;
  QChar c;
  uint i;
  bool in_comment = false;
  char wait = 0;

  for (i = 0; i < s.length(); i++)
  {
    c = s[i];

    switch(wait)
    {
      case 0:

        if (in_comment)
        {
        	if (!comment)
          	c = ' ';
				}
        else if (c == '"')
          wait = '"';
        else if (c == '\'')
          in_comment = true;

        break;

      case '"':
        if (c == '"')
          wait = 0;
				else if (c == '\\')
				{
					if (string)
					{
						if (i < s.length())
							r += c;
						i++;
						c = s[i];
					}
					else
					{
						i++;
						if (i < s.length())
							r += ' ';
						c = ' ';
					}
				}
        else
        {
        	if (!string)
          	c = ' ';
				}
        break;
    }

    r += c;
  }

  return r;
}

static void analyze(QString &s)
{
  GHighlightArray data;
  GB_ARRAY garray, tarray, parray;
  int i, n, pos, len, p;
  char *str;
  bool proc;
  uint state = 0;
  int tag = 0;

  GString gs = s;
  GDocument::highlightGambas(0, state, tag, gs, &data, proc);
  s = gs.getString();

  n = 0;
  for (i = 0; i < GB.Count(data); i++)
  {
    if (data[i].state != GLine::Normal)
      n++;
  }

  //n = data.count() >> 1;
  GB.Array.New(&garray, GB_T_STRING, n);
  GB.Array.New(&tarray, GB_T_INTEGER, n);
  GB.Array.New(&parray, GB_T_INTEGER, n);

  pos = 0;
  i = 0;
  for (p = 0; p < GB.Count(data); p++)
  {
    len = data[p].len;

    if (data[p].state != GLine::Normal)
    {
      GB.NewString(&str, TO_UTF8(s.mid(pos, len)), 0);
      *((char **)GB.Array.Get(garray, i)) = str;
      *((int *)GB.Array.Get(tarray, i)) = data[p].state;
      *((int *)GB.Array.Get(parray, i)) = pos;
      i++;
    }

    pos += len;
  }

	GB.FreeArray(&data);

  GB.Unref(&analyze_symbol);
  analyze_symbol = garray;
  GB.Ref(garray);

  GB.Unref(&analyze_type);
  analyze_type = tarray;
  GB.Ref(tarray);

  GB.Unref(&analyze_pos);
  analyze_pos = parray;
  GB.Ref(parray);
}

static void highlightCustom(GEditor *master, uint &state, int &tag, GString &s, GHighlightArray *data, bool &proc)
{
  void *object = QT.GetObject((QWidget *)master);

	_highlight_state = state;
	_highlight_tag = tag;
	_highlight_text = s.getString();
	_highlight_show_limit = proc;
	_highlight_data = data;

	GB.NewArray(_highlight_data, sizeof(GHighlight), 0);

	GB.Raise(object, EVENT_Highlight, 0);

	state = _highlight_state;
	tag = _highlight_tag;
	s = GString(_highlight_text);
	proc = _highlight_show_limit;
	_highlight_data = 0;
}


BEGIN_METHOD_VOID(CHIGHLIGHT_exit)

  GB.Unref(&analyze_symbol);
  GB.Unref(&analyze_type);
  GB.Unref(&analyze_pos);

END_METHOD


BEGIN_PROPERTY(CHIGHLIGHT_state)

	if (READ_PROPERTY)
		GB.ReturnInteger(_highlight_state);
	else
		_highlight_state = VPROP(GB_INTEGER);

END_PROPERTY

BEGIN_PROPERTY(CHIGHLIGHT_tag)

	if (READ_PROPERTY)
		GB.ReturnInteger(_highlight_tag);
	else
		_highlight_tag = VPROP(GB_INTEGER);

END_PROPERTY

BEGIN_PROPERTY(CHIGHLIGHT_show_limit)

	if (READ_PROPERTY)
		GB.ReturnBoolean(_highlight_show_limit);
	else
		_highlight_show_limit = VPROP(GB_BOOLEAN);

END_PROPERTY

BEGIN_PROPERTY(CHIGHLIGHT_text)

	if (READ_PROPERTY)
		GB.ReturnNewZeroString(TO_UTF8(_highlight_text));
	else
		_highlight_text = QSTRING_PROP();

END_PROPERTY

BEGIN_METHOD(CHIGHLIGHT_add, GB_INTEGER state; GB_INTEGER len)

	GHighlight *h;

	if (!_highlight_data)
		return;

	int count = GB.Count(*_highlight_data) - 1;
	int state = VARG(state);
	int len = VARGOPT(len, 1);

	if (len < 1)
		return;

	if (count < 0 || (*_highlight_data)[count].state != (uint)state)
	{
		count++;
		h = (GHighlight *)GB.Add(_highlight_data);
		h->state = state;
		h->len = len;
	}
	else
		(*_highlight_data)[count].len += len;

END_METHOD

BEGIN_PROPERTY(CEDITORDOC_text)

  if (READ_PROPERTY)
    GB.ReturnNewZeroString(DOC->getText().utf8());
  else
    DOC->setText(QSTRING_PROP());

END_PROPERTY

BEGIN_PROPERTY(CEDITORDOC_length)

  GB.ReturnInteger(DOC->getLength());

END_PROPERTY

BEGIN_PROPERTY(CEDITORDOC_tab_length)

  if (READ_PROPERTY)
    GB.ReturnInteger(DOC->getTabWidth());
  else
    DOC->setTabWidth(VPROP(GB_INTEGER));

END_PROPERTY

BEGIN_METHOD_VOID(CEDITORDOC_reset)

  DOC->reset();

END_METHOD

BEGIN_PROPERTY(CEDITORDOC_read_only)

  if (READ_PROPERTY)
    GB.ReturnBoolean(DOC->isReadOnly());
  else
    DOC->setReadOnly(VPROP(GB_BOOLEAN));

END_PROPERTY

BEGIN_METHOD_VOID(CEDITORDOC_clear)

  DOC->clear();

END_METHOD

BEGIN_METHOD(CEDITORDOC_insert, GB_STRING str; GB_INTEGER y; GB_INTEGER x)

	if (MISSING(y) || MISSING(x))
		WIDGET->insert(QSTRING_ARG(str));
	else
  	DOC->insert(VARG(y), VARG(x), QSTRING_ARG(str));

END_METHOD

static void print_text(void *_object, const QString &s)
{
  int line, col;
	
	WIDGET->getCursor(&line, &col);
	if (col == 0)
	{
		//qDebug("remove: %p: %d, %d, %d, %d", THIS, line, 0, line, DOC->lineLength(line));
		DOC->remove(line, 0, line, DOC->lineLength(line));
		WIDGET->cursorGoto(line, 0, false);
	}
	WIDGET->insert(s);
	
}

BEGIN_METHOD(CEDITORDOC_print, GB_STRING str; GB_INTEGER y; GB_INTEGER x)

	QString s = QSTRING_ARG(str);
	uint i, j, code;
  int line, col;

	if (!MISSING(y) && !MISSING(x))
		WIDGET->cursorGoto(VARG(y), VARG(x), false);
	
	j = 0;
	for (i = 0; i < s.length(); i++)
	{
		//if (s[i].latin1() < 32)
		//	qDebug("%d", s[i].latin1());
		code = s[i].unicode();
		if (code < 32 && code != 10)
		{
			if (i > j)
				print_text(THIS, s.mid(j, i - j));
				
			j = i + 1;
			
 			WIDGET->getCursor(&line, &col);
 			
			if (code == '\t')
			{
				QString spc = "        ";
				int l = spc.length() - col % spc.length();
				print_text(THIS, spc.left(l));
			}
			else if (code == '\r')
			{
				WIDGET->cursorGoto(line, 0, false);
			}
			else
			{
				QString tmp;
				tmp.sprintf("^%c", code + 64);
				print_text(THIS, tmp);
			}
		}
	}

	if (i > j)
		print_text(THIS, s.mid(j, i - j));
		
END_METHOD


BEGIN_METHOD(CEDITORDOC_remove, GB_INTEGER y; GB_INTEGER x; GB_INTEGER y2; GB_INTEGER x2)

  DOC->remove(VARG(y), VARG(x), VARG(y2), VARG(x2));

END_METHOD

BEGIN_METHOD_VOID(CEDITORDOC_undo)

  WIDGET->undo();

END_METHOD

BEGIN_METHOD_VOID(CEDITORDOC_redo)

  WIDGET->redo();

END_METHOD

BEGIN_METHOD_VOID(CEDITORDOC_begin)

  DOC->begin();

END_METHOD

BEGIN_METHOD_VOID(CEDITORDOC_end)

  DOC->end();

END_METHOD

BEGIN_PROPERTY(CEDITORDOC_line_count)

  GB.ReturnInteger((long)DOC->numLines());

END_PROPERTY

BEGIN_METHOD(CEDITORDOC_line_get, GB_INTEGER line)

  long line = VARG(line);

  if (line < 0 || line >= DOC->numLines())
    GB.ReturnNull();
  else
    GB.ReturnNewZeroString(DOC->getLine(line).utf8());

END_METHOD

BEGIN_METHOD(CEDITORDOC_line_put, GB_STRING text; GB_INTEGER line)

  long line = VARG(line);
  GString s;

  if (line >= 0 && line < DOC->numLines())
  {
    s = GString(QSTRING_PROP());
    DOC->setLine(line, s);
  }

END_METHOD

BEGIN_METHOD(CEDITORDOC_line_get_flag, GB_INTEGER line; GB_INTEGER flag)

  GB.ReturnBoolean(DOC->getLineFlag(VARG(line), VARG(flag)));

END_METHOD

BEGIN_METHOD(CEDITORDOC_line_set_flag, GB_INTEGER line; GB_INTEGER flag; GB_BOOLEAN value)

  DOC->setLineFlag(VARG(line), VARG(flag), VARG(value));

END_METHOD

BEGIN_PROPERTY(CEDITORDOC_selected)

  GB.ReturnBoolean(DOC->hasSelection());

END_PROPERTY

BEGIN_PROPERTY(CEDITORDOC_sel)

  if (DOC->hasSelection())
    DOC->getSelection(&_y1, &_x1, &_y2, &_x2);
  else
    _x1 = _y1 = _x2 = _y2 = -1;

  RETURN_SELF();

END_PROPERTY

BEGIN_PROPERTY(CEDITORDOC_sel_text)

  GB.ReturnNewZeroString(DOC->getSelectedText().utf8());

END_PROPERTY

BEGIN_PROPERTY(CEDITORDOC_sel_start_line)

  GB.ReturnInteger(_y1);

END_PROPERTY

BEGIN_PROPERTY(CEDITORDOC_sel_start_column)

  GB.ReturnInteger(_x1);

END_PROPERTY

BEGIN_PROPERTY(CEDITORDOC_sel_end_line)

  GB.ReturnInteger(_y2);

END_PROPERTY

BEGIN_PROPERTY(CEDITORDOC_sel_end_column)

  GB.ReturnInteger(_x2);

END_PROPERTY

BEGIN_METHOD_VOID(CEDITORDOC_sel_hide)

  DOC->hideSelection();

END_METHOD

BEGIN_METHOD(CEDITORDOC_find_next_breakpoint, GB_INTEGER line)

  int line = VARG(line);

  for(;;)
  {
    if (line >= DOC->numLines())
    {
      GB.ReturnInteger(-1);
      break;
    }

    if (DOC->getLineFlag(line, GLine::BreakpointFlag))
    {
      GB.ReturnInteger(line);
      break;
    }

    line++;
  }

END_METHOD

BEGIN_METHOD(CEDITORDOC_find_next_word, GB_STRING word; GB_INTEGER line)

  int line = VARG(line);
  QString word = QSTRING_ARG(word);
  QString s;

  for(;;)
  {
    if (line >= DOC->numLines())
    {
      GB.ReturnInteger(-1);
      break;
    }

		s = DOC->getLine(line).getString();
		if (s.find(word, 0, false) >= 0)
		{
		  GB.ReturnInteger(line);
		  break;
		}
		
    line++;
  }

END_METHOD

BEGIN_METHOD(CEDITORDOC_purge_line, GB_INTEGER line; GB_BOOLEAN comment; GB_BOOLEAN string)

  bool comment = VARGOPT(comment, false);
  bool string = VARGOPT(string, false);
  
  if (comment && string)	
  	GB.ReturnNewZeroString(DOC->getLine(VARG(line)).utf8());
	else
	{
		QString s = DOC->getLine(VARG(line)).getString();
  	GB.ReturnNewZeroString(TO_UTF8(purge(s, VARGOPT(comment, false), VARGOPT(string, false))));
	}

END_METHOD

BEGIN_METHOD(CEDITORDOC_analyze, GB_STRING text)

  QString s = QSTRING_ARG(text);
  analyze(s);
  GB.ReturnObject(analyze_symbol);

END_METHOD

BEGIN_PROPERTY(CEDITORDOC_analyze_symbols)

  GB.ReturnObject(analyze_symbol);

END_PROPERTY

BEGIN_PROPERTY(CEDITORDOC_analyze_types)

  GB.ReturnObject(analyze_type);

END_PROPERTY

BEGIN_PROPERTY(CEDITORDOC_analyze_positions)

  GB.ReturnObject(analyze_pos);

END_PROPERTY

BEGIN_PROPERTY(CEDITORDOC_highlight)

  int mode;

  if (READ_PROPERTY)
    GB.ReturnInteger(DOC->getHighlightMode());
  else
  {
    mode = VPROP(GB_INTEGER);
    if (mode == GDocument::Gambas)
    {
      if (MAIN_load_eval_component())
      {
        GB.Error("Cannot load Gambas syntax highlighter");
        return;
      }
    }
  	DOC->setHighlightMode(mode, highlightCustom);
  }

END_PROPERTY


BEGIN_METHOD(CEDITOR_new, GB_OBJECT parent)

  GEditor *wid = new GEditor(QT.GetContainer(VARG(parent)));

  QObject::connect(wid, SIGNAL(cursorMoved()), &CEditor::manager, SLOT(moved()));
  QObject::connect(wid, SIGNAL(textChanged()), &CEditor::manager, SLOT(changed()));
  QObject::connect(wid, SIGNAL(marginDoubleClicked(int)), &CEditor::manager, SLOT(marginDoubleClicked(int)));
  QObject::connect(wid, SIGNAL(contentsMoving(int, int)), &CEditor::manager, SLOT(scrolled(int, int)));

  QT.InitWidget(wid, _object);

  wid->show();

END_METHOD

BEGIN_METHOD_VOID(CEDITOR_free)

	GB.Unref((void **)&THIS->view);

END_METHOD

BEGIN_METHOD_VOID(CEDITOR_exit)

	GB.Unref((void **)&_breakpoint_picture);	

END_METHOD

BEGIN_PROPERTY(CEDITOR_view)

  if (READ_PROPERTY)
  {
  	if (THIS->view)
  		GB.ReturnObject(THIS->view);
		else
			RETURN_SELF();
	}
  else
  {
  	GB.StoreObject(PROP(GB_OBJECT), (void **)&THIS->view);
  	if (THIS->view && THIS->view != THIS)
  	{
  		GEditor *view = (GEditor *)((QT_WIDGET *)THIS->view)->widget;
    	WIDGET->setDocument(view->getDocument());
		}
		else
		{
			WIDGET->setDocument(0);
	  	GB.StoreObject(NULL, (void **)&THIS->view);
		}
  }

END_PROPERTY

BEGIN_PROPERTY(CEDITOR_line)

  int line, col;

  WIDGET->getCursor(&line, &col);
  GB.ReturnInteger(line);

END_PROPERTY

BEGIN_PROPERTY(CEDITOR_column)

  int line, col;

  WIDGET->getCursor(&line, &col);
  GB.ReturnInteger(col);

END_PROPERTY

BEGIN_METHOD(CEDITOR_goto, GB_INTEGER line; GB_INTEGER col; GB_BOOLEAN center)

  if (VARGOPT(center, FALSE))
  	WIDGET->cursorCenter();
  WIDGET->cursorGoto(VARG(line), VARG(col), false);

END_METHOD

BEGIN_METHOD(CEDITOR_flags_get, GB_INTEGER flag)

  GB.ReturnInteger(WIDGET->getFlag(VARG(flag)));

END_METHOD

BEGIN_METHOD(CEDITOR_flags_set, GB_BOOLEAN value; GB_INTEGER flag)

  WIDGET->setFlag(VARG(flag), VARG(value));

END_METHOD

BEGIN_METHOD(CEDITOR_insert, GB_STRING text)

  WIDGET->insert(QSTRING_ARG(text));

END_METHOD

BEGIN_METHOD_VOID(CEDITOR_indent)

  WIDGET->tab(false);

END_METHOD

BEGIN_METHOD_VOID(CEDITOR_unindent)

  WIDGET->tab(true);

END_METHOD

BEGIN_METHOD_VOID(CEDITOR_copy)

  WIDGET->copy();

END_METHOD

BEGIN_METHOD_VOID(CEDITOR_cut)

  WIDGET->cut();

END_METHOD

BEGIN_METHOD_VOID(CEDITOR_paste)

  WIDGET->paste();

END_METHOD

BEGIN_METHOD_VOID(CEDITOR_undo)

  WIDGET->undo();

END_METHOD

BEGIN_METHOD_VOID(CEDITOR_redo)

  WIDGET->redo();

END_METHOD

BEGIN_METHOD(CEDITOR_to_pos_x, GB_INTEGER x)

  int px, py;

  WIDGET->cursorToPos(0, VARG(x), &px, &py);
  GB.ReturnInteger(px);

END_METHOD

BEGIN_METHOD(CEDITOR_styles_get, GB_INTEGER style)

  _style = VARG(style);
  RETURN_SELF();

END_METHOD

BEGIN_PROPERTY(CEDITOR_style_color)

  GHighlightStyle style;

  WIDGET->getStyle(_style, &style);

  if (READ_PROPERTY)
    GB.ReturnInteger(style.color.rgb() & 0xFFFFFF);
  else
  {
    style.color = QColor(VPROP(GB_INTEGER) & 0xFFFFFF);
    WIDGET->setStyle(_style, &style);
  }

END_PROPERTY

BEGIN_PROPERTY(CEDITOR_style_bold)

  GHighlightStyle style;

  WIDGET->getStyle(_style, &style);

  if (READ_PROPERTY)
    GB.ReturnBoolean(style.bold);
  else
  {
    style.bold = VPROP(GB_BOOLEAN);
    WIDGET->setStyle(_style, &style);
  }

END_PROPERTY

BEGIN_PROPERTY(CEDITOR_style_underline)

  GHighlightStyle style;

  WIDGET->getStyle(_style, &style);

  if (READ_PROPERTY)
    GB.ReturnBoolean(style.underline);
  else
  {
    style.underline = VPROP(GB_BOOLEAN);
    WIDGET->setStyle(_style, &style);
  }

END_PROPERTY

BEGIN_PROPERTY(CEDITOR_style_italic)

  GHighlightStyle style;

  WIDGET->getStyle(_style, &style);

  if (READ_PROPERTY)
    GB.ReturnBoolean(style.italic);
  else
  {
    style.italic = VPROP(GB_BOOLEAN);
    WIDGET->setStyle(_style, &style);
  }

END_PROPERTY


BEGIN_METHOD(CEDITOR_select, GB_INTEGER y1; GB_INTEGER x1; GB_INTEGER y2; GB_INTEGER x2)

  WIDGET->cursorGoto(VARG(y1), VARG(x1), false);
  WIDGET->cursorGoto(VARG(y2), VARG(x2), true);

END_METHOD

BEGIN_METHOD_VOID(CEDITOR_select_all)

  WIDGET->selectAll();

END_METHOD

BEGIN_PROPERTY(CEDITOR_cursor_x)

  int x, y, px, py;

  WIDGET->getCursor(&y, &x);
  WIDGET->cursorToPos(y, x, &px, &py);
  GB.ReturnInteger(px);

END_PROPERTY

BEGIN_PROPERTY(CEDITOR_cursor_y)

  int x, y, px, py;

  WIDGET->getCursor(&y, &x);
  WIDGET->cursorToPos(y, x, &px, &py);
  GB.ReturnInteger(py);

END_PROPERTY

BEGIN_PROPERTY(CEDITOR_line_height)

  GB.ReturnInteger(WIDGET->getLineHeight());

END_PROPERTY

BEGIN_PROPERTY(CEDITOR_char_width)

  GB.ReturnInteger(WIDGET->getCharWidth());

END_PROPERTY

BEGIN_PROPERTY(CEDITOR_breakpoint_picture)

	if (READ_PROPERTY)
		GB.ReturnObject(_breakpoint_picture);
	else
	{
		GB.StoreObject(PROP(GB_OBJECT), (void **)&_breakpoint_picture);
		GEditor::setBreakpointPixmap(QT.GetPixmap(_breakpoint_picture));
	}

END_PROPERTY

GB_DESC CHighlightDesc[] =
{
  GB_DECLARE("Highlight", 0), GB_VIRTUAL_CLASS(),

  GB_CONSTANT("None", "i", GDocument::None),
  GB_CONSTANT("Gambas", "i", GDocument::Gambas),
  GB_CONSTANT("Custom", "i", GDocument::Custom),

  GB_CONSTANT("Background", "i", GLine::Background),
  GB_CONSTANT("Normal", "i", GLine::Normal),
  GB_CONSTANT("Keyword", "i", GLine::Keyword),
  GB_CONSTANT("Function", "i", GLine::Subr),
  GB_CONSTANT("Operator", "i", GLine::Operator),
  GB_CONSTANT("Symbol", "i", GLine::Symbol),
  GB_CONSTANT("Number", "i", GLine::Number),
  GB_CONSTANT("String", "i", GLine::String),
  GB_CONSTANT("Comment", "i", GLine::Comment),
  GB_CONSTANT("Breakpoint", "i", GLine::Breakpoint),
  GB_CONSTANT("Current", "i", GLine::Current),
  GB_CONSTANT("DataType", "i", GLine::Datatype),
  GB_CONSTANT("Selection", "i", GLine::Selection),
  GB_CONSTANT("Highlight", "i", GLine::Highlight),
  GB_CONSTANT("CurrentLine", "i", GLine::Line),
  GB_CONSTANT("Error", "i", GLine::Error),

  GB_STATIC_METHOD("_exit", NULL, CHIGHLIGHT_exit, NULL),
  GB_STATIC_METHOD("Analyze", "String[]", CEDITORDOC_analyze, "(Text)s"),
  GB_STATIC_PROPERTY_READ("Symbols", "String[]", CEDITORDOC_analyze_symbols),
  GB_STATIC_PROPERTY_READ("Types", "Integer[]", CEDITORDOC_analyze_types),
  GB_STATIC_PROPERTY_READ("Positions", "Integer[]", CEDITORDOC_analyze_positions),

  GB_STATIC_PROPERTY("State", "i", CHIGHLIGHT_state),
  GB_STATIC_PROPERTY("Tag", "i", CHIGHLIGHT_tag),
  GB_STATIC_PROPERTY("ShowLimit", "b", CHIGHLIGHT_show_limit),
  GB_STATIC_PROPERTY("Text", "s", CHIGHLIGHT_text),
  GB_STATIC_METHOD("Add", NULL, CHIGHLIGHT_add, "(State)i[(Count)i]"),

  GB_END_DECLARE
};


GB_DESC CEditorSelectionDesc[] =
{
  GB_DECLARE(".Editor.Selection", 0), GB_VIRTUAL_CLASS(),

  GB_PROPERTY_READ("Text", "s", CEDITORDOC_sel_text),
  GB_PROPERTY_READ("StartLine", "i", CEDITORDOC_sel_start_line),
  GB_PROPERTY_READ("StartColumn", "i", CEDITORDOC_sel_start_column),
  GB_PROPERTY_READ("EndLine", "i", CEDITORDOC_sel_end_line),
  GB_PROPERTY_READ("EndColumn", "i", CEDITORDOC_sel_end_column),

  GB_METHOD("Hide", NULL, CEDITORDOC_sel_hide, NULL),

  GB_END_DECLARE
};

GB_DESC CEditorLinesDesc[] =
{
  GB_DECLARE(".Editor.Lines", 0), GB_VIRTUAL_CLASS(),

  GB_METHOD("_get", "s", CEDITORDOC_line_get, "(Line)i"),
  GB_METHOD("_put", NULL, CEDITORDOC_line_put, "(Text)s(Line)i"),
  GB_METHOD("SetFlag", NULL, CEDITORDOC_line_set_flag, "(Line)i(Flag)i(Value)b"),
  GB_METHOD("GetFlag", "b", CEDITORDOC_line_get_flag, "(Line)i(Flag)i"),
  GB_PROPERTY_READ("Count", "i", CEDITORDOC_line_count),

  GB_END_DECLARE
};

GB_DESC CEditorStyleDesc[] =
{
  GB_DECLARE(".Editor.Style", 0), GB_VIRTUAL_CLASS(),

  GB_PROPERTY("Color", "i", CEDITOR_style_color),
  GB_PROPERTY("Bold", "b", CEDITOR_style_bold),
  GB_PROPERTY("Italic", "b", CEDITOR_style_italic),
  GB_PROPERTY("Underline", "b", CEDITOR_style_underline),

  GB_END_DECLARE
};

GB_DESC CEditorStylesDesc[] =
{
  GB_DECLARE(".Editor.Styles", 0), GB_VIRTUAL_CLASS(),

  GB_METHOD("_get", ".Editor.Style", CEDITOR_styles_get, "(Style)i"),

  GB_END_DECLARE
};

GB_DESC CEditorFlagsDesc[] =
{
  GB_DECLARE(".Editor.Flags", 0), GB_VIRTUAL_CLASS(),

  GB_METHOD("_get", "b", CEDITOR_flags_get, "(Flag)i"),
  GB_METHOD("_put", NULL, CEDITOR_flags_set, "(Value)b(Flag)i"),

  GB_END_DECLARE
};

GB_DESC CEditorDesc[] =
{
  GB_DECLARE("Editor", sizeof(CEDITOR)), GB_INHERITS("Control"),

  GB_CONSTANT("Current", "i", GLine::CurrentFlag),
  GB_CONSTANT("Breakpoint", "i", GLine::BreakpointFlag),

  GB_CONSTANT("ShowProcedureLimits", "i", GEditor::ShowProcedureLimits),
  GB_CONSTANT("DrawWithRelief", "i", GEditor::DrawWithRelief),
  GB_CONSTANT("ShowModifiedLines", "i", GEditor::ShowModifiedLines),
  GB_CONSTANT("ShowCurrentLine", "i", GEditor::ShowCurrentLine),
  GB_CONSTANT("ShowLineNumbers", "i", GEditor::ShowLineNumbers),
  GB_CONSTANT("HighlightBraces", "i", GEditor::HighlightBraces),
  GB_CONSTANT("HighlightCurrent", "i", GEditor::HighlightCurrent),
  GB_CONSTANT("BlendedProcedureLimits", "i", GEditor::BlendedProcedureLimits),

  GB_METHOD("_new", NULL, CEDITOR_new, "(Parent)Container;"),
  GB_METHOD("_free", NULL, CEDITOR_free, NULL),
	GB_STATIC_METHOD("_exit", NULL, CEDITOR_exit, NULL),
	
	GB_STATIC_PROPERTY("BreakpointPicture", "Picture", CEDITOR_breakpoint_picture),

  GB_PROPERTY("View", "Editor", CEDITOR_view),

  GB_PROPERTY_READ("Line", "i", CEDITOR_line),
  GB_PROPERTY_READ("Column", "i", CEDITOR_column),
  GB_METHOD("Goto", NULL, CEDITOR_goto, "(Line)i(Column)i[(Center)b]"),

  GB_PROPERTY_SELF("Flags", ".Editor.Flags"),

  //GB_METHOD("Insert", NULL, CEDITOR_insert, "(Text)s"),
  GB_METHOD("Indent", NULL, CEDITOR_indent, NULL),
  GB_METHOD("Unindent", NULL, CEDITOR_unindent, NULL),
  GB_METHOD("Copy", NULL, CEDITOR_copy, NULL),
  GB_METHOD("Cut", NULL, CEDITOR_cut, NULL),
  GB_METHOD("Paste", NULL, CEDITOR_paste, NULL),
  GB_METHOD("Undo", NULL, CEDITOR_undo, NULL),
  GB_METHOD("Redo", NULL, CEDITOR_redo, NULL),

  GB_PROPERTY_SELF("Styles", ".Editor.Styles"),

  GB_METHOD("Select", NULL, CEDITOR_select, "(StartLine)i(StartColumn)i(EndLine)i(EndColumn)i"),
  GB_METHOD("SelectAll", NULL, CEDITOR_select_all, NULL),

  GB_PROPERTY_READ("CursorX", "i", CEDITOR_cursor_x),
  GB_PROPERTY_READ("CursorY", "i", CEDITOR_cursor_y),
  GB_METHOD("ToPosX", "i", CEDITOR_to_pos_x, "(Column)i"),
  GB_PROPERTY_READ("LineHeight", "i", CEDITOR_line_height),
  GB_PROPERTY_READ("CharWidth", "i", CEDITOR_char_width),

	// Document specific

  GB_PROPERTY("Text", "s", CEDITORDOC_text),
  GB_PROPERTY_READ("Length", "i", CEDITORDOC_length),

  GB_PROPERTY("Highlight", "i<Highlight,None,Gambas,Custom>", CEDITORDOC_highlight),

  GB_PROPERTY("TabSize", "i", CEDITORDOC_tab_length),
  GB_METHOD("Reset", NULL, CEDITORDOC_reset, NULL),

  GB_PROPERTY("ReadOnly", "b", CEDITORDOC_read_only),

  GB_METHOD("Clear", NULL, CEDITORDOC_clear, NULL),

  GB_METHOD("Insert", NULL, CEDITORDOC_insert, "(Text)s[(Line)i(Column)i]"),
  GB_METHOD("Print", NULL, CEDITORDOC_print, "(Text)s[(Line)i(Column)i]"),
  GB_METHOD("Remove", NULL, CEDITORDOC_remove, "(Line)i(Column)i(EndLine)i(EndColumn)i"),
  GB_METHOD("Undo", NULL, CEDITORDOC_undo, NULL),
  GB_METHOD("Redo", NULL, CEDITORDOC_redo, NULL),
  GB_METHOD("Begin", NULL, CEDITORDOC_begin, NULL),
  GB_METHOD("End", NULL, CEDITORDOC_end, NULL),

  GB_PROPERTY_READ("Selected", "b", CEDITORDOC_selected),
  GB_PROPERTY_READ("Selection", ".Editor.Selection", CEDITORDOC_sel),
  GB_PROPERTY_SELF("Lines", ".Editor.Lines"),
  GB_METHOD("GetPurgedLine", "s", CEDITORDOC_purge_line, "(Line)i[(Comment)b(String)b]"),

  GB_METHOD("FindNextBreakpoint", "i", CEDITORDOC_find_next_breakpoint, "(Line)i"),
  GB_METHOD("FindNextWord", "i", CEDITORDOC_find_next_word, "(Word)s(Line)i"),

  GB_EVENT("Cursor", NULL, NULL, &EVENT_Cursor),
  GB_EVENT("Scroll", NULL, NULL, &EVENT_Scroll),
  GB_EVENT("Change", NULL, NULL, &EVENT_Change),
  GB_EVENT("Highlight", NULL, NULL, &EVENT_Highlight),
  GB_EVENT("Margin", NULL, "(LineNumber)i", &EVENT_Margin),
  
  GB_CONSTANT("_DefaultEvent", "s", "KeyPress"),
  GB_CONSTANT("_Properties", "s", "*,Font{Font:Fixed},Highlight=0,ReadOnly=False,TabSize{Range:1;16}=2"),

  GB_END_DECLARE
};


CEditor CEditor::manager;

void CEditor::changed(void)
{
  void *object = QT.GetObject((QWidget *)sender());
  GB.Raise(object, EVENT_Change, 0);
}

static void post_event(void *_object, int event)
{
  GB.Raise(THIS, event, 0);
  GB.Unref(&_object);
}

void CEditor::moved(void)
{
  void *_object = QT.GetObject((QWidget *)sender());
  GB.Ref(THIS);
  GB.Post2((GB_POST_FUNC)post_event, (long)THIS, EVENT_Cursor);
}

void CEditor::scrolled(int, int)
{
  void *_object = QT.GetObject((QWidget *)sender());
  GB.Ref(THIS);
  GB.Post2((GB_POST_FUNC)post_event, (long)THIS, EVENT_Scroll);
}

void CEditor::marginDoubleClicked(int line)
{
  void *object = QT.GetObject((QWidget *)sender());
  GB.Raise(object, EVENT_Margin, 1, GB_T_INTEGER, line);
}

