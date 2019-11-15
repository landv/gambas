/***************************************************************************

  CDrawingArea.cpp

  (c) 2000-2017 Benoît Minisini <g4mba5@gmail.com>

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
#include <QTimer>
#include <QEvent>

#include "CDraw.h"
#include "cpaint_impl.h"
#include "CColor.h"
#include "CDrawingArea.h"

#ifndef QT5
#ifndef NO_X_WINDOW
#include <QX11Info>
#include <X11/Xlib.h>
#endif

#ifdef FontChange
#undef FontChange
#endif
#endif

DECLARE_EVENT(EVENT_Draw);
DECLARE_EVENT(EVENT_Font);
DECLARE_EVENT(EVENT_Change);


static void send_change_event_widget(CWIDGET *widget)
{
	if (GB.Is(widget, CLASS_DrawingArea))
		GB.Raise(widget, EVENT_Change, 0);
}

void CDRAWINGAREA_send_change_event()
{
	CWidget::each(send_change_event_widget);
}


/***************************************************************************

	class MyDrawingArea

***************************************************************************/

int MyDrawingArea::_in_any_draw_event = 0;

MyDrawingArea::MyDrawingArea(QWidget *parent) : MyContainer(parent)
{
	drawn = 0;
	cache = 0;
#ifndef QT5
	_background = (Qt::HANDLE)0;
#endif
	_frozen = false;
	_event_mask = 0;
	_set_background = true;
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

void MyDrawingArea::setVisible(bool visible)
{
	MyContainer::setVisible(visible);
#ifndef QT5
	if (_cached)
	{
		if (visible)
			QTimer::singleShot(10, this, SLOT(setBackground()));
		else
			parentWidget()->update();
	}
#endif
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

#ifndef QT5
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
	XFlush(QX11Info::display());
#endif
#endif

	_frozen = f;
}

static void cleanup_drawing(intptr_t _object)
{
	PAINT_end();
}

void MyDrawingArea::redraw(QRect &r, bool frame)
{
	QPainter *p;
	void *_object = CWidget::get(this);
	int fw;
	GB_COLOR bg;
	GB_RAISE_HANDLER handler;

	if (!_object)
		return;

	//qDebug("paint: %d %d %d %d", r.x(), r.y(), r.width(), r.height());

	_in_draw_event = true;
	_in_any_draw_event++;

	PAINT_begin(THIS);
	p = PAINT_get_current();

	/*if (!isTransparent())
	{
		p->translate(-r.x(), -r.y());
	}*/

	//p->save();

	fw = frameWidth();

	bg = CWIDGET_get_background((CWIDGET *)THIS);
	if (bg != COLOR_DEFAULT)
	{
		p->fillRect(fw, fw, width() - fw * 2, height() - fw * 2, TO_QCOLOR(bg));
	}

	PAINT_clip(r.x(), r.y(), r.width(), r.height());

	//p->setClipRegion(event->region().intersect(contentsRect()));
	//p->setBrushOrigin(-r.x(), -r.y());

	handler.callback = cleanup_drawing;
	handler.data = (intptr_t)THIS;

	GB.RaiseBegin(&handler);
	GB.Raise(THIS, _draw_event, 0);
	GB.RaiseEnd(&handler);

	//p->restore();

	if (frame)
	{
		QPainter pf(this);
		pf.setClipping(false);
		//pf.begin(this); // Qt 5.x: avoid message "QPainter::begin: Painter already active"
		pf.setRenderHint(QPainter::Antialiasing, false);
		drawFrame(&pf);
	}

	PAINT_end();

	_in_draw_event = false;
	_in_any_draw_event--;
}

void MyDrawingArea::createBackground(int w, int h)
{
	void *_object = CWidget::get(this);
	QPixmap p;
#ifndef QT5
	QX11Info xinfo = x11Info();
	Qt::HANDLE old = _background;
#endif

#ifndef QT5
	_background = (Qt::HANDLE)XCreatePixmap(QX11Info::display(), RootWindow(QX11Info::display(), xinfo.screen()), w, h, xinfo.depth());
	_background_pixmap = QPixmap::fromX11Pixmap(_background, QPixmap::ExplicitlyShared);
#else
	_background_pixmap = QPixmap(w, h);
#endif
	_background_w = w;
	_background_h = h;

	//qDebug("createBackground: %d x %d : %d -> %08X / winId = %08X", w, h, xinfo.depth(), (int)_background, (int)winId());
	//p = QPixmap::fromX11Pixmap(_background, QPixmap::ExplicitlyShared);
	//qDebug("color = %06X -> %06X", palette().color(backgroundRole()).rgb(), QColormap::instance().pixel((unsigned long)palette().color(backgroundRole()).rgb()));

#if 0
	gc = XCreateGC(QX11Info::display(), _background, 0, 0);
	//XSetForeground(QX11Info::display(), gc, QColormap::instance().pixel((unsigned long)palette().color(backgroundRole()).rgb()));
	XSetForeground(QX11Info::display(), gc, QColormap::instance().pixel(CWIDGET_get_real_background((CWIDGET *)THIS)));
	XFillRectangle(QX11Info::display(), _background, gc, 0, 0, w, h);
	XFreeGC(QX11Info::display(), gc);
#endif

	_background_pixmap.fill(CCOLOR_make(CWIDGET_get_real_background((CWIDGET *)THIS)));

	//qDebug("XSetWindowBackgroundPixmap: %08X %08X", (int)winId(), (int)_background);
#ifndef QT5
	XSetWindowBackgroundPixmap(QX11Info::display(), winId(), _background);
	XClearArea(QX11Info::display(), winId(), 0, 0, 0, 0, True);

	if (old)
		XFreePixmap(QX11Info::display(), (Pixmap)old);

	XFlush(QX11Info::display());
#else
	update();
#endif

	_cached = true;
}

void MyDrawingArea::deleteBackground()
{
	if (hasCacheBackground())
	{
#ifndef QT5
		//qDebug("XSetWindowBackgroundPixmap: %08X None", (int)winId());
		XSetWindowBackgroundPixmap(QX11Info::display(), winId(), None);
		XFreePixmap(QX11Info::display(), (Pixmap)_background);
		XFlush(QX11Info::display());
		_background = 0;
#else
		_background_pixmap = QPixmap();
#endif
		_cached = false;
		_background_w = _background_h = 0;
	}
}

QPixmap *MyDrawingArea::getBackgroundPixmap()
{
	if (!hasCacheBackground())
		return NULL;
	else
		return &_background_pixmap;
}

void MyDrawingArea::paintEvent(QPaintEvent *event)
{
	if (_cached)
	{
#ifndef QT5
#ifndef NO_X_WINDOW
		if (_set_background)
		{
			//qDebug("XSetWindowBackgroundPixmap: %08X %08X (paint)", (int)winId(), (int)_background);
			XSetWindowBackgroundPixmap(QX11Info::display(), winId(), _background);
			XFlush(QX11Info::display());
			_set_background = false;
		}
#endif
#endif

		QPainter p(this);

#ifdef QT5
		p.drawPixmap(0, 0, _background_pixmap);
#endif

		if (frameWidth())
		{
			QRegion r(0, 0, width(), height());
			r = r.subtracted(QRegion(frameWidth(), frameWidth(), width() - frameWidth() * 2, height() - frameWidth() * 2));
			p.setClipRegion(r);
			p.setClipping(true);
			//p.drawPixmap(0, 0, *getBackgroundPixmap());
		}

		drawFrame(&p);
	}
	else
	{
		//QPainter paint( this );
		QRect r;

		r = event->rect().intersected(contentsRect());
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
#ifndef QT5
	if (_cached)
	{
		XSetWindowBackgroundPixmap(QX11Info::display(), winId(), _background);
		XFlush(QX11Info::display());
		refreshBackground();
	}
#endif
}

void MyDrawingArea::refreshBackground()
{
	if (_cached)
	{
#ifndef QT5
		int fw = frameWidth();
		XClearArea(QX11Info::display(), winId(), fw, fw, width() - fw * 2, height() - fw * 2, False);
		XFlush(QX11Info::display());
#else
		update();
#endif
	}
}


void MyDrawingArea::clearBackground()
{
	if (drawn)
	{
		GB.Error("DrawingArea is being painted");
		return;
	}

	if (_cached)
	{
		createBackground(width(), height());
	}
	else
	{
#ifndef QT5
		XClearArea(QX11Info::display(), winId(), 0, 0, 0, 0, True);
		XFlush(QX11Info::display());
#else
 		update();
#endif
 	}
}

void MyDrawingArea::resizeEvent(QResizeEvent *e)
{
	MyContainer::resizeEvent(e);
	updateBackground();
}

void MyDrawingArea::updateBackground()
{
	int w, h;
#ifndef QT5
	int wb, hb;
#endif

	if (_cached)
	{
		if (drawn)
		{
			GB.Error("DrawingArea is being drawn");
			return;
		}

		w = qMax(width(), 1);
		h = qMax(height(), 1);

		if (w != _background_w || h != _background_h)
		{
#ifndef QT5
			Qt::HANDLE old = _background;

			wb = qMin(w, _background_w);
			hb = qMin(h, _background_h);

			_background = 0;
			createBackground(w, h);

			GC gc = XCreateGC(QX11Info::display(), old, 0, 0);
			XCopyArea(QX11Info::display(), old, _background, gc, 0, 0, wb, hb, 0, 0);
			XFreeGC(QX11Info::display(), gc);

			XFreePixmap(QX11Info::display(), old);
#else
			QPixmap old = _background_pixmap;
			createBackground(w, h);
			QPainter p(&_background_pixmap);
			p.drawPixmap(0, 0, old);
			p.end();
#endif

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
#ifndef QT5
		setAttribute(Qt::WA_DontCreateNativeAncestors, true);
		setAttribute(Qt::WA_NativeWindow, true);
		setAttribute(Qt::WA_PaintOnScreen, true);
#endif
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
		//repaint();
		update();
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
	update();
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

void MyDrawingArea::changeEvent(QEvent *e)
{
	if (e->type() == QEvent::FontChange)
	{
		MyContainer::changeEvent(e);
		void *_object = CWidget::get(this);
		GB.Raise(THIS, EVENT_Font, 0);
	}
}

/***************************************************************************

	DrawingArea

***************************************************************************/

BEGIN_METHOD(DrawingArea_new, GB_OBJECT parent)

	MyDrawingArea *wid = new MyDrawingArea(QCONTAINER(VARG(parent)));

	//THIS->widget.background = QColorGroup::Base;
	THIS->container = wid;
	THIS->widget.flag.noBackground = true;

	CWIDGET_new(wid, (void *)_object);

END_METHOD


BEGIN_PROPERTY(DrawingArea_Cached)

	if (READ_PROPERTY)
		GB.ReturnBoolean(WIDGET->isCached());
	else
	{
		GB_COLOR bg = CWIDGET_get_background((CWIDGET *)THIS);
		GB_COLOR fg = CWIDGET_get_foreground((CWIDGET *)THIS);

		if (bg == COLOR_DEFAULT)
		{
			CWIDGET_set_color((CWIDGET *)THIS, WIDGET->palette().color(WIDGET->backgroundRole()).rgb() & 0xFFFFFF, fg);
			WIDGET->clearBackground();
		}
		WIDGET->setCached(VPROP(GB_BOOLEAN));
	}

END_PROPERTY


DECLARE_METHOD(Control_Background);


BEGIN_METHOD_VOID(DrawingArea_Clear)

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


BEGIN_PROPERTY(DrawingArea_Border)

	Container_Border(_object, _param);

	if (!READ_PROPERTY)
	{
		WIDGET->clearBackground();
	}

END_PROPERTY

BEGIN_PROPERTY(DrawingArea_Enabled)

	Control_Enabled(_object, _param);

	if (!READ_PROPERTY)
		WIDGET->setFrozen(!VPROP(GB_BOOLEAN));

END_PROPERTY

BEGIN_PROPERTY(DrawingArea_Focus)

	if (READ_PROPERTY)
		GB.ReturnBoolean(CWIDGET_get_allow_focus(THIS));
	else
		CWIDGET_set_allow_focus(THIS, VPROP(GB_BOOLEAN));

END_PROPERTY

BEGIN_PROPERTY(DrawingArea_Painted)

	static bool deprecated = false;

	if (!deprecated)
	{
		deprecated = true;
		GB.Deprecated(QT_NAME, "DrawingArea.Painted", NULL);
	}

	if (READ_PROPERTY)
		GB.ReturnBoolean(true);

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

BEGIN_PROPERTY(DrawingArea_Tablet)

	if (READ_PROPERTY)
		GB.ReturnBoolean(THIS->widget.flag.use_tablet);
	else
		THIS->widget.flag.use_tablet = VPROP(GB_BOOLEAN);

END_PROPERTY


GB_DESC CDrawingAreaDesc[] =
{
	GB_DECLARE("DrawingArea", sizeof(CDRAWINGAREA)), GB_INHERITS("Container"),

	GB_METHOD("_new", NULL, DrawingArea_new, "(Parent)Container;"),

	GB_PROPERTY("Cached", "b", DrawingArea_Cached),

	GB_PROPERTY("Arrangement", "i", Container_Arrangement),
	GB_PROPERTY("AutoResize", "b", Container_AutoResize),
	GB_PROPERTY("Spacing", "b", Container_Spacing),
	GB_PROPERTY("Margin", "b", Container_Margin),
	GB_PROPERTY("Padding", "i", Container_Padding),
	GB_PROPERTY("Indent", "b", Container_Indent),
  GB_PROPERTY("Invert", "b", Container_Invert),

	GB_PROPERTY("Border", "i", DrawingArea_Border),
	GB_PROPERTY("NoBackground", "b", DrawingArea_NoBackground),
	GB_PROPERTY("Background", "i", DrawingArea_Background),

	GB_PROPERTY("Focus", "b", DrawingArea_Focus),
	GB_PROPERTY("Enabled", "b", DrawingArea_Enabled),
	GB_PROPERTY("Painted", "b", DrawingArea_Painted),
	GB_PROPERTY("Tablet", "b", DrawingArea_Tablet),

	GB_METHOD("Clear", NULL, DrawingArea_Clear, NULL),
	GB_METHOD("Refresh", NULL, DrawingArea_Refresh, "[(X)i(Y)i(Width)i(Height)i]"),

	GB_EVENT("Draw", NULL, NULL, &EVENT_Draw),
	GB_EVENT("Font", NULL, NULL, &EVENT_Font),
	GB_EVENT("Change", NULL, NULL, &EVENT_Change),

	//GB_INTERFACE("Draw", &DRAW_Interface),
	GB_INTERFACE("Paint", &PAINT_Interface),

	DRAWINGAREA_DESCRIPTION,

	GB_END_DECLARE
};
