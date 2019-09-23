/***************************************************************************

  gtrayicon.h

  (c) 2000-2017 Benoît Minisini <g4mba5@gmail.com>

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

#ifndef __GTRAYICON_H
#define __GTRAYICON_H

class gTrayIcon
{
public:
//"Properties"
	gTrayIcon();
	~gTrayIcon();

	void *hFree;

	char *key();
	gPicture* picture() { return _icon; }
	void setPicture(gPicture *pic);
	char* tooltip() const { return _tooltip; }
	void setTooltip(char *txt);
	bool isVisible();
	void setVisible(bool vl);
	
//"Methods"
	void destroy();
	void show() { setVisible(true); }
	void hide() { setVisible(false); }
	int loopLevel() { return _loopLevel; }

//"Events"
	void (*onClick)(gTrayIcon *sender, int button);
	void (*onScroll)(gTrayIcon *sender);
	void (*onMenu)(gTrayIcon *sender);
	void (*onDestroy)(gTrayIcon *sender);

//"Static"

	static int count() { return g_list_length(trayicons); }
	static int visibleCount() { return _visible_count; }
	static gTrayIcon *get(int index) { return (gTrayIcon *)g_list_nth_data(trayicons, index); }
	static void exit();	
	static gPicture *defaultIcon();
	static bool hasSystemTray();

//"Private"
	GtkStatusIcon *plug;
	gPicture *_icon;
	int _iconw, _iconh;
	char *_tooltip;
	bool onHide;
	int _loopLevel;
	gPicture *getIcon() { return _icon ? _icon : defaultIcon(); }
	void updateTooltip();
	void updatePicture();
	
	static GList *trayicons;
	static gPicture *_default_icon;

private:

	static int _visible_count;
};

#endif
