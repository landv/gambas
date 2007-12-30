/***************************************************************************

  CScrollView.cpp

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

#define __CSCROLLVIEW_CPP

#include "gambas.h"
#include "main.h"
#include "widgets.h"
#include "CConst.h"
#include "CScrollView.h"

DECLARE_EVENT(EVENT_Scroll);

void gb_raise_scrollview_Scroll(gScrollView *sender)
{
	CWIDGET *_ob=GetObject(sender);
	
	if (!_ob) return;
	GB.Raise((void*)_ob,EVENT_Scroll,0);
}

/***************************************************************************

  ScrollView

***************************************************************************/

BEGIN_METHOD(CSCROLLVIEW_new, GB_OBJECT parent)

	CCONTAINER *Parent=(CCONTAINER*)VPROP(GB_OBJECT);
	Parent=(CCONTAINER*)GetContainer ((CWIDGET*)Parent);

	THIS->widget=new gScrollView(Parent->widget);
	InitControl(THIS->widget,(CWIDGET*)THIS);
	SCROLLVIEW->onScroll=gb_raise_scrollview_Scroll;

END_METHOD



BEGIN_PROPERTY(CSCROLLVIEW_scroll_x)

	if (READ_PROPERTY) { GB.ReturnInteger(SCROLLVIEW->scrollX()); return; }
	SCROLLVIEW->setScrollX(VPROP(GB_INTEGER));

END_PROPERTY


BEGIN_PROPERTY(CSCROLLVIEW_scroll_y)

	if (READ_PROPERTY) { GB.ReturnInteger(SCROLLVIEW->scrollY()); return; }
	SCROLLVIEW->setScrollY(VPROP(GB_INTEGER));

END_PROPERTY


BEGIN_METHOD(CSCROLLVIEW_scroll, GB_INTEGER x; GB_INTEGER y)

	SCROLLVIEW->scroll(VARG(x),VARG(y));

END_METHOD


BEGIN_PROPERTY(CSCROLLVIEW_scrollbar)

	if (READ_PROPERTY) { GB.ReturnInteger(SCROLLVIEW->scrollBar()); return; }
	SCROLLVIEW->setScrollBar(VPROP(GB_INTEGER));

END_PROPERTY



BEGIN_PROPERTY(CSCROLLVIEW_border_full)

	if (READ_PROPERTY) { GB.ReturnInteger(SCROLLVIEW->getBorder()); return; }
	SCROLLVIEW->setBorder(VPROP(GB_INTEGER)); 

END_PROPERTY

BEGIN_METHOD(CSCROLLVIEW_ensure_visible, GB_INTEGER x; GB_INTEGER y; GB_INTEGER w; GB_INTEGER h)

  long w=VARG(w);
  long h=VARG(h);
  long x=VARG(x);
  long y=VARG(y);
  
  SCROLLVIEW->ensureVisible(x,y,w,h);

END_METHOD

/***************************************************************************

  Descriptions

***************************************************************************/

DECLARE_METHOD(CWIDGET_border);

GB_DESC CScrollViewDesc[] =
{
  GB_DECLARE("ScrollView", sizeof(CSCROLLVIEW)), GB_INHERITS("Container"),

  GB_METHOD("_new", NULL, CSCROLLVIEW_new, "(Parent)Container;"),

  GB_PROPERTY("ScrollBar", "i<Scroll>", CSCROLLVIEW_scrollbar),
  GB_PROPERTY("Border", "i<Border>", CSCROLLVIEW_border_full),
  GB_PROPERTY("ScrollX", "i", CSCROLLVIEW_scroll_x),
  GB_PROPERTY("ScrollY", "i", CSCROLLVIEW_scroll_y),
  GB_PROPERTY("Spacing", "i", CCONTAINER_spacing),
  GB_PROPERTY("Padding", "i", CCONTAINER_padding),
  GB_PROPERTY("Arrangement", "i<Arrange>", CCONTAINER_arrangement),
  
  GB_METHOD("Scroll", NULL, CSCROLLVIEW_scroll, "(X)i(Y)i"),
  GB_METHOD("EnsureVisible", NULL, CSCROLLVIEW_ensure_visible, "(X)i(Y)i(Width)i(Height)i"),
  
  GB_EVENT("Scroll", NULL, NULL, &EVENT_Scroll),

  GB_CONSTANT("_Properties", "s", CSCROLLVIEW_PROPERTIES),

  GB_END_DECLARE
};





