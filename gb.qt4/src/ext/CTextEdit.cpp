/***************************************************************************

  CTextEdit.cpp

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

#define __CTEXTEDIT_CPP

#include <QApplication>
#include <QPalette>
#include <QPlainTextEdit>
#include <QTextDocumentFragment>
#include <QTextBlock>
#include <QScrollBar>
#include <QAbstractTextDocumentLayout>
 
#include "gambas.h"
#include "main.h"
#include "../CConst.h"
#include "../CFont.h"
#include "CTextEdit.h"


DECLARE_EVENT(EVENT_Change);
DECLARE_EVENT(EVENT_Cursor);

#if 0
static void to_pos(Q3TextEdit *wid, int par, int car, int *pos)
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


static void from_pos(Q3TextEdit *wid, int pos, int *par, int *car)
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


static void look_pos(Q3TextEdit *wid, int *line, int *col)
{
  if (*line == -1)
    *line = wid->paragraphs();

  if (*col == -1)
    *col = wid->paragraphLength(*line);
}


static void get_selection(Q3TextEdit *wid, int *start, int *length)
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
#endif

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

static int get_column(void *_object)
{
 	QTextCursor cursor = WIDGET->textCursor();
	return cursor.position() - cursor.block().position();
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


static void from_pos(void *_object, int pos, int *par, int *car)
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


/** MyTextEdit *************************************************************/

MyTextEdit::MyTextEdit(QWidget *parent) : QTextEdit(parent)
{
	setTabChangesFocus(true);
  //viewport()->setMouseTracking(true);
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
    GB.ReturnNewZeroString(TO_UTF8(WIDGET->document()->toPlainText()));
  else
	{
    WIDGET->document()->setPlainText(QSTRING_PROP());
		//THIS->length = -1;
	}

END_PROPERTY

BEGIN_PROPERTY(CTEXTAREA_rich_text)

  if (READ_PROPERTY)
    GB.ReturnNewZeroString(TO_UTF8(WIDGET->document()->toHtml("utf-8")));
  else
    WIDGET->document()->setHtml(QSTRING_PROP());

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

BEGIN_METHOD_VOID(CTEXTAREA_clear)

  WIDGET->clear();

END_METHOD

BEGIN_METHOD(CTEXTAREA_insert, GB_STRING text)

  WIDGET->textCursor().insertText(QSTRING_ARG(text));

END_METHOD


/** .TextEdit.Selection ****************************************************/

BEGIN_PROPERTY(CTEXTAREA_sel_text)

  if (READ_PROPERTY)
    GB.ReturnNewZeroString(TO_UTF8(WIDGET->textCursor().selection().toPlainText()));
  else
    WIDGET->textCursor().insertText(QSTRING_PROP());

END_PROPERTY

BEGIN_PROPERTY(CTEXTAREA_sel_rich_text)

  if (READ_PROPERTY)
    GB.ReturnNewZeroString(TO_UTF8(WIDGET->textCursor().selection().toHtml()));
  else
    WIDGET->textCursor().insertFragment(QTextDocumentFragment::fromHtml(QSTRING_PROP()));

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

BEGIN_METHOD_VOID(CTEXTAREA_sel_all)

	QTextCursor cursor = WIDGET->textCursor();
  cursor.select(QTextCursor::Document);
	WIDGET->setTextCursor(cursor);

END_METHOD

/***************************************************************************/

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

  QTextEdit *wid = new QTextEdit(QT.GetContainer(VARG(parent)));

  QObject::connect(wid, SIGNAL(textChanged()), &CTextArea::manager, SLOT(changed()));
  QObject::connect(wid, SIGNAL(cursorPositionChanged()), &CTextArea::manager, SLOT(cursor()));

  wid->setLineWrapMode(QTextEdit::NoWrap);
	
  QT.InitWidget(wid, _object, true);
	
	THIS->length = -1;
	
END_METHOD

BEGIN_PROPERTY(CTEXTEDIT_scroll_x)

  if (READ_PROPERTY)
    GB.ReturnInteger(WIDGET->horizontalScrollBar()->value());
  else
    WIDGET->horizontalScrollBar()->setValue(VPROP(GB_INTEGER));

END_PROPERTY

BEGIN_PROPERTY(CTEXTEDIT_scroll_y)

  if (READ_PROPERTY)
    GB.ReturnInteger(WIDGET->verticalScrollBar()->value());
  else
    WIDGET->verticalScrollBar()->setValue(VPROP(GB_INTEGER));

END_PROPERTY

BEGIN_PROPERTY(CTEXTEDIT_text_width)

  if (WIDGET->document()->isEmpty())
    GB.ReturnInteger(0);
  else
    GB.ReturnInteger(WIDGET->document()->documentLayout()->documentSize().toSize().width());

END_PROPERTY

BEGIN_PROPERTY(CTEXTEDIT_text_height)

  if (WIDGET->document()->isEmpty())
    GB.ReturnInteger(0);
  else
    GB.ReturnInteger(WIDGET->document()->documentLayout()->documentSize().toSize().height());

END_PROPERTY

/***************************************************************************/

BEGIN_PROPERTY(CTEXTEDIT_format_alignment)

  if (READ_PROPERTY)
    GB.ReturnInteger(QT.Alignment(WIDGET->alignment() + Qt::AlignVCenter, ALIGN_NORMAL, false));
  else
    WIDGET->setAlignment((Qt::Alignment)(QT.Alignment(VPROP(GB_INTEGER), ALIGN_NORMAL, true) & Qt::AlignHorizontal_Mask));

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
    QT.SetFont(set_font, VPROP(GB_OBJECT), THIS);

END_PROPERTY


BEGIN_PROPERTY(CTEXTEDIT_format_color)

  if (READ_PROPERTY)
    GB.ReturnInteger(WIDGET->textColor().rgb() & 0xFFFFFF);
  else
    WIDGET->setTextColor(QColor((QRgb)VPROP(GB_INTEGER)));

END_PROPERTY

BEGIN_PROPERTY(CTEXTEDIT_format_background)

  if (READ_PROPERTY)
    GB.ReturnInteger(WIDGET->textBackgroundColor().rgb() & 0xFFFFFF);
  else
    WIDGET->setTextBackgroundColor(QColor((QRgb)VPROP(GB_INTEGER)));

END_PROPERTY

/***************************************************************************/

GB_DESC CTextEditFormatDesc[] =
{
  GB_DECLARE(".TextEdit.Format", 0), GB_VIRTUAL_CLASS(),

  GB_PROPERTY("Alignment", "i", CTEXTEDIT_format_alignment),
  //GB_PROPERTY("Position", "i", CTEXTEDIT_format_position),
  GB_PROPERTY("Font", "Font", CTEXTEDIT_format_font),
  GB_PROPERTY("Color", "i", CTEXTEDIT_format_color),
  GB_PROPERTY("Background", "i", CTEXTEDIT_format_background),
    
  GB_END_DECLARE
};

GB_DESC CTextEditSelectionDesc[] =
{
  GB_DECLARE(".TextEdit.Selection", 0), GB_VIRTUAL_CLASS(),

  GB_PROPERTY("Text", "s", CTEXTAREA_sel_text),
  GB_PROPERTY("RichText", "s", CTEXTAREA_sel_rich_text),
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
  GB_PROPERTY("RichText", "s", CTEXTAREA_rich_text),
  GB_METHOD("Insert", NULL, CTEXTAREA_insert, "(Text)s"),

  GB_PROPERTY("Paragraph", "i", CTEXTAREA_line),
  GB_PROPERTY("Index", "i", CTEXTAREA_column),
  GB_PROPERTY("Pos", "i", CTEXTAREA_pos),
  //GB_PROPERTY_READ("Line", "i", CTEXTEDIT_line),

  GB_METHOD("ToPos", "i", CTEXTAREA_to_pos, "(Paragraph)i(Index)i"),
  GB_METHOD("ToParagraph", "i", CTEXTAREA_to_line, "(Pos)i"),
  GB_METHOD("ToIndex", "i", CTEXTAREA_to_col, "(Pos)i"),

  GB_METHOD("EnsureVisible", NULL, CTEXTAREA_ensure_visible, NULL),

  GB_PROPERTY_SELF("Selection", ".TextEdit.Selection"),
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
  GB_PROPERTY("Wrap", "b", CTEXTAREA_wrap),

  GB_PROPERTY("ScrollX", "i", CTEXTEDIT_scroll_x),
  GB_PROPERTY("ScrollY", "i", CTEXTEDIT_scroll_y),
  
  GB_PROPERTY_READ("TextWidth", "i", CTEXTEDIT_text_width),
  GB_PROPERTY_READ("TextHeight", "i", CTEXTEDIT_text_height),

  GB_PROPERTY_SELF("Format", ".TextEdit.Format"),
  
  GB_EVENT("Change", NULL, NULL, &EVENT_Change),
  GB_EVENT("Cursor", NULL, NULL, &EVENT_Cursor),
  //GB_EVENT("Link", NULL, "(Path)s", &EVENT_Link),

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

#if 0
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
#endif
