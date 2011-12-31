/***************************************************************************

  CDrawingArea.cpp

  (c) 2000-2012 Benoît Minisini <gambas@users.sourceforge.net>

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

#define __CDRAWINGAREA_CPP

#include <QApplication>
#include <QPaintEvent>
#include <QPixmap>
#include <QPainter>
#include <QX11Info>
#include <QColormap>

#include "CDraw.h"
#include "cpaint_impl.h"
#include "CDrawingArea.h"

#ifndef NO_X_WINDOW
#include <QX11Info>
#include <X11/Xlib.h>
#endif

DECLARE_EVENT(EVENT_Draw);


/***************************************************************************

	class MyDrawingArea

***************************************************************************/

MyDrawingArea::MyDrawingArea(QWidget *parent) : MyContainer(parent)
{
	drawn = 0;
	cache = 0;
	_background = (Qt::HANDLE)0;
	_frozen = false;
	_event_mask = 0;
	_use_paint = false;
	_set_background = false;
	_cached = false;
	_no_background = false;
	_in_draw_event = false;
	_draw_event = EVENT_Draw;
	
	setAttribute(Qt::WA_KeyCompression, false);
	setAttribute(Qt::WA_PaintOnScreen, false);
	setAttribute(Qt::WA_OpaquePaintEvent, false);
	setAttribute(Qt::WA_StaticContents, false);
	
	setAllowFocus(false);
}


MyDrawingArea::~MyDrawingArea()
{
	deleteBackground();
}

void MyDrawingArea::setAllowFocus(bool f)
{
	if (f)
	{
		void *_object = CWidget::getReal(this);
		setFocusPolicy(GB.CanRaise(THIS, EVENT_MouseWheel) ? Qt::WheelFocus : Qt::StrongFocus);
		setAttribute(Qt::WA_InputMethodEnabled, true);
	}
	else
	{
		setFocusPolicy(Qt::NoFocus);
	}
}


void MyDrawingArea::setFrozen(bool f)
{
	if (f == _frozen)
		return;

	#ifndef NO_X_WINDOW
	XWindowAttributes attr;

	if (f)
	{
		//setBackgroundMode(Qt::NoBackground);
		XGetWindowAttributes(QX11Info::display(), winId(), &attr);
		_event_mask = attr.your_event_mask;
		XSelectInput(QX11Info::display(), winId(), ExposureMask);
		//clearWFlags(Qt::WPaintClever);
		//qDebug("frozen");
	}
	else
	{
		//setBackgroundMode(Qt::PaletteBackground);
		XSelectInput(QX11Info::display(), winId(), _event_mask);
		//qDebug("unfrozen");
	}
	#endif
	
	_frozen = f;
}

void MyDrawingArea::redraw(QRect &r, bool frame)
{
	QPainter *p;
	void *_object = CWidget::get(this);
	int fw;
	
	if (!_object)
		return;
			
	//qDebug("paint: %d %d %d %d", r.x(), r.y(), r.width(), r.height());

	_in_draw_event = true;
		
	if (_use_paint)
	{
		PAINT_begin(THIS);
		p = PAINT_get_current();
	}
	else
	{
		DRAW_begin(THIS);
		p = DRAW_get_current();
	}
		
	/*if (!isTransparent())
	{
		p->translate(-r.x(), -r.y());
	}*/
	
	//p->save();
	
	fw = frameWidth();
	
	if (THIS->widget.bg != COLOR_DEFAULT)
	{
		p->fillRect(fw, fw, width() - fw * 2, height() - fw * 2, QColor((QRgb)THIS->widget.bg));
	}
	
	if (!_use_paint)
	{
		//p->setBrushOrigin(-r.x(), -r.y());
		DRAW_clip(r.x(), r.y(), r.width(), r.height());
	}
	else
		PAINT_clip(r.x(), r.y(), r.width(), r.height());
	
	//p->setClipRegion(event->region().intersect(contentsRect()));
	//p->setBrushOrigin(-r.x(), -r.y());
	
	GB.Raise(THIS, _draw_event, 0);
		
	//p->restore();
	
	if (frame)
	{
		QPainter pf(this);
		pf.setClipping(false);
		pf.initFrom(this);
		pf.setRenderHint(QPainter::Antialiasing, false);
		drawFrame(&pf);
	}
		
	if (_use_paint)
		PAINT_end();
	else
		DRAW_end();

	_in_draw_event = false;
}

void MyDrawingArea::createBackground(int w, int h)
{
	QX11Info xinfo = x11Info();
	QPixmap p;
	Qt::HANDLE old = _background;
	GC gc;
	
	_background = (Qt::HANDLE)XCreatePixmap(QX11Info::display(), RootWindow(QX11Info::display(), xinfo.screen()), w, h, xinfo.depth());
	_background_w = w;
	_background_h = h;
	
	//qDebug("createBackground: %d x %d : %d -> %08X / winId = %08X", w, h, xinfo.depth(), (int)_background, (int)winId());
	//p = QPixmap::fromX11Pixmap(_background, QPixmap::ExplicitlyShared);
	//qDebug("color = %06X -> %06X", palette().color(backgroundRole()).rgb(), QColormap::instance().pixel((unsigned long)palette().color(backgroundRole()).rgb()));
	
	gc = XCreateGC(QX11Info::display(), _background, 0, 0);
	XSetForeground(QX11Info::display(), gc, QColormap::instance().pixel((unsigned long)palette().color(backgroundRole()).rgb()));
	XFillRectangle(QX11Info::display(), _background, gc, 0, 0, w, h);
	XFreeGC(QX11Info::display(), gc);
	
	XSetWindowBackgroundPixmap(QX11Info::display(), winId(), _background);
	XClearArea(QX11Info::display(), winId(), 0, 0, 0, 0, True);
	
	_cached = true;
	
	if (old)
		XFreePixmap(QX11Info::display(), (Pixmap)old);
}

void MyDrawingArea::deleteBackground()
{
	if (_cached && _background)
	{
		XSetWindowBackgroundPixmap(QX11Info::display(), winId(), None);
		XFreePixmap(QX11Info::display(), (Pixmap)_background);
		_cached = false;
		_background = 0;
	}
}

QPixmap *MyDrawingArea::getBackgroundPixmap()
{
	if (!_cached)
		return NULL;
	
	_background_pixmap = QPixmap::fromX11Pixmap(_background, QPixmap::ExplicitlyShared);
	//qDebug("getBackgroundPixmap: %08X", (int)_background_pixmap.handle());
	return &_background_pixmap;
}

void MyDrawingArea::paintEvent(QPaintEvent *event)
{
	if (_cached)
	{
		#ifndef NO_X_WINDOW
		if (_set_background)
		{
			//qDebug("set_background: %08X %08X", (int)winId(), (int)_background);
			XSetWindowBackgroundPixmap(QX11Info::display(), winId(), _background);
			_set_background = false;
		}
		#endif
		QPainter paint( this );
		drawFrame(&paint);
		//MyContainer::paintEvent(event);
	}
	else
	{
		//QPainter paint( this );
		QRect r;

		r = event->rect().intersect(contentsRect());
		if (r.isValid())
		{
			/*if (!isTransparent())
			{
				cache = new QPixmap(r.width(), r.height());
				cache->fill(this, r.x(), r.y());
			}*/
			
			redraw(r, true);

			/*if (!isTransparent())
			{
				paint.drawPixmap(r.x(), r.y(), *cache);
				delete cache;
				cache = 0;
			}*/
		}
	}
}

void MyDrawingArea::setBackground()
{
	if (_cached)
	{
		#ifdef NO_X_WINDOW
		setErasePixmap(*_background);
		#else
		//if (isVisible())
		//	XSetWindowBackgroundPixmap(QX11Info::display(), winId(), _background->handle());
		//else
		_set_background = true;
		//XSetWindowBackgroundPixmap(QX11Info::display(), winId(), _background);
		refreshBackground();
		#endif
	}
}

void MyDrawingArea::refreshBackground()
{
	if (_cached)
	{
		#ifdef NO_X_WINDOW
		update();
		#else
		int fw = frameWidth();
		if (fw == 0)
			XClearWindow(QX11Info::display(), winId());
		else
		{
			XClearArea(QX11Info::display(), winId(), fw, fw, width() - fw * 2, height() - fw * 2, False);
			repaint();
		}
		#endif
	}
}


void MyDrawingArea::clearBackground()
{
	if (_cached)
		createBackground(width(), height());
	else
		XClearArea(QX11Info::display(), winId(), 0, 0, 0, 0, True);
}

void MyDrawingArea::resizeEvent(QResizeEvent *e)
{
	MyContainer::resizeEvent(e);
	updateBackground();
}

void MyDrawingArea::updateBackground()
{
	int wb, hb, w, h;

	if (drawn)
	{
		GB.Error("DrawingArea is being drawn");
		return;
	}

	if (_cached)
	{
		w = QMAX(width(), 1);
		h = QMAX(height(), 1);
		
		if (w != _background_w || h != _background_h)
		{
			Qt::HANDLE old = _background;
			
			wb = QMIN(w, _background_w);
			hb = QMIN(h, _background_h);

			_background = 0;
			createBackground(w, h);

			GC gc = XCreateGC(QX11Info::display(), old, 0, 0);
			XCopyArea(QX11Info::display(), old, _background, gc, 0, 0, wb, hb, 0, 0);
			XFreeGC(QX11Info::display(), gc);

			XFreePixmap(QX11Info::display(), old);

			setBackground();
		}
	}
}

void MyDrawingArea::setStaticContents(bool on)
{
}


void MyDrawingArea::updateCache()
{
	//qDebug("updateCache: %d %08X", _cached, (int)winId());

	if (_cached) // && !_transparent)
	{
		setAttribute(Qt::WA_DontCreateNativeAncestors, true);
		setAttribute(Qt::WA_NativeWindow, true);
		setAttribute(Qt::WA_PaintOnScreen, true);
		setAttribute(Qt::WA_OpaquePaintEvent, true);
		setAttribute(Qt::WA_StaticContents, true);
		createBackground(width(), height());
		setBackground();
	}
	else //if (_background)
	{
		deleteBackground();
		setAttribute(Qt::WA_PaintOnScreen, false);
		setAttribute(Qt::WA_OpaquePaintEvent, false);
		setAttribute(Qt::WA_StaticContents, false);
		#ifdef NO_X_WINDOW
		setBackgroundMode(Qt::NoBackground);
		#else
		//XClearArea(QX11Info::display(), winId(), 0, 0, 0, 0, True);
		repaint();
		#endif
	}
	
	updateNoBackground();
}

void MyDrawingArea::setCached(bool c)
{
	if (c == _cached)
		return;
	
	_cached = c;
	updateCache();
}

void MyDrawingArea::setPalette(const QPalette &pal)
{
	if (_cached)
		return;
	MyContainer::setPalette(pal);
	repaint();
}

void MyDrawingArea::updateNoBackground()
{
	setAttribute(Qt::WA_NoSystemBackground, _no_background);
	if (_cached)
		setBackground();
}

void MyDrawingArea::setNoBackground(bool on)
{
	_no_background = on;
	updateNoBackground();
}

void MyDrawingArea::hideEvent(QHideEvent *e)
{
	if (_cached)
		_set_background = true;
	MyContainer::hideEvent(e);
}

/***************************************************************************

	DrawingArea

***************************************************************************/

BEGIN_METHOD(CDRAWINGAREA_new, GB_OBJECT parent)

	MyDrawingArea *wid = new MyDrawingArea(QCONTAINER(VARG(parent)));

	//THIS->widget.background = QColorGroup::Base;
	THIS->container = wid;
	THIS->widget.flag.noBackground = true;

	CWIDGET_new(wid, (void *)_object);

END_METHOD


BEGIN_PROPERTY(CDRAWINGAREA_cached)

	if (READ_PROPERTY)
		GB.ReturnBoolean(WIDGET->isCached());
	else
	{
		if (THIS->widget.bg == COLOR_DEFAULT)
		{
			CWIDGET_set_color((CWIDGET *)THIS, THIS->widget.fg, WIDGET->palette().color(WIDGET->backgroundRole()).rgb() & 0xFFFFFF);
			WIDGET->clearBackground();
		}
		WIDGET->setCached(VPROP(GB_BOOLEAN));
	}

END_PROPERTY


DECLARE_METHOD(Control_Background);


BEGIN_METHOD_VOID(CDRAWINGAREA_clear)

	WIDGET->clearBackground();

END_METHOD


BEGIN_PROPERTY(DrawingArea_Background)

	Control_Background(_object, _param);

	if (!READ_PROPERTY)
		WIDGET->clearBackground();

END_PROPERTY


BEGIN_PROPERTY(DrawingArea_NoBackground)

	if (READ_PROPERTY)
		GB.ReturnBoolean(WIDGET->hasNoBackground());
	else
		WIDGET->setNoBackground(VPROP(GB_BOOLEAN));

END_PROPERTY


BEGIN_PROPERTY(CDRAWINGAREA_border)

	CCONTAINER_border(_object, _param);

	if (!READ_PROPERTY)
	{
		WIDGET->clearBackground();
	}

END_PROPERTY

BEGIN_PROPERTY(CDRAWINGAREA_enabled)

	Control_Enabled(_object, _param);

	if (!READ_PROPERTY)
		WIDGET->setFrozen(!VPROP(GB_BOOLEAN));

END_PROPERTY

BEGIN_PROPERTY(CDRAWINGAREA_focus)

	if (READ_PROPERTY)
		GB.ReturnBoolean(WIDGET->isAllowFocus());
	else
		WIDGET->setAllowFocus(VPROP(GB_BOOLEAN));

END_PROPERTY

BEGIN_PROPERTY(CDRAWINGAREA_painted)

	if (READ_PROPERTY)
		GB.ReturnBoolean(WIDGET->isPaint());
	else
		WIDGET->setPaint(VPROP(GB_BOOLEAN));

END_PROPERTY

#if 0
BEGIN_PROPERTY(CDRAWINGAREA_transparent)

	if (READ_PROPERTY)
		GB.ReturnBoolean(WIDGET->isTransparent());
	else
	{
		WIDGET->setTransparent(VPROP(GB_BOOLEAN));
		//THIS->widget.flag.fillBackground = !WIDGET->isTransparent();
		//CWIDGET_reset_color((CWIDGET *)THIS);
	}

END_PROPERTY
#endif

BEGIN_METHOD(DrawingArea_Refresh, GB_INTEGER x; GB_INTEGER y; GB_INTEGER w; GB_INTEGER h)

	int x, y, w, h;

	/*if (WIDGET->isCached())
	{
		QRect r;
		
		if (!MISSING(x) && !MISSING(y))
			r.setRect(VARG(x), VARG(y), VARGOPT(w, WIDGET->width()), VARGOPT(h, WIDGET->height()));
		else
			r.setRect(0, 0, WIDGET->width(), WIDGET->height());
		
		WIDGET->redraw(r, false);
	}*/
	
	if (!MISSING(x) && !MISSING(y))
	{
		x = VARG(x);
		y = VARG(y);
		w = VARGOPT(w, QWIDGET(_object)->width());
		h = VARGOPT(h, QWIDGET(_object)->height());
		WIDGET->update(x, y, w, h);
	}
	else
	{
		WIDGET->update();
	}
	
END_METHOD

GB_DESC CDrawingAreaDesc[] =
{
	GB_DECLARE("DrawingArea", sizeof(CDRAWINGAREA)), GB_INHERITS("Container"),

	GB_METHOD("_new", NULL, CDRAWINGAREA_new, "(Parent)Container;"),

	GB_PROPERTY("Cached", "b", CDRAWINGAREA_cached),
	
	GB_PROPERTY("Arrangement", "i", CCONTAINER_arrangement),
	GB_PROPERTY("AutoResize", "b", CCONTAINER_auto_resize),
	GB_PROPERTY("Spacing", "b", CCONTAINER_spacing),
	GB_PROPERTY("Margin", "b", CCONTAINER_margin),
	GB_PROPERTY("Padding", "i", CCONTAINER_padding),
	GB_PROPERTY("Indent", "b", CCONTAINER_indent),

	GB_PROPERTY("Border", "i", CDRAWINGAREA_border),
	GB_PROPERTY("NoBackground", "b", DrawingArea_NoBackground),
	GB_PROPERTY("Background", "i", DrawingArea_Background),
	
	GB_PROPERTY("Focus", "b", CDRAWINGAREA_focus),
	GB_PROPERTY("Enabled", "b", CDRAWINGAREA_enabled),
	GB_PROPERTY("Painted", "b", CDRAWINGAREA_painted),

	GB_METHOD("Clear", NULL, CDRAWINGAREA_clear, NULL),
	GB_METHOD("Refresh", NULL, DrawingArea_Refresh, "[(X)i(Y)i(Width)i(Height)i]"),

	GB_EVENT("Draw", NULL, NULL, &EVENT_Draw),

	GB_INTERFACE("Draw", &DRAW_Interface),
	GB_INTERFACE("Paint", &PAINT_Interface),

	DRAWINGAREA_DESCRIPTION,

	GB_END_DECLARE
};




