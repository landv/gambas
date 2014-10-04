/***************************************************************************

  gmenu.h

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
	static void updateFont(gMainWindow *win);
	static void updateColor(gMainWindow *win);
	static gMenu *findFromName(gMainWindow *win, const char *name);

	static int popupCount() { return _popup_count; }

//"Properties"
	bool checked() const { return _checked; }
	bool toggle() const { return _toggle; }
	bool radio() const { return _radio; }
	bool enabled();
	gMenu* childMenu(int pos);
	int childCount();
	char* shortcut() const { return _shortcut; }
	char* text() const { return _text; }
	bool isVisible();
	gPicture* picture() const { return _picture; }
	gMainWindow* window();
	char *name() const { return _name; }
	bool topLevel() const { return top_level; }
	bool isSeparator() const { return _style == SEPARATOR; }
	void *parent() const { return pr; }

	void setChecked(bool vl);
	void setToggle(bool vl);
	void setRadio(bool vl);
	void setEnabled(bool vl);
	void setShortcut(char *txt);
	void setText(const char *vl);
	void setVisible(bool vl);
	void show() { setVisible(true); }
	void hide() { setVisible(false); }
	void setPicture(gPicture *pic);
	void setName(char *name);
	bool action() const { return _action; }
	void setAction(bool v) { _action = v; }
	void setFont();
	void setColor();
	//bool isTearOff() const;
	//void setTearOff(bool v);

//"Methods"
	void popup();
	void popup(int x, int y);
	void destroy();
	static bool insidePopup() { return _in_popup > 0; }
	static gMenu *currentPopup() { return _current_popup; }

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
	GtkWidget *hbox;
	GtkWidget *label;
	GtkWidget *aclbl;
	GtkWidget *image;
	GtkWidget *check;
	GtkSizeGroup *sizeGroup;
	void initialize();
	gMenuStyle style() const { return _style; }
  void hideSeparators();
	void willBeDeletedLater() { _delete_later = TRUE; }
	void setRadio();

private:

	gMenuStyle _style, _oldstyle;
	char *_name;
  gPicture *_picture;
	char *_shortcut;
	char *_text;
	unsigned _checked : 1;
	unsigned _toggle : 1;
	unsigned _radio : 1;
	unsigned _no_update : 1;
	unsigned _destroyed : 1;
	unsigned _delete_later : 1;
	unsigned top_level : 1;
	unsigned _action : 1;
	unsigned _visible : 1;

	static gMenu *_current_popup;
	static int _in_popup;
	static int _popup_count;
  
	void doPopup(bool move, int x = 0, int y = 0);
  void update();
  void updateVisible();
};

#endif
