/***************************************************************************

  CTextEdit.cpp

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

#define __CTEXTEDIT_CPP

#include <qapplication.h>
#include <qpalette.h>
#include <qtextedit.h>
#include <qscrollview.h>

#include "gambas.h"
#include "main.h"
#include "../CConst.h"
#include "../CFont.h"
#include "CTextEdit.h"


DECLARE_EVENT(EVENT_Change);
DECLARE_EVENT(EVENT_Cursor);
DECLARE_EVENT(EVENT_Link);

static void to_pos(QTextEdit *wid, int par, int car, int *pos)
{
  int i, l;
  int p = 0;

  for (i = 0; i < par; i++)
  {
    l = wid->paragraphLength(i);
    if (l < 0)
      break;
    p += l + 1;
  }

  *pos = p + car;
}


static void from_pos(QTextEdit *wid, int pos, int *par, int *car)
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


static void get_selection(QTextEdit *wid, int *start, int *length)
{
  int pStart, iStart, pEnd, iEnd;
  int posEnd;

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


/** MyTextEdit *************************************************************/

MyTextEdit::MyTextEdit(QWidget *parent, const char *name)
  : QTextEdit( parent, name )
{
  viewport()->setMouseTracking(true);
}

MyTextEdit::~MyTextEdit()
{
}

void MyTextEdit::emitLinkClicked(const QString &s) 
{ 
  //d->textOrSourceChanged = FALSE;
  emit linkClicked( s );
  //if ( !d->textOrSourceChanged )
	//  setSource( s );
}

/** TextArea ***************************************************************/

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
    WIDGET->setAlignment(PROPERTY(int));

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
  int pos;

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

  WIDGET->insert(QSTRING_ARG(text));

END_METHOD

/*
BEGIN_METHOD(CTEXTAREA_line_get, int line)

  int line = PARAM(line);

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

  WIDGET->selectAll(false);

END_METHOD


BEGIN_PROPERTY(CTEXTAREA_selected)

	GB.ReturnBoolean(WIDGET->hasSelectedText());

END_PROPERTY


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

  int pos;

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


BEGIN_METHOD_VOID(CTEXTAREA_ensure_visible)

  WIDGET->ensureCursorVisible();

END_METHOD


BEGIN_PROPERTY(CTEXTAREA_border)

  QT.BorderProperty(_object, _param);

END_PROPERTY


BEGIN_PROPERTY(CTEXTAREA_scrollbar)

  QT.ScrollBarProperty(_object, _param);

END_PROPERTY


/** TextEdit ***************************************************************/

BEGIN_METHOD(CTEXTEDIT_new, GB_OBJECT parent)

  MyTextEdit *wid = new MyTextEdit(QT.GetContainer(VARG(parent)));

  QObject::connect(wid, SIGNAL(textChanged()), &CTextArea::manager, SLOT(changed()));
  QObject::connect(wid, SIGNAL(cursorPositionChanged(int, int)), &CTextArea::manager, SLOT(cursor()));
  QObject::connect(wid, SIGNAL(linkClicked(const QString &)), &CTextArea::manager, SLOT(link(const QString &)));
  
  QT.InitWidget(wid, _object);

  wid->setTextFormat(Qt::RichText);
  wid->setMimeSourceFactory(QT.MimeSourceFactory());
  wid->show();

END_METHOD

#if 0
BEGIN_METHOD(CTEXTEDIT_paste, GB_STRING format)

  QCString format;
  bool html = false;

  if (!MISSING(format))
  {
    format = GB.ToZeroString(ARG(format));
    if (format == "text/html")
      html = true;
  }
  
  if (html)
    WIDGET->pasteSubType("html");
  else
    WIDGET->paste();

END_METHOD
#endif

BEGIN_PROPERTY(CTEXTEDIT_scroll_x)

  if (READ_PROPERTY)
    GB.ReturnInteger(WIDGET->contentsX());
  else
    WIDGET->setContentsPos(VPROP(GB_INTEGER), WIDGET->contentsY());

END_PROPERTY


BEGIN_PROPERTY(CTEXTEDIT_scroll_y)

  if (READ_PROPERTY)
    GB.ReturnInteger(WIDGET->contentsY());
  else
    WIDGET->setContentsPos(WIDGET->contentsX(), VPROP(GB_INTEGER));

END_PROPERTY


BEGIN_PROPERTY(CTEXTEDIT_text_width)

  if (WIDGET->paragraphs() <= 0)
    GB.ReturnInteger(0);
  else
  {
    WIDGET->sync();
    GB.ReturnInteger(WIDGET->contentsWidth());
  }

END_PROPERTY


BEGIN_PROPERTY(CTEXTEDIT_text_height)

  if (WIDGET->paragraphs() <= 0)
    GB.ReturnInteger(0);
  else
  {
    WIDGET->sync();
    GB.ReturnInteger(WIDGET->contentsHeight());
  }

END_PROPERTY


BEGIN_PROPERTY(CTEXTEDIT_format_alignment)

  if (READ_PROPERTY)
    GB.ReturnInteger(QT.Alignment(WIDGET->alignment() + Qt::AlignVCenter, ALIGN_NORMAL, false));
  else
    WIDGET->setAlignment(QT.Alignment(VPROP(GB_INTEGER), ALIGN_NORMAL, true) & Qt::AlignHorizontal_Mask);

END_PROPERTY


/*BEGIN_PROPERTY(CTEXTEDIT_format_position)

  if (READ_PROPERTY)
    GB.ReturnInteger(WIDGET->verticalAlignment());
  else
    WIDGET->setVerticalAlignment((QTextEdit::VerticalAlignment)VPROP(GB_INTEGER));

END_PROPERTY*/

static void set_font(QFont &font, void *_object = 0)
{
  WIDGET->setCurrentFont(font);
}

BEGIN_PROPERTY(CTEXTEDIT_format_font)

  if (READ_PROPERTY)
    GB.ReturnObject(QT.CreateFont(WIDGET->currentFont(), set_font, _object));
  else
    SET_FONT(set_font, VPROP(GB_OBJECT));

END_PROPERTY


BEGIN_PROPERTY(CTEXTEDIT_format_color)

  if (READ_PROPERTY)
    GB.ReturnInteger(WIDGET->color().rgb() & 0xFFFFFF);
  else
    WIDGET->setColor(QColor((QRgb)VPROP(GB_INTEGER)));

END_PROPERTY


/*BEGIN_PROPERTY(CTEXTEDIT_line)

  int para, col;

  WIDGET->getCursorPosition(&para, &col);
  GB.ReturnInteger(WIDGET->lineOfChar(para, col));

END_PROPERTY*/


GB_DESC CTextEditFormatDesc[] =
{
  GB_DECLARE(".TextEditFormat", 0), GB_VIRTUAL_CLASS(),

  GB_PROPERTY("Alignment", "i", CTEXTEDIT_format_alignment),
  //GB_PROPERTY("Position", "i", CTEXTEDIT_format_position),
  GB_PROPERTY("Font", "Font", CTEXTEDIT_format_font),
  GB_PROPERTY("Color", "i", CTEXTEDIT_format_color),
    
  GB_END_DECLARE
};

GB_DESC CTextEditSelectionDesc[] =
{
  GB_DECLARE(".TextEditSelection", 0), GB_VIRTUAL_CLASS(),

  GB_PROPERTY("Text", "s", CTEXTAREA_sel_text),
  GB_PROPERTY_READ("Length", "i", CTEXTAREA_sel_length),
  GB_PROPERTY_READ("Start", "i", CTEXTAREA_sel_start),
  GB_METHOD("Hide", NULL, CTEXTAREA_sel_clear, NULL),

  GB_END_DECLARE
};


GB_DESC CTextEditDesc[] =
{
  GB_DECLARE("TextEdit", sizeof(CTEXTEDIT)), GB_INHERITS("Control"),

  GB_METHOD("_new", NULL, CTEXTEDIT_new, "(Parent)Container;"),

  //GB_CONSTANT("Normal", "i", QTextEdit::AlignNormal),
  //GB_CONSTANT("SubScript", "i", QTextEdit::AlignSubScript),
  //GB_CONSTANT("SuperScript", "i", QTextEdit::AlignSuperScript),

  GB_PROPERTY("ReadOnly", "b", CTEXTAREA_read_only),
  
  GB_METHOD("Clear", NULL, CTEXTAREA_clear, NULL),

  GB_PROPERTY("Text", "s", CTEXTAREA_text),
  GB_METHOD("Insert", NULL, CTEXTAREA_insert, "(Text)s"),

  GB_PROPERTY("Paragraph", "i", CTEXTAREA_line),
  GB_PROPERTY("Index", "i", CTEXTAREA_column),
  GB_PROPERTY("Pos", "i", CTEXTAREA_pos),
  //GB_PROPERTY_READ("Line", "i", CTEXTEDIT_line),

  GB_METHOD("ToPos", "i", CTEXTAREA_to_pos, "(Paragraph)i(Index)i"),
  GB_METHOD("ToParagraph", "i", CTEXTAREA_to_line, "(Pos)i"),
  GB_METHOD("ToIndex", "i", CTEXTAREA_to_col, "(Pos)i"),

  GB_METHOD("EnsureVisible", NULL, CTEXTAREA_ensure_visible, NULL),

  GB_PROPERTY_SELF("Selection", ".TextEditSelection"),
  GB_METHOD("Select", NULL, CTEXTAREA_sel_select, "[(Start)i(Length)i]"),
  GB_METHOD("SelectAll", NULL, CTEXTAREA_sel_all, NULL),
  GB_METHOD("Unselect", NULL, CTEXTAREA_sel_clear, NULL),
	GB_PROPERTY_READ("Selected", "b", CTEXTAREA_selected),
	
  GB_METHOD("Copy", NULL, CTEXTAREA_copy, NULL),
  GB_METHOD("Cut", NULL, CTEXTAREA_cut, NULL),
  GB_METHOD("Paste", NULL, CTEXTAREA_paste, NULL),
  GB_METHOD("Undo", NULL, CTEXTAREA_undo, NULL),
  GB_METHOD("Redo", NULL, CTEXTAREA_redo, NULL),  
  
  GB_PROPERTY("Border", "b", CTEXTAREA_border),
  GB_PROPERTY("ScrollBar", "i", CTEXTAREA_scrollbar),

  GB_PROPERTY("ScrollX", "i", CTEXTEDIT_scroll_x),
  GB_PROPERTY("ScrollY", "i", CTEXTEDIT_scroll_y),
  
  GB_PROPERTY_READ("TextWidth", "i", CTEXTEDIT_text_width),
  GB_PROPERTY_READ("TextHeight", "i", CTEXTEDIT_text_height),

  GB_PROPERTY_SELF("Format", ".TextEditFormat"),
  
  GB_EVENT("Change", NULL, NULL, &EVENT_Change),
  GB_EVENT("Cursor", NULL, NULL, &EVENT_Cursor),
  GB_EVENT("Link", NULL, "(Path)s", &EVENT_Link),

	TEXTEDIT_DESCRIPTION,

  GB_END_DECLARE
};


/** CTextArea **************************************************************/

CTextArea CTextArea::manager;

void CTextArea::changed(void)
{
  void *_object = QT.GetObject((QWidget *)sender());
  GB.Raise(THIS, EVENT_Change, 0);
}

void CTextArea::cursor(void)
{
  void *_object = QT.GetObject((QWidget *)sender());
  GB.Raise(THIS, EVENT_Cursor, 0);
}

void CTextArea::link(const QString &path)
{
  void *_object = QT.GetObject((QWidget *)sender());

  //THIS_EDIT->change = false;

  GB.Raise(_object, EVENT_Link, 1,
    GB_T_STRING, TO_UTF8(path), 0);

  /*if (!THIS_EDIT->change)
  {
    // This cancels the click on the link (see qt source code)
    WIDGET->setSource(WIDGET->source());
  }*/
}
