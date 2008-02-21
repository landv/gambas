/***************************************************************************

  CTrayIcon.cpp

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

#define __CTRAYICON_CPP

#include <qnamespace.h>
#include <qapplication.h>
#include <qtooltip.h>
#include <qpixmap.h>
#include <qbitmap.h>
#include <qptrlist.h>
#include <qevent.h>

#if QT_VERSION >= 0x030100
  #include <qeventloop.h>
#endif

#include "qtxembed.h"

#include "gambas.h"
#include "main.h"
#include "x11.h"

#include "CMouse.h"
#define DO_NOT_DECLARE_EVENTS
#include "CWidget.h"
#include "CTrayIcon.h"

DECLARE_METHOD(CCONTROL_screen_x);
DECLARE_METHOD(CCONTROL_screen_y);
DECLARE_METHOD(CCONTROL_w);
DECLARE_METHOD(CCONTROL_h);

DECLARE_EVENT(EVENT_MouseDown);
DECLARE_EVENT(EVENT_MouseUp);
DECLARE_EVENT(EVENT_MouseMove);
DECLARE_EVENT(EVENT_MouseWheel);
DECLARE_EVENT(EVENT_DblClick);
DECLARE_EVENT(EVENT_Enter);
DECLARE_EVENT(EVENT_Leave);
DECLARE_EVENT(EVENT_GotFocus);
DECLARE_EVENT(EVENT_LostFocus);
DECLARE_EVENT(EVENT_Menu);

static int _state;
static QList<CTRAYICON> _list;

static const char * _default_trayicon[] = {
"24 24 136 2",
"  	c None",
". 	c #4167C6",
"+ 	c #446ECF",
"@ 	c #4975D8",
"# 	c #4E7DDF",
"$ 	c #5586E7",
"% 	c #314F9E",
"& 	c #2D4A91",
"* 	c #3153A9",
"= 	c #304F9B",
"- 	c #365AB7",
"; 	c #3C64C5",
"> 	c #446FD3",
", 	c #4D7DE1",
"' 	c #5A8EED",
") 	c #2D4371",
"! 	c #46628B",
"~ 	c #688CB7",
"{ 	c #5D7FAA",
"] 	c #668AB6",
"^ 	c #3B5CA7",
"/ 	c #3556A2",
"( 	c #365BB8",
"_ 	c #3F67C9",
": 	c #4977DC",
"< 	c #568AEB",
"[ 	c #33496F",
"} 	c #749CC7",
"| 	c #80AAD7",
"1 	c #81ADD9",
"2 	c #82AED9",
"3 	c #81ACD8",
"4 	c #7AA4D0",
"5 	c #729BCA",
"6 	c #3457AE",
"7 	c #476AB0",
"8 	c #577CB9",
"9 	c #4870C1",
"0 	c #5486E6",
"a 	c #7BA4D0",
"b 	c #7DA8D5",
"c 	c #7AA5D3",
"d 	c #80ABD8",
"e 	c #82AEDA",
"f 	c #83AFDA",
"g 	c #7EA8D6",
"h 	c #668CC1",
"i 	c #7EA9D5",
"j 	c #81ADD8",
"k 	c #8CB5DC",
"l 	c #9FBDDC",
"m 	c #BBD7ED",
"n 	c #9AAEC3",
"o 	c #79A2D1",
"p 	c #77A1D1",
"q 	c #5278BB",
"r 	c #3D5DA1",
"s 	c #6F97C8",
"t 	c #587DBA",
"u 	c #7EA9D6",
"v 	c #82ADDA",
"w 	c #81ACD9",
"x 	c #7FAAD6",
"y 	c #6F91B9",
"z 	c #6A85A0",
"A 	c #9DC4E5",
"B 	c #849AA4",
"C 	c #1E1F20",
"D 	c #6189C5",
"E 	c #335195",
"F 	c #395BA9",
"G 	c #4667A2",
"H 	c #4E72B6",
"I 	c #2E4E9F",
"J 	c #729ACA",
"K 	c #7FAAD7",
"L 	c #83AFDB",
"M 	c #7CA6D3",
"N 	c #202B38",
"O 	c #020202",
"P 	c #495B6D",
"Q 	c #C4E6FB",
"R 	c #718186",
"S 	c #4A6EB5",
"T 	c #3E5E9E",
"U 	c #587DBC",
"V 	c #78A1CF",
"W 	c #466AB6",
"X 	c #5F84BB",
"Y 	c #78A0CE",
"Z 	c #7FA9D6",
"` 	c #24313F",
" .	c #324256",
"..	c #97C1E6",
"+.	c #C7E9FC",
"@.	c #2F4E98",
"#.	c #719ACA",
"$.	c #6990C2",
"%.	c #81ABD6",
"&.	c #78A0CA",
"*.	c #5E7DA0",
"=.	c #77A0CB",
"-.	c #80ABD7",
";.	c #93BEE4",
">.	c #C4E5FA",
",.	c #4B6DA9",
"'.	c #6F97C5",
").	c #84AFDA",
"!.	c #82ADD9",
"~.	c #8AB5DE",
"{.	c #B0D6F1",
"].	c #719ACC",
"^.	c #7197C3",
"/.	c #85B0DB",
"(.	c #84B0DB",
"_.	c #91BCE2",
":.	c #638DD3",
"<.	c #799DDB",
"[.	c #4A6EC2",
"}.	c #718AB8",
"|.	c #8DA6CA",
"1.	c #92ACD1",
"2.	c #C4D9EE",
"3.	c #AEC6E7",
"4.	c #38539B",
"5.	c #405A99",
"6.	c #7D95BC",
"7.	c #9EB6D7",
"8.	c #C3D8EE",
"9.	c #5771AD",
"0.	c #25418F",
"a.	c #4863A1",
"b.	c #C5D9EF",
"c.	c #6384CA",
"d.	c #3D65C5",
"e.	c #395AB0",
"                                                ",
"                                                ",
"            . + @ # $                           ",
"        % & * = - ; > , '                       ",
"      ) ! ~ { ] ^ / ( _ : <                     ",
"    [ } | 1 2 3 4 5 6 7 8 9 0                   ",
"    a b c d 1 e f g h i j k l m n               ",
"  o p q r s t u v w f v x y z A B C             ",
"  D E F G H I J K f L e M N O P Q R             ",
"  S T U V W X Y u f L f Z ` O  ...+.            ",
"  @.#.$.        %.e L L e &.*.=.-.;.>.          ",
"  ,.'.              ).L L f e e e !.~.{.        ",
"  ].^.                  /.(.L L f f f f _.      ",
"  b                                             ",
"  -.                                            ",
"                                                ",
"    :.                                          ",
"    <.[.}.|.1.                                  ",
"    2.3.4.5.6.7.                                ",
"      8.9.0.a.                                  ",
"      b.c.d.e.                                  ",
"                                                ",
"                                                ",
"                                                "};


static CTRAYICON *find_object(const QObject *o)
{
  QListIterator<CTRAYICON> it(_list);
  CTRAYICON *_object;

  while ((_object = it.current()))
  {
    ++it;
    if (THIS->widget == o)
      return THIS;
  }

  return NULL;
}

static void destroy_widget(CTRAYICON *_object)
{
  if (WIDGET)
  {
    delete WIDGET;
    THIS->widget = NULL;
  }
}

void CTRAYICON_close_all(void)
{
  //qDebug("CTRAYICON_close_all");

  QListIterator<CTRAYICON> it(_list);
  CTRAYICON *_object;

  while ((_object = it.current()))
  {
    ++it;
    destroy_widget(THIS);
    GB.Unref(POINTER(&_object));
  }

  GB.StopAllEnum(GB.FindClass("TrayIcons"));
}


BEGIN_METHOD_VOID(CTRAYICON_new)

  THIS->tag.type = GB_T_NULL;
  _list.append((const CTRAYICON *)THIS);
  GB.Ref(THIS);

END_METHOD


BEGIN_METHOD_VOID(CTRAYICON_free)

  //qDebug("CTRAYICON_free");

  _list.removeRef((const CTRAYICON *)THIS);

  GB.StoreObject(NULL, POINTER(&THIS->icon));
  GB.FreeString(&THIS->tooltip);
  GB.StoreVariant(NULL, &THIS->tag);

  destroy_widget(THIS);

END_METHOD


static void define_mask(CTRAYICON *_object)
{
  QPixmap *p;

  if (!WIDGET)
    return;

  if (THIS->icon)
    p = THIS->icon->pixmap;
  else
    p = new QPixmap(_default_trayicon);

  WIDGET->clearMask();
  if (p->hasAlpha())
    WIDGET->setMask(*(p->mask()));

  WIDGET->setErasePixmap(*p);
  WIDGET->resize(p->width(), p->height());

  if (!THIS->icon)
    delete p;
}


static void define_tooltip(CTRAYICON *_object)
{
  char *tooltip;

  if (!WIDGET)
    return;

  tooltip = THIS->tooltip;
  if (tooltip)
    QToolTip::add(WIDGET, TO_QSTRING(tooltip));
  else
    QToolTip::remove(WIDGET);
}


BEGIN_PROPERTY(CTRAYICON_picture)

  if (READ_PROPERTY)
  {
    GB.ReturnObject(THIS->icon);
    return;
  }

  GB.StoreObject(PROP(GB_OBJECT), POINTER(&THIS->icon));
  define_mask(THIS);

END_PROPERTY


BEGIN_PROPERTY(CTRAYICON_tooltip)

  if (READ_PROPERTY)
    GB.ReturnString(THIS->tooltip);
  else
  {
    GB.StoreString(PROP(GB_STRING), &(THIS->tooltip));
    define_tooltip(THIS);
  }

END_PROPERTY


BEGIN_METHOD_VOID(CTRAYICON_show)

	int i;

  if (!WIDGET)
  {
    QtXEmbedClient *wid = new QtXEmbedClient(true);

    THIS->widget = wid;
    wid->installEventFilter(&CTrayIcon::manager);

    QObject::connect(WIDGET, SIGNAL(embedded()), &CTrayIcon::manager, SLOT(embedded()));
    //QObject::connect(WIDGET, SIGNAL(containerClosed()), &CTrayIcon::manager, SLOT(closed()));
    QObject::connect(WIDGET, SIGNAL(error(int)), &CTrayIcon::manager, SLOT(error()));

    //qDebug("XEMBED: EmbedState: %d", CWINDOW_EmbedState);
    X11_window_dock(WIDGET->winId());

    _state = EMBED_WAIT;
    for(i = 0; i < 500; i++)
    {
      qApp->eventLoop()->processEvents(QEventLoop::ExcludeUserInput);
      if (_state)
        break;
      usleep(10000);
    }

    if (_state != EMBED_OK)
    {
      GB.Error("Embedding has failed");
      return;
		}

    WIDGET->show();
    define_mask(THIS);
    define_tooltip(THIS);
  }

END_METHOD


BEGIN_METHOD_VOID(CTRAYICON_hide)

  destroy_widget(THIS);

END_METHOD


BEGIN_PROPERTY(CTRAYICON_visible)

  if (READ_PROPERTY)
  {
    GB.ReturnBoolean(WIDGET != NULL);
    return;
  }

  if (VPROP(GB_BOOLEAN))
    CTRAYICON_show(_object, _param);
  else
    CTRAYICON_hide(_object, _param);

END_PROPERTY


BEGIN_PROPERTY(CTRAYICON_tag)

  if (READ_PROPERTY)
    GB.ReturnPtr(GB_T_VARIANT, &THIS->tag);
  else
    GB.StoreVariant(PROP(GB_VARIANT), (void *)&THIS->tag);

END_METHOD


BEGIN_METHOD_VOID(CTRAYICON_next)

  unsigned int index;

  index = ENUM(int);

  if (index >= _list.count())
  {
    GB.StopEnum();
    return;
  }

  ENUM(int) = index + 1;

  GB.ReturnObject(_list.at(index));

END_METHOD


BEGIN_METHOD(CTRAYICON_get, GB_INTEGER index)

  uint index = (uint)VARG(index);

  if (index >= _list.count())
	{
		GB.Error("Bad index");
		return;
	}
	
  GB.ReturnObject(_list.at(index));

END_METHOD


BEGIN_PROPERTY(CTRAYICON_count)

  GB.ReturnInteger(_list.count());

END_PROPERTY


GB_DESC CTrayIconsDesc[] =
{
  GB_DECLARE("TrayIcons", 0), GB_NOT_CREATABLE(),

  GB_STATIC_METHOD("_next", "TrayIcon", CTRAYICON_next, NULL),
  GB_STATIC_METHOD("_get", "TrayIcon", CTRAYICON_get, "(Index)i"),
  GB_STATIC_PROPERTY_READ("Count", "i", CTRAYICON_count),

  GB_END_DECLARE
};


GB_DESC CTrayIconDesc[] =
{
  GB_DECLARE("TrayIcon", sizeof(CTRAYICON)),

  GB_METHOD("_new", NULL, CTRAYICON_new, NULL),
  GB_METHOD("_free", NULL, CTRAYICON_free, NULL),

  GB_METHOD("Show", NULL, CTRAYICON_show, NULL),
  GB_METHOD("Hide", NULL, CTRAYICON_hide, NULL),
  GB_METHOD("Delete", NULL, CTRAYICON_hide, NULL),

  GB_PROPERTY("Picture", "Picture", CTRAYICON_picture),
  GB_PROPERTY("Icon", "Picture", CTRAYICON_picture),
  GB_PROPERTY("Visible", "b", CTRAYICON_visible),

  GB_PROPERTY("Text", "s", CTRAYICON_tooltip),
  GB_PROPERTY("Tooltip", "s", CTRAYICON_tooltip),
  GB_PROPERTY("Tag", "v", CTRAYICON_tag),

  GB_PROPERTY_READ("ScreenX", "i", CCONTROL_screen_x),
  GB_PROPERTY_READ("ScreenY", "i", CCONTROL_screen_y),
  GB_PROPERTY_READ("Width", "i", CCONTROL_w),
  GB_PROPERTY_READ("Height", "i", CCONTROL_h),
  GB_PROPERTY_READ("W", "i", CCONTROL_w),
  GB_PROPERTY_READ("H", "i", CCONTROL_h),

  GB_EVENT("Enter", NULL, NULL, &EVENT_Enter),
  GB_EVENT("GotFocus", NULL, NULL, &EVENT_GotFocus),
  GB_EVENT("LostFocus", NULL, NULL, &EVENT_LostFocus),
  GB_EVENT("Leave", NULL, NULL, &EVENT_Leave),
  GB_EVENT("MouseDown", NULL, NULL, &EVENT_MouseDown),
  GB_EVENT("MouseMove", NULL, NULL, &EVENT_MouseMove),
  GB_EVENT("MouseUp", NULL, NULL, &EVENT_MouseUp),
  GB_EVENT("MouseWheel", NULL, NULL, &EVENT_MouseWheel),
  GB_EVENT("DblClick", NULL, NULL, &EVENT_DblClick),
  GB_EVENT("Menu", NULL, NULL, &EVENT_Menu),

	TRAYICON_DESCRIPTION,

  GB_END_DECLARE
};


/*--- CTrayIcon -----------------------------------------------------------------------------------------*/

CTrayIcon CTrayIcon::manager;

void CTrayIcon::error(void)
{
  //CWINDOW *_object = (CWINDOW *)CWidget::getReal((QObject *)sender());
  //qDebug("XEMBED: CWindow::error %p -> %p", sender(), THIS);
  _state = EMBED_ERROR;
}

void CTrayIcon::embedded(void)
{
  //CWINDOW *_object = (CWINDOW *)CWidget::getReal((QObject *)sender());
  //qDebug("XEMBED: CWindow::embedded %p -> %p", sender(), THIS);
  _state = EMBED_OK;
}

void CTrayIcon::closed(void)
{
  //CWINDOW *_object = (CWINDOW *)CWidget::getReal((QObject *)sender());
  //qDebug("XEMBED: CWindow::closed %p -> %p", sender(), THIS);
  //CWIDGET_destroy(CWidget::getReal((QObject *)sender()));
  destroy_widget(find_object(sender()));
}


bool CTrayIcon::eventFilter(QObject *widget, QEvent *event)
{
  CTRAYICON *_object;
  int event_id;
  int type = event->type();
  bool original;
  int state;
  QPoint p;

  _object = find_object(widget);

  if (!_object)
    goto _STANDARD;

  original = event->spontaneous();

  if (type == QEvent::Enter)
  {
    GB.Raise(THIS, EVENT_Enter, 0);
  }
  else if (type == QEvent::Leave)
  {
    GB.Raise(THIS, EVENT_Leave, 0);
  }
  else if (type == QEvent::FocusIn)
  {
    GB.Raise(THIS, EVENT_GotFocus, 0);
  }
  else if (type == QEvent::FocusOut)
  {
    GB.Raise(THIS, EVENT_LostFocus, 0);
  }
  else if (type == QEvent::ContextMenu)
  {
    if (GB.CanRaise(THIS, EVENT_Menu))
    {
      ((QContextMenuEvent *)event)->accept();
      GB.Raise(THIS, EVENT_Menu, 0);
      return true;
    }
  }
  else if ((type == QEvent::MouseButtonPress)
           || (type == QEvent::MouseButtonRelease)
           || (type == QEvent::MouseMove))
  {
    QMouseEvent *mevent = (QMouseEvent *)event;

    if (!original)
      goto _DESIGN;

    if (type == QEvent::MouseButtonPress)
    {
      event_id = EVENT_MouseDown;
      state = mevent->stateAfter();
    }
    else
    {
      event_id = (type == QEvent::MouseButtonRelease) ? EVENT_MouseUp : EVENT_MouseMove;
      state = mevent->state();
    }

    if (GB.CanRaise(THIS, event_id))
    {
      p.setX(mevent->globalX());
      p.setY(mevent->globalY());

      p = WIDGET->mapFromGlobal(p);

      CMOUSE_clear(true);
      CMOUSE_info.x = p.x();
      CMOUSE_info.y = p.y();
      CMOUSE_info.state = state;

      GB.Raise(THIS, event_id, 0);

      CMOUSE_clear(false);
    }
  }
  else if (type == QEvent::MouseButtonDblClick)
  {
    if (!original)
      goto _DESIGN;

    GB.Raise(THIS, EVENT_DblClick, 0);
  }
  else if (type == QEvent::Wheel)
  {
    QWheelEvent *ev = (QWheelEvent *)event;

    //qDebug("Event on %p %s%s%s", widget,
    //  real ? "REAL " : "", design ? "DESIGN " : "", child ? "CHILD " : "");

    if (!original)
      goto _DESIGN;

    if (GB.CanRaise(THIS, EVENT_MouseWheel))
    {
      p.setX(ev->x());
      p.setY(ev->y());

      //p = WIDGET->mapTo(QWIDGET(control), p);

      CMOUSE_clear(true);
      CMOUSE_info.x = p.x();
      CMOUSE_info.y = p.y();
      CMOUSE_info.state = ev->state();
      CMOUSE_info.orientation = ev->orientation();
      CMOUSE_info.delta = ev->delta();

      GB.Raise(THIS, EVENT_MouseWheel, 0);

      CMOUSE_clear(false);
    }
  }

_DESIGN:
_STANDARD:

	if (!find_object(widget))
		return true;
		
  return QObject::eventFilter(widget, event);    // standard event processing
}
