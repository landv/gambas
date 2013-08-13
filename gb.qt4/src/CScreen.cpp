/***************************************************************************

  CScreen.cpp

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

#define __CSCREEN_CPP

#include <QApplication>
#include <QDesktopWidget>
#include <QToolTip>

#include "gambas.h"
#include "main.h"
#include "gb.draw.h"
#include "cpaint_impl.h"
#include "CPicture.h"
#include "CWidget.h"
#include "CWindow.h"
#include "CFont.h"
#include "CScreen.h"

#ifndef NO_X_WINDOW
#include <QX11Info>
#include "x11.h"
#endif

#if QT_VERSION >= 0x040600
#define NUM_SCREENS() (QApplication::desktop()->screenCount())
#else
#define NUM_SCREENS() (QApplication::desktop()->numScreens())
#endif

#define MAX_SCREEN 16

static int screen_busy = 0;
char *CAPPLICATION_Theme = 0;
static CSCREEN *_screens[MAX_SCREEN] = { NULL };

static CSCREEN *get_screen(int num)
{
	if (num < 0 || num >= MAX_SCREEN || num >= NUM_SCREENS())
		return NULL;
	
	if (!_screens[num])
	{
		_screens[num] = (CSCREEN *)GB.New(GB.FindClass("Screen"), NULL, 0);
		_screens[num]->index = num;
		GB.Ref(_screens[num]);
	}
	
	return _screens[num];
}

static void free_screens(void)
{
	int i;
	
	for (i = 0; i < MAX_SCREEN; i++)
	{
		if (_screens[i])
			GB.Unref(POINTER(&_screens[i]));
	}
}


BEGIN_METHOD_VOID(Application_exit)

	GB.FreeString(&CAPPLICATION_Theme);
	free_screens();

END_METHOD


BEGIN_PROPERTY(Desktop_X)

	GB.ReturnInteger(QApplication::desktop()->availableGeometry().x());

END_PROPERTY

BEGIN_PROPERTY(Desktop_Y)

	GB.ReturnInteger(QApplication::desktop()->availableGeometry().y());

END_PROPERTY

BEGIN_PROPERTY(Desktop_Width)

	GB.ReturnInteger(QApplication::desktop()->availableGeometry().width());

END_PROPERTY

BEGIN_PROPERTY(Desktop_Height)

	GB.ReturnInteger(QApplication::desktop()->availableGeometry().height());

END_PROPERTY


BEGIN_PROPERTY(Desktop_Resolution)

	#ifdef NO_X_WINDOW
		GB.ReturnInteger(72);
	#else
		GB.ReturnInteger(QX11Info::appDpiY());
	#endif

END_PROPERTY

BEGIN_PROPERTY(Desktop_HasSystemTray)

	#ifdef NO_X_WINDOW
		GB.Return(FALSE);
	#else
		GB.ReturnBoolean(X11_get_system_tray() != None);
	#endif

END_PROPERTY

static void set_font(QFont &font, void *object = 0)
{
	qApp->setFont(font);
	MAIN_update_scale();
}

BEGIN_PROPERTY(Application_Font)

	if (READ_PROPERTY)
		GB.ReturnObject(CFONT_create(qApp->font(), set_font));
	else
		CFONT_set(set_font, (CFONT *)VPROP(GB_OBJECT), NULL);

END_PROPERTY


BEGIN_PROPERTY(Application_ActiveWindow)

	//GB.ReturnObject(CWidget::get(qApp->activeWindow()));
	GB.ReturnObject(CWINDOW_Active);

END_PROPERTY


BEGIN_PROPERTY(Application_ActiveControl)

	GB.ReturnObject(CWIDGET_active_control);

END_PROPERTY


BEGIN_PROPERTY(Application_Busy)

	int busy;

	if (READ_PROPERTY)
		GB.ReturnInteger(screen_busy);
	else
	{
		busy = VPROP(GB_INTEGER);

		if (screen_busy == 0 && busy > 0)
			qApp->setOverrideCursor(Qt::WaitCursor);
		else if (screen_busy > 0 && busy == 0)
			qApp->restoreOverrideCursor();

		screen_busy = busy;
		if (MAIN_debug_busy)
			qDebug("%s: Application.Busy = %d", GB.Debug.GetCurrentPosition(), busy);
	}

END_PROPERTY


BEGIN_METHOD(Desktop_Screenshot, GB_INTEGER x; GB_INTEGER y; GB_INTEGER w; GB_INTEGER h)

	GB.ReturnObject(CPICTURE_grab(0, VARGOPT(x, 0), VARGOPT(y, 0), VARGOPT(w, 0), VARGOPT(h, 0)));

END_METHOD


BEGIN_PROPERTY(Application_ShowTooltips)

	if (READ_PROPERTY)
		GB.ReturnBoolean(MyApplication::isTooltipEnabled());
	else
		MyApplication::setTooltipEnabled(VPROP(GB_BOOLEAN));

END_PROPERTY


BEGIN_PROPERTY(Application_MainWindow)

	if (READ_PROPERTY)
		GB.ReturnObject(CWINDOW_Main);
	else
		CWINDOW_Main = (CWINDOW *)VPROP(GB_OBJECT);

END_PROPERTY


BEGIN_PROPERTY(Application_Embedder)

	if (READ_PROPERTY)
		GB.ReturnInteger(CWINDOW_Embedder);
	else
	{
		if (CWINDOW_Embedded)
		{
			GB.Error("Application is already embedded");
			return;
		}

		CWINDOW_Embedder = VPROP(GB_INTEGER);
	}

END_PROPERTY


BEGIN_PROPERTY(Application_Theme)

	if (READ_PROPERTY)
		GB.ReturnString(CAPPLICATION_Theme);
	else
		GB.StoreString(PROP(GB_STRING), &CAPPLICATION_Theme);

END_PROPERTY


BEGIN_PROPERTY(Desktop_Scale)

	GB.ReturnInteger(MAIN_scale);

END_PROPERTY



BEGIN_PROPERTY(Screens_Count)

	GB.ReturnInteger(NUM_SCREENS());

END_PROPERTY


BEGIN_METHOD(Screens_get, GB_INTEGER screen)

	GB.ReturnObject(get_screen(VARG(screen)));

END_METHOD


BEGIN_METHOD_VOID(Screens_next)

	int *index = (int *)GB.GetEnum();

	if (*index >= NUM_SCREENS())
		GB.StopEnum();
	else
	{
		GB.ReturnObject(get_screen(*index));
		(*index)++;
	}
	
END_METHOD


BEGIN_PROPERTY(Screen_X)

	GB.ReturnInteger(QApplication::desktop()->screenGeometry(SCREEN->index).x());

END_PROPERTY

BEGIN_PROPERTY(Screen_Y)

	GB.ReturnInteger(QApplication::desktop()->screenGeometry(SCREEN->index).y());

END_PROPERTY

BEGIN_PROPERTY(Screen_Width)

	GB.ReturnInteger(QApplication::desktop()->screenGeometry(SCREEN->index).width());

END_PROPERTY

BEGIN_PROPERTY(Screen_Height)

	GB.ReturnInteger(QApplication::desktop()->screenGeometry(SCREEN->index).height());

END_PROPERTY


BEGIN_PROPERTY(Screen_AvailableX)

	GB.ReturnInteger(QApplication::desktop()->availableGeometry(SCREEN->index).x());

END_PROPERTY

BEGIN_PROPERTY(Screen_AvailableY)

	GB.ReturnInteger(QApplication::desktop()->availableGeometry(SCREEN->index).y());

END_PROPERTY

BEGIN_PROPERTY(Screen_AvailableWidth)

	GB.ReturnInteger(QApplication::desktop()->availableGeometry(SCREEN->index).width());

END_PROPERTY

BEGIN_PROPERTY(Screen_AvailableHeight)

	GB.ReturnInteger(QApplication::desktop()->availableGeometry(SCREEN->index).height());

END_PROPERTY

GB_DESC ScreenDesc[] =
{
	GB_DECLARE("Screen", sizeof(CSCREEN)), GB_NOT_CREATABLE(), GB_AUTO_CREATABLE(),

	GB_PROPERTY_READ("X", "i", Screen_X),
	GB_PROPERTY_READ("Y", "i", Screen_Y),
	GB_PROPERTY_READ("W", "i", Screen_Width),
	GB_PROPERTY_READ("H", "i", Screen_Height),
	GB_PROPERTY_READ("Width", "i", Screen_Width),
	GB_PROPERTY_READ("Height", "i", Screen_Height),

	GB_PROPERTY_READ("AvailableX", "i", Screen_AvailableX),
	GB_PROPERTY_READ("AvailableY", "i", Screen_AvailableY),
	GB_PROPERTY_READ("AvailableWidth", "i", Screen_AvailableWidth),
	GB_PROPERTY_READ("AvailableHeight", "i", Screen_AvailableHeight),

	GB_END_DECLARE
};

GB_DESC ScreensDesc[] =
{
	GB_DECLARE("Screens", 0), GB_VIRTUAL_CLASS(),

	GB_STATIC_PROPERTY_READ("Count", "i", Screens_Count),
	GB_STATIC_METHOD("_get", "Screen", Screens_get, "(Screen)i"),
	GB_STATIC_METHOD("_next", "Screen", Screens_next, NULL),
	
	GB_END_DECLARE
};

GB_DESC DesktopDesc[] =
{
	GB_DECLARE("Desktop", 0), GB_VIRTUAL_CLASS(),

	GB_STATIC_PROPERTY_READ("X", "i", Desktop_X),
	GB_STATIC_PROPERTY_READ("Y", "i", Desktop_Y),
	GB_STATIC_PROPERTY_READ("W", "i", Desktop_Width),
	GB_STATIC_PROPERTY_READ("H", "i", Desktop_Height),
	GB_STATIC_PROPERTY_READ("Width", "i", Desktop_Width),
	GB_STATIC_PROPERTY_READ("Height", "i", Desktop_Height),

	GB_CONSTANT("Charset", "s", "UTF-8"),
	GB_STATIC_PROPERTY_READ("Resolution", "i", Desktop_Resolution),
	GB_STATIC_PROPERTY_READ("Scale", "i", Desktop_Scale),

	GB_STATIC_PROPERTY_READ("HasSystemTray", "b", Desktop_HasSystemTray),
	
	GB_STATIC_METHOD("Screenshot", "Picture", Desktop_Screenshot, "[(X)i(Y)i(Width)i(Height)i]"),

	GB_END_DECLARE
};

GB_DESC ApplicationDesc[] =
{
	GB_DECLARE("Application", 0), GB_VIRTUAL_CLASS(),

	GB_STATIC_METHOD("_exit", NULL, Application_exit, NULL),
	GB_STATIC_PROPERTY("Font", "Font", Application_Font),
	GB_STATIC_PROPERTY_READ("ActiveWindow", "Window", Application_ActiveWindow),
	GB_STATIC_PROPERTY_READ("ActiveControl", "Control", Application_ActiveControl),
	GB_STATIC_PROPERTY("MainWindow", "Window", Application_MainWindow),
	GB_STATIC_PROPERTY("Busy", "i", Application_Busy),
	GB_STATIC_PROPERTY("ShowTooltips", "b", Application_ShowTooltips),
	GB_STATIC_PROPERTY("Embedder", "i", Application_Embedder),
	GB_STATIC_PROPERTY("Theme", "s", Application_Theme),

	GB_END_DECLARE
};

//-- Style ------------------------------------------------------------------

static void init_option(QStyleOption &opt, int x, int y, int w, int h, int state)
{
	opt.rect = QRect(x, y, w ,h);
	if (state & GB_DRAW_STATE_DISABLED)
		return;
	
	opt.state |= QStyle::State_Enabled;
	
	if (state & GB_DRAW_STATE_FOCUS)
		opt.state |= QStyle::State_HasFocus;
	if (state & GB_DRAW_STATE_HOVER)
		opt.state |= QStyle::State_MouseOver;
	if (state & GB_DRAW_STATE_ACTIVE)
		opt.state |= QStyle::State_On | QStyle::State_Sunken | QStyle::State_Active;
}

static void paint_focus(QPainter *p, int x, int y, int w, int h, int state)
{
	//bool do_clip = FALSE;
	QStyleOptionFocusRect opt;
	
	if ((state & GB_DRAW_STATE_DISABLED) || !(state & GB_DRAW_STATE_FOCUS))
		return;
	
	init_option(opt, x, y, w, h, state);
	
	/*if (::strcmp(qApp->style()->metaObject()->className(), "QtCurve::Style") == 0)
	{
		QPainterPath clip;

		p->save();

		clip.addRect(x, y, w, 1);
		clip.addRect(x, y, 1, h);
		clip.addRect(x, y + h - 1, w, 1);
		clip.addRect(x + w - 1, y, 1, h);
		p->setClipPath(clip);
		do_clip = TRUE;
	}*/

	p->save();
	p->setBrush(QBrush());

	QApplication::style()->drawPrimitive(QStyle::PE_FrameFocusRect, &opt, p);

	p->restore();

	/*if (do_clip)
		p->restore();*/
}

static void style_arrow(QPainter *p, int x, int y, int w, int h, int type, int state)
{
	QStyleOption opt;
	QStyle::PrimitiveElement pe;
	
	init_option(opt, x, y, w, h, state);
	
	switch (type)
	{
		case ALIGN_NORMAL: pe = GB.System.IsRightToLeft() ? QStyle::PE_IndicatorArrowLeft : QStyle::PE_IndicatorArrowRight; break;
		case ALIGN_LEFT: pe = QStyle::PE_IndicatorArrowLeft; break;
		case ALIGN_RIGHT: pe = QStyle::PE_IndicatorArrowRight; break;
		case ALIGN_TOP: pe = QStyle::PE_IndicatorArrowUp; break;
		case ALIGN_BOTTOM: pe = QStyle::PE_IndicatorArrowDown; break;
		default:
			return;
	}

	QApplication::style()->drawPrimitive(pe, &opt, p);
}

static void style_check(QPainter *p, int x, int y, int w, int h, int value, int state)
{
	QStyleOptionButton opt;
	init_option(opt, x, y, w, h, state);
	
	if (value)
	{
		if (value == 1)
			opt.state |= QStyle::State_NoChange;
		else
			opt.state |= QStyle::State_On;
	}
	
	QApplication::style()->drawPrimitive(QStyle::PE_IndicatorCheckBox, &opt, p);
	paint_focus(p, x, y, w, h, state);
}

static void style_option(QPainter *p, int x, int y, int w, int h, int value, int state)
{
	QStyleOptionButton opt;
	init_option(opt, x, y, w, h, state);
	
	if (value)
		opt.state |= QStyle::State_On;
	
	QApplication::style()->drawPrimitive(QStyle::PE_IndicatorRadioButton, &opt, p);
	paint_focus(p, x, y, w, h, state);
}

static void style_separator(QPainter *p, int x, int y, int w, int h, int vertical, int state)
{
	QStyleOption opt;
	init_option(opt, x, y, w, h, state);
	
	if (vertical)
		opt.state |= QStyle::State_Horizontal;
	
	QApplication::style()->drawPrimitive(QStyle::PE_IndicatorToolBarSeparator, &opt, p);
}


static void style_button(QPainter *p, int x, int y, int w, int h, int value, int state, int flat)
{
	if (flat)
	{
		QStyleOptionToolButton opt;
		
		init_option(opt, x, y, w, h, state);
		
		//opt.state |= QStyle::State_Raised;
		
		if (value)
			opt.state |= QStyle::State_On;

		//opt.state &= ~QStyle::State_HasFocus;
		opt.state |= QStyle::State_AutoRaise;
		if (opt.state & QStyle::State_MouseOver)
			opt.state |= QStyle::State_Raised;
		
		if (opt.state & (QStyle::State_Sunken | QStyle::State_On | QStyle::State_MouseOver))
		{
			QApplication::style()->drawPrimitive(QStyle::PE_PanelButtonTool, &opt, p);
		}
	}
	else
	{
		QStyleOptionButton opt;
	
		init_option(opt, x, y, w, h, state);
		
		opt.state |= QStyle::State_Raised;
		
		if (value)
			opt.state |= QStyle::State_On;
	
		QApplication::style()->drawPrimitive(QStyle::PE_PanelButtonCommand, &opt, p);
	}
	
	paint_focus(p, x, y, w, h, state);
}
			
static void style_panel(QPainter *p, int x, int y, int w, int h, int border, int state)
{
	QStyleOptionFrame opt;
	init_option(opt, x, y, w, h, state);
	
	CCONTAINER_draw_frame(p, border, opt);
	
	/*opt.lineWidth = 2;
	opt.midLineWidth = 0;	
	
	if (border == BORDER_NONE)
		return;
		
	if (border == BORDER_PLAIN)
	{
		DP(d)->save();
		DP(d)->setPen(state == GB_DRAW_STATE_DISABLED ? QApplication::palette().color(QPalette::Active, QPalette::WindowText) : QApplication::palette().color(QPalette::Disabled, QPalette::WindowText));
		DP(d)->drawRect(x, y, w - 1, h - 1);
		DP(d)->restore();
		if DPM(d) 
		{	
			DPM(d)->save();
			DPM(d)->setPen(Qt::color1);
			DPM(d)->drawRect(x, y, w - 1, h - 1);
			DPM(d)->restore();
		}
		return;
	}
	
	if (border == BORDER_ETCHED)
	{
		pe = QStyle::PE_FrameGroupBox;
	}
	else
	{
		if (border == BORDER_RAISED)
			opt.state |= QStyle::State_Raised;
		else if (border == BORDER_SUNKEN)
			opt.state |= QStyle::State_Sunken;
		
		pe = QStyle::PE_Frame;
	}
	
	QApplication::style()->drawPrimitive(pe, &opt, DP(d));
	if (DPM(d)) 
	{
		//DPM(d)->setRasterOp(Qt::OrROP);
		QApplication::style()->drawPrimitive(pe, &opt, DPM(d));
		//DPM(d)->setRasterOp(Qt::CopyROP);
	}*/
}
			
static void style_handle(QPainter *p, int x, int y, int w, int h, int vertical, int state)
{
	QStyleOption opt;
	init_option(opt, x, y, w, h, state);
	
	if (!vertical)
		opt.state |= QStyle::State_Horizontal;
	
	QApplication::style()->drawPrimitive(QStyle::PE_IndicatorDockWidgetResizeHandle, &opt, p);
	paint_focus(p, x, y, w, h, state);
}


static void style_box(QPainter *p, int x, int y, int w, int h, int state)
{
	QStyleOptionFrame opt;
	
	//if (GB.Is(d->device, CLASS_DrawingArea))
	//	opt.initFrom(QWIDGET(d->device));
	
	init_option(opt, x, y, w, h, state);

	opt.lineWidth = QApplication::style()->pixelMetric(QStyle::PM_DefaultFrameWidth, &opt);
	opt.midLineWidth = 0;
	opt.state |= QStyle::State_Sunken;
	//opt.features = QStyleOptionFrameV2::None;
	
  QApplication::style()->drawPrimitive(QStyle::PE_PanelLineEdit, &opt, p);
	
	//paint_focus(d, x, y, w, h, state);
	//if (state & GB_DRAW_STATE_FOCUS)
	//	QApplication::style()->drawControl(QStyle::CE_FocusFrame, &opt, DP(d), GB.Is(d->device, CLASS_DrawingArea) ? QWIDGET(d->device) : NULL);
}

BEGIN_PROPERTY(Style_ScrollbarSize)

	GB.ReturnInteger(qApp->style()->pixelMetric(QStyle::PM_ScrollBarExtent));

END_PROPERTY

BEGIN_PROPERTY(Style_ScrollbarSpacing)

	GB.ReturnInteger(QMAX(0, qApp->style()->pixelMetric(QStyle::PM_ScrollView_ScrollBarSpacing)));

END_PROPERTY

BEGIN_PROPERTY(Style_FrameWidth)

	GB.ReturnInteger(qApp->style()->pixelMetric(QStyle::QStyle::PM_ComboBoxFrameWidth));

END_PROPERTY

BEGIN_PROPERTY(Style_BoxFrameWidth)

	int w = qApp->style()->pixelMetric(QStyle::QStyle::PM_ComboBoxFrameWidth);
	
	if (::strcmp(qApp->style()->metaObject()->className(), "Oxygen::Style") == 0)
		w++;
	
	GB.ReturnInteger(w);

END_PROPERTY

BEGIN_PROPERTY(Style_Name)

	const char *name = qApp->style()->metaObject()->className();
	int len = strlen(name);
	
	if (len >= 6 && strncasecmp(&name[len - 5], "style", 5) == 0)
		len -= 5;
	if (len >= 3 && strncmp(&name[len - 2], "::", 2) == 0)
		len -= 2;

	GB.ReturnNewString(name, len);

END_PROPERTY

#define GET_COORD() \
	int x, y, w, h; \
\
	QPainter *p = PAINT_get_current(); \
	if (!p) \
		return; \
\
	x = VARG(x); \
	y = VARG(y); \
	w = VARG(w); \
	h = VARG(h); \
\
	if (w < 1 || h < 1) \
		return;

BEGIN_METHOD(Style_PaintArrow, GB_INTEGER x; GB_INTEGER y; GB_INTEGER w; GB_INTEGER h; GB_INTEGER type; GB_INTEGER state)

	GET_COORD();
	style_arrow(p, x, y, w, h, VARG(type), VARGOPT(state, GB_DRAW_STATE_NORMAL));

END_METHOD

BEGIN_METHOD(Style_PaintCheck, GB_INTEGER x; GB_INTEGER y; GB_INTEGER w; GB_INTEGER h; GB_INTEGER value; GB_INTEGER state)

	GET_COORD();
	style_check(p, x, y, w, h, VARG(value), VARGOPT(state, GB_DRAW_STATE_NORMAL));

END_METHOD

BEGIN_METHOD(Style_PaintOption, GB_INTEGER x; GB_INTEGER y; GB_INTEGER w; GB_INTEGER h; GB_BOOLEAN value; GB_INTEGER state)

	GET_COORD();
	style_option(p, x, y, w, h, VARG(value), VARGOPT(state, GB_DRAW_STATE_NORMAL));

END_METHOD

BEGIN_METHOD(Style_PaintSeparator, GB_INTEGER x; GB_INTEGER y; GB_INTEGER w; GB_INTEGER h; GB_BOOLEAN vertical; GB_INTEGER state)

	GET_COORD();
	style_separator(p, x, y, w, h, VARGOPT(vertical, FALSE), VARGOPT(state, GB_DRAW_STATE_NORMAL));

END_METHOD

BEGIN_METHOD(Style_PaintButton, GB_INTEGER x; GB_INTEGER y; GB_INTEGER w; GB_INTEGER h; GB_BOOLEAN value; GB_INTEGER state; GB_BOOLEAN flat)

	GET_COORD();
	style_button(p, x, y, w, h, VARG(value), VARGOPT(state, GB_DRAW_STATE_NORMAL), VARGOPT(flat, FALSE));

END_METHOD

BEGIN_METHOD(Style_PaintPanel, GB_INTEGER x; GB_INTEGER y; GB_INTEGER w; GB_INTEGER h; GB_INTEGER border; GB_INTEGER state)

	GET_COORD();
	style_panel(p, x, y, w, h, VARG(border), VARGOPT(state, GB_DRAW_STATE_NORMAL));

END_METHOD

BEGIN_METHOD(Style_PaintHandle, GB_INTEGER x; GB_INTEGER y; GB_INTEGER w; GB_INTEGER h; GB_BOOLEAN vertical; GB_INTEGER state)

	GET_COORD();
	style_handle(p, x, y, w, h, VARGOPT(vertical, FALSE), VARGOPT(state, GB_DRAW_STATE_NORMAL));

END_METHOD

BEGIN_METHOD(Style_PaintBox, GB_INTEGER x; GB_INTEGER y; GB_INTEGER w; GB_INTEGER h; GB_INTEGER state)

	GET_COORD();
	style_box(p, x, y, w, h, VARGOPT(state, GB_DRAW_STATE_NORMAL));

END_METHOD


GB_DESC StyleDesc[] =
{
	GB_DECLARE("Style", 0), GB_VIRTUAL_CLASS(),
	
	GB_STATIC_PROPERTY_READ("ScrollbarSize", "i", Style_ScrollbarSize),
	GB_STATIC_PROPERTY_READ("ScrollbarSpacing", "i", Style_ScrollbarSpacing),
	GB_STATIC_PROPERTY_READ("FrameWidth", "i", Style_FrameWidth),
	GB_STATIC_PROPERTY_READ("TextBoxFrameWidth", "i", Style_FrameWidth),
	GB_STATIC_PROPERTY_READ("BoxFrameWidth", "i", Style_BoxFrameWidth),
	GB_STATIC_PROPERTY_READ("BoxFrameHeight", "i", Style_BoxFrameWidth),
	GB_STATIC_PROPERTY_READ("Name", "s", Style_Name),
	
	GB_STATIC_METHOD("PaintArrow", NULL, Style_PaintArrow, "(X)i(Y)i(Width)i(Height)i(Type)i[(Flag)i]"),
	GB_STATIC_METHOD("PaintCheck", NULL, Style_PaintCheck, "(X)i(Y)i(Width)i(Height)i(Value)i[(Flag)i]"),
	GB_STATIC_METHOD("PaintOption", NULL, Style_PaintOption, "(X)i(Y)i(Width)i(Height)i(Value)b[(Flag)i]"),
	GB_STATIC_METHOD("PaintSeparator", NULL, Style_PaintSeparator, "(X)i(Y)i(Width)i(Height)i[(Vertical)b(Flag)i]"),
	GB_STATIC_METHOD("PaintButton", NULL, Style_PaintButton, "(X)i(Y)i(Width)i(Height)i(Value)b[(Flag)i(Flat)b]"),
	GB_STATIC_METHOD("PaintPanel", NULL, Style_PaintPanel, "(X)i(Y)i(Width)i(Height)i(Border)i[(Flag)i]"),
	GB_STATIC_METHOD("PaintHandle", NULL, Style_PaintHandle, "(X)i(Y)i(Width)i(Height)i[(Vertical)b(Flag)i]"),
	GB_STATIC_METHOD("PaintBox", NULL, Style_PaintBox, "(X)i(Y)i(Width)i(Height)i[(Flag)i]"),
	
	GB_END_DECLARE
};

