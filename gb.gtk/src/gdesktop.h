#ifndef __GDESKTOP_H
#define __GDESKTOP_H

class gPicture;
class gMainWindow;
class gControl;
class gFont;

class gDesktop
{
public:

	static void init();
	static void exit();

	static gColor buttonfgColor();
	static gColor buttonbgColor();
	static gColor fgColor();
	static gColor bgColor();
	static gColor textfgColor();
	static gColor textbgColor();
	static gColor selfgColor();
	static gColor selbgColor();

	static gFont* font();
	static void setFont(gFont *vl);
	static int height();
	static int width();
	static int resolution();
	static int scale();
	static gPicture* grab();
	static gMainWindow* activeWindow();
	static gControl* activeControl();

	static bool rightToLeft();
};

#endif
