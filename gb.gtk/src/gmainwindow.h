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
	int getBorder();
	gPicture *icon() { return _icon; }
	gPicture *picture() { return _picture; }
	bool mask() { return _mask; }
	int menuCount();
	bool modal();
	const char* text();
	bool topOnly();
	bool skipTaskBar();
	bool minimized();
	bool maximized();
	bool fullscreen();
	bool getSticky();
	int  getStacking();
	bool isPersistent() { return persistent; }
	bool isToolBox() { return _toolbox; }
	bool isClosed() { return !opened; }
	
	int controlCount();
	gControl *getControl(char *name);
	gControl *getControl(int i);

	void setBorder(int b);
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
  void setToolBox(bool vl);
	virtual void setVisible(bool vl);
	virtual void setRealBackground(gColor vl);
	virtual void reparent(gContainer *newpr, int x, int y);

	virtual int clientWidth();
	virtual int clientHeight();
	virtual int clientX();
	virtual int clientY();
	
	bool spontaneous() { return !_not_spontaneous; }

//"Methods"
	void center();
	void showModal();
	void raise();
	virtual void move(int x, int y);
	virtual void resize(int w, int h);
	bool close();

//"Signals"
	void (*onOpen)(gMainWindow *sender);
	void (*onShow)(gMainWindow *sender);
	void (*onHide)(gMainWindow *sender);
	void (*onMove)(gMainWindow *sender);
	void (*onResize)(gMainWindow *sender);
	bool (*onClose)(gMainWindow *sender);
	void (*onActivate)(gMainWindow *sender);
	void (*onDeactivate)(gMainWindow *sender);

//"Static"
	static GList *windows;
	static int count() { return g_list_length(windows); }
	static gMainWindow *get(int index) { return (gMainWindow *)g_list_nth_data(windows, index); }
	static gMainWindow *_active;
	static void setActiveWindow(gControl *control);
	static gMainWindow *_current;
	
//"Private"
  void initialize();
	void drawMask();
	void initWindow();
	void emitOpen();
	void remap();
	bool doClose();
	void afterShow();

	GtkWindowGroup *group;
	GtkAccelGroup *accel;
	GtkMenuBar *menuBar;
	int stack;
	gPicture *_icon;
	gPicture *_picture;
	char *_title;
	GdkPixmap *_background;
	GtkStyle *_style;
	
	gControl *focus;
	gButton *_default;
	gButton *_cancel;
	
	int _next_w, _next_h;
	guint _next_timer;
	
	unsigned _mask : 1;
	unsigned top_only : 1;
	unsigned _resized : 1;
	unsigned persistent : 1;
	unsigned sticky : 1;
	unsigned opened : 1;
	unsigned _closing : 1;
	unsigned _toolbox : 1;
	unsigned _not_spontaneous : 1;
	unsigned _skip_taskbar : 1;
	unsigned _masked : 1;
	unsigned _xembed : 1;
	unsigned _activate : 1;
};

#endif
