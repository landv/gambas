#ifndef __GMENU_H
#define __GMENU_H

class gMainWindow;
class gPicture;

class gMenu
{
public:
	gMenu(gMainWindow *par,bool hidden);
	gMenu(gMenu *par,bool hidden);
	~gMenu();

	void *hFree;

	static int   winChildCount(gMainWindow *win);
	static gMenu* winChildMenu(gMainWindow *par,int pos);

//"Properties"
	bool checked() { return _checked; }
	bool toggle() { return _toggle; }
	bool enabled();
	gMenu* childMenu(int pos);
	int childCount();
	char* shortcut() { return _shortcut; }
	char* text() { return _text; }
	bool isVisible();
	gPicture* picture() { return _picture; }
	gMainWindow* window();
	char *name() { return _name; }
	bool topLevel() { return top_level; }

	void setChecked(bool vl);
	void setToggle(bool vl);
	void setEnabled(bool vl);
	void setShortcut(char *txt);
	void setText(const char *vl);
	void setVisible(bool vl);
	void show() { setVisible(true); }
	void hide() { setVisible(false); }
	void setPicture(gPicture *pic);
	void setName(char *name);
	bool action() { return _action; }
	void setAction(bool v) { _action = v; }

//"Methods"
	void popup();
	void popup(int x, int y);
	void destroy();

// "Signals"
	void (*onFinish)(gMenu *sender); // Special
	void (*onClick)(gMenu *sender);
	void (*onShow)(gMenu *sender);
	void (*onHide)(gMenu *sender);

//"Private"
	enum gMenuStyle { NOTHING, SEPARATOR, MENU };
	
	void *pr;
	bool stop_signal;
	GtkAccelGroup *accel;
	GtkMenu *child;
	GtkMenuItem *menu;
	GtkWidget *label;
	GtkWidget *aclbl;
	GtkWidget *image;
	GtkWidget *check;
	GtkSizeGroup *sizeGroup;
	void initialize();
	gMenuStyle style() { return _style; }
  void hideSeparators();

	static void embedMenuBar(gMainWindow *win, GtkWidget *border);
	static void checkMenuBar(gMainWindow *win);
private:

	gMenuStyle _style, _oldstyle;
	char *_name;
  gPicture *_picture;
	char *_shortcut;
	char *_text;
	unsigned _checked : 1;
	unsigned _toggle : 1;
	unsigned _no_update : 1;
	unsigned _destroyed : 1;
	unsigned top_level : 1;
	unsigned _action : 1;
	unsigned _visible : 1;
  
  void update();
  void updateVisible();
};

#endif
