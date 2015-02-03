/***************************************************************************

	CEditor.cpp

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

#define __CEDITOR_CPP

#include "main.h"

#include <qobject.h>
#include <qpalette.h>
#include "gdocument.h"

#include "CEditor.h"

#define MAX_CONSOLE_WIDTH 256

DECLARE_EVENT(EVENT_Change);
DECLARE_EVENT(EVENT_Cursor);
DECLARE_EVENT(EVENT_Scroll);
DECLARE_EVENT(EVENT_Highlight);
DECLARE_EVENT(EVENT_Margin);

enum { LIMIT_NONE, LIMIT_LINE, LIMIT_BLEND, LIMIT_BACKGROUND };

enum
{
	HIGHLIGHT_CUSTOM = GDocument::Custom,
	HIGHLIGHT_HTML,
	HIGHLIGHT_CSS,
	HIGHLIGHT_WEBPAGE,
	HIGHLIGHT_DIFF,
	HIGHLIGHT_JAVASCRIPT,
	HIGHLIGHT_C,
	HIGHLIGHT_CPLUSPLUS,
	HIGHLIGHT_SQL
};

typedef
	struct {
		int style;
		const char *name;
	}
	HIGHLIGHT_NAME;

typedef
	struct
	{
		const char *cmd;
		void *label;
		bool select;
	}
	COMMAND;

static HIGHLIGHT_NAME _highlight_name[] = {
	{ HIGHLIGHT_HTML, "_DoHtml" },
	{ HIGHLIGHT_CSS, "_DoCss" },
	{ HIGHLIGHT_WEBPAGE, "_DoWebpage" },
	{ HIGHLIGHT_DIFF, "_DoDiff" },
	{ HIGHLIGHT_JAVASCRIPT, "_DoJavascript" },
	{ HIGHLIGHT_C, "_DoC" },
	{ HIGHLIGHT_CPLUSPLUS, "_DoCPlusPlus" },
	{ HIGHLIGHT_SQL, "_DoSQL" },
	{ HIGHLIGHT_CUSTOM, NULL }
};


static int _x1, _y1, _x2, _y2;
static int _style;

static int _highlight_state = 0;
static bool _highlight_alternate = false;
static int _highlight_tag = 0;
static bool _highlight_show_limit = false;
static QString _highlight_text = "";
static GHighlightArray *_highlight_data = NULL;
static int _highlight_line = -1;

static QT_PICTURE _breakpoint_picture = 0;
static QT_PICTURE _bookmark_picture = 0;

static void highlightCustom(GEditor *master, int line, uint &state, bool &alternate, int &tag, GString &s, GHighlightArray *data, bool &proc)
{
	void *_object = QT.GetObject((QWidget *)master);

	_highlight_state = state;
	_highlight_alternate = alternate;
	_highlight_tag = tag;
	_highlight_text = s.getString();
	_highlight_show_limit = proc;
	_highlight_data = data;
	_highlight_line = line;

	GB.NewArray(_highlight_data, sizeof(GHighlight), 0);

	if (DOC->getHighlightMode() == GDocument::Custom)
		GB.Raise(THIS, EVENT_Highlight, 0);
	else
		GB.Call(&THIS->highlight, 0, FALSE);
		
	state = _highlight_state;
	alternate = _highlight_alternate;
	tag = _highlight_tag;
	s = GString(_highlight_text);
	proc = _highlight_show_limit;
	_highlight_data = 0;
}

/****************************************************************************
	
	Highlight

****************************************************************************/

BEGIN_PROPERTY(Highlight_Line)

	GB.ReturnInteger(_highlight_line);

END_PROPERTY

BEGIN_PROPERTY(Highlight_State)

	if (READ_PROPERTY)
		GB.ReturnInteger(_highlight_state);
	else
		_highlight_state = VPROP(GB_INTEGER);

END_PROPERTY

BEGIN_PROPERTY(Highlight_AlternateState)

	if (READ_PROPERTY)
		GB.ReturnBoolean(_highlight_alternate);
	else
		_highlight_alternate = VPROP(GB_BOOLEAN);

END_PROPERTY

BEGIN_PROPERTY(Highlight_Tag)

	if (READ_PROPERTY)
		GB.ReturnInteger(_highlight_tag);
	else
		_highlight_tag = VPROP(GB_INTEGER);

END_PROPERTY

BEGIN_PROPERTY(Highlight_ShowLimit)

	if (READ_PROPERTY)
		GB.ReturnBoolean(_highlight_show_limit);
	else
		_highlight_show_limit = VPROP(GB_BOOLEAN);

END_PROPERTY

BEGIN_PROPERTY(Highlight_Text)

	if (READ_PROPERTY)
		GB.ReturnNewZeroString(TO_UTF8(_highlight_text));
	else
		_highlight_text = QSTRING_PROP();

END_PROPERTY

BEGIN_METHOD(Highlight_Add, GB_INTEGER state; GB_INTEGER len)

	GHighlight *h;

	if (!_highlight_data)
		return;

	int count = GB.Count(*_highlight_data) - 1;
	int state = VARG(state);
	int len = VARGOPT(len, 1);

	if (len < 1)
		return;

	if (count < 0 || (*_highlight_data)[count].state != (uint)state || (*_highlight_data)[count].alternate != _highlight_alternate || ((*_highlight_data)[count].len + len) > HIGHLIGHT_LEN_MAX)
	{
		count++;
		h = (GHighlight *)GB.Add(_highlight_data);
		h->state = state;
		h->alternate = _highlight_alternate;
		h->len = len;
	}
	else
		(*_highlight_data)[count].len += len;

END_METHOD

BEGIN_PROPERTY(Highlight_Alternate)

	GB.Deprecated("gb.qt4.ext", "Highlight.Alternate", NULL);
	GB.ReturnInteger(-1);

END_PROPERTY


/****************************************************************************
	
	Editor

****************************************************************************/

BEGIN_PROPERTY(Editor_Text)

	if (READ_PROPERTY)
		GB.ReturnNewZeroString(DOC->getText().utf8());
	else
		DOC->setText(QSTRING_PROP());

END_PROPERTY

BEGIN_PROPERTY(Editor_Length)

	GB.ReturnInteger(DOC->getLength());

END_PROPERTY

BEGIN_PROPERTY(Editor_EndOfLine)

	if (READ_PROPERTY)
		GB.ReturnInteger(DOC->getEndOfLine());
	else
		DOC->setEndOfLine(VPROP(GB_INTEGER));

END_PROPERTY

BEGIN_METHOD_VOID(Editor_HighlightAll)

	DOC->colorizeAll();

END_METHOD

BEGIN_PROPERTY(CEDITOR_tab_length)

	if (READ_PROPERTY)
		GB.ReturnInteger(DOC->getTabWidth());
	else
		DOC->setTabWidth(VPROP(GB_INTEGER));

END_PROPERTY

BEGIN_METHOD_VOID(CEDITOR_reset)

	DOC->reset(true);

END_METHOD

BEGIN_PROPERTY(CEDITOR_read_only)

	if (READ_PROPERTY)
		GB.ReturnBoolean(DOC->isReadOnly());
	else
		DOC->setReadOnly(VPROP(GB_BOOLEAN));

END_PROPERTY

BEGIN_METHOD_VOID(CEDITOR_clear)

	DOC->clear();

END_METHOD

BEGIN_METHOD(CEDITOR_insert, GB_STRING str; GB_INTEGER y; GB_INTEGER x)

	if (MISSING(y) || MISSING(x))
		WIDGET->insert(QSTRING_ARG(str));
	else
		DOC->insert(VARG(y), VARG(x), QSTRING_ARG(str));

END_METHOD

static void print_text(void *_object, const char *str, int lstr, bool esc = false)
{
	QString s = QString::fromUtf8(str, lstr);
	int line, col;
	uint i, len;
	
	WIDGET->getCursor(&line, &col);
	if (col == 0)
	{
		//qDebug("remove: %p: %d, %d, %d, %d", THIS, line, 0, line, DOC->lineLength(line));
		DOC->remove(line, 0, line, DOC->lineLength(line));
		WIDGET->cursorGoto(line, 0, false);
	}
// 	if (col < DOC->lineLength(line))
// 	{
// 		end = col + s.length();
// 		if (end > DOC->lineLength(line))
// 			end = DOC->lineLength(line);
// 		DOC->remove(line, col, line, end);
// 	}
	
	if (!esc)
	{
		i = 0;
		for (;;)
		{
			if (col == MAX_CONSOLE_WIDTH)
			{
				WIDGET->insert("\n");
				col = 0;
			}
			len = s.length() - i;
			if ((col + len) >= MAX_CONSOLE_WIDTH)
				len = MAX_CONSOLE_WIDTH - col;
			WIDGET->insert(s.mid(i, len));
			i += len;	
			if (i >= (uint)s.length())
				break;
			col += len;
		}
	}
	else
	{
		if (col >= MAX_CONSOLE_WIDTH)
			WIDGET->insert("\n");
		WIDGET->insert(s);
	}
}

BEGIN_METHOD(CEDITOR_print, GB_STRING str; GB_INTEGER y; GB_INTEGER x)

	char *str = STRING(str);
	int len = LENGTH(str);
	int i, j;
	int line, col;
	unsigned char c;

	if (!MISSING(y) && !MISSING(x))
		WIDGET->cursorGoto(VARG(y), VARG(x), false);
	
	j = 0;
	for (i = 0; i < len; i++)
	{
		//if (s[i].latin1() < 32)
		//	qDebug("%d", s[i].latin1());
		
		c = str[i];
		if (c < 32)
		{
			if (i > j)
				print_text(THIS, &str[j], i - j);
				
			j = i + 1;
			
			WIDGET->getCursor(&line, &col);		
			
			if (c == '\t')
			{
				//int l = 8 - col % 8;
				//print_text(THIS, "        ", l);
				WIDGET->insert("\t");
			}
			else if (c == '\r')
			{
				WIDGET->cursorGoto(line, 0, false);
			}
			else if (c == '\n')
			{
				WIDGET->cursorGoto(line, DOC->lineLength(line), false);
				WIDGET->insert("\n");
			}
			else if (c == '\f') // CTRL+L
			{
				DOC->clear();
			}
			else if (c == '\a')
			{
				WIDGET->flash();
			}
			else
			{
				QString tmp;
				tmp.sprintf("^%c", c + 64);
				print_text(THIS, tmp, 2, true);
			}
		}
	}

	if (i > j)
		print_text(THIS, &str[j], i - j);
		
END_METHOD


BEGIN_METHOD(CEDITOR_remove, GB_INTEGER y; GB_INTEGER x; GB_INTEGER y2; GB_INTEGER x2)

	DOC->remove(VARG(y), VARG(x), VARG(y2), VARG(x2));

END_METHOD

BEGIN_METHOD_VOID(CEDITOR_undo)

	WIDGET->undo();

END_METHOD

BEGIN_METHOD_VOID(CEDITOR_redo)

	WIDGET->redo();

END_METHOD

BEGIN_METHOD_VOID(CEDITOR_begin)

	DOC->begin();

END_METHOD

BEGIN_METHOD_VOID(CEDITOR_end)

	DOC->end();

END_METHOD

BEGIN_METHOD(CEDITOR_find_next_breakpoint, GB_INTEGER line)

	int line = VARG(line);

	GB.Deprecated("gb.qt4.ext", "Editor.FindNextBreakpoint", "Editor.Breakpoints");
	
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

static void return_flagged_lines(CEDITOR *_object, int flag)
{
	GB_ARRAY array;
	int i;
	
	GB.Array.New(&array, GB_T_INTEGER, 0);
	
	for (i = 0; i < DOC->numLines(); i++)
	{
		if (DOC->getLineFlag(i, flag))
			*((int *)GB.Array.Add(array)) = i;
	}
	
	GB.ReturnObject(array);
}

static void set_flagged_lines(CEDITOR *_object, int flag, GB_ARRAY array)
{
	int i, line;
	
	if (GB.CheckObject(array))
		return;
	
	for (i = 0; i < DOC->numLines(); i++)
	{
		if (DOC->getLineFlag(i, flag))
			DOC->setLineFlag(i, flag, false);
	}
	
	for (i = 0; i < GB.Array.Count(array); i++)
	{
		line = *((int *)GB.Array.Get(array, i));
		DOC->setLineFlag(line, flag, true);
	}
}

BEGIN_PROPERTY(Editor_Breakpoints)

	if (READ_PROPERTY)
		return_flagged_lines(THIS, GLine::BreakpointFlag);
	else
		set_flagged_lines(THIS, GLine::BreakpointFlag, VPROP(GB_OBJECT));

END_PROPERTY

BEGIN_PROPERTY(Editor_Bookmarks)

	if (READ_PROPERTY)
		return_flagged_lines(THIS, GLine::BookmarkFlag);
	else
		set_flagged_lines(THIS, GLine::BookmarkFlag, VPROP(GB_OBJECT));

END_PROPERTY

BEGIN_PROPERTY(Editor_CurrentLine)

	if (READ_PROPERTY)
		GB.ReturnInteger(DOC->currentLine());
	else
		DOC->setCurrentLine(VPROP(GB_INTEGER));

END_PROPERTY

BEGIN_METHOD(CEDITOR_find_next_limit, GB_INTEGER line)

	GB.ReturnInteger(DOC->getNextLimit(VARG(line)));

END_METHOD

BEGIN_METHOD(CEDITOR_find_next_word, GB_STRING word; GB_INTEGER line)

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


BEGIN_PROPERTY(CEDITOR_highlight)

	HIGHLIGHT_NAME *p;
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
		else if (mode > GDocument::Custom)
		{
			if (GB.Component.Load("gb.eval.highlight"))
			{
				GB.Error("Cannot load Gambas custom syntax highlighter component");
				return;
			}
		
			p = _highlight_name;
			while (p->name)
			{
				if (p->style == mode)
				{
					if (GB.GetFunction(&THIS->highlight, (void *)GB.FindClass("Highlight"), p->name, "", "") == 0)
					{
						p = 0;
						break;
					}
				}
				p++;
			}
			
			if (p)
				mode = GDocument::Custom;
		}
		
		DOC->setHighlightMode(mode, highlightCustom);
	}

END_PROPERTY


/****************************************************************************
	
	Editor.Selection

****************************************************************************/

BEGIN_PROPERTY(CEDITOR_selected)

	GB.ReturnBoolean(WIDGET->hasSelection());

END_PROPERTY

BEGIN_PROPERTY(CEDITOR_sel)

	if (WIDGET->hasSelection())
		WIDGET->getSelection(&_y1, &_x1, &_y2, &_x2);
	else
		_x1 = _y1 = _x2 = _y2 = -1;

	RETURN_SELF();

END_PROPERTY

BEGIN_PROPERTY(CEDITOR_sel_text)

	GB.ReturnNewZeroString(WIDGET->getSelectedText().utf8());

END_PROPERTY

BEGIN_PROPERTY(CEDITOR_sel_start_line)

	GB.ReturnInteger(_y1);

END_PROPERTY

BEGIN_PROPERTY(CEDITOR_sel_start_column)

	GB.ReturnInteger(_x1);

END_PROPERTY

BEGIN_PROPERTY(CEDITOR_sel_end_line)

	GB.ReturnInteger(_y2);

END_PROPERTY

BEGIN_PROPERTY(CEDITOR_sel_end_column)

	GB.ReturnInteger(_x2);

END_PROPERTY

BEGIN_METHOD_VOID(CEDITOR_sel_hide)

	WIDGET->hideSelection();

END_METHOD



/****************************************************************************
	
	Editor.Lines

****************************************************************************/

BEGIN_PROPERTY(CEDITOR_lines_count)

	GB.ReturnInteger((int)DOC->numLines());

END_PROPERTY

BEGIN_METHOD(CEDITOR_lines_get, GB_INTEGER line)

	int line = VARG(line);

	if (line < 0 || line >= DOC->numLines())
		GB.ReturnNull();
	else
	{
		THIS->line = line;
		RETURN_SELF();
	}

END_METHOD

BEGIN_METHOD_VOID(Editor_ExpandAll)

	WIDGET->unfoldAll();

END_METHOD

BEGIN_METHOD_VOID(Editor_CollapseAll)

	WIDGET->foldAll();

END_METHOD

/****************************************************************************
	
	Editor.Line

****************************************************************************/

BEGIN_PROPERTY(CEDITOR_line_text)

	if (READ_PROPERTY)
		GB.ReturnNewZeroString(DOC->getLine(THIS->line).utf8());
	else
	{
		GString s = GString(QSTRING_PROP());
		DOC->setLine(THIS->line, s);
	}

END_PROPERTY

BEGIN_PROPERTY(CEDITOR_line_length)

	GB.ReturnInteger(DOC->lineLength(THIS->line));

END_PROPERTY

BEGIN_PROPERTY(CEDITOR_line_expanded)

	if (READ_PROPERTY)
		GB.ReturnBoolean(!WIDGET->isFolded(THIS->line));
	else
	{
		if (VPROP(GB_BOOLEAN))
			WIDGET->unfoldLine(THIS->line);
		else
			WIDGET->foldLine(THIS->line);
	}

END_PROPERTY

BEGIN_PROPERTY(EditorLine_Limit)

	GB.ReturnBoolean(DOC->hasLimit(THIS->line));

END_PROPERTY

/*BEGIN_PROPERTY(EditorLine_Edited)

	GB.ReturnBoolean(DOC->isLineEditedSomewhere(THIS->line));

END_PROPERTY*/

BEGIN_METHOD_VOID(CEDITOR_line_get_initial_state)

	uint state;
	int tag;
	bool alternate;
	
	DOC->getState(THIS->line, true, state, tag, alternate);
	
	_highlight_state = state;
	_highlight_tag = tag;
	_highlight_alternate = alternate;

END_METHOD

BEGIN_METHOD_VOID(CEDITOR_line_refresh)

	DOC->invalidate(THIS->line);
	DOC->colorize(THIS->line);
	WIDGET->updateLine(THIS->line);

END_METHOD

BEGIN_PROPERTY(EditorLine_Bookmark)

	if (READ_PROPERTY)
		GB.ReturnBoolean(DOC->getLineFlag(THIS->line, GLine::BookmarkFlag));
	else
		DOC->setLineFlag(THIS->line, GLine::BookmarkFlag, VPROP(GB_BOOLEAN));

END_PROPERTY

BEGIN_PROPERTY(EditorLine_Breakpoint)

	if (READ_PROPERTY)
		GB.ReturnBoolean(DOC->getLineFlag(THIS->line, GLine::BreakpointFlag));
	else
		DOC->setLineFlag(THIS->line, GLine::BreakpointFlag, VPROP(GB_BOOLEAN));

END_PROPERTY

BEGIN_METHOD(CEDITOR_line_purge, GB_BOOLEAN comment; GB_BOOLEAN string; GB_STRING replace)

	bool comment = VARGOPT(comment, FALSE);
	bool string = VARGOPT(string, FALSE);
	GString line, result;
	uint i, state;
	GString replace;
	
	if (MISSING(replace))
		replace = " ";
	else
		replace = QSTRING_ARG(replace);
	
	line = DOC->getLine(THIS->line);
	
	/*if (comment && string)	
	{
		GB.ReturnNewZeroString(line.utf8());
		return;
	}*/
	
	for (i = 0; i < line.length(); i++)
	{
		state = DOC->getCharState(THIS->line, i);
		if ((!string && state == GLine::String) || (!comment && (state == GLine::Comment || state == GLine::Help)))
			result += replace;
		else
			result += line.at(i);
	}
	
	GB.ReturnNewZeroString(result.utf8());

END_METHOD



/****************************************************************************
	
	Editor (again)

****************************************************************************/

BEGIN_METHOD(CEDITOR_new, GB_OBJECT parent)

	GEditor *wid = new GEditor(QT.GetContainer(VARG(parent)));

	QObject::connect(wid, SIGNAL(cursorMoved()), &CEditor::manager, SLOT(moved()));
	QObject::connect(wid, SIGNAL(textChanged()), &CEditor::manager, SLOT(changed()));
	QObject::connect(wid, SIGNAL(marginDoubleClicked(int)), &CEditor::manager, SLOT(marginDoubleClicked(int)));
	QObject::connect(wid, SIGNAL(contentsMoving(int, int)), &CEditor::manager, SLOT(scrolled(int, int)));

	QT.InitWidget(wid, _object, true);

	THIS->line = -1;

	wid->show();

END_METHOD

BEGIN_METHOD_VOID(CEDITOR_free)

	GB.Unref((void **)&THIS->view);

END_METHOD

BEGIN_METHOD_VOID(CEDITOR_exit)

	GB.Unref((void **)&_breakpoint_picture);	
	GB.Unref((void **)&_bookmark_picture);	

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

	GB.ReturnBoolean(WIDGET->getFlag(VARG(flag)));

END_METHOD

BEGIN_METHOD(CEDITOR_flags_set, GB_BOOLEAN value; GB_INTEGER flag)

	WIDGET->setFlag(VARG(flag), VARG(value));

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

BEGIN_METHOD(CEDITOR_to_pos_x, GB_INTEGER x; GB_INTEGER y)

	int line, col;
	int px, py;

	WIDGET->getCursor(&line, &col);
	WIDGET->cursorToPos(VARGOPT(y, line), VARG(x), &px, &py);
	GB.ReturnInteger(px);

END_METHOD

BEGIN_METHOD(CEDITOR_pos_to_line, GB_INTEGER y)

	int line = WIDGET->posToLine(VARG(y));
	
	if (WIDGET->isPosOutside())
		GB.ReturnInteger(-1);
	else
		GB.ReturnInteger(line);
	
END_METHOD

BEGIN_METHOD(CEDITOR_pos_to_column, GB_INTEGER x; GB_INTEGER y)

	int line, col;

	if (WIDGET->posToCursor(VARG(x), VARG(y), &line, &col))
		GB.ReturnInteger(-1);
	else
		GB.ReturnInteger(col);

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

BEGIN_PROPERTY(CEDITOR_style_background)

	GHighlightStyle style;

	WIDGET->getStyle(_style, &style);

	if (READ_PROPERTY)
	{
		if (style.background)
			GB.ReturnInteger(style.backgroundColor.rgb() & 0xFFFFFF);
		else
			GB.ReturnInteger(-1);
	}
	else
	{
		if (VPROP(GB_INTEGER) == -1)
			style.background = false;
		else
		{
			style.background = true;
			style.backgroundColor = QColor(VPROP(GB_INTEGER) & 0xFFFFFF);
		}
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

	GB.ReturnInteger(WIDGET->getCharWidth('m'));

END_PROPERTY

BEGIN_PROPERTY(Editor_BreakpointPicture)

	if (READ_PROPERTY)
		GB.ReturnObject(_breakpoint_picture);
	else
	{
		GB.StoreObject(PROP(GB_OBJECT), (void **)&_breakpoint_picture);
		GEditor::setBreakpointPixmap(QT.GetPixmap(_breakpoint_picture));
	}

END_PROPERTY

BEGIN_PROPERTY(Editor_BookmarkPicture)

	if (READ_PROPERTY)
		GB.ReturnObject(_bookmark_picture);
	else
	{
		GB.StoreObject(PROP(GB_OBJECT), (void **)&_bookmark_picture);
		GEditor::setBookmarkPixmap(QT.GetPixmap(_bookmark_picture));
	}

END_PROPERTY

BEGIN_PROPERTY(CEDITOR_keywords_ucase)

	if (READ_PROPERTY)
		GB.ReturnBoolean(DOC->isKeywordsUseUpperCase());
	else
		DOC->setKeywordsUseUpperCase(VPROP(GB_BOOLEAN));

END_PROPERTY

BEGIN_METHOD(CEDITOR_show_string, GB_STRING str; GB_BOOLEAN ignoreCase)

	GString s;
	
	if (!MISSING(str))
		s = GString(QSTRING_ARG(str));
	
	WIDGET->showString(s, VARGOPT(ignoreCase, false));

END_METHOD

/*BEGIN_METHOD(CEDITOR_show_word, GB_INTEGER line; GB_INTEGER col; GB_INTEGER len)

	int line = VARGOPT(line, -1);
	int col = VARGOPT(col, -1);
	int len = VARGOPT(len, -1);

	WIDGET->showWord(line, col, len);

END_METHOD*/

BEGIN_PROPERTY(CEDITOR_border)

	//QT.BorderProperty(_object, _param);
	if (READ_PROPERTY)
		GB.ReturnBoolean(WIDGET->hasBorder());
	else
		WIDGET->setBorder(VPROP(GB_BOOLEAN));

END_PROPERTY

BEGIN_PROPERTY(Editor_ScrollBar)

	int scroll;

	if (READ_PROPERTY)
	{
		scroll = 0;
		if (WIDGET->hScrollBarMode() == Q3ScrollView::Auto)
			scroll += 1;
		if (WIDGET->vScrollBarMode() == Q3ScrollView::Auto)
			scroll += 2;

		GB.ReturnInteger(scroll);
	}
	else
	{
		scroll = VPROP(GB_INTEGER) & 3;
		WIDGET->setHScrollBarMode((scroll & 1) ? Q3ScrollView::Auto : Q3ScrollView::AlwaysOff);
		WIDGET->setVScrollBarMode((scroll & 2) ? Q3ScrollView::Auto : Q3ScrollView::AlwaysOff);
	}

END_PROPERTY

BEGIN_PROPERTY(Editor_ScrollX)

	if (READ_PROPERTY)
		GB.ReturnInteger(WIDGET->horizontalScrollBar()->value());
	else
		WIDGET->horizontalScrollBar()->setValue(VPROP(GB_INTEGER));

END_PROPERTY

BEGIN_PROPERTY(Editor_ScrollY)

	if (READ_PROPERTY)
		GB.ReturnInteger(WIDGET->verticalScrollBar()->value());
	else
		WIDGET->verticalScrollBar()->setValue(VPROP(GB_INTEGER));

END_PROPERTY

BEGIN_METHOD(Editor_Scroll, GB_INTEGER x; GB_INTEGER y)

	WIDGET->horizontalScrollBar()->setValue(VARG(x));
	WIDGET->verticalScrollBar()->setValue(VARG(y));

END_METHOD

BEGIN_PROPERTY(Editor_LineOffset)

	if (READ_PROPERTY)
		GB.ReturnInteger(WIDGET->lineOffset());
	else
		WIDGET->setLineOffset(VPROP(GB_INTEGER));

END_PROPERTY

BEGIN_PROPERTY(Editor_Mouse)

	QT.MouseProperty(_object, _param);

	if (!READ_PROPERTY)
		WIDGET->saveCursor();

END_PROPERTY

#if 0
BEGIN_METHOD(Editor_Command, GB_STRING command)

	static COMMAND CommandList[] =
	{
		{ "move-left", &&__MOVE_LEFT },
		{ "move-left-word", &&__MOVE_LEFT_WORD },
		{ "select-left", &&__MOVE_LEFT, TRUE },
		{ "select-left-word", &&__MOVE_LEFT_WORD, TRUE },
		{ "move-right", &&__MOVE_RIGHT },
		{ "move-right-word", &&__MOVE_RIGHT_WORD },
		{ "select-right", &&__MOVE_RIGHT, TRUE },
		{ "select-right-word", &&__MOVE_RIGHT_WORD, TRUE },
		{ "move-up", &&__MOVE_UP },
		{ "move-right-word", &&__MOVE_RIGHT_WORD },
		{ "select-right", &&__MOVE_RIGHT, TRUE },
		{ "select-right-word", &&__MOVE_RIGHT_WORD, TRUE },
	};

END_METHOD
#endif

#if 0
BEGIN_PROPERTY(Editor_ShowLimits)

	int limit;

	if (READ_PROPERTY)
	{
		if (!WIDGET->getFlag(GEditor::ShowProcedureLimits))
			limit = LIMIT_NONE;
		else if (WIDGET->getFlag(GEditor::BlendedProcedureLimits))
			limit = LIMIT_BLEND;
		else if (WIDGET->getFlags(GEditor::ChangeBackgroundAtLimit))
			limit = LIMIT_BACKGROUND;
		else
			limit = LIMIT_LINE;
		
		GB.ReturnInteger(limit);
	}
	else
	{
		limit = VPROP(GB_INTEGER);
		
		WIDGET->setFlag(GEditor::ShowProcedureLimits, limit != LIMIT_NONE);
		switch(limit)
		{
			case LIMIT_BLEND:
				WIDGET->setFlag(GEditor::BlendedProcedureLimits, true);
				WIDGET->setFlag(GEditor::ChangeBackgroundAtLimit, false);
				break;
			case LIMIT_BACKGROUND:
				WIDGET->setFlag(GEditor::BlendedProcedureLimits, false);
				WIDGET->setFlag(GEditor::ChangeBackgroundAtLimit, true);
				break;
			default:
				WIDGET->setFlag(GEditor::BlendedProcedureLimits, false);
				WIDGET->setFlag(GEditor::ChangeBackgroundAtLimit, false);
				break;
		}
	}

END_PROPERTY

#define IMPLEMENT_FLAG_PROPERTY(_name, _constant) \
BEGIN_PROPERTY(Editor_##_name) \
\
	if (READ_PROPERTY) \
		GB.ReturnBoolean(WIDGET->getFlag(GEditor::_constant)); \
	else \
		WIDGET->setFlag(GEditor::_constant, VPROP(GB_BOOLEAN)); \
\
END_PROPERTY

IMPLEMENT_FLAG_PROPERTY(ShowLimits, ShowProcedureLimits)

	GB_CONSTANT("ShowLimits", "i", GEditor::ShowProcedureLimits),
	GB_CONSTANT("BlendedLimits", "i", GEditor::BlendedProcedureLimits),
	GB_CONSTANT("BackgroundLimits", "i", GEditor::ChangeBackgroundAtLimit),
	//GB_CONSTANT("DrawWithRelief", "i", GEditor::DrawWithRelief),
	GB_CONSTANT("ShowModifiedLines", "i", GEditor::ShowModifiedLines),
	GB_CONSTANT("ShowCurrentLine", "i", GEditor::ShowCurrentLine),
	GB_CONSTANT("ShowLineNumbers", "i", GEditor::ShowLineNumbers),
	GB_CONSTANT("HighlightBraces", "i", GEditor::HighlightBraces),
	GB_CONSTANT("HighlightImmediately", "i", GEditor::HighlightImmediately),
	GB_CONSTANT("ShowDots", "i", GEditor::ShowDots),
	//GB_CONSTANT("ShowCursorPosition", "i", GEditor::ShowCursorPosition),
	GB_CONSTANT("HideMargin", "i", GEditor::HideMargin),
	Gb_CONSTANT("BlinkCursor", "i", GEditor::BlinkCursor),
#endif

/***************************************************************************/

GB_DESC CHighlightDesc[] =
{
	GB_DECLARE("Highlight", 0), GB_VIRTUAL_CLASS(),

	GB_CONSTANT("None", "i", GDocument::None),
	GB_CONSTANT("Gambas", "i", GDocument::Gambas),
	GB_CONSTANT("Custom", "i", GDocument::Custom),
	GB_CONSTANT("HTML", "i", HIGHLIGHT_HTML),
	GB_CONSTANT("CSS", "i", HIGHLIGHT_CSS),
	GB_CONSTANT("WebPage", "i", HIGHLIGHT_WEBPAGE),
	GB_CONSTANT("Diff", "i", HIGHLIGHT_DIFF),
	GB_CONSTANT("JavaScript", "i", HIGHLIGHT_JAVASCRIPT),
	GB_CONSTANT("C", "i", HIGHLIGHT_C),
	GB_CONSTANT("CPlusPlus", "i", HIGHLIGHT_CPLUSPLUS),
	GB_CONSTANT("SQL", "i", HIGHLIGHT_SQL),

	GB_CONSTANT("Background", "i", HIGHLIGHT_BACKGROUND),
	GB_CONSTANT("Normal", "i", HIGHLIGHT_NORMAL),
	GB_CONSTANT("Keyword", "i", HIGHLIGHT_KEYWORD),
	GB_CONSTANT("Function", "i", HIGHLIGHT_SUBR),
	GB_CONSTANT("Operator", "i", HIGHLIGHT_OPERATOR),
	GB_CONSTANT("Symbol", "i", HIGHLIGHT_SYMBOL),
	GB_CONSTANT("Number", "i", HIGHLIGHT_NUMBER),
	GB_CONSTANT("String", "i", HIGHLIGHT_STRING),
	GB_CONSTANT("Comment", "i", HIGHLIGHT_COMMENT),
	GB_CONSTANT("Breakpoint", "i", HIGHLIGHT_BREAKPOINT),
	GB_CONSTANT("Current", "i", HIGHLIGHT_CURRENT),
	GB_CONSTANT("DataType", "i", HIGHLIGHT_DATATYPE),
	GB_CONSTANT("Selection", "i", HIGHLIGHT_SELECTION),
	GB_CONSTANT("Highlight", "i", HIGHLIGHT_HIGHLIGHT),
	GB_CONSTANT("CurrentLine", "i", HIGHLIGHT_LINE),
	GB_CONSTANT("Error", "i", HIGHLIGHT_ERROR),
	GB_CONSTANT("Help", "i", HIGHLIGHT_HELP),
	GB_CONSTANT("Preprocessor", "i", HIGHLIGHT_PREPROCESSOR),
	GB_STATIC_PROPERTY_READ("Alternate", "i", Highlight_Alternate),

	GB_STATIC_PROPERTY_READ("Line", "i", Highlight_Line),
	GB_STATIC_PROPERTY("State", "i", Highlight_State),
	GB_STATIC_PROPERTY("Tag", "i", Highlight_Tag),
	GB_STATIC_PROPERTY("ShowLimit", "b", Highlight_ShowLimit),
	GB_STATIC_PROPERTY("Text", "s", Highlight_Text),
	GB_STATIC_PROPERTY("AlternateState", "b", Highlight_AlternateState),
	GB_STATIC_METHOD("Add", NULL, Highlight_Add, "(State)i[(Count)i]"),

	GB_END_DECLARE
};


GB_DESC CEditorSelectionDesc[] =
{
	GB_DECLARE(".Editor.Selection", 0), GB_VIRTUAL_CLASS(),

	GB_PROPERTY_READ("Text", "s", CEDITOR_sel_text),
	GB_PROPERTY_READ("StartLine", "i", CEDITOR_sel_start_line),
	GB_PROPERTY_READ("StartColumn", "i", CEDITOR_sel_start_column),
	GB_PROPERTY_READ("EndLine", "i", CEDITOR_sel_end_line),
	GB_PROPERTY_READ("EndColumn", "i", CEDITOR_sel_end_column),

	GB_METHOD("Hide", NULL, CEDITOR_sel_hide, NULL),

	GB_END_DECLARE
};

GB_DESC CEditorLineDesc[] =
{
	GB_DECLARE(".Editor.Line", 0), GB_VIRTUAL_CLASS(),

	GB_PROPERTY("Text", "s", CEDITOR_line_text),
	GB_PROPERTY_READ("Length", "i", CEDITOR_line_length),
	GB_PROPERTY("Expanded", "b", CEDITOR_line_expanded),
	GB_PROPERTY("Bookmark", "b", EditorLine_Bookmark),
	GB_PROPERTY("Breakpoint", "b", EditorLine_Breakpoint),
	GB_PROPERTY_READ("Limit", "b", EditorLine_Limit),
	//GB_PROPERTY_READ("Edited", "b", EditorLine_Edited),
	GB_METHOD("GetInitialState", NULL, CEDITOR_line_get_initial_state, NULL),
	GB_METHOD("Refresh", NULL, CEDITOR_line_refresh, NULL),
	GB_METHOD("Purge", "s", CEDITOR_line_purge, "[(Comment)b(String)b(Replace)s]"),

	GB_END_DECLARE
};

GB_DESC CEditorLinesDesc[] =
{
	GB_DECLARE(".Editor.Lines", 0), GB_VIRTUAL_CLASS(),

	GB_METHOD("_get", ".Editor.Line", CEDITOR_lines_get, "(Line)i"),
	GB_PROPERTY_READ("Count", "i", CEDITOR_lines_count),

	GB_END_DECLARE
};

GB_DESC CEditorStyleDesc[] =
{
	GB_DECLARE(".Editor.Style", 0), GB_VIRTUAL_CLASS(),

	GB_PROPERTY("Color", "i", CEDITOR_style_color),
	GB_PROPERTY("Background", "i", CEDITOR_style_background),
	GB_PROPERTY("Foreground", "i", CEDITOR_style_color),
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

	GB_CONSTANT("ShowLimits", "i", GEditor::ShowProcedureLimits),
	GB_CONSTANT("BlendedLimits", "i", GEditor::BlendedProcedureLimits),
	GB_CONSTANT("BackgroundLimits", "i", GEditor::ChangeBackgroundAtLimit),
	GB_CONSTANT("ShowModifiedLines", "i", GEditor::ShowModifiedLines),
	GB_CONSTANT("ShowCurrentLine", "i", GEditor::ShowCurrentLine),
	GB_CONSTANT("ShowLineNumbers", "i", GEditor::ShowLineNumbers),
	GB_CONSTANT("HighlightBraces", "i", GEditor::HighlightBraces),
	GB_CONSTANT("HighlightImmediately", "i", GEditor::HighlightImmediately),
	GB_CONSTANT("ShowDots", "i", GEditor::ShowDots),
	GB_CONSTANT("HideMargin", "i", GEditor::HideMargin),
	GB_CONSTANT("NoFolding", "i", GEditor::NoFolding),

	GB_METHOD("_new", NULL, CEDITOR_new, "(Parent)Container;"),
	GB_METHOD("_free", NULL, CEDITOR_free, NULL),
	GB_STATIC_METHOD("_exit", NULL, CEDITOR_exit, NULL),
	
	GB_STATIC_PROPERTY("BreakpointPicture", "Picture", Editor_BreakpointPicture),
	GB_STATIC_PROPERTY("BookmarkPicture", "Picture", Editor_BookmarkPicture),

	GB_PROPERTY("View", "Editor", CEDITOR_view),

	GB_PROPERTY_READ("Line", "i", CEDITOR_line),
	GB_PROPERTY_READ("Column", "i", CEDITOR_column),
	GB_METHOD("Goto", NULL, CEDITOR_goto, "(Line)i(Column)i[(Center)b]"),

	GB_PROPERTY_SELF("Flags", ".Editor.Flags"),
	
	GB_CONSTANT("NoLimit", "i", LIMIT_NONE),
	GB_CONSTANT("LineLimit", "i", LIMIT_LINE),
	GB_CONSTANT("BlendLimit","i", LIMIT_BLEND),
	GB_CONSTANT("BackgroundLimit", "i", LIMIT_BACKGROUND),
	
	/*GB_PROPERTY("ShowLimits", "i", Editor_ShowLimits),
	GB_PROPERTY("ShowModifiedLines", "b", Editor_ShowModifiedLines),
	GB_PROPERTY("ShowCurrentLine", "b", Editor_ShowCurrentLines),
	GB_PROPERTY("ShowLineNumbers", "b", Editor_ShowLineNumbers),
	GB_PROPERTY("HighlightBraces", "b", Editor_HighlightBraces),
	GB_PROPERTY("HighlightImmediately", "b", Editor_HighlightImmediately),
	GB_PROPERTY("ShowDots", "b", Editor_ShowDots),
	GB_PROPERTY("HideMargin", "b", Editor_HideMargin),
	GB_PROPERTY("BlinkCursor", "b", Editor_BlinkCursor),*/

	GB_METHOD("Indent", NULL, CEDITOR_indent, NULL),
	GB_METHOD("Unindent", NULL, CEDITOR_unindent, NULL),
	GB_METHOD("Copy", NULL, CEDITOR_copy, NULL),
	GB_METHOD("Cut", NULL, CEDITOR_cut, NULL),
	GB_METHOD("Paste", NULL, CEDITOR_paste, NULL),
	GB_METHOD("Undo", NULL, CEDITOR_undo, NULL),
	GB_METHOD("Redo", NULL, CEDITOR_redo, NULL),

	//GB_METHOD("Command", NULL, Editor_command, "(Command)s"),

	GB_PROPERTY_SELF("Styles", ".Editor.Styles"),

	GB_METHOD("Select", NULL, CEDITOR_select, "(StartLine)i(StartColumn)i(EndLine)i(EndColumn)i"),
	GB_METHOD("SelectAll", NULL, CEDITOR_select_all, NULL),

	GB_PROPERTY_READ("CursorX", "i", CEDITOR_cursor_x),
	GB_PROPERTY_READ("CursorY", "i", CEDITOR_cursor_y),
	GB_METHOD("ToPosX", "i", CEDITOR_to_pos_x, "(Column)i[(Row)i]"),
	GB_METHOD("PosToLine","i", CEDITOR_pos_to_line, "(Y)i"),
	GB_METHOD("PosToColumn","i", CEDITOR_pos_to_column, "(X)i(Y)i"),
	GB_PROPERTY_READ("LineHeight", "i", CEDITOR_line_height),
	GB_PROPERTY_READ("CharWidth", "i", CEDITOR_char_width),

	GB_PROPERTY("Border", "b", CEDITOR_border),
	GB_PROPERTY("ScrollBar", "i", Editor_ScrollBar),
	//GB_PROPERTY("Background", "i", CEDITOR_background),

	GB_METHOD("ExpandAll", NULL, Editor_ExpandAll, NULL),
	GB_METHOD("CollapseAll", NULL, Editor_CollapseAll, NULL),

	GB_METHOD("HighlightAll", NULL, Editor_HighlightAll, NULL),
	GB_PROPERTY("Text", "s", Editor_Text),
	GB_PROPERTY_READ("Length", "i", Editor_Length),
	GB_PROPERTY("EndOfLine", "i", Editor_EndOfLine),

	GB_PROPERTY("Highlight", "i", CEDITOR_highlight),
	GB_PROPERTY("KeywordsUseUpperCase", "b", CEDITOR_keywords_ucase),
	GB_METHOD("ShowString", NULL, CEDITOR_show_string, "[(String)s(IgnoreCase)b]"),
	//GB_METHOD("ShowWord", NULL, CEDITOR_show_word, "[(Line)i(Column)i(Length)i]"),

	GB_PROPERTY("TabSize", "i", CEDITOR_tab_length),
	GB_PROPERTY("LineNumberOffset", "i", Editor_LineOffset),
	GB_METHOD("Reset", NULL, CEDITOR_reset, NULL),

	GB_PROPERTY("ReadOnly", "b", CEDITOR_read_only),

	GB_METHOD("Clear", NULL, CEDITOR_clear, NULL),

	GB_METHOD("Insert", NULL, CEDITOR_insert, "(Text)s[(Line)i(Column)i]"),
	GB_METHOD("Print", NULL, CEDITOR_print, "(Text)s[(Line)i(Column)i]"),
	GB_METHOD("Remove", NULL, CEDITOR_remove, "(Line)i(Column)i(EndLine)i(EndColumn)i"),
	GB_METHOD("Begin", NULL, CEDITOR_begin, NULL),
	GB_METHOD("End", NULL, CEDITOR_end, NULL),

	GB_PROPERTY_READ("Selected", "b", CEDITOR_selected),
	GB_PROPERTY_READ("Selection", ".Editor.Selection", CEDITOR_sel),
	GB_PROPERTY_SELF("Lines", ".Editor.Lines"),

	GB_METHOD("FindNextBreakpoint", "i", CEDITOR_find_next_breakpoint, "(Line)i"),
	GB_METHOD("FindNextWord", "i", CEDITOR_find_next_word, "(Word)s(Line)i"),
	GB_METHOD("FindNextLimit", "i", CEDITOR_find_next_limit, "(Line)i"),
	//GB_METHOD("FindPreviousLimit", "i", CEDITOR_find_next_limit, "(Line)i"),
	
	GB_PROPERTY("Breakpoints", "Integer[]", Editor_Breakpoints),
	GB_PROPERTY("Bookmarks", "Integer[]", Editor_Bookmarks),
	GB_PROPERTY("CurrentLine", "i", Editor_CurrentLine),
	
	GB_PROPERTY("ScrollX", "i", Editor_ScrollX),
	GB_PROPERTY("ScrollY", "i", Editor_ScrollY),
	GB_METHOD("Scroll", NULL, Editor_Scroll, "(X)i(Y)i"),

	GB_PROPERTY("Mouse", "i", Editor_Mouse),

	GB_EVENT("Cursor", NULL, NULL, &EVENT_Cursor),
	GB_EVENT("Scroll", NULL, NULL, &EVENT_Scroll),
	GB_EVENT("Change", NULL, NULL, &EVENT_Change),
	GB_EVENT("Highlight", NULL, NULL, &EVENT_Highlight),
	GB_EVENT("Margin", NULL, "(LineNumber)i", &EVENT_Margin),
	
	EDITOR_DESCRIPTION,
	
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
	GB.Post2((GB_CALLBACK)post_event, (intptr_t)THIS, EVENT_Cursor);
}

void CEditor::scrolled(int, int)
{
	void *_object = QT.GetObject((QWidget *)sender());
	GB.Ref(THIS);
	GB.Post2((GB_CALLBACK)post_event, (intptr_t)THIS, EVENT_Scroll);
}

void CEditor::marginDoubleClicked(int line)
{
	void *object = QT.GetObject((QWidget *)sender());
	GB.Raise(object, EVENT_Margin, 1, GB_T_INTEGER, line);
}

