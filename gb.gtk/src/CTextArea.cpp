/***************************************************************************

  CTextArea.cpp

  The TextArea class

  (c) 2004-2006 - Daniel Campos Fernández <dcamposf@gmail.com>
  
  GTK+ component
  
  Realizado para la Junta de Extremadura. 
  Consejería de Educación Ciencia y Tecnología. 
  Proyecto gnuLinEx
  
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

#include "gambas.h"
#include "main.h"
#include "widgets.h"

#include "CTextArea.h"
#include "CWidget.h"
#include "CContainer.h"

#include <stdlib.h>

DECLARE_EVENT(EVENT_Change);
DECLARE_EVENT(EVENT_Cursor);
//DECLARE_EVENT(EVENT_Link); //TODO

static void cb_change(gTextArea *sender)
{
	CWIDGET *_object = GetObject((gControl*)sender);
	GB.Raise(THIS, EVENT_Change, 0);
}

static void cb_cursor(gTextArea *sender)
{
	CWIDGET *_object = GetObject((gControl*)sender);
	GB.Raise(THIS, EVENT_Cursor, 0);
}

BEGIN_METHOD(CTEXTAREA_new, GB_OBJECT parent)

	InitControl(new gTextArea(CONTAINER(VARG(parent))), (CWIDGET*)THIS);
	WIDGET->onChange = cb_change;
	WIDGET->onCursor = cb_cursor;
	
END_METHOD


BEGIN_PROPERTY(CTEXTAREA_text)

	char *buf;

	if (READ_PROPERTY)
	{
		buf=TEXTAREA->text();
		GB.ReturnNewString(buf,0);
		free(buf);
		return;
	}
	
	TEXTAREA->setText( GB.ToZeroString(PROP(GB_STRING)) );
	

END_PROPERTY


BEGIN_PROPERTY(CTEXTAREA_length)

	GB.ReturnInteger(TEXTAREA->length());

END_PROPERTY


BEGIN_PROPERTY(CTEXTAREA_read_only)

	if (READ_PROPERTY) { GB.ReturnBoolean(TEXTAREA->readOnly()); return; }
	TEXTAREA->setReadOnly(VPROP(GB_BOOLEAN));

END_PROPERTY


BEGIN_PROPERTY(CTEXTAREA_wrap)

	if (READ_PROPERTY) { GB.ReturnBoolean(TEXTAREA->wrap()); return; }
	TEXTAREA->setWrap(VPROP(GB_BOOLEAN));

END_PROPERTY


BEGIN_PROPERTY(CTEXTAREA_line)

	if (READ_PROPERTY) { GB.ReturnInteger(TEXTAREA->line()); return; }
	TEXTAREA->setLine(VPROP(GB_INTEGER));

END_PROPERTY


BEGIN_PROPERTY(CTEXTAREA_column)

	if (READ_PROPERTY) { GB.ReturnInteger(TEXTAREA->column()); return; }
	TEXTAREA->setColumn(VPROP(GB_INTEGER));

END_PROPERTY


BEGIN_PROPERTY(CTEXTAREA_pos)

	if (READ_PROPERTY) { GB.ReturnInteger(TEXTAREA->position()); return; }
	TEXTAREA->setPosition(VPROP(GB_INTEGER));

END_PROPERTY


BEGIN_METHOD_VOID(CTEXTAREA_clear)

	TEXTAREA->setText("");

END_METHOD


BEGIN_METHOD(CTEXTAREA_insert, GB_STRING text)

	TEXTAREA->insert(  GB.ToZeroString(ARG(text)) );

END_METHOD

BEGIN_PROPERTY(CTEXTAREA_border)

	if (READ_PROPERTY) { GB.ReturnBoolean(TEXTAREA->hasBorder()); return; }
	TEXTAREA->setBorder(VPROP(GB_BOOLEAN));

END_PROPERTY



/***************************************************************************

  .TextArea.Selection

***************************************************************************/

BEGIN_PROPERTY(CTEXTAREA_sel_text)

	char *buf;

	if (READ_PROPERTY)
	{
		buf=TEXTAREA->selText();
		GB.ReturnNewString(buf,0);
		if (buf) free(buf);
		return;
	}
	
	TEXTAREA->setSelText(GB.ToZeroString(PROP(GB_STRING)));

END_PROPERTY


BEGIN_PROPERTY(CTEXTAREA_sel_length)

	GB.ReturnInteger(TEXTAREA->selEnd()-TEXTAREA->selStart());

END_PROPERTY


BEGIN_PROPERTY(CTEXTAREA_sel_start)

	GB.ReturnInteger(TEXTAREA->selStart());

END_PROPERTY


BEGIN_METHOD_VOID(CTEXTAREA_sel_clear)

	TEXTAREA->selDelete();

END_METHOD


BEGIN_METHOD(CTEXTAREA_sel_select, GB_INTEGER start; GB_INTEGER length)

	long start=0;
	long length=0;
	
	if (MISSING(start)) length=TEXTAREA->length();
	else start=VARG(start);
	
	if (!MISSING(length)) length=VARG(length);
	
	TEXTAREA->selSelect(start,length);

END_METHOD

BEGIN_METHOD_VOID(CTEXTAREA_sel_all)

	long length=TEXTAREA->length();

	TEXTAREA->selSelect(0,length);

END_METHOD



BEGIN_METHOD(CTEXTAREA_to_pos, GB_INTEGER line; GB_INTEGER col)

	GB.ReturnInteger(TEXTAREA->toPosition(VARG(line),VARG(col)));

END_METHOD


BEGIN_METHOD(CTEXTAREA_to_line, GB_INTEGER pos)

	GB.ReturnInteger(TEXTAREA->toLine(VARG(pos)));

END_METHOD


BEGIN_METHOD(CTEXTAREA_to_col, GB_INTEGER pos)

	GB.ReturnInteger(TEXTAREA->toColumn(VARG(pos)));

END_METHOD


BEGIN_METHOD_VOID(CTEXTAREA_copy)


	TEXTAREA->copy();

END_METHOD


BEGIN_METHOD_VOID(CTEXTAREA_cut)

	TEXTAREA->cut();

END_METHOD


BEGIN_METHOD_VOID(CTEXTAREA_paste)


	TEXTAREA->paste();

END_METHOD


BEGIN_METHOD_VOID(CTEXTAREA_ensure_visible)

	TEXTAREA->ensureVisible();

END_METHOD

BEGIN_PROPERTY(CTEXTAREA_scrollbar)

	if (READ_PROPERTY) { GB.ReturnInteger(TEXTAREA->scrollBar()); return; }
	TEXTAREA->setScrollBar(VPROP(GB_INTEGER));

END_METHOD

BEGIN_METHOD_VOID(CTEXTAREA_undo)

	stub ("CTEXTAREA_undo");

END_METHOD


BEGIN_METHOD_VOID(CTEXTAREA_redo)

	stub ("CTEXTAREA_redo");

END_METHOD

BEGIN_METHOD_VOID(CTEXTAREA_selected)

  GB.ReturnBoolean(TEXTAREA->isSelected());

END_METHOD


GB_DESC CTextAreaSelectionDesc[] =
{
  GB_DECLARE(".TextAreaSelection", 0), GB_VIRTUAL_CLASS(),

  GB_PROPERTY("Text", "s", CTEXTAREA_sel_text),
  GB_PROPERTY_READ("Length", "i", CTEXTAREA_sel_length),
  GB_PROPERTY_READ("Start", "i", CTEXTAREA_sel_start),
  GB_PROPERTY_READ("Pos", "i", CTEXTAREA_sel_start),

  GB_METHOD("Hide", 0, CTEXTAREA_sel_clear, 0),

  GB_END_DECLARE
};

GB_DESC CTextAreaDesc[] =
{
  GB_DECLARE("TextArea", sizeof(CTEXTAREA)), GB_INHERITS("Control"),

  GB_METHOD("_new", 0, CTEXTAREA_new, "(Parent)Container;"),

  GB_PROPERTY("Text", "s", CTEXTAREA_text),
  GB_PROPERTY_READ("Length", "i", CTEXTAREA_length),
  GB_PROPERTY("ReadOnly", "b", CTEXTAREA_read_only),

  GB_PROPERTY("Wrap", "b", CTEXTAREA_wrap),
  GB_PROPERTY("Border", "b", CTEXTAREA_border),

  GB_PROPERTY("Line", "i", CTEXTAREA_line),
  GB_PROPERTY("Column", "i", CTEXTAREA_column),
  GB_PROPERTY("Pos", "i", CTEXTAREA_pos),
  GB_PROPERTY("ScrollBar", "i", CTEXTAREA_scrollbar),

  GB_PROPERTY_SELF("Selection", ".TextAreaSelection"),
  GB_METHOD("Select", 0, CTEXTAREA_sel_select, "[(Start)i(Length)i]"),
  GB_METHOD("SelectAll", 0, CTEXTAREA_sel_all, 0),
  GB_METHOD("Unselect", 0, CTEXTAREA_sel_clear, 0),
	GB_PROPERTY_READ("Selected", "b", CTEXTAREA_selected),

  GB_METHOD("Clear", 0, CTEXTAREA_clear, 0),
  GB_METHOD("Insert", 0, CTEXTAREA_insert, "(Text)s"),

  GB_METHOD("Copy", 0, CTEXTAREA_copy, 0),
  GB_METHOD("Cut", 0, CTEXTAREA_cut, 0),
  GB_METHOD("Paste", 0, CTEXTAREA_paste, 0),
  GB_METHOD("Undo", 0, CTEXTAREA_undo, 0),
  GB_METHOD("Redo", 0, CTEXTAREA_redo, 0),

  GB_METHOD("ToPos", "i", CTEXTAREA_to_pos, "(Line)i(Column)i"),
  GB_METHOD("ToLine", "i", CTEXTAREA_to_line, "(Pos)i"),
  GB_METHOD("ToColumn", "i", CTEXTAREA_to_col, "(Pos)i"),

  GB_METHOD("EnsureVisible", 0, CTEXTAREA_ensure_visible, 0),
  
  GB_EVENT("Change", 0, 0, &EVENT_Change),
  GB_EVENT("Cursor", 0, 0, &EVENT_Cursor),
  
  TEXTAREA_DESCRIPTION,

  GB_END_DECLARE
};

#if 0
/** TextEdit ***************************************************************/

BEGIN_METHOD(CTEXTEDIT_new, GB_OBJECT parent)

	InitControl(new gTextArea(CONTAINER(VARG(parent))), (CWIDGET*)THIS);
	WIDGET->setWrap(true);

END_METHOD

BEGIN_PROPERTY(CTEXTEDIT_scroll_x)

	if (READ_PROPERTY)
		GB.ReturnInteger(WIDGET->scrollX());
	else
		WIDGET->setScrollX(VPROP(GB_INTEGER));

END_PROPERTY

BEGIN_PROPERTY(CTEXTEDIT_scroll_y)

	if (READ_PROPERTY)
		GB.ReturnInteger(WIDGET->scrollY());
	else
		WIDGET->setScrollY(VPROP(GB_INTEGER));
		
END_PROPERTY

BEGIN_PROPERTY(CTEXTEDIT_text_width)

	GB.ReturnInteger(WIDGET->textWidth());

END_PROPERTY

BEGIN_PROPERTY(CTEXTEDIT_text_height)

	GB.ReturnInteger(WIDGET->textHeight());

END_PROPERTY

BEGIN_PROPERTY(CTEXTEDIT_format_alignment)

END_PROPERTY

BEGIN_PROPERTY(CTEXTEDIT_format_font)

END_PROPERTY

BEGIN_PROPERTY(CTEXTEDIT_format_color)

END_PROPERTY


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
  GB_DECLARE("TextEdit", sizeof(CTEXTAREA)), GB_INHERITS("Control"),

  GB_METHOD("_new", NULL, CTEXTEDIT_new, "(Parent)Container;"),

  GB_PROPERTY("ReadOnly", "b", CTEXTAREA_read_only),
  
  GB_METHOD("Clear", NULL, CTEXTAREA_clear, NULL),

  GB_PROPERTY("Text", "s", CTEXTAREA_text),
  GB_METHOD("Insert", NULL, CTEXTAREA_insert, "(Text)s"),

  GB_PROPERTY("Paragraph", "i", CTEXTAREA_line),
  GB_PROPERTY("Index", "i", CTEXTAREA_column),
  GB_PROPERTY("Pos", "i", CTEXTAREA_pos),

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
  
  GB_PROPERTY("TextWidth", "i", CTEXTEDIT_text_width),
  GB_PROPERTY("TextHeight", "i", CTEXTEDIT_text_height),

  GB_PROPERTY_SELF("Format", ".TextEditFormat"),
  
  GB_EVENT("Change", NULL, NULL, &EVENT_Change),
  GB_EVENT("Cursor", NULL, NULL, &EVENT_Cursor),
  GB_EVENT("Link", NULL, "(Path)s", &EVENT_Link),

  TEXTEDIT_DESCRIPTION,

  GB_END_DECLARE
};

#endif
