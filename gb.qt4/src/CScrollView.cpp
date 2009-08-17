/***************************************************************************

  CScrollView.cpp

  (c) 2000-2009 Benoît Minisini <gambas@users.sourceforge.net>

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
  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

***************************************************************************/

#define __CSCROLLVIEW_CPP

#include <qobject.h>
#include <qtimer.h>
//Added by qt3to4:
#include <QShowEvent>
#include <QResizeEvent>
//#include <QChildEvent>
#include <QEvent>
#include <QScrollArea>
#include <QScrollBar>

#include "gambas.h"

#include "CConst.h"
#include "CColor.h"

#include "CScrollView.h"

//#define DEBUG

DECLARE_EVENT(EVENT_Scroll);


/***************************************************************************

  class MyScrollView

***************************************************************************/

MyScrollView::MyScrollView(QWidget *parent)
: QScrollArea(parent)
{
	setMouseTracking(false);
	_noscroll = false;
}

void MyScrollView::showEvent(QShowEvent *e)
{
  CSCROLLVIEW *_object = (CSCROLLVIEW *)CWidget::get(this);
  
  QScrollArea::showEvent(e);
  if (THIS->container)
	  THIS->container->autoResize();
}

/***************************************************************************

  class MyContents

***************************************************************************/

MyContents::MyContents(MyScrollView *scrollview)
: MyContainer(scrollview)
{
  CSCROLLVIEW *_object = (CSCROLLVIEW *)CWidget::get(scrollview);
	bool shown;

	right = 0;
  bottom = 0;
  sw = scrollview;
	shown = THIS->widget.flag.shown;
  sw->setWidget(this);
	THIS->widget.flag.shown = shown;
  timer = false;
	_mustfind = false;
}

void MyContents::autoResize(void)
{
  CSCROLLVIEW *_object = (CSCROLLVIEW *)CWidget::get(sw);
  int w, h;
  int ww, hh;
  bool cw, ch;
	int oldw, oldh;
  bool locked;
  int i;
	int x, y;

	//qDebug("autoResize: (%s %p)", THIS->widget.name, THIS);
	
	locked = THIS->arrangement.locked;
	THIS->arrangement.locked = true;
	ww = hh = -1;
	oldw = width(); oldh = height();

	x = sw->horizontalScrollBar()->value();
	y = sw->verticalScrollBar()->value();
	sw->_noscroll = true;
	
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
	
		//qDebug("autoResize: (%d %d) (%d %d)", sw->viewport()->width(), sw->viewport()->height(), ww, hh);
	
		resize(ww, hh);
		//sw->updateScrollBars();
	}
	else if (_mustfind)
	{
		findRightBottom();
	}

	for(i = 0; i < 3; i++)
	{
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
			ww = sw->viewport()->width();
			hh = sw->viewport()->height(); //visibleHeight();	
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
			
			//sw->updateScrollBars();
					
			if (cw)
				w = sw->viewport()->width();
			if (ch)
				h = sw->viewport()->height();
				
			if (w != width() || h != height())
			{
				resize(w, h);
				
				//THIS->arrangement.locked = locked;
				//CCONTAINER_arrange(THIS);
				//CCONTAINER_arrange(THIS);
				ww = -1;
				continue;
			}
		}
		break;
	}

	THIS->arrangement.locked = locked;
	if (width() != oldw || height() != oldh)
	{
		#ifdef DEBUG
		if (!qstrcmp(THIS->widget.name, "svwWorkspace"))
		{
			CWIDGET *wr = CWidget::get(right);
			CWIDGET *wb = CWidget::get(bottom);
			qDebug("MyContents::autoResize: %d %d --> %d %d", oldw, oldh, width(), height());
			qDebug("right = %s %s bottom = %s %s", wr ? GB.GetClassName(wr) : 0, wr ? wr->name : 0, wb ? GB.GetClassName(wb) : 0, wb ? wb->name : 0);
		}
		#endif
		CCONTAINER_arrange(THIS);
	}
	
	sw->horizontalScrollBar()->setValue(x);
	sw->verticalScrollBar()->setValue(y);
	sw->_noscroll = false;
	
	timer = false;
}


void MyContents::findRightBottom(void)
{
  QObjectList list = children();
  int i;
  QWidget *wid;
  QObject *ob;
  int w = 0, h = 0;
  int ww, hh;

	right = 0;
	bottom = 0;

	for (i = 0; i < list.count(); i++)
	{
  	ob = list.at(i);
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
  }

	_mustfind = false;

	#ifdef DEBUG
  CSCROLLVIEW *_object = (CSCROLLVIEW *)CWidget::get(sw);
	if (!qstrcmp(THIS->widget.name, "svwWorkspace"))
	{
		CWIDGET *wr = CWidget::get(right);
		CWIDGET *wb = CWidget::get(bottom);
		qDebug("MyContents::findRightBottom: right = %s %s bottom = %s %s", wr ? GB.GetClassName(wr) : 0, wr ? wr->name : 0, wb ? GB.GetClassName(wb) : 0, wb ? wb->name : 0);
	}
	#endif
}


void MyContents::checkWidget(QWidget *wid)
{
  bool doResize = false;
  CSCROLLVIEW *_object = (CSCROLLVIEW *)CWidget::get(sw);

	//CCONTAINER_arrange(CWidget::get(sw));
	
	if (THIS->arrangement.mode)
	{
		checkAutoResizeLater();
		return;
	}

	#ifdef DEBUG
	CWIDGET *ob = CWidget::get(wid);
	qDebug("MyContents::checkWidget: %p: %s %p: %s", wid, GB.GetClassName(ob), ob, ob->name);
	qDebug("MyContents::checkWidget: %d %d", wid->x(), wid->y());
	#endif

	if (wid == right || wid == bottom)
  {
    _mustfind = true; //findRightBottom();
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
		_mustfind = true;
	
  checkAutoResizeLater();
}

void MyContents::childEvent(QChildEvent *e)
{
	MyContainer::childEvent(e);

	if (!e->child()->isWidgetType())
		return;
	if (!CWidget::get(e->child()))
		return;

	if (e->added())
  {
		//qDebug("childEvent: added: %p: %s %p: %d", e->child(), GB.GetClassName(CWidget::get(e->child())), CWidget::get(e->child()), CWIDGET_test_flag(CWidget::get(e->child()), WF_DELETED));
    //checkWidget((QWidget *)e->child());
		_mustfind = true;
		checkAutoResizeLater();
  }
  else if (e->removed())
  {
		//qDebug("childEvent: removed: %p: %s %p: %d", e->child(), GB.GetClassName(CWidget::get(e->child())), CWidget::get(e->child()), CWIDGET_test_flag(CWidget::get(e->child()), WF_DELETED));
    if (e->child() == right || e->child() == bottom)
		{
			if (e->child() == right)
				right = 0;
			else
				bottom = 0;
			_mustfind = true;
		}
		checkAutoResizeLater();
  }
}

// void MyContents::afterArrange()
// {
// 	//qDebug("MyContents::afterArrange");
// 	_mustfind = true;
// 	checkAutoResizeLater();
// }

void MyContents::checkAutoResizeLater()
{
	if (timer)
		return;
	
	QTimer::singleShot(0, this, SLOT(autoResize(void)));
	timer = true;	
}

void CSCROLLVIEW_arrange(void *_object)
{
	if (THIS->container)
		THIS->container->checkAutoResizeLater();
		//THIS->container->checkWidget(child->widget);
}

/***************************************************************************

  ScrollView

***************************************************************************/

BEGIN_METHOD(CSCROLLVIEW_new, GB_OBJECT parent)

  MyScrollView *wid = new MyScrollView(QCONTAINER(VARG(parent)));
  MyContents *cont = new MyContents(wid);

  CWIDGET_new(wid, (void *)_object, true);
  //wid->setBackgroundOrigin(QWidget::WindowOrigin);
  //cont->setBackgroundOrigin(QWidget::AncestorOrigin);

  THIS->container = cont;
  //wid->setWidget(THIS->container);

  QObject::connect(wid->horizontalScrollBar(), SIGNAL(valueChanged(int)), &CScrollView::manager, SLOT(scrolled()));
  QObject::connect(wid->verticalScrollBar(), SIGNAL(valueChanged(int)), &CScrollView::manager, SLOT(scrolled()));

  //CWidget::installFilter(THIS->container);
  //CWidget::removeFilter(wid->horizontalScrollBar());
  //CWidget::removeFilter(wid->verticalScrollBar());

  // Border.Sunken by default
  wid->setLineWidth(2);
  wid->setFrameStyle(QFrame::StyledPanel | QFrame::Sunken);

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
    GB.ReturnInteger(WIDGET->horizontalScrollBar()->value());
  else
    WIDGET->horizontalScrollBar()->setValue(VPROP(GB_INTEGER));

END_PROPERTY


BEGIN_PROPERTY(CSCROLLVIEW_scroll_y)

  if (READ_PROPERTY)
    GB.ReturnInteger(WIDGET->verticalScrollBar()->value());
  else
    WIDGET->verticalScrollBar()->setValue(VPROP(GB_INTEGER));

END_PROPERTY


BEGIN_PROPERTY(CSCROLLVIEW_scroll_w)

  GB.ReturnInteger(WIDGET->widget()->width());

END_PROPERTY


BEGIN_PROPERTY(CSCROLLVIEW_scroll_h)

  GB.ReturnInteger(WIDGET->widget()->height());

END_PROPERTY


BEGIN_METHOD(CSCROLLVIEW_scroll, GB_INTEGER x; GB_INTEGER y)

  WIDGET->horizontalScrollBar()->setValue(VARG(x));
  WIDGET->verticalScrollBar()->setValue(VARG(y));

END_METHOD


BEGIN_METHOD(CSCROLLVIEW_ensure_visible, GB_INTEGER x; GB_INTEGER y; GB_INTEGER w; GB_INTEGER h)

  WIDGET->ensureVisible(VARG(x) + VARG(w) / 2, VARG(y) + VARG(h) / 2, VARG(w) / 2, VARG(h) / 2);

END_METHOD


BEGIN_PROPERTY(CSCROLLVIEW_scrollbar)

  int scroll;

  if (READ_PROPERTY)
  {
    scroll = 0;
    if (WIDGET->horizontalScrollBarPolicy() == Qt::ScrollBarAsNeeded)
      scroll += 1;
    if (WIDGET->verticalScrollBarPolicy() == Qt::ScrollBarAsNeeded)
      scroll += 2;

    GB.ReturnInteger(scroll);
  }
  else
  {
    scroll = VPROP(GB_INTEGER) & 3;
    WIDGET->setHorizontalScrollBarPolicy((scroll & 1) ? Qt::ScrollBarAsNeeded : Qt::ScrollBarAlwaysOff);
    WIDGET->setVerticalScrollBarPolicy((scroll & 2) ? Qt::ScrollBarAsNeeded : Qt::ScrollBarAlwaysOff);
  }

END_PROPERTY

#if 0
BEGIN_PROPERTY(CSCROLLVIEW_background)

  if (READ_PROPERTY)
    GB.ReturnInteger(THIS->container->palette().color(QPalette::Window).rgb() & 0xFFFFFF);
  else
  {
  	QPalette pal;
  	
  	if (VPROP(GB_INTEGER) != COLOR_DEFAULT)
  	{
  		pal = THIS->container->palette();
  		pal.setColor(QPalette::Window, QColor((QRgb)VPROP(GB_INTEGER)));
  	}
    THIS->container->setPalette(pal);
  }

END_PROPERTY
#endif


/***************************************************************************

  Descriptions

***************************************************************************/

DECLARE_METHOD(CWIDGET_border);

GB_DESC CScrollViewDesc[] =
{
  GB_DECLARE("ScrollView", sizeof(CSCROLLVIEW)), GB_INHERITS("Container"),

  GB_METHOD("_new", NULL, CSCROLLVIEW_new, "(Parent)Container;"),

  GB_PROPERTY("ScrollBar", "i", CSCROLLVIEW_scrollbar),

  GB_PROPERTY("Border", "b", CWIDGET_border_simple),

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

  GB_PROPERTY("Arrangement", "i", CCONTAINER_arrangement),
  GB_PROPERTY("Padding", "i", CCONTAINER_padding),
  GB_PROPERTY("Spacing", "b", CCONTAINER_spacing),
  GB_PROPERTY("Margin", "b", CCONTAINER_margin),
  GB_PROPERTY("Indent", "b", CCONTAINER_indent),

  GB_EVENT("Scroll", NULL, NULL, &EVENT_Scroll),

  SCROLLVIEW_DESCRIPTION,

  GB_END_DECLARE
};


/***************************************************************************

  class CScrollView

***************************************************************************/

CScrollView CScrollView::manager;


static void send_scroll(void *param)
{
  GB.Raise(param, EVENT_Scroll, 0);
  GB.Unref(&param);
}

void CScrollView::scrolled(void)
{
	GET_SENDER();
	
	if (WIDGET->_noscroll)
		return;

  GB.Ref(THIS);
  GB.Post((void (*)())send_scroll, (intptr_t)THIS);
}



