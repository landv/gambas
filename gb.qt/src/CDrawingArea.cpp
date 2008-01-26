/***************************************************************************

  CDrawingArea.cpp

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

#define __CDRAWINGAREA_CPP

#include <qapplication.h>
#include <qpainter.h>

#ifndef NO_X_WINDOW
#include <X11/Xlib.h>
#endif

#include "CDraw.h"
#include "CDrawingArea.h"

DECLARE_EVENT(EVENT_draw);


/***************************************************************************

  class MyDrawingArea

***************************************************************************/

MyDrawingArea::MyDrawingArea(QWidget *parent) :
QFrame(parent)
{
	drawn = 0;
	cache = 0;
  _background = 0;
  _frozen = false;
  _event_mask = 0;
  setMerge(false);
  setCached(false);
  setBackground();
  setAllowFocus(false);
	setKeyCompression(false);
}


MyDrawingArea::~MyDrawingArea()
{
  if (_background)
    delete _background;
}

void MyDrawingArea::setAllowFocus(bool f)
{
	if (f)
	{
  	setFocusPolicy(WheelFocus);
  	setInputMethodEnabled(true);
	}
	else
		setFocusPolicy(NoFocus);
}


void MyDrawingArea::setMerge(bool m)
{
  _merge = m;
  if (_merge)
    clearWFlags(Qt::WPaintClever);
  else
    setWFlags(Qt::WPaintClever);
}

void MyDrawingArea::setFrozen(bool f)
{
  XWindowAttributes attr;

  if (f == _frozen)
    return;

	#ifndef NO_X_WINDOW
  if (f)
  {
    //setBackgroundMode(Qt::NoBackground);
    XGetWindowAttributes(x11Display(), winId(), &attr);
    _event_mask = attr.your_event_mask;
    XSelectInput(x11Display(), winId(), ExposureMask);
    clearWFlags(Qt::WPaintClever);
    //qDebug("frozen");
  }
  else
  {
    //setBackgroundMode(Qt::PaletteBackground);
    XSelectInput(x11Display(), winId(), _event_mask);
    setMerge(_merge);
    //qDebug("unfrozen");
  }
	#endif
	
  _frozen = f;
}


#if 1
void MyDrawingArea::paintEvent(QPaintEvent *event)
{
  QPainter paint( this );
  QRect r;

  //QFrame::paintEvent(event);

  if (!_background)
  {
    r = event->rect().intersect(frameRect());
    if (r.isValid())
    {
    	QPainter *p;
      void *object = CWidget::get(this);
			bool frame = !contentsRect().contains(event->rect());
			
			cache = new QPixmap(r.width(), r.height());
      cache->fill(this, r.x(), r.y());

      //qDebug("paint: %d %d %d %d", event->rect().x(), event->rect().y(), event->rect().width(), event->rect().height());

      //status = DRAW_status();
      //DRAW_begin(NULL, p, width(), height());
      DRAW_begin(object);

			p = DRAW_get_current();
      
      p->translate(-r.x(), -r.y());
      p->setClipRect(r, QPainter::CoordPainter);
      //p->setClipRegion(event->region().intersect(contentsRect()));
      p->setBrushOrigin(-r.x(), -r.y());
      
			if (frame)
				p->save();
      //qDebug("MyDrawingArea::paintEvent %p", CWidget::get(this));
      GB.Raise(object, EVENT_draw, 0);
			
			if (!contentsRect().contains(event->rect()))
			{
				p->restore();
				//paint.setClipRegion( event->region().intersect(frameRect()) );
				drawFrame(p);
			}
      
      //DRAW_restore(status);
      DRAW_end();
      //delete p;

      //paint.setClipRegion( event->region().intersect( contentsRect() ) );
      paint.drawPixmap(r.x(), r.y(), *cache);
      delete cache;
      cache = 0;
    }
  }
}
#endif

#if 0
void MyDrawingArea::paintEvent(QPaintEvent *event)
{
  QPainter paint( this );

  if ( !contentsRect().contains( event->rect() ) )
  {
      paint.save();
      paint.setClipRegion( event->region().intersect(frameRect()) );
      drawFrame( &paint );
      paint.restore();
  }

  if (!_frozen && !_background)
  {
    if (event->rect().intersects(contentsRect()))
    {
      void *object = CWidget::get(this);
      int status;

      QPainter *p = new QPainter(this);

      p->setClipRegion( event->region().intersect( contentsRect() ) );

      //p->translate(-event->rect().x(), -event->rect().y());

      //p->setClipRegion(event->region().intersect(contentsRect()));
      //p->setBrushOrigin(-event->rect().x(), -event->rect().y());

      status = DRAW_status();
      DRAW_begin(NULL, p);

      //qDebug("MyDrawingArea::paintEvent %p", CWidget::get(this));
      GB.Raise(object, EVENT_draw, 0);

      DRAW_restore(status);
      //delete p;

      //paint.drawPixmap(event->rect().x(), event->rect().y(), cache);
    }
  }
}
#endif

/*
void MyDrawingArea::backgroundPixmapChange(const QPixmap &)
{
  if (!lock)
    update();
}
*/


void MyDrawingArea::setBackground()
{
  if (_background)
  {
    _background->detach();

    #ifdef NO_X_WINDOW
    setErasePixmap(*_background);
    #else
    XSetWindowBackgroundPixmap(x11AppDisplay(), winId(), _background->handle());
    #endif
    setWFlags(Qt::WNoAutoErase);
    setWFlags(Qt::WStaticContents);
  }
  else
  {
    #ifdef NO_X_WINDOW
    setBackgroundMode(Qt::NoBackground);
    #else
    XSetWindowBackgroundPixmap(x11AppDisplay(), winId(), None);
    #endif
    setWFlags(Qt::WNoAutoErase);
    clearWFlags(Qt::WStaticContents);
  }
}

void MyDrawingArea::refreshBackground()
{
	QPainter p;
	p.begin(_background, this);
	drawFrame(&p);
	p.end();
	#ifndef NO_X_WINDOW
  XClearWindow(x11AppDisplay(), winId());
  #endif
}


void MyDrawingArea::clearBackground()
{
  if (_background)
  {
    QPainter p(_background);
    p.fillRect(0, 0, _background->width(), _background->height(), paletteBackgroundColor());
    p.end();

    setBackground();
  }
}


bool MyDrawingArea::doResize(int w, int h)
{
  int wb, hb;

	if (drawn)
	{
		GB.Error("DrawingArea is being drawn");
		return true;
	}

  if (w == width() && h == height())
    return false;

  if (_background)
  {
  	w = QMAX(w, 1);
    h = QMAX(h, 1);

    QPixmap *p = new QPixmap(w, h);
    p->fill(paletteBackgroundColor());

    wb = QMIN(w, _background->width());
    hb = QMIN(h, _background->height());

    bitBlt(p, 0, 0, _background, 0, 0, wb, hb, CopyROP);

    delete _background;
    _background = p;

    setBackground();
  }
  
  return false;
}

void MyDrawingArea::resize(int w, int h)
{
  if (doResize(w, h))
  	return;
  QFrame::resize(w, h);
}


void MyDrawingArea::setGeometry(int x, int y, int w, int h)
{
  if (doResize(w, h))
  	return;
  QFrame::setGeometry(x, y, w, h);
}


void MyDrawingArea::setCached(bool c)
{
  if (_background)
    delete _background;

  if (c)
  {
    _background = new QPixmap(width(), height());
    _background->fill(paletteBackgroundColor());
    clearWFlags(WRepaintNoErase | WResizeNoErase);
  }
  else
  {
    _background = 0;
    setWFlags(WRepaintNoErase | WResizeNoErase);
  }

  setBackground();
}



/*
void MyDrawingArea::setTransparent(bool trans)
{
  if (trans == transparent)
    return;

  if (trans)
  {
    setWFlags(WRepaintNoErase | WResizeNoErase | WPaintClever);
    setBackgroundMode(NoBackground);
  }
  else
  {
    clearWFlags(WRepaintNoErase | WResizeNoErase | WPaintClever);
    setBackgroundMode(PaletteBackground);
  }

  transparent = trans;
  update();
}
*/

/***************************************************************************

  DrawingArea

***************************************************************************/

BEGIN_METHOD(CDRAWINGAREA_new, GB_OBJECT parent)

  MyDrawingArea *wid = new MyDrawingArea(QCONTAINER(VARG(parent)));

  CWIDGET_new(wid, (void *)_object);

  //THIS->widget.background = QColorGroup::Base;
  THIS->container = wid;

  wid->show();

END_METHOD


BEGIN_PROPERTY(CDRAWINGAREA_cached)

  if (READ_PROPERTY)
    GB.ReturnBoolean(WIDGET->isCached());
  else
    WIDGET->setCached(VPROP(GB_BOOLEAN));

END_PROPERTY


BEGIN_PROPERTY(CDRAWINGAREA_track_mouse)

  if (READ_PROPERTY)
    GB.ReturnBoolean(WIDGET->hasMouseTracking());
  else
    WIDGET->setMouseTracking(VPROP(GB_BOOLEAN));

END_PROPERTY


DECLARE_METHOD(CCONTROL_background);


BEGIN_METHOD_VOID(CDRAWINGAREA_clear)

  WIDGET->clearBackground();

END_METHOD


BEGIN_PROPERTY(CDRAWINGAREA_background)

  CCONTROL_background(_object, _param);
  WIDGET->setBackground();

  if (!READ_PROPERTY)
  {
    //WIDGET->setBackgroundMode(Qt::NoBackground);
    WIDGET->clearBackground();
  }

END_PROPERTY

BEGIN_PROPERTY(CDRAWINGAREA_enabled)

  CCONTROL_enabled(_object, _param);

  if (!READ_PROPERTY)
    WIDGET->setFrozen(!VPROP(GB_BOOLEAN));

END_PROPERTY

BEGIN_PROPERTY(CDRAWINGAREA_merge)

  if (READ_PROPERTY)
    GB.ReturnBoolean(WIDGET->isMerge());
  else
    WIDGET->setMerge(VPROP(GB_BOOLEAN));

END_PROPERTY

BEGIN_PROPERTY(CDRAWINGAREA_focus)

  if (READ_PROPERTY)
    GB.ReturnBoolean(WIDGET->isAllowFocus());
  else
    WIDGET->setAllowFocus(VPROP(GB_BOOLEAN));

END_PROPERTY


GB_DESC CDrawingAreaDesc[] =
{
  GB_DECLARE("DrawingArea", sizeof(CDRAWINGAREA)), GB_INHERITS("Container"),

  GB_METHOD("_new", NULL, CDRAWINGAREA_new, "(Parent)Container;"),

  GB_PROPERTY("Cached", "b", CDRAWINGAREA_cached),
  GB_PROPERTY("Merge", "b", CDRAWINGAREA_merge),
  
  GB_PROPERTY("Border", "i", CWIDGET_border_full),
  GB_PROPERTY("Background", "i", CDRAWINGAREA_background),
  GB_PROPERTY("BackColor", "i", CDRAWINGAREA_background),
  
  GB_PROPERTY("Tracking", "b", CDRAWINGAREA_track_mouse),
  GB_PROPERTY("Focus", "b", CDRAWINGAREA_focus),
  GB_PROPERTY("Enabled", "b", CDRAWINGAREA_enabled),

  GB_METHOD("Clear", NULL, CDRAWINGAREA_clear, NULL),

  GB_EVENT("Draw", NULL, NULL, &EVENT_draw),

	GB_INTERFACE("Draw", &DRAW_Interface),

	DRAWINGAREA_DESCRIPTION,

  GB_END_DECLARE
};




