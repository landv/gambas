/***************************************************************************

  CWidget.h

  The Control class

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

#ifndef __CWIDGET_H
#define __CWIDGET_H

#include "main.h"
#include "share/gb.form.properties.h"

#include <qobject.h>
#include <qwidget.h>
#include <qiconset.h>
#include <qptrdict.h>
#include <qasciidict.h>
#include <qpixmap.h>
#include <qcstring.h>

/* (!) Reporter les modifications de CWIDGET dans gb.qt.h */

typedef
  struct CWIDGET {
    GB_BASE ob;
    QWidget *widget;
    struct {
    	unsigned short f;
    	unsigned default_bg : 1;
    	unsigned default_fg : 1;
    	unsigned expand : 1;
    	unsigned ignore : 1;
    	unsigned notified : 1;
    	unsigned _reserved : 11;
    	} flag;
    GB_VARIANT_VALUE tag;
    char *tooltip;
    char *name;
    void *cursor;
    CWIDGET *next;
    CWIDGET *prev;
    long level;
    }
  CWIDGET;

typedef
	CWIDGET CCONTROL;

typedef
  struct {
    CWIDGET widget;
    QWidget *container;
    long arrangement;
    }
  CCONTAINER;

enum {
  WF_DESIGN           = (1 << 0),
  WF_DESIGN_LEADER    = (1 << 1),
  WF_PERSISTENT       = (1 << 2),
  WF_IN_SHOW          = (1 << 3),
  WF_IN_CLOSE         = (1 << 4),
  WF_CLOSED           = (1 << 5),
  WF_DELETED          = (1 << 6),
  WF_VISIBLE					= (1 << 7),  // Only for menus
  WF_ACTION           = (1 << 8),  // Has an action
  WF_SCROLLVIEW				= (1 << 9)   // Inherits QScrollView
  };



#ifndef __CWIDGET_CPP

extern GB_DESC CControlDesc[];
extern CWIDGET *CWIDGET_destroy_list;
extern CWIDGET *CWIDGET_destroy_last;

extern GB_CLASS CLASS_Control;
extern GB_CLASS CLASS_Container;
extern GB_CLASS CLASS_UserControl;
extern GB_CLASS CLASS_UserContainer;
extern GB_CLASS CLASS_Window;
extern GB_CLASS CLASS_Menu;
extern GB_CLASS CLASS_Picture;
extern GB_CLASS CLASS_Drawing;
extern GB_CLASS CLASS_DrawingArea;
extern GB_CLASS CLASS_Printer;

#else

#define THIS (((CWIDGET *)_object))
#define WIDGET QWIDGET(_object)

#endif

#define QWIDGET(object) (((CWIDGET *)object)->widget)
#define QCONTAINER(_ob) CWidget::getContainerWidget((CCONTAINER *)_ob)

DECLARE_METHOD(CCONTROL_delete);
DECLARE_PROPERTY(CCONTROL_tag);
DECLARE_PROPERTY(CWIDGET_border_full);
DECLARE_PROPERTY(CWIDGET_border_simple);
DECLARE_PROPERTY(CWIDGET_scrollbar);
DECLARE_PROPERTY(CCONTROL_design);
DECLARE_PROPERTY(CCONTROL_enabled);
DECLARE_PROPERTY(CCONTROL_font);
DECLARE_PROPERTY(CCONTROL_action);

#define CWIDGET_set_flag(wid, _f) (((CWIDGET *)wid)->flag.f |= _f)
#define CWIDGET_clear_flag(wid, _f) (((CWIDGET *)wid)->flag.f &= ~_f)
#define CWIDGET_test_flag(wid, _f) ((((CWIDGET *)wid)->flag.f & _f) != 0)

#define GET_SENDER(_ob)   CWIDGET *_ob = CWidget::get((QObject *)sender())

#define RAISE_EVENT(_event) \
{ \
  GET_SENDER(ob); \
\
  if (ob == NULL) \
    return; \
\
  GB.Raise(ob, _event, 0); \
}

#define RAISE_EVENT_ACTION(_event) \
{ \
  GET_SENDER(ob); \
\
  if (ob == NULL) \
    return; \
\
  GB.Raise(ob, _event, 0); \
  CACTION_raise(ob); \
}


#define EVENT_CLOSE (QEvent::User)
#define EVENT_POST ((QEvent::Type)(QEvent::User + 1))
#define EVENT_EXPAND ((QEvent::Type)(QEvent::User + 2))
#define EVENT_TITLE ((QEvent::Type)(QEvent::User + 3))
#define EVENT_ICON ((QEvent::Type)(QEvent::User + 4))
#define EVENT_DESTROY ((QEvent::Type)(QEvent::User + 5))
#define EVENT_INSERT ((QEvent::Type)(QEvent::User + 6))
#define EVENT_REMOVE ((QEvent::Type)(QEvent::User + 7))

/*#define ALIGN_MASK (Qt::AlignLeft | Qt::AlignRight | Qt::AlignTop | Qt::AlignBottom | Qt::AlignCenter)
#define ALIGN_HMASK (Qt::AlignLeft | Qt::AlignRight | Qt::AlignHCenter)
#define ALIGN_VMASK (Qt::AlignTop | Qt::Bottom | Qt::AlignVCenter)*/

#define ALIGN_HMASK (Qt::AlignHorizontal_Mask)
#define ALIGN_VMASK (Qt::AlignVertical_Mask)
#define ALIGN_MASK (ALIGN_HMASK | ALIGN_VMASK)

#define SET_WIDGET(_ob, _wid) (((CWIDGET *)_ob)->widget = (_wid))
#define CLEAR_WIDGET(_ob) SET_WIDGET(_ob, 0)

#define EMBED_WAIT   0
#define EMBED_OK     1
#define EMBED_ERROR  2

void CWIDGET_new(QWidget *w, void *_object, char *klass = NULL,
                 bool no_filter = false, bool no_tag = false);
void CWIDGET_init_name(CWIDGET *_object);
int CWIDGET_check(void *object);
QString CWIDGET_Utf8ToQString(GB_STRING *str);
void CWIDGET_destroy(CWIDGET *);
void CWIDGET_update_design(CWIDGET *_object);
void CWIDGET_iconset(QIconSet &icon, QPixmap &p, int size = 0);
void CWIDGET_set_color(CWIDGET *_object, int bg, int fg);
int CWIDGET_get_background(CWIDGET *_object);
int CWIDGET_get_foreground(CWIDGET *_object);

void CACTION_register(void *control, const char *key);
void CACTION_raise(void *control);
void CACTION_get(void *control);

#ifndef DO_NOT_DECLARE_EVENTS
#ifndef __CWIDGET_CPP
extern
#endif
long
//EVENT_Move,
//EVENT_Resize,
EVENT_MouseDown,
EVENT_MouseUp,
EVENT_MouseMove,
EVENT_MouseDrag,
EVENT_MouseWheel,
EVENT_DblClick,
EVENT_KeyPress,
EVENT_KeyRelease,
EVENT_Enter,
EVENT_Leave,
EVENT_GotFocus,
EVENT_LostFocus,
EVENT_Menu,
EVENT_Drag,
EVENT_DragMove,
EVENT_Drop;
#endif

struct CWINDOW;

class CWidget : public QObject
{
  Q_OBJECT

public:

  static CWidget manager;

  static void add(QObject *, void *, bool no_filter);
  static CWIDGET *get(QObject *);
  static CWIDGET *getReal(QObject *);
  static CWIDGET *getDesign(QObject *);

  static QWidget *getContainerWidget(CCONTAINER *object);

  static CWINDOW *getWindow(CWIDGET *object);
  static CWINDOW *getTopLevel(CWIDGET *object);

  //static void setName(CWIDGET *, const char *);
  //static void installFilter(QObject *);
  //static void removeFilter(QObject *);

  //static const char *getProperties(const void *klass);
  //static void setProperties(const void *klass, const char *prop);

  static void resetTooltip(CWIDGET *);
  static void removeFocusPolicy(QWidget *);

public slots:

  void destroy(void);

protected:

  bool eventFilter(QObject *, QEvent *);

private:

  static bool real;

  static CWIDGET *enter;
  static QPtrDict<CWIDGET> dict;
};

#endif
