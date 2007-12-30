/***************************************************************************

  CScrollView.cpp

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

#define __CSCROLLVIEW_CPP



#include <qframe.h>
#if QT_VERSION >= 0x030200
#include <qobjectlist.h>
#else
#include <qobjcoll.h>
#endif
#include <qscrollview.h>

#include "gambas.h"

#include "CConst.h"
#include "CScrollView.h"


/*DECLARE_EVENT(EVENT_ChildInsert);
DECLARE_EVENT(EVENT_ChildRemove);
DECLARE_EVENT(EVENT_ChildShow);
DECLARE_EVENT(EVENT_ChildHide);*/

/***************************************************************************

  class MyScrollView

***************************************************************************/

MyScrollView::MyScrollView(QWidget *parent)
: QScrollView(parent)
{
}


void MyScrollView::frameChanged(void)
{
  CSCROLLVIEW *_object = (CSCROLLVIEW *)CWidget::get(this);

  QScrollView::frameChanged();
  if (THIS->container)
  	THIS->container->autoResize();
}

void MyScrollView::resizeEvent(QResizeEvent *e)
{
  CSCROLLVIEW *_object = (CSCROLLVIEW *)CWidget::get(this);
  
  QScrollView::resizeEvent(e);
  if (THIS->container)
	  THIS->container->autoResize();
}


void MyScrollView::showEvent(QShowEvent *e)
{
  CSCROLLVIEW *_object = (CSCROLLVIEW *)CWidget::get(this);
  
  QScrollView::showEvent(e);
  if (THIS->container)
	  THIS->container->autoResize();
}

/***************************************************************************

  class MyContents

***************************************************************************/

MyContents::MyContents(QWidget *parent, MyScrollView *scrollview)
: MyContainer(parent)
{
  right = 0;
  bottom = 0;
  sw = scrollview;
}

void MyContents::autoResize(void)
{
  CSCROLLVIEW *_object = (CSCROLLVIEW *)CWidget::get(sw);
  int w, h;
  int ww, hh;
  bool cw, ch;
  bool locked;
  
	locked = THIS->arrangement.locked;
	THIS->arrangement.locked = true;
	ww = hh = -1;

	if (THIS->arrangement.mode)
	{
		/*ww = sw->visibleWidth();
		hh = sw->visibleHeight();*/
	
		/*if (!sw->horizontalScrollBar()->isHidden())
			ww += sw->horizontalScrollBar()->sizeHint().width();
		if (!sw->verticalScrollBar()->isHidden())
			hh += sw->verticalScrollBar()->sizeHint().height();*/
	
		ww = sw->width() - sw->frameWidth() * 2;
		hh = sw->height() - sw->frameWidth() * 2;
	
		//qDebug("autoResize: (%d %d) (%d %d)", sw->visibleWidth(), sw->visibleHeight(), ww, hh);
	
		resize(ww, hh);
		//sw->updateScrollBars();
	}

__AGAIN:

	if (THIS->arrangement.mode)
	{
		//findRightBottom();
		CCONTAINER_get_max_size(THIS, &w, &h);
		//resize(w, h);
		//sw->updateScrollBars();
	}
	else
	{
		w = h = 0;
		
		if (right)
			w = right->x() + right->width();
		if (bottom)
			h = bottom->y() + bottom->height();	
	}
	
	if (ww < 0)
	{
		ww = sw->visibleWidth();
		hh = sw->visibleHeight();	
	}
	
	if (w < ww || THIS->arrangement.mode == ARRANGE_VERTICAL || THIS->arrangement.mode == ARRANGE_ROW)
	{
		w = ww;
		cw = true;
	}
	else
		cw = false;
		
	if (h < hh || THIS->arrangement.mode == ARRANGE_HORIZONTAL || THIS->arrangement.mode == ARRANGE_COLUMN)
	{
		h = hh;
		ch = true;
	}
	else
		ch = false;
	
	if (w != width() || h != height())
	{
		resize(w, h);
		
		//CCONTAINER_arrange(THIS);
		
		sw->updateScrollBars();
				
		if (cw)
			w = sw->visibleWidth();
		if (ch)
			h = sw->visibleHeight();
			
		if (w != width() || h != height())
		{
			resize(w, h);
			
			//THIS->arrangement.locked = locked;
			//CCONTAINER_arrange(THIS);
			//CCONTAINER_arrange(THIS);
			ww = -1;
			goto __AGAIN;
		}
	}

	THIS->arrangement.locked = locked;
	CCONTAINER_arrange(THIS);
}


void MyContents::findRightBottom(void)
{
  QObjectList *list = (QObjectList *)queryList(0, 0, false, false);
  QWidget *wid;
  QObject *ob;
  int w = 0, h = 0;
  int ww, hh;

	right = 0;
	bottom = 0;

  ob = list->first();
  while (ob)
  {
    if (ob->isWidgetType())
    {
      wid = (QWidget *)ob;
      if (!wid->isHidden())
      {
				ww = wid->x() + wid->width();
				hh = wid->y() + wid->height();

				if (ww > w)
				{
					w = ww;
					right = wid;
				}

				if (hh > h)
				{
					h = hh;
					bottom = wid;
				}
			}
    }
    ob = list->next();
  }
  
  delete list;
}


void MyContents::checkWidget(QWidget *wid)
{
  bool doResize = false;
  CSCROLLVIEW *_object = (CSCROLLVIEW *)CWidget::get(sw);

	//CCONTAINER_arrange(CWidget::get(sw));
	
	if (THIS->arrangement.mode)
		return;

  if (wid == right || wid == bottom)
  {
    findRightBottom();
    doResize = true;
  }
  else
  {
    if (right == 0 || (wid->x() + wid->width()) > (right->x() + right->width()))
    {
      right = wid;
      doResize = true;
    }

    if (bottom == 0 || (wid->y() + wid->height()) > (bottom->y() + bottom->height()))
    {
      bottom = wid;
      doResize = true;
    }
  }

  if (doResize)
    autoResize();
}


void MyContents::childEvent(QChildEvent *e)
{
  if (!e->child()->isWidgetType())
    return;

  MyContainer::childEvent(e);

  if (e->inserted())
  {
    //e->child()->installEventFilter(this);
    checkWidget((QWidget *)e->child());
    autoResize();
  }
  else if (e->removed())
  {
    //e->child()->removeEventFilter(this);
    if (e->child() == right || e->child() == bottom)
    {
      findRightBottom();
      autoResize();
    }
  }
}

bool MyContents::eventFilter(QObject *o, QEvent *e)
{
  int type = e->type();
  QWidget *wid = (QWidget *)o;

  if (type == QEvent::Move || type == QEvent::Resize || type == QEvent::Show || type == QEvent::Hide)
    checkWidget(wid);

  return MyContainer::eventFilter(o, e);
}




/***************************************************************************

  ScrollView

***************************************************************************/

BEGIN_METHOD(CSCROLLVIEW_new, GB_OBJECT parent)

  MyScrollView *wid = new MyScrollView(QCONTAINER(VARG(parent)));
  MyContents *cont = new MyContents(wid->viewport(), wid);

  CWIDGET_new(wid, (void *)_object, "ScrollView");
  //wid->setBackgroundOrigin(QWidget::WindowOrigin);
  cont->setBackgroundOrigin(QWidget::AncestorOrigin);

  THIS->container = cont;
  wid->addChild(THIS->container);

  //CWidget::installFilter(THIS->container);
  //CWidget::removeFilter(wid->horizontalScrollBar());
  //CWidget::removeFilter(wid->verticalScrollBar());

  // Border.Sunken by default
  wid->setLineWidth(2);
  wid->setFrameStyle(QFrame::StyledPanel + QFrame::Sunken);

	//THIS->arrangement.autoresize = true;

  wid->show();

END_METHOD


BEGIN_PROPERTY(CSCROLLVIEW_client_x)

  GB.ReturnInteger(WIDGET->viewport()->x());

END_PROPERTY


BEGIN_PROPERTY(CSCROLLVIEW_client_y)

  GB.ReturnInteger(WIDGET->viewport()->y());

END_PROPERTY


BEGIN_PROPERTY(CSCROLLVIEW_client_width)

  GB.ReturnInteger(WIDGET->viewport()->width());

END_PROPERTY


BEGIN_PROPERTY(CSCROLLVIEW_client_height)

  GB.ReturnInteger(WIDGET->viewport()->height());

END_PROPERTY


BEGIN_PROPERTY(CSCROLLVIEW_scroll_x)

  if (READ_PROPERTY)
    GB.ReturnInteger(WIDGET->contentsX());
  else
    WIDGET->setContentsPos(VPROP(GB_INTEGER), WIDGET->contentsY());

END_PROPERTY


BEGIN_PROPERTY(CSCROLLVIEW_scroll_y)

  if (READ_PROPERTY)
    GB.ReturnInteger(WIDGET->contentsY());
  else
    WIDGET->setContentsPos(WIDGET->contentsX(), VPROP(GB_INTEGER));

END_PROPERTY


BEGIN_PROPERTY(CSCROLLVIEW_scroll_w)

  GB.ReturnInteger(WIDGET->contentsWidth());

END_PROPERTY


BEGIN_PROPERTY(CSCROLLVIEW_scroll_h)

  GB.ReturnInteger(WIDGET->contentsHeight());

END_PROPERTY


BEGIN_METHOD(CSCROLLVIEW_scroll, GB_INTEGER x; GB_INTEGER y)

  WIDGET->setContentsPos(VARG(x), VARG(y));

END_METHOD


BEGIN_METHOD(CSCROLLVIEW_ensure_visible, GB_INTEGER x; GB_INTEGER y; GB_INTEGER w; GB_INTEGER h)

  WIDGET->ensureVisible(VARG(x) + VARG(w) / 2, VARG(y) + VARG(h) / 2, VARG(w) / 2, VARG(h) / 2);

END_METHOD


BEGIN_PROPERTY(CSCROLLVIEW_scrollbar)

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


BEGIN_PROPERTY(CSCROLLVIEW_background)

  if (READ_PROPERTY)
    GB.ReturnInteger(THIS->container->paletteBackgroundColor().rgb() & 0xFFFFFF);
  else
    THIS->container->setPaletteBackgroundColor(QColor((QRgb)VPROP(GB_INTEGER)));

END_PROPERTY



/***************************************************************************

  Descriptions

***************************************************************************/

DECLARE_METHOD(CWIDGET_border);

GB_DESC CScrollViewDesc[] =
{
  GB_DECLARE("ScrollView", sizeof(CSCROLLVIEW)), GB_INHERITS("Container"),

  GB_METHOD("_new", NULL, CSCROLLVIEW_new, "(Parent)Container;"),

  GB_PROPERTY("ScrollBar", "i", CSCROLLVIEW_scrollbar),

  GB_PROPERTY("Background", "i", CSCROLLVIEW_background),
  GB_PROPERTY("BackColor", "i", CSCROLLVIEW_background),

  GB_PROPERTY("Border", "i", CWIDGET_border_full),

  GB_PROPERTY("ScrollX", "i", CSCROLLVIEW_scroll_x),
  GB_PROPERTY("ScrollY", "i", CSCROLLVIEW_scroll_y),
  GB_PROPERTY_READ("ScrollW", "i", CSCROLLVIEW_scroll_w),
  GB_PROPERTY_READ("ScrollWidth", "i", CSCROLLVIEW_scroll_w),
  GB_PROPERTY_READ("ScrollH", "i", CSCROLLVIEW_scroll_h),
  GB_PROPERTY_READ("ScrollHeight", "i", CSCROLLVIEW_scroll_h),
  GB_PROPERTY_READ("ClientX", "i", CSCROLLVIEW_client_x),
  GB_PROPERTY_READ("ClientY", "i", CSCROLLVIEW_client_y),
  GB_PROPERTY_READ("ClientW", "i", CSCROLLVIEW_client_width),
  GB_PROPERTY_READ("ClientH", "i", CSCROLLVIEW_client_height),
  GB_PROPERTY_READ("ClientWidth", "i", CSCROLLVIEW_client_width),
  GB_PROPERTY_READ("ClientHeight", "i", CSCROLLVIEW_client_height),
  GB_METHOD("Scroll", NULL, CSCROLLVIEW_scroll, "(X)i(Y)i"),
  GB_METHOD("EnsureVisible", NULL, CSCROLLVIEW_ensure_visible, "(X)i(Y)i(Width)i(Height)i"),

  GB_PROPERTY("Arrangement", "i<Arrange>", CCONTAINER_arrangement),
  GB_PROPERTY("Padding", "i", CCONTAINER_padding),
  GB_PROPERTY("Spacing", "i", CCONTAINER_spacing),

  /*GB_EVENT("ChildInsert", NULL, "(Child)Control;", &EVENT_ChildInsert),
  GB_EVENT("ChildRemove", NULL, "(Child)Control;", &EVENT_ChildRemove),
  GB_EVENT("ChildShow", NULL, "(Child)Control;", &EVENT_ChildShow),
  GB_EVENT("ChildHide", NULL, "(Child)Control;", &EVENT_ChildHide),*/

  GB_CONSTANT("_Properties", "s", 
  	"*," CARRANGEMENT_PROPERTY "," CPADDING_PROPERTIES ",Border{Border.*}=Sunken,ScrollBar{Scroll.*}=Both"),
  GB_CONSTANT("_Arrangement", "i", ARRANGE_FILL),

  GB_END_DECLARE
};


/***************************************************************************

  class CScrollView

***************************************************************************/

CScrollView CScrollView::manager;


bool CScrollView::eventFilter(QObject *o, QEvent *e)
{
  if (e->type() == QEvent::LayoutHint)
    qDebug("Layout hint %p", sender());

  return QObject::eventFilter(o, e);
}



