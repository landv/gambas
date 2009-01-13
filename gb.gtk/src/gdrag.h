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
	
	static void dragText(gControl *source, char *text, char *format = 0);
	static void dragImage(gControl *source, gPicture *image);
	static void cancel();
	
	static gControl *getSource() { return _source; }
	static gControl *getDestination() { return _destination; }
	static int getAction() { return _action; }
	
	static int getType();
	static char *getFormat(int n = 0);
	static char *getText();
	static gPicture *getImage();
	
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
	static char *_format;
	static int _enabled;
	static int _x;
	static int _y;
	static GdkDragContext *_context;
	static gControl *_dest;
	static guint32 _time;
	static bool _local;
};

#endif
