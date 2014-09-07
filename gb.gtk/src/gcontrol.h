/***************************************************************************

  gcontrol.h

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

#ifndef __GCONTROL_H
#define __GCONTROL_H

#include "gcolor.h"
#include "gdrag.h"

class gContainer;
class gMainWindow;

class gControl
{
public:
	gControl();
	gControl(gContainer *parent);
	virtual ~gControl();

	void *hFree;

// "Properties"
	int getClass() const { return g_typ; }
	
	bool isContainer() const { return (g_typ & 0x100) != 0; }
	bool isWindow() const;
	bool isTopLevel() const { return pr == NULL; }
	bool isDestroyed() const { return _destroyed; }
	
	gMainWindow *window();
	gMainWindow *topLevel();
	
	gContainer *parent() const { return pr; }
	bool isAncestorOf(gControl *child);
	gCursor* cursor();
	bool design();
	virtual bool isEnabled() const;
	bool expand() const { return expa; }
	bool ignore() const { return igno; }
	bool hovered();
	virtual int handle();
	
	int left() const { return bufX; }
	int x() const { return left(); }
	int top() const { return bufY; }
	int y() const { return top(); }
	int width() const { return bufW; }
	int height() const { return bufH; }
	
	int mouse();
	gControl *next();
	gControl *previous();
	int screenX();
	int screenY();
	virtual bool getScreenPos(int *x, int *y);
	char *toolTip();
	bool isVisible() const { return visible; }
	bool isReallyVisible();
	bool acceptDrops() { return _accept_drops; }
	char *name() { return _name; }
	void setName(char *name);
	bool action() { return _action; }
	void setAction(bool v) { _action = v; }

	void setCursor(gCursor *vl);
	void setAcceptDrops(bool vl);
	void setDesign(bool vl);
	virtual void setEnabled(bool vl);
	void setExpand (bool vl);
	void setIgnore (bool vl);
	virtual void setHeight(int h);
	void setLeft(int l);
	void setMouse(int m);
	virtual void updateCursor(GdkCursor *cursor);
	void setToolTip(char *vl);
	void setTop(int t);
	virtual void setVisible(bool v);
	virtual void setWidth(int w);
	void setPrevious(gControl *prev);
	void setNext(gControl *next);
	
	bool isTracking() const;
	void setTracking(bool vl);
	
	bool isNoTabFocus() const { return _no_tab_focus; }
	void setNoTabFocus(bool v);

	gColor background();
	gColor foreground();
	virtual void setBackground(gColor color = COLOR_DEFAULT);
	virtual void setForeground(gColor color = COLOR_DEFAULT);
	gColor realBackground(bool no_default = false);
	gColor realForeground(bool no_default = false);
	virtual void setRealBackground(gColor color);
	virtual void setRealForeground(gColor color);

	virtual gFont *font();
	void actualFontTo(gFont *ft);
	virtual void setFont(gFont *ft);
	bool ownFont() { return _font != NULL; }
	virtual void updateFont();
	virtual void updateSize();
#ifdef GTK3
	virtual void updateColor();
	void setColorNames(const char *bg_names[], const char *fg_names[]);
	void setColorBase();
	void setColorButton();
#else
	void setColorNames(const char **, const char **) {}
	void setColorBase() { use_base = TRUE; }
	void setColorButton() { use_base = FALSE; }
#endif

	bool canFocus() const;
	void setCanFocus(bool vl);

	gControl *proxy() const { return _proxy; }
	bool setProxy(gControl *proxy);

	int scrollX();
	int scrollY();
	void scroll(int x, int y);
	void setScrollX(int vl);
	void setScrollY(int vl);
	//virtual int scrollWidth();
	//virtual int scrollHeight();
	int scrollBar();
	void setScrollBar(int vl);

// "Methods"
	void dragText(char *txt, char *format = NULL) { gDrag::dragText(this, txt, format); }
	void dragImage(gPicture *pic) { gDrag::dragImage(this, pic); }
	
	virtual void reparent(gContainer *newpr, int x, int y);
	void hide() { setVisible(false); }
	void lower();
	void raise();
	virtual void move(int x, int y);
	virtual void resize(int w, int h);
	virtual void moveResize(int x, int y, int w, int h);
	virtual void setFocus();
	bool hasFocus() const;
#if GTK_CHECK_VERSION(3, 2, 0)
	bool hasVisibleFocus() const;
#else
	bool hasVisibleFocus() const { return hasFocus(); }
#endif
	void resize() { resize(width(), height()); }
	void show() { setVisible(true); }
	void refresh();
	void refresh(int x, int y, int w, int h);
	virtual void afterRefresh();
	bool grab();
	void destroy();
	void destroyNow() { destroy(); cleanRemovedControls(); }
	
	void lock() { _locked++; }
	void unlock() { _locked--; }
	bool locked() { return _locked; }
	
	void getGeometry(GdkRectangle *rect) const { rect->x = bufX; rect->y = bufY; rect->width = bufW; rect->height = bufH; }
	void setGeometry(GdkRectangle *rect) { moveResize(rect->x, rect->y, rect->width, rect->height); }
	
	void emit(void *signal);
	void emit(void *signal, intptr_t arg);
	void emit(void *signal, char *arg) { emit(signal, (intptr_t)arg); }

// "Signals"
	bool (*canRaise)(gControl *sender, int type);
	void (*onFinish)(gControl *sender); // Special
	void (*onFocusEvent)(gControl *sender, int type);
	bool (*onKeyEvent)(gControl *sender, int type);
	bool (*onMouseEvent)(gControl *sender, int type);
	void (*onEnterLeave)(gControl *sender, int type);
	bool (*onDrag)(gControl *sender);
	bool (*onDragMove)(gControl *sender);
	bool (*onDrop)(gControl *sender);
	void (*onDragLeave)(gControl *sender);
	//void (*onMove)(gControl *sender);
	//void (*onResize)(gControl *sender);

// "Private"
	gint bufW,bufH,bufX,bufY;
	gCursor *curs;
	gFont *_font;
	gFont *_resolved_font;
	GtkWidget *widget;
	GtkWidget *border;
	GtkWidget *frame;
	GtkScrolledWindow *_scroll;
	short g_typ;
	short _mouse;
	gControl *_proxy, *_proxy_for;
	gColor _bg, _fg;
#ifdef GTK3
	const char *_bg_name;
	const char **_bg_name_list;
	GdkRGBA _bg_default;
	const char *_fg_name;
	const char **_fg_name_list;
	GdkRGBA _fg_default;
#endif
	
	unsigned dsg : 1;
	unsigned expa : 1;
	unsigned igno : 1;
	unsigned _action : 1;                  // *reserved*
	unsigned _accept_drops : 1;            // If the control accepts drops
	unsigned _drag_get_data : 1;           // If we got information on the dragged data
	unsigned _drag_enter : 1;              // If we have entered the control for drag & drop
	unsigned _tracking : 1;                // If we are tracking mouse move even if no mouse button is pressed
	
	unsigned _old_tracking : 1;            // real value when Tracking is false
	unsigned _bg_set : 1;                  // Have a private background
	unsigned _fg_set : 1;                  // Have a private foreground
	unsigned have_cursor : 1;              // If gApplication::setBusy() must update the cursor
	unsigned use_base : 1;                 // Use base and text color for foreground and background
	unsigned visible : 1;                  // A control can be hidden if its width or height is zero
	unsigned _destroyed : 1;               // If the control has already been added to the destroy list
	
	unsigned _locked : 4;                  // For locking events
	unsigned frame_border : 4;

	unsigned frame_padding : 8;
	
	unsigned _scrolled_window : 1;
	unsigned _dirty_pos : 1;               // If the position of the widget has changed
	unsigned _dirty_size : 1;              // If the size of the widget has changed
	unsigned _no_delete : 1;               // Do not delete on destroy signal
	unsigned no_input_method : 1;          // No input method management
	unsigned _no_default_mouse_event : 1;  // No default mouse events
	unsigned _grab : 1;                    // control is currently grabbing mouse and keyboard
	unsigned _has_border : 1;              // if the control has a border
	unsigned _no_tab_focus : 1;            // Don't put inside focus chain
	unsigned _inside : 1;                  // if we got an enter event, but not a leave event yet.
	unsigned _no_auto_grab : 1;            // do not automatically grab widget on button press event
	unsigned _no_background : 1;           // Don't draw the background automatically
	
  void removeParent() { pr = NULL; }
	void initSignals();
	void borderSignals();
	void widgetSignals();
	void connectParent();
	void setParent(gContainer *parent) { pr = parent; }
	void initAll(gContainer *pr);
	void realize(bool make_frame = false);
	void realizeScrolledWindow(GtkWidget *wid, bool doNotRealize = false);
	void registerControl();
	void updateGeometry();
	bool mustUpdateCursor() { return mouse() != -1 || have_cursor; }
	
	bool noInputMethod() { return no_input_method; }
	
	GdkCursor *getGdkCursor();
	virtual void updateBorder();
	int getFrameBorder() const { return frame_border; }
	void setFrameBorder(int border);
	void setBorder(bool b);
	bool hasBorder() const;
	int getFramePadding() const { return frame_padding; }
	void setFramePadding(int padding);
	virtual int getFrameWidth();
	virtual gColor getFrameColor();
#ifdef GTK3
	void drawBorder(cairo_t *cr);
	void drawBackground(cairo_t *cr);
#else
	void drawBorder(GdkEventExpose *e);
	void drawBackground(GdkEventExpose *e);
#endif
	
	virtual int minimumHeight();
	void resolveFont();
	
	void emitEnterEvent(bool no_leave = false);
	void emitLeaveEvent();
	
/*	static gControl* dragWidget();
	static void setDragWidget(gControl *ct);
	static char *dragTextBuffer();
	static GdkPixbuf *dragPictureBuffer();
	static void freeDragBuffer();*/
	static GList* controlList();
	static void cleanRemovedControls();

private:
	gContainer *pr;
	char *_name;
};

#define SIGNAL(_signal) ((void *)_signal)

#endif
