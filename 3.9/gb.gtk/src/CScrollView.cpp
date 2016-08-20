/***************************************************************************

  CScrollView.cpp

  (c) 2004-2006 - Daniel Campos Fernández <dcamposf@gmail.com>

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

	InitControl(new gScrollView(CONTAINER(VARG(parent))),(CWIDGET*)THIS);
	
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


BEGIN_PROPERTY(CSCROLLVIEW_scroll_w)

	GB.ReturnInteger(SCROLLVIEW->scrollWidth());
	
END_PROPERTY


BEGIN_PROPERTY(CSCROLLVIEW_scroll_h)

	GB.ReturnInteger(SCROLLVIEW->scrollHeight());
	
END_PROPERTY


BEGIN_METHOD(CSCROLLVIEW_scroll, GB_INTEGER x; GB_INTEGER y)

	SCROLLVIEW->scroll(VARG(x),VARG(y));

END_METHOD


BEGIN_PROPERTY(CSCROLLVIEW_scrollbar)

	if (READ_PROPERTY) { GB.ReturnInteger(SCROLLVIEW->scrollBar()); return; }
	SCROLLVIEW->setScrollBar(VPROP(GB_INTEGER));

END_PROPERTY



BEGIN_PROPERTY(CSCROLLVIEW_border)

	if (READ_PROPERTY) { GB.ReturnBoolean(SCROLLVIEW->hasBorder()); return; }
	SCROLLVIEW->setBorder(VPROP(GB_BOOLEAN)); 

END_PROPERTY

BEGIN_METHOD(CSCROLLVIEW_ensure_visible, GB_INTEGER x; GB_INTEGER y; GB_INTEGER w; GB_INTEGER h)

	SCROLLVIEW->ensureVisible(VARG(x),VARG(y), VARG(w), VARG(h));

END_METHOD

/***************************************************************************

	Descriptions

***************************************************************************/

DECLARE_METHOD(CWIDGET_border);

GB_DESC CScrollViewDesc[] =
{
	GB_DECLARE("ScrollView", sizeof(CSCROLLVIEW)), GB_INHERITS("Container"),

	GB_METHOD("_new", 0, CSCROLLVIEW_new, "(Parent)Container;"),

	GB_PROPERTY("ScrollBar", "i", CSCROLLVIEW_scrollbar),
	GB_PROPERTY("Border", "b", CSCROLLVIEW_border),
	GB_PROPERTY("ScrollX", "i", CSCROLLVIEW_scroll_x),
	GB_PROPERTY("ScrollY", "i", CSCROLLVIEW_scroll_y),
	GB_PROPERTY_READ("ScrollW", "i", CSCROLLVIEW_scroll_w),
	GB_PROPERTY_READ("ScrollWidth", "i", CSCROLLVIEW_scroll_w),
	GB_PROPERTY_READ("ScrollH", "i", CSCROLLVIEW_scroll_h),
	GB_PROPERTY_READ("ScrollHeight", "i", CSCROLLVIEW_scroll_h),
	GB_PROPERTY_READ("ContentsW", "i", CSCROLLVIEW_scroll_w),
	GB_PROPERTY_READ("ContentsWidth", "i", CSCROLLVIEW_scroll_w),
	GB_PROPERTY_READ("ContentsH", "i", CSCROLLVIEW_scroll_h),
	GB_PROPERTY_READ("ContentsHeight", "i", CSCROLLVIEW_scroll_h),

	GB_PROPERTY("Arrangement", "i", Container_Arrangement),
	//GB_PROPERTY("AutoResize", "b", Container_AutoResize),
	GB_PROPERTY("Padding", "i", Container_Padding),
	GB_PROPERTY("Spacing", "b", Container_Spacing),
	GB_PROPERTY("Margin", "b", Container_Margin),
	GB_PROPERTY("Indent", "b", Container_Indent),
	GB_PROPERTY("Invert", "b", Container_Invert),
	
	GB_METHOD("Scroll", 0, CSCROLLVIEW_scroll, "(X)i(Y)i"),
	GB_METHOD("EnsureVisible", 0, CSCROLLVIEW_ensure_visible, "(X)i(Y)i(Width)i(Height)i"),
	
	GB_EVENT("Scroll", 0, 0, &EVENT_Scroll),

	SCROLLVIEW_DESCRIPTION,
	
	GB_END_DECLARE
};





