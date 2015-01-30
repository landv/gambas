/***************************************************************************

  CWidget.h

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

#ifndef __CWIDGET_H
#define __CWIDGET_H

#include "main.h"
#include "share/gb.form.properties.h"

#include <QObject>
#include <QWidget>
#include <QIcon>
#include <QPixmap>
#include <QEvent>
#include <QHash>

typedef
	struct {
		GB_COLOR fg;
		GB_COLOR bg;
		GB_VARIANT_VALUE tag;
		void *cursor;
		char *popup;
		void *proxy;
		void *proxy_for;
		char *action;
		int focusPolicy;
		void *container_for;
	}
	CWIDGET_EXT;

typedef
	struct CWIDGET {
		GB_BASE ob;
		QWidget *widget;
		CWIDGET_EXT *ext;
		struct {
			unsigned char f;
			unsigned expand : 1;
			unsigned ignore : 1;
			unsigned notified : 1;
			unsigned visible : 1;
			unsigned fillBackground : 1;
			unsigned noBackground : 1;
			unsigned shown : 1; // for containers
			unsigned tracking : 1;
			unsigned old_tracking : 1;
			unsigned grab : 1;
			unsigned dragging: 1;
			unsigned noTabFocus : 1;
			unsigned inside : 1;
			unsigned inside_later : 1;
			unsigned use_tablet : 1;
			unsigned no_keyboard : 1;
			unsigned tablet_pressed : 1;
			unsigned has_action : 1;
			unsigned drop : 1;
			unsigned resized : 1;
			unsigned _reserved : 4;
			} flag;
		int level;
		char *name;
		void *font;
		}
	PACKED
	CWIDGET; // BEWARE: gb.qt.h MUST be updated accordingly!

typedef
	CWIDGET CCONTROL;

typedef
	struct {
		CWIDGET widget;
		QWidget *container;
		int32_t arrangement;
		}
	CCONTAINER;

enum {
	WF_DESIGN           = (1 << 0),
	WF_DESIGN_LEADER    = (1 << 1),
	WF_PERSISTENT       = (1 << 2),
	WF_CLOSED           = (1 << 3),
	WF_DELETED          = (1 << 4),
	WF_VISIBLE          = (1 << 5),  // Only for menus
	WF_SCROLLVIEW       = (1 << 6)   // Inherits QScrollView
	};



#ifndef __CWIDGET_CPP

extern GB_DESC CControlDesc[];
extern CWIDGET *CWIDGET_active_control;
extern CWIDGET *CWIDGET_previous_control;

extern int CCONTROL_last_event_type;

#else

#define THIS (((CWIDGET *)_object))
#define THIS_EXT (((CWIDGET *)_object)->ext)
#define WIDGET QWIDGET(_object)

#endif

#define QWIDGET(object) (((CWIDGET *)object)->widget)
#define QCONTAINER(_ob) CWidget::getContainerWidget((CCONTAINER *)_ob)

DECLARE_METHOD(Control_Delete);
DECLARE_METHOD(Control_Refresh);
DECLARE_PROPERTY(Control_Tag);
DECLARE_PROPERTY(CWIDGET_border_full);
DECLARE_PROPERTY(CWIDGET_border_simple);
DECLARE_PROPERTY(CWIDGET_scrollbar);
DECLARE_PROPERTY(Control_Background);
DECLARE_PROPERTY(Control_Design);
DECLARE_PROPERTY(Control_Enabled);
DECLARE_PROPERTY(Control_Font);
DECLARE_PROPERTY(Control_Action);
DECLARE_PROPERTY(Control_Mouse);

#define CWIDGET_set_flag(wid, _f) (((CWIDGET *)wid)->flag.f |= _f)
#define CWIDGET_clear_flag(wid, _f) (((CWIDGET *)wid)->flag.f &= ~_f)
#define CWIDGET_test_flag(wid, _f) ((((CWIDGET *)wid)->flag.f & _f) != 0)

#define RAISE_EVENT(_event) \
{ \
	GET_SENDER(); \
\
	if (_object == NULL) \
		return; \
\
	GB.Raise(_object, _event, 0); \
}

#define RAISE_EVENT_ACTION(_event) \
{ \
	GET_SENDER(); \
\
	if (_object == NULL) \
		return; \
\
	GB.Ref(_object); \
	GB.Raise(_object, _event, 0); \
	CACTION_raise(_object); \
	GB.Unref(POINTER(&_object)); \
}


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

void CWIDGET_new(QWidget *w, void *_object, bool no_show = false, bool no_filter = false, bool no_tag = false);
void CWIDGET_init_name(CWIDGET *_object);
void CWIDGET_set_name(CWIDGET *_object, const char *name);
int CWIDGET_check(void *object);
QString CWIDGET_Utf8ToQString(GB_STRING *str);
void CWIDGET_destroy(CWIDGET *_object);
void CWIDGET_update_design(CWIDGET *_object);
void CWIDGET_iconset(QIcon &icon, const QPixmap &p, int size = 0);
void CWIDGET_set_color(CWIDGET *_object, int bg, int fg, bool handle_proxy = false);
void CWIDGET_reset_color(CWIDGET *_object);
QT_COLOR_FUNC CWIDGET_after_set_color(QT_COLOR_FUNC func);
GB_COLOR CWIDGET_get_background(CWIDGET *_object, bool handle_proxy = false);
GB_COLOR CWIDGET_get_foreground(CWIDGET *_object, bool handle_proxy = false);

GB_COLOR CWIDGET_get_real_background(CWIDGET *_object);
GB_COLOR CWIDGET_get_real_foreground(CWIDGET *_object);
void *CWIDGET_get_real_font(CWIDGET *_object);

void *CWIDGET_get_parent(void *_object);
int CWIDGET_get_handle(void *_object);
bool CWIDGET_is_visible(void *_object);
void CWIDGET_set_visible(CWIDGET *_object, bool v);
void CWIDGET_grab(CWIDGET *_object);
void CWIDGET_move(void *_object, int x, int y);
void CWIDGET_resize(void *_object, int w, int h);
void CWIDGET_move_resize(void *_object, int x, int y, int w, int h);
void CWIDGET_move_cached(void *_object, int x, int y);
void CWIDGET_resize_cached(void *_object, int w, int h);
void CWIDGET_move_resize_cached(void *_object, int x, int y, int w, int h);
void CWIDGET_handle_focus(CWIDGET *control, bool on);
void CWIDGET_register_proxy(void *_object, void *proxy);
bool CWIDGET_container_for(void *_object, void *container_for);
void *CWIDGET_enter_popup();
void CWIDGET_leave_popup(void *save);
void CACTION_register(void *control, const char *old, const char *key);
void CACTION_raise(void *control);
bool CWIDGET_get_allow_focus(void *_object);
void CWIDGET_set_allow_focus(void *_object, bool f);
bool CWIDGET_is_design(CWIDGET *_object);
void CWIDGET_check_visibility(CWIDGET *_object);


#ifndef DO_NOT_DECLARE_EVENTS
#ifndef __CWIDGET_CPP
extern
#endif
int
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
EVENT_Drop,
EVENT_DragLeave;
#endif

struct CWINDOW;

class CWidget : public QObject
{
	Q_OBJECT

public:

	static CWidget manager;

	static void add(QObject *, void *, bool no_filter);
	static CWIDGET *get(QObject *);
	static CWIDGET *getReal(QObject *o) { return dict[o]; }
	static CWIDGET *getRealExisting(QObject *);
	static CWIDGET *getDesign(QObject *);

	static QWidget *getContainerWidget(CCONTAINER *object);

	static CWINDOW *getWindow(CWIDGET *object);
	static CWINDOW *getTopLevel(CWIDGET *object);

	//static void setName(CWIDGET *, const char *);
	//static void installFilter(QObject *);
	//static void removeFilter(QObject *);

	//static const char *getProperties(const void *klass);
	//static void setProperties(const void *klass, const char *prop);

	static void removeFocusPolicy(QWidget *);

public slots:

	void destroy(void);

protected:

	bool eventFilter(QObject *, QEvent *);

private:

	static bool real;
	static QHash<QObject *, CWIDGET *> dict;
};

#endif
