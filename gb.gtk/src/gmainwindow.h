/***************************************************************************

  gmainwindow.h

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

#ifndef __GMAINWINDOW_H
#define __GMAINWINDOW_H

#include "gbutton.h"

class gMainWindow : public gContainer
{
public:
	gMainWindow(int plug = 0);
	gMainWindow(gContainer *parent);
	~gMainWindow();

//"Properties"
	bool hasBorder();
	bool isResizable();
	bool isUtility() const;
	gPicture *icon() { return _icon; }
	gPicture *picture() { return _picture; }
	bool mask() { return _mask; }
	int menuCount();
	bool isModal() const;
	const char* text();
	bool topOnly();
	bool skipTaskBar();
	bool minimized() const { return _minimized; }
	bool maximized() const { return _maximized; }
	bool fullscreen() const { return _fullscreen; }
	bool getSticky();
	int  getStacking();
	bool isPersistent() const { return persistent; }
	bool isOpened() const { return opened; }
	bool isClosed() const { return !opened; }
	bool isHidden() const { return _hidden; }
	bool isPopup() const { return _popup; }
	bool isTransparent() const { return _transparent; }
	int screen();
	
	int controlCount();
	gControl *getControl(char *name);
	gControl *getControl(int i);

	void setBorder(bool b);
	void setResizable(bool b);
	void setUtility(bool v);
	void setIcon(gPicture *pic);
	void setMask(bool vl);
	void setPicture(gPicture *pic);
	void setText(const char *txt);
	void setTopOnly(bool vl);
	void setSkipTaskBar(bool b);
	void setMinimized(bool vl);
	void setMaximized(bool vl);
	void setFullscreen(bool vl);
	void setSticky(bool vl);
	void setStacking(int vl);
  void setPersistent(bool vl);
	void setTransparent(bool vl);
	
	virtual void setVisible(bool vl);
	virtual void setBackground(gColor vl);
	virtual void setRealBackground(gColor vl);
	virtual void setRealForeground(gColor vl);

	virtual int clientWidth();
	virtual int clientHeight();
	virtual int clientX();
	virtual int clientY();
	virtual int containerX();
	virtual int containerY();

	//virtual bool getScreenPos(int *x, int *y);
	
	bool spontaneous() { return !_not_spontaneous; }
	
	void setMenuBarVisible(bool v);
	bool isMenuBarVisible();
	
	double opacity();
	void setOpacity(double v);

//"Methods"
	void center();
	void showActivate();
	void showModal();
	void showPopup();
	void showPopup(int x, int y);
	void raise();
	virtual void move(int x, int y);
	virtual void resize(int w, int h);
	virtual void moveResize(int x, int y, int w, int h);
	bool close();
	virtual void reparent(gContainer *newpr, int x, int y);

//"Signals"
	void (*onOpen)(gMainWindow *sender);
	void (*onShow)(gMainWindow *sender);
	void (*onHide)(gMainWindow *sender);
	void (*onMove)(gMainWindow *sender);
	void (*onResize)(gMainWindow *sender);
	bool (*onClose)(gMainWindow *sender);
	void (*onActivate)(gMainWindow *sender);
	void (*onDeactivate)(gMainWindow *sender);
	void (*onState)(gMainWindow *sender);

//"Static"
	static GList *windows;
	static int count() { return g_list_length(windows); }
	static gMainWindow *get(int index) { return (gMainWindow *)g_list_nth_data(windows, index); }
	static gMainWindow *_active;
	static void setActiveWindow(gControl *control);
	static gMainWindow *_current;
	static bool closeAll();

//"Private"
  void initialize();
	void drawMask();
	void initWindow();
	void emitOpen();
	void remap();
	bool doClose();
	void afterShow();
	void checkMenuBar();
	int menuBarHeight();
	void configure();
	void embedMenuBar(GtkWidget *border);
	void emitResize();
	void setGeometryHints();
	virtual void updateFont();
	
	GtkWindowGroup *group;
	GtkAccelGroup *accel;
	GtkMenuBar *menuBar;
	GtkFixed *layout;
	int stack;
	int _type;
	gPicture *_icon;
	gPicture *_picture;
	char *_title;
	GtkStyle *_style;
	
	gControl *focus;
	gButton *_default;
	gButton *_cancel;
	
	int _resize_last_w;
	int _resize_last_h;

	int _min_w;
	int _min_h;
	
	unsigned _mask : 1;
	unsigned top_only : 1;
	unsigned _resized : 1;
	unsigned persistent : 1;
	unsigned sticky : 1;
	unsigned opened : 1;
	unsigned _closing : 1;
	unsigned _not_spontaneous : 1;
	unsigned _skip_taskbar : 1;
	unsigned _masked : 1;
	unsigned _xembed : 1;
	unsigned _activate : 1;
	unsigned _hidden : 1;
	unsigned _hideMenuBar : 1;
	unsigned _showMenuBar : 1;
	unsigned _popup : 1;
	unsigned _maximized : 1;
	unsigned _minimized : 1;
	unsigned _fullscreen : 1;
	unsigned _utility : 1;
	unsigned _transparent : 1;
};

#endif
