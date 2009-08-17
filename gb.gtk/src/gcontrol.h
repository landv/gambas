/***************************************************************************

  gcontrol.h

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
	int getClass() { return g_typ; }
	
	bool isContainer() { return (g_typ & 0x100) != 0; }
	bool isWindow();
	bool isTopLevel() { return pr == NULL; }
	bool isDestroyed() { return _destroyed; }
	
	gMainWindow *window();
	gMainWindow *topLevel();
	
	gContainer *parent() const { return pr; }
	gCursor* cursor();
	bool design();
	virtual bool enabled();
	bool expand();
	bool ignore();
	virtual int handle();
	virtual int height();
	virtual int left();
	int x() { return left(); }
	int mouse();
	gControl *next();
	gControl *previous();
	int screenX();
	int screenY();
	char *toolTip();
	virtual int top();
	int y() { return top(); }
	virtual int width();
	virtual bool isVisible();
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
	void setTracking(bool vl);
	bool isTracking() { return _tracking; }

	gColor background();
	gColor foreground();
	virtual void setBackground(gColor color = COLOR_DEFAULT);
	virtual void setForeground(gColor color = COLOR_DEFAULT);
	gColor realBackground();
	gColor realForeground();
	virtual void setRealBackground(gColor color);
	virtual void setRealForeground(gColor color);

	virtual gFont *font();
	virtual void setFont(gFont *ft);
	bool ownFont() { return fnt != 0; }
	
	int scrollX();
	int scrollY();
	void scroll(int x, int y);
	void setScrollX(int vl);
	void setScrollY(int vl);
	virtual int scrollWidth();
	virtual int scrollHeight();
	int scrollBar();
	void setScrollBar(int vl);

// "Methods"
	void dragText(char *txt, char *format = NULL) { gDrag::dragText(this, txt, format); }
	void dragImage(gPicture *pic) { gDrag::dragImage(this, pic); }
	
	virtual void reparent(gContainer *newpr, int x, int y);
	void hide() { setVisible(false); }
	void lower();
	void raise();
	void move(int x, int y, int w, int h);
	virtual void move(int x, int y);
	virtual void resize(int w, int h);
	virtual void setFocus();
	bool hasFocus();
	void resize() { resize(width(), height()); }
	void show() { setVisible(true); }
	void refresh();
	void refresh(int x, int y, int w, int h);
	virtual void afterRefresh();
	gPicture *grab();
	void destroy();
	void destroyNow() { destroy(); cleanRemovedControls(); }
	
	void lock() { _locked++; }
	void unlock() { _locked--; }
	bool locked() { return _locked; }
	
	void emit(void *signal);
	void emit(void *signal, intptr_t arg);
	void emit(void *signal, char *arg) { emit(signal, (intptr_t)arg); }

// "Signals"
	void (*onFinish)(gControl *sender); // Special
	void (*onFocusEvent)(gControl *sender, int type);
	bool (*onKeyEvent)(gControl *sender, int type);
	bool (*onMouseEvent)(gControl *sender, int type);
	void (*onEnterLeave)(gControl *sender, int type);
	bool (*onDrag)(gControl *sender);
	bool (*onDragMove)(gControl *sender);
	void (*onDrop)(gControl *sender);
	//void (*onMove)(gControl *sender);
	//void (*onResize)(gControl *sender);

// "Private"
	gint bufW,bufH,bufX,bufY;
	gCursor *curs;
	gFont *fnt;
	GtkWidget *widget;
	GtkWidget *border;
	GtkWidget *frame;
	GtkScrolledWindow *_scroll;
	short g_typ;
	short mous;
	
	unsigned dsg : 1;
	unsigned expa : 1;
	unsigned igno : 1;
	unsigned _action : 1;         // *reserved*
	unsigned _accept_drops : 1;   // If the control accepts drops
	unsigned _drag_get_data : 1;  // If we got information on the dragged data
	unsigned _drag_enter : 1;     // If we have entered the control for drag & drop
	unsigned _tracking : 1;       // If we are tracking mouse move even if no mouse button is pressed
	unsigned _old_tracking : 1;   // real value when Tracking is false
	
	unsigned bg_set : 1;          // Have a private background
	unsigned fg_set : 1;          // Have a private foreground
	unsigned have_cursor : 1;     // If gApplication::setBusy() must update the cursor
	unsigned use_base : 1;        // Use base and text color for foreground and background
	unsigned visible : 1;         // A control can be hidden if its width or height is zero
	unsigned _destroyed : 1;      // If the control has already been added to the destroy list
	unsigned _dirty_pos : 1;      // If the position of the widget has changed
	unsigned _dirty_size : 1;     // If the size of the widget has changed
	
	unsigned _locked : 4;         // For locking events
	unsigned frame_border : 4;
	unsigned frame_padding : 8;
	unsigned _scrolled_window : 1;
	
	unsigned _no_delete : 1;         // Do not delete on destroy signal
	unsigned no_input_method : 1;    // No input method management
	unsigned _no_default_mouse_event : 1; // No default mouse events
	
	
  void removeParent() { pr = NULL; }
	void initSignals();
	void borderSignals();
	void widgetSignals();
	void connectParent();
	void setParent(gContainer *parent) { pr = parent; }
	void initAll(gContainer *pr);
	void realize(bool make_frame = false);
	void realizeScrolledWindow(GtkWidget *wid, bool doNotRealize = false);
	void updateGeometry();
	bool mustUpdateCursor() { return mouse() != -1 || have_cursor; }
	
	bool noInputMethod() { return no_input_method; }
	
	virtual void updateBorder();
	int getFrameBorder() { return frame_border; }
	void setFrameBorder(int border);
	void setBorder(bool b);
	bool hasBorder();
	int getFramePadding() { return frame_padding; }
	void setFramePadding(int padding);
	virtual int getFrameWidth();
	void drawBorder(GdkDrawable *win = 0);
	
	virtual int minimumHeight();
	void resolveFont(gFont *new_font);
	
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
