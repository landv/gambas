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
	char* toolTip();
	void setToolTip(char *txt);
	bool isVisible();
	void setVisible(bool vl);
	long screenX();
	long screenY();
	long width();
	long height();
	
//"Methods"
	void destroy();
	void show() { setVisible(true); }
	void hide() { setVisible(false); }

//"Events"
	void (*onDoubleClick)(gTrayIcon *sender);
	void (*onMousePress)(gTrayIcon *sender);
	void (*onMouseRelease)(gTrayIcon *sender);
	void (*onMouseWheel)(gTrayIcon *sender);
	void (*onMenu)(gTrayIcon *sender);
	void (*onDestroy)(gTrayIcon *sender);
	void (*onFocusEnter)(gTrayIcon *sender);
	void (*onFocusLeave)(gTrayIcon *sender);
	void (*onEnter)(gTrayIcon *sender);
	void (*onLeave)(gTrayIcon *sender);

//"Static"

	static int count() { return g_list_length(trayicons); }
	static gTrayIcon *get(int index) { return (gTrayIcon *)g_list_nth_data(trayicons, index); }
	static void exit();	
	static gPicture *defaultIcon();

//"Private"
	GtkWidget *plug;
	gPicture *_icon;
	char *buftext;
	bool onHide;
	gPicture *getIcon() { return _icon ? _icon : defaultIcon(); }
	void updateMask();
	void updateTooltip();
	void cleanUp();
	
	static GList *trayicons;
	static gPicture *_default_icon;

private:

	GtkStyle *_style;
};

#endif
