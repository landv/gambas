/***************************************************************************

  gdrag.h

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

#ifndef __GDRAG_H
#define __GDRAG_H

class gPicture;
class gControl;

class gDrag
{
public:
	enum
	{
		Nothing = 0,
		Text = 1,
		Image = 2
	};
	enum {
		Copy = 0,
		Move = 1,
		Link = 2
	};
	
	static void exit();
	
	static bool isActive() { return _active; }
	static bool isEnabled() { return _enabled; }
	
	static void setIcon(gPicture *vl);
	static gPicture *getIcon() { return _icon; }
	static void getIconPos(int *x, int *y) { *x = _icon_x; *y = _icon_y; }
	static void setIconPos(int x, int y) { _icon_x = x; _icon_y = y; }
	
	static gControl *dragText(gControl *source, char *text, char *format = 0);
	static gControl *dragImage(gControl *source, gPicture *image);
	static void end();
	static void cancel();
	
	static gControl *getSource() { return _source; }
	static gControl *getDestination() { return _destination; }
	static int getAction() { return _action; }
	
	static int getType();
	static char *getFormat(int n = 0);
	static char *getText(int *len, const char *format, bool fromOutside = false);
	static gPicture *getImage(bool fromOutside = false);
	
	static int getDropX() { return _x; }
	static int getDropY() { return _y; }
	
	static void show(gControl *control, int x = 0, int y = 0, int w = -1, int h = -1);
	static void hide(gControl *control = NULL);
	
	static bool checkThreshold(gControl *control, int x, int y, int sx, int sy);
	
	// "Private"
	static void setDropInfo(int type, char *format);
	static void setDropData(int action, int x, int y, gControl *source, gControl *dest);
	static void setDropText(char *text, int len = -1);
	static void setDropImage(gPicture *image);
	static void setDropImage(char *buf, int len);
	
	static GdkDragContext *enable(GdkDragContext *context, gControl *control, guint32 time);
	static GdkDragContext *disable(GdkDragContext *context);
	static bool getData(const char *prefix);

	static volatile bool _got_data;

private:	

	static gControl *drag(gControl *source, GtkTargetList *list);
	
	static bool _active;
	static gPicture *_icon;
	static int _icon_x;
	static int _icon_y;
	static gControl *_source;
	static gControl *_destination;
	static int _action;
	static int _type;
	static gPicture *_picture;
	static char *_text;
	static int _text_len;
	static char *_format;
	static int _enabled;
	static int _x;
	static int _y;
	static GdkDragContext *_context;
	static gControl *_dest;
	static guint32 _time;
	static bool _local;
	static volatile bool _end;
};

#endif
