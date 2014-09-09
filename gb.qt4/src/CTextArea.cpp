/***************************************************************************

  CTextArea.cpp

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

#define __CTEXTAREA_CPP

#include <qapplication.h>
#include <qpalette.h>
#include <QPlainTextEdit>
#include <QTextBlock>
#include <QTextDocumentFragment>

#include "gambas.h"
#include "main.h"
#include "CConst.h"
#include "CFont.h"
#include "CTextArea.h"


DECLARE_EVENT(EVENT_Change);
DECLARE_EVENT(EVENT_Cursor);
DECLARE_EVENT(EVENT_Link);

static int get_length(void *_object)
{
	if (THIS->length < 0)
	{
		QTextBlock block = WIDGET->document()->begin();
		int len = 0;
		
		while (block.isValid())
		{
			len += block.length();
			block = block.next();
		}

		THIS->length = len - 1;
	}
	
	return THIS->length;
}

static void to_pos(QTextEdit *wid, int par, int car, int *pos)
{
	QTextCursor cursor = wid->textCursor();
	QTextBlock block = cursor.block();
	int p = 0;

	while (par)
	{
		if (!block.isValid())
			break;
		p += block.length() + 1;
		block = block.next();
		par--;
	}

	*pos = p + car;
}


static void from_pos(CTEXTAREA *_object, int pos, int *par, int *car)
{
	QTextCursor cursor = WIDGET->textCursor();
	
	if (pos >= get_length(THIS))
		cursor.movePosition(QTextCursor::End);
	else
		cursor.setPosition(pos);
	
	*par = cursor.blockNumber();
	*car = cursor.position() - cursor.block().position();
}


static void get_selection(QTextEdit *wid, int *start, int *length)
{
	QTextCursor cursor = wid->textCursor();
	
	*start = cursor.selectionStart();
	*length = cursor.selectionEnd() - *start;
}

static void update_alignment(void *_object)
{
	THIS->no_change = TRUE;
	
	QTextOption opt = WIDGET->document()->defaultTextOption();
	opt.setAlignment((Qt::Alignment)CCONST_horizontal_alignment(THIS->align, ALIGN_NORMAL, true));
	WIDGET->document()->setDefaultTextOption(opt);
	
	THIS->no_change = FALSE;
}

static void set_text_color(void *_object)
{
	QTextCharFormat fmt;
	QBrush col;
	GB_COLOR fg = CWIDGET_get_foreground((CWIDGET *)THIS);
	
	fmt = WIDGET->currentCharFormat();
	
	if (fg == COLOR_DEFAULT)
	{
		fmt.clearForeground();
		//col = WIDGET->palette().text();
	}
	else
	{
		fmt.setForeground(TO_QCOLOR(fg));
	}
	
	//WIDGET->setTextColor(col);
	THIS->no_change = TRUE;
	WIDGET->setCurrentCharFormat(fmt);
	THIS->no_change = FALSE;
}

void CTEXTAREA_set_foreground(void *_object)
{
	THIS->no_change = TRUE;
	
	QTextCursor oldCursor = WIDGET->textCursor();
	
	WIDGET->selectAll();
	
	WIDGET->setTextColor(Qt::black);
	set_text_color(THIS);
	
	WIDGET->setTextCursor(oldCursor);
	
	set_text_color(THIS);
	
	THIS->no_change = FALSE;
}


/** TextArea ***************************************************************/

BEGIN_METHOD(CTEXTAREA_new, GB_OBJECT parent)

	QTextEdit *wid = new QTextEdit(QCONTAINER(VARG(parent)));

	QObject::connect(wid, SIGNAL(textChanged()), &CTextArea::manager, SLOT(changed()));
	QObject::connect(wid, SIGNAL(cursorPositionChanged()), &CTextArea::manager, SLOT(cursor()));

	wid->setLineWrapMode(QTextEdit::NoWrap);
	wid->setAcceptRichText(false);
	
	THIS->widget.flag.fillBackground = true;
	CWIDGET_new(wid, (void *)_object);
	
	THIS->length = -1;
	THIS->align = ALIGN_NORMAL;

	wid->document()->setDocumentMargin(2);
	
END_METHOD


BEGIN_PROPERTY(CTEXTAREA_text)

	if (READ_PROPERTY)
		GB.ReturnNewZeroString(TO_UTF8(WIDGET->toPlainText()));
	else
	{
		WIDGET->document()->setPlainText(QSTRING_PROP());
		update_alignment(THIS);
		CTEXTAREA_set_foreground(THIS);
	}

END_PROPERTY


BEGIN_PROPERTY(CTEXTAREA_length)

	GB.ReturnInteger(get_length(THIS));

END_PROPERTY


BEGIN_PROPERTY(CTEXTAREA_read_only)

	if (READ_PROPERTY)
		GB.ReturnBoolean(WIDGET->isReadOnly());
	else
		WIDGET->setReadOnly(VPROP(GB_BOOLEAN));

END_PROPERTY


BEGIN_PROPERTY(CTEXTAREA_wrap)

	if (READ_PROPERTY)
		GB.ReturnBoolean(WIDGET->lineWrapMode() != QTextEdit::NoWrap);
	else
		WIDGET->setLineWrapMode(VPROP(GB_BOOLEAN) ? QTextEdit::WidgetWidth : QTextEdit::NoWrap);

END_PROPERTY


/*
BEGIN_PROPERTY(CTEXTAREA_max_length)

	int max;

	if (READ_PROPERTY)
	{
		max = WIDGET->maxLength();
		GB.ReturnInteger((max < 0) ? 0 : max);
	}
	else
	{
		max = PROPERTY(int);
		if (max <= 0)
			max = -1;
		WIDGET->setMaxLength(max);
	}

END_PROPERTY
*/

static int get_column(CTEXTAREA *_object)
{
	QTextCursor cursor = WIDGET->textCursor();
	return cursor.position() - cursor.block().position();
}

BEGIN_PROPERTY(CTEXTAREA_column)

	QTextCursor cursor = WIDGET->textCursor();
	
	if (READ_PROPERTY)
		//GB.ReturnInteger(WIDGET->textCursor().columnNumber());
		GB.ReturnInteger(get_column(THIS));
	else
	{
		int col = VPROP(GB_INTEGER);
		
		if (col <= 0)
			cursor.movePosition(QTextCursor::QTextCursor::StartOfBlock);
		else if (col >= cursor.block().length())
			cursor.movePosition(QTextCursor::QTextCursor::EndOfBlock);
		else
			cursor.setPosition(cursor.block().position() + col);
		
		WIDGET->setTextCursor(cursor);
	}

END_PROPERTY

BEGIN_PROPERTY(CTEXTAREA_line)

	QTextCursor cursor = WIDGET->textCursor();
	
	if (READ_PROPERTY)
		GB.ReturnInteger(cursor.blockNumber());
	else
	{
		int col = get_column(THIS);
		int line = VPROP(GB_INTEGER);
		
		if (line < 0)
			cursor.movePosition(QTextCursor::Start);
		else if (line >= WIDGET->document()->blockCount())
			cursor.movePosition(QTextCursor::End);
		else
		{
			cursor.setPosition(WIDGET->document()->findBlockByNumber(line).position());
			if (col > 0)
			{
				if (col >= cursor.block().length())
					cursor.movePosition(QTextCursor::QTextCursor::EndOfBlock);
				else
					cursor.setPosition(cursor.block().position() + col);
			}
		}
		
		WIDGET->setTextCursor(cursor);
	}

END_PROPERTY

BEGIN_PROPERTY(CTEXTAREA_pos)

	if (READ_PROPERTY)
	{
		GB.ReturnInteger(WIDGET->textCursor().position());
	}
	else
	{
		int pos = VPROP(GB_INTEGER);
		QTextCursor cursor = WIDGET->textCursor();
		
		if (pos >= get_length(THIS))
			cursor.movePosition(QTextCursor::End);
		else
			cursor.setPosition(pos);
		
		WIDGET->setTextCursor(cursor);
	}

END_PROPERTY


/*
BEGIN_METHOD(CTEXTAREA_select, int line; int col; int selline; int selcol)

	MyMultiLineEdit *wid = QMULTILINEEDIT(_object);

	int line = PARAM(line);
	int col = PARAM(col);

	look_pos(wid, &line, &col);
	wid->setCursorPosition(line, col);

	line = PARAM(selline);
	col = PARAM(selcol);

	look_pos(wid, &line, &col);
	wid->setCursorPosition(line, col, TRUE);

END_METHOD
*/

BEGIN_METHOD_VOID(CTEXTAREA_clear)

	WIDGET->clear();

END_METHOD


BEGIN_METHOD(CTEXTAREA_insert, GB_STRING text)

	WIDGET->textCursor().insertText(QSTRING_ARG(text));

END_METHOD

/*
BEGIN_METHOD(CTEXTAREA_line_get, int line)

	int line = PARAM(line);

	if (line < 0 || line >= WIDGET->numLines())
		GB.ReturnVoidString();
	else
		GB.ReturnNewZeroString(WIDGET->textLine(PARAM(line)));

END_METHOD


BEGIN_PROPERTY(CTEXTAREA_line_count)

	GB.ReturnInteger(WIDGET->numLines());

END_PROPERTY
*/

/***************************************************************************

	.TextArea.Selection

***************************************************************************/


BEGIN_PROPERTY(CTEXTAREA_sel_text)

	if (READ_PROPERTY)
		GB.ReturnNewZeroString(TO_UTF8(WIDGET->textCursor().selection().toPlainText()));
	else
		WIDGET->textCursor().insertText(QSTRING_PROP());

END_PROPERTY


BEGIN_PROPERTY(CTEXTAREA_sel_length)

	int start, length;

	get_selection(WIDGET, &start, &length);
	GB.ReturnInteger(length);

END_PROPERTY


BEGIN_PROPERTY(CTEXTAREA_sel_start)

	int start, length;

	get_selection(WIDGET, &start, &length);
	GB.ReturnInteger(start);

END_PROPERTY


BEGIN_METHOD_VOID(CTEXTAREA_sel_clear)

	QTextCursor cursor = WIDGET->textCursor();
	cursor.clearSelection();	
	WIDGET->setTextCursor(cursor);

END_METHOD


BEGIN_PROPERTY(CTEXTAREA_selected)

	GB.ReturnBoolean(WIDGET->textCursor().hasSelection());

END_PROPERTY


BEGIN_METHOD(CTEXTAREA_sel_select, GB_INTEGER start; GB_INTEGER length)

	if (MISSING(start) && MISSING(length))
		WIDGET->textCursor().select(QTextCursor::Document);
	else if (!MISSING(start) && !MISSING(length))
	{
		QTextCursor cursor = WIDGET->textCursor();
		
		cursor.setPosition(VARG(start));
		cursor.setPosition(VARG(start) + VARG(length), QTextCursor::KeepAnchor);
		
		WIDGET->setTextCursor(cursor);
	}

END_METHOD


BEGIN_METHOD_VOID(CTEXTAREA_sel_all) //, GB_BOOLEAN sel)

	QTextCursor cursor = WIDGET->textCursor();
	cursor.select(QTextCursor::Document);
	WIDGET->setTextCursor(cursor);

END_METHOD


BEGIN_METHOD(CTEXTAREA_to_pos, GB_INTEGER line; GB_INTEGER col)

	int pos;

	to_pos(WIDGET, VARG(line), VARG(col), &pos);

	GB.ReturnInteger(pos);

END_METHOD


BEGIN_METHOD(CTEXTAREA_to_line, GB_INTEGER pos)

	int line, col;

	from_pos(THIS, VARG(pos), &line, &col);

	GB.ReturnInteger(line);

END_METHOD


BEGIN_METHOD(CTEXTAREA_to_col, GB_INTEGER pos)

	int line, col;

	from_pos(THIS, VARG(pos), &line, &col);

	GB.ReturnInteger(col);

END_METHOD


BEGIN_METHOD_VOID(CTEXTAREA_copy)

	WIDGET->copy();

END_METHOD


BEGIN_METHOD_VOID(CTEXTAREA_cut)

	WIDGET->cut();

END_METHOD


BEGIN_METHOD_VOID(CTEXTAREA_paste)

	WIDGET->paste();

END_METHOD


BEGIN_METHOD_VOID(CTEXTAREA_undo)

	WIDGET->undo();

END_METHOD


BEGIN_METHOD_VOID(CTEXTAREA_redo)

	WIDGET->redo();

END_METHOD


/*BEGIN_PROPERTY(CTEXTAREA_scrollbar)

	int scroll;

	if (READ_PROPERTY)
	{
		scroll = 0;
		if (WIDGET->hScrollBarMode() == QScrollView::Auto)
			scroll += 1;
		if (WIDGET->vScrollBarMode() == QScrollView::Auto)
			scroll += 2;

		GB.ReturnInteger(scroll);
	}
	else
	{
		scroll = VPROP(GB_INTEGER) & 3;
		WIDGET->setHScrollBarMode( (scroll & 1) ? QScrollView::Auto : QScrollView::AlwaysOff);
		WIDGET->setVScrollBarMode( (scroll & 2) ? QScrollView::Auto : QScrollView::AlwaysOff);
	}

END_PROPERTY*/


BEGIN_METHOD_VOID(CTEXTAREA_ensure_visible)

	WIDGET->ensureCursorVisible();

END_METHOD


BEGIN_PROPERTY(TextArea_Alignment)

	if (READ_PROPERTY)
		GB.ReturnInteger(THIS->align);
	else
	{
		THIS->align = VPROP(GB_INTEGER);
		update_alignment(THIS);
	}

END_PROPERTY

BEGIN_PROPERTY(TextArea_Border)

	CWIDGET_border_simple(_object, _param);

	if (!READ_PROPERTY)
		WIDGET->document()->setDocumentMargin(VPROP(GB_BOOLEAN) ? 2 : 0);
	
END_PROPERTY

/*
GB_DESC CTextAreaLinesDesc[] =
{
	GB_DECLARE(".TextArea.Lines", 0), GB_VIRTUAL_CLASS(),

	GB_METHOD("_get", "i", CTEXTAREA_line_get, "s"),
	GB_PROPERTY_READ("Count", "i", CTEXTAREA_line_count),

	GB_END_DECLARE
};
*/

GB_DESC CTextAreaSelectionDesc[] =
{
	GB_DECLARE(".TextArea.Selection", 0), GB_VIRTUAL_CLASS(),

	GB_PROPERTY("Text", "s", CTEXTAREA_sel_text),
	GB_PROPERTY_READ("Length", "i", CTEXTAREA_sel_length),
	GB_PROPERTY_READ("Start", "i", CTEXTAREA_sel_start),
	GB_PROPERTY_READ("Pos", "i", CTEXTAREA_sel_start),

	//GB_METHOD("Clear", NULL, CTEXTAREA_sel_clear, NULL),
	GB_METHOD("Hide", NULL, CTEXTAREA_sel_clear, NULL),

	GB_END_DECLARE
};

GB_DESC CTextAreaDesc[] =
{
	GB_DECLARE("TextArea", sizeof(CTEXTAREA)), GB_INHERITS("Control"),

	GB_METHOD("_new", NULL, CTEXTAREA_new, "(Parent)Container;"),

	GB_PROPERTY("Text", "s", CTEXTAREA_text),
	GB_PROPERTY_READ("Length", "i", CTEXTAREA_length),
	GB_PROPERTY("ReadOnly", "b", CTEXTAREA_read_only),

	//GB_PROPERTY_READ("Lines", ".TextArea.Line", CTEXTAREA_line_or_selection),

	GB_PROPERTY("ScrollBar", "i", CWIDGET_scrollbar),
	GB_PROPERTY("Wrap", "b", CTEXTAREA_wrap),
	GB_PROPERTY("Border", "b", TextArea_Border),
	GB_PROPERTY("Alignment", "i", TextArea_Alignment),

	GB_PROPERTY("Line", "i", CTEXTAREA_line),
	GB_PROPERTY("Column", "i", CTEXTAREA_column),
	GB_PROPERTY("Pos", "i", CTEXTAREA_pos),

	GB_PROPERTY_SELF("Selection", ".TextArea.Selection"),
	GB_METHOD("Select", NULL, CTEXTAREA_sel_select, "[(Start)i(Length)i]"),
	GB_METHOD("SelectAll", NULL, CTEXTAREA_sel_all, NULL),
	GB_METHOD("Unselect", NULL, CTEXTAREA_sel_clear, NULL),
	GB_PROPERTY_READ("Selected", "b", CTEXTAREA_selected),
	
	GB_METHOD("Clear", NULL, CTEXTAREA_clear, NULL),
	GB_METHOD("Insert", NULL, CTEXTAREA_insert, "(Text)s"),

	GB_METHOD("Copy", NULL, CTEXTAREA_copy, NULL),
	GB_METHOD("Cut", NULL, CTEXTAREA_cut, NULL),
	GB_METHOD("Paste", NULL, CTEXTAREA_paste, NULL),
	GB_METHOD("Undo", NULL, CTEXTAREA_undo, NULL),
	GB_METHOD("Redo", NULL, CTEXTAREA_redo, NULL),

	GB_METHOD("ToPos", "i", CTEXTAREA_to_pos, "(Line)i(Column)i"),
	GB_METHOD("ToLine", "i", CTEXTAREA_to_line, "(Pos)i"),
	GB_METHOD("ToColumn", "i", CTEXTAREA_to_col, "(Pos)i"),

	GB_METHOD("EnsureVisible", NULL, CTEXTAREA_ensure_visible, NULL),

	GB_EVENT("Change", NULL, NULL, &EVENT_Change),
	GB_EVENT("Cursor", NULL, NULL, &EVENT_Cursor),

	TEXTAREA_DESCRIPTION,

	GB_END_DECLARE
};


/** CTextArea **************************************************************/

CTextArea CTextArea::manager;

void CTextArea::changed(void)
{
	GET_SENDER();
	
	if (THIS->no_change)
		return;
	
	set_text_color(THIS);
	THIS->length = -1;
	GB.Raise(THIS, EVENT_Change, 0);
}

void CTextArea::cursor(void)
{
	GET_SENDER();
	//set_text_color(THIS);
	GB.Raise(THIS, EVENT_Cursor, 0);
}

void CTextArea::link(const QString &path)
{
	GET_SENDER();
	GB.Raise(THIS, EVENT_Link, 1, GB_T_STRING, TO_UTF8(path), 0);
}
