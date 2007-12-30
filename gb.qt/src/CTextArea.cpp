/***************************************************************************

  CTextArea.cpp

  The TextArea class

  (c) 2000-2005 Beno� Minisini <gambas@users.sourceforge.net>

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

#define __CTEXTAREA_CPP

#include <qapplication.h>
#include <qpalette.h>
#include <qtextedit.h>
#include <qscrollview.h>

#include "gambas.h"

#include "CTextArea.h"


DECLARE_EVENT(EVENT_Change);
DECLARE_EVENT(EVENT_Cursor);


static void to_pos(QTextEdit *wid, int par, int car, long *pos)
{
  int i, l;
  long p = 0;

  for (i = 0; i < par; i++)
  {
    l = wid->paragraphLength(i);
    if (l < 0)
      break;
    p += l + 1;
  }

  *pos = p + car;
}


static void from_pos(QTextEdit *wid, long pos, int *par, int *car)
{
  int i;
  int l;

  for (i = 0; i <= wid->paragraphs(); i++)
  {
    l = wid->paragraphLength(i);
    if (l < 0)
    {
      pos = wid->length();
      i--;
      break;
    }
    if (pos <= l)
      break;
    pos -= l + 1;
  }

  *par = i;
  *car = pos;
}


static void look_pos(QTextEdit *wid, int *line, int *col)
{
  if (*line == -1)
    *line = wid->paragraphs();

  if (*col == -1)
    *col = wid->paragraphLength(*line);
}


static void get_selection(QTextEdit *wid, long *start, long *length)
{
  int pStart, iStart, pEnd, iEnd;
  long posEnd;

  wid->getSelection(&pStart, &iStart, &pEnd, &iEnd);

  if (pStart < 0)
  {
    wid->getCursorPosition(&pStart, &iStart);
    to_pos(wid, pStart, iStart, start);
    *length = 0;
    return;
  }

  to_pos(wid, pStart, iStart, start);
  to_pos(wid, pEnd, iEnd, &posEnd);

  *length = posEnd - *start;
}


BEGIN_METHOD(CTEXTAREA_new, GB_OBJECT parent)

  QTextEdit *wid = new QTextEdit(QCONTAINER(VARG(parent)));

  QObject::connect(wid, SIGNAL(textChanged()), &CTextArea::manager, SLOT(changed()));
  QObject::connect(wid, SIGNAL(cursorPositionChanged(int, int)), &CTextArea::manager, SLOT(cursor()));

  CWIDGET_new(wid, (void *)_object, "TextArea");

  wid->setTextFormat(Qt::PlainText);
  wid->setWordWrap(QTextEdit::NoWrap);
  wid->show();

END_METHOD


BEGIN_PROPERTY(CTEXTAREA_text)

  if (READ_PROPERTY)
    GB.ReturnNewZeroString(TO_UTF8(WIDGET->text()));
  else
    WIDGET->setText(QSTRING_PROP());

END_PROPERTY

/*
BEGIN_PROPERTY(CTEXTAREA_alignment)

  if (READ_PROPERTY)
    GB.ReturnInteger(WIDGET->alignment());
  else
    WIDGET->setAlignment(PROPERTY(long));

END_PROPERTY
*/

BEGIN_PROPERTY(CTEXTAREA_length)

#if QT_VERSION <= 0x030005

  GB.ReturnInteger(WIDGET->length());

#else

  int np = WIDGET->paragraphs();

  if (np > 0)
    np--;

  GB.ReturnInteger(WIDGET->length() + np);

#endif

END_PROPERTY


BEGIN_PROPERTY(CTEXTAREA_read_only)

  if (READ_PROPERTY)
    GB.ReturnBoolean(WIDGET->isReadOnly());
  else
    WIDGET->setReadOnly(VPROP(GB_BOOLEAN));

END_PROPERTY


BEGIN_PROPERTY(CTEXTAREA_wrap)

  if (READ_PROPERTY)
    GB.ReturnBoolean(WIDGET->wordWrap() != QTextEdit::NoWrap);
  else
    WIDGET->setWordWrap(
      VPROP(GB_BOOLEAN) ? QTextEdit::WidgetWidth : QTextEdit::NoWrap
      );

END_PROPERTY


/*
BEGIN_PROPERTY(CTEXTAREA_max_length)

  long max;

  if (READ_PROPERTY)
  {
    max = WIDGET->maxLength();
    GB.ReturnInteger((max < 0) ? 0 : max);
  }
  else
  {
    max = PROPERTY(long);
    if (max <= 0)
      max = -1;
    WIDGET->setMaxLength(max);
  }

END_PROPERTY
*/

BEGIN_PROPERTY(CTEXTAREA_line_or_selection)

  RETURN_SELF();

END_PROPERTY


BEGIN_PROPERTY(CTEXTAREA_line)

  int line, col;

  WIDGET->getCursorPosition(&line, &col);

  if (READ_PROPERTY)
    GB.ReturnInteger(line);
  else
  {
    line = VPROP(GB_INTEGER);
    look_pos(WIDGET, &line, &col);

    WIDGET->setCursorPosition(line, col);
  }

END_PROPERTY


BEGIN_PROPERTY(CTEXTAREA_column)

  int line, col;

  WIDGET->getCursorPosition(&line, &col);

  if (READ_PROPERTY)
    GB.ReturnInteger(col);
  else
  {
    col = VPROP(GB_INTEGER);
    look_pos(WIDGET, &line, &col);

    WIDGET->setCursorPosition(line, col);
  }

END_PROPERTY


BEGIN_PROPERTY(CTEXTAREA_pos)

  int par, car;
  long pos;

  if (READ_PROPERTY)
  {
    WIDGET->getCursorPosition(&par, &car);
    to_pos(WIDGET, par, car, &pos);
    GB.ReturnInteger(pos);
  }
  else
  {
    from_pos(WIDGET, VPROP(GB_INTEGER), &par, &car);
    WIDGET->setCursorPosition(par, car);
  }

END_PROPERTY


/*
BEGIN_METHOD(CTEXTAREA_select, long line; long col; long selline; long selcol)

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

  WIDGET->insert(QSTRING_ARG(text));

END_METHOD

/*
BEGIN_METHOD(CTEXTAREA_line_get, long line)

  long line = PARAM(line);

  if (line < 0 || line >= WIDGET->numLines())
    GB.ReturnNull();
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
    GB.ReturnNewZeroString(TO_UTF8(WIDGET->selectedText()));
  else
    WIDGET->insert(QSTRING_PROP());

END_PROPERTY


BEGIN_PROPERTY(CTEXTAREA_sel_length)

  long start, length;

  get_selection(WIDGET, &start, &length);
  GB.ReturnInteger(length);

END_PROPERTY


BEGIN_PROPERTY(CTEXTAREA_sel_start)

  long start, length;

  get_selection(WIDGET, &start, &length);
  GB.ReturnInteger(start);

END_PROPERTY


BEGIN_METHOD_VOID(CTEXTAREA_sel_clear)

  WIDGET->selectAll(false);

END_METHOD


BEGIN_METHOD(CTEXTAREA_sel_select, GB_INTEGER start; GB_INTEGER length)

  int ps, is, pe, ie;

	if (MISSING(start) && MISSING(length))
	  WIDGET->selectAll(true);
	else if (!MISSING(start) && !MISSING(length))
	{
		from_pos(WIDGET, VARG(start), &ps, &is);
		from_pos(WIDGET, VARG(start) + VARG(length), &pe, &ie);

		WIDGET->setSelection(ps, is, pe, ie);
	}

END_METHOD


BEGIN_METHOD_VOID(CTEXTAREA_sel_all) //, GB_BOOLEAN sel)

  WIDGET->selectAll(true);

END_METHOD



BEGIN_METHOD(CTEXTAREA_to_pos, GB_INTEGER line; GB_INTEGER col)

  long pos;

  to_pos(WIDGET, VARG(line), VARG(col), &pos);

  GB.ReturnInteger(pos);

END_METHOD


BEGIN_METHOD(CTEXTAREA_to_line, GB_INTEGER pos)

  int line, col;

  from_pos(WIDGET, VARG(pos), &line, &col);

  GB.ReturnInteger(line);

END_METHOD


BEGIN_METHOD(CTEXTAREA_to_col, GB_INTEGER pos)

  int line, col;

  from_pos(WIDGET, VARG(pos), &line, &col);

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


BEGIN_PROPERTY(CTEXTAREA_scrollbar)

  long scroll;

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

END_PROPERTY


BEGIN_METHOD_VOID(CTEXTAREA_ensure_visible)

  WIDGET->ensureCursorVisible();

END_METHOD


GB_DESC CTextAreaSelectionDesc[] =
{
  GB_DECLARE(".TextAreaSelection", 0), GB_VIRTUAL_CLASS(),

  GB_PROPERTY("Text", "s", CTEXTAREA_sel_text),
  GB_PROPERTY_READ("Length", "i", CTEXTAREA_sel_length),
  GB_PROPERTY_READ("Start", "i", CTEXTAREA_sel_start),
  GB_PROPERTY_READ("Pos", "i", CTEXTAREA_sel_start),

  GB_METHOD("Clear", NULL, CTEXTAREA_sel_clear, NULL),
  GB_METHOD("Hide", NULL, CTEXTAREA_sel_clear, NULL),
  //GB_METHOD("All", NULL, CTEXTAREA_sel_select, NULL),
  //GB_METHOD("_call", NULL, CTEXTAREA_sel_select, "[(Start)i(Length)i]"),

  GB_END_DECLARE
};


/*
GB_DESC CTextAreaLinesDesc[] =
{
  GB_DECLARE(".TextArea.Lines", 0), GB_VIRTUAL_CLASS(),

  GB_METHOD("_get", "i", CTEXTAREA_line_get, "s"),
  GB_PROPERTY_READ("Count", "i", CTEXTAREA_line_count),

  GB_END_DECLARE
};
*/


GB_DESC CTextAreaDesc[] =
{
  GB_DECLARE("TextArea", sizeof(CTEXTAREA)), GB_INHERITS("Control"),

  GB_METHOD("_new", NULL, CTEXTAREA_new, "(Parent)Container;"),

  GB_PROPERTY("Text", "s", CTEXTAREA_text),
  GB_PROPERTY_READ("Length", "i", CTEXTAREA_length),
  GB_PROPERTY("ReadOnly", "b", CTEXTAREA_read_only),

  //GB_PROPERTY_READ("Lines", ".TextArea.Line", CTEXTAREA_line_or_selection),

  GB_PROPERTY("ScrollBar", "i", CTEXTAREA_scrollbar),
  GB_PROPERTY("Wrap", "b", CTEXTAREA_wrap),
  GB_PROPERTY("Border", "b", CWIDGET_border_simple),

  GB_PROPERTY("Line", "i", CTEXTAREA_line),
  GB_PROPERTY("Column", "i", CTEXTAREA_column),
  GB_PROPERTY("Pos", "i", CTEXTAREA_pos),

  GB_PROPERTY_SELF("Selection", ".TextAreaSelection"),
  GB_METHOD("Select", NULL, CTEXTAREA_sel_select, "[(Start)i(Length)i]"),
  GB_METHOD("SelectAll", NULL, CTEXTAREA_sel_all, NULL),

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

  GB_CONSTANT("_Properties", "s", "*,Text,ReadOnly,Wrap,Border=True,ScrollBar{Scroll.*}=Both"),
  GB_CONSTANT("_DefaultEvent", "s", "KeyPress"),

  GB_END_DECLARE
};


/* Class MyMultiLineEdit */

//class QMultiLineEditRow;

#if 0
long MyMultiLineEdit::getLineLength(long line)
{
  return lineLength(line);
}


long MyMultiLineEdit::toPos(int row, int col)
{
  int i;
  long pos;

  row = QMAX( QMIN( row, numLines() - 1), 0 );
  col = QMAX( QMIN( col,  lineLength( row )), 0 );

  if (row == 0)
	  return QMIN(col, lineLength(0));

  pos = 0;

	for(i = 0; i < row ; i++)
  {
    pos += lineLength(i);
    if (isEndOfParagraph(i))
      pos++;
	}

	pos += col;
  return pos;
}


void MyMultiLineEdit::fromPos(long pos, int *row, int *col)
{
  int l = 0;

  if (pos <= 0)
  {
    *row = 0;
    *col = 0;
    return;
  }

  for (l = 0; l < numLines(); l++)
  {
    if (pos <= lineLength(l))
    {
      *row = l;
      *col = pos;
      return;
    }

    pos -= lineLength(l);
    if (isEndOfParagraph(l))
      pos--;
  }

  *row = l - 1;
  *col = lineLength(l - 1);
}


void MyMultiLineEdit::getSelection(long *start, long *length)
{
  bool sel;
  int line1, col1, line2, col2;

  sel = getMarkedRegion(&line1, &col1, &line2, &col2 );

  if (!sel)
  {
    getCursorPosition(&line1, &col1);
    *start = toPos(line1, col1);
    *length = 0;
    return;
  }

  *start = toPos(line1, col1);
  *length = toPos(line2, col2) - *start;
}


void MyMultiLineEdit::setSelection(long start, long length)
{
  int line1, col1, line2, col2;

  if (length <= 0)
  {
    deselect();
    fromPos(start, &line1, &col1);
    setCursorPosition(line1, col1);
    return;
  }

  fromPos(start + length, &line2, &col2);
  QtMultiLineEdit::setSelection(line1, col1, line2, col2 );
}


const char *MyMultiLineEdit::selText(void)
{
  return markedText().latin1();
}
#endif

/* Classe CTextArea */


CTextArea CTextArea::manager;

void CTextArea::changed(void)
{
  RAISE_EVENT(EVENT_Change);
}

void CTextArea::cursor(void)
{
  RAISE_EVENT(EVENT_Cursor);
}
