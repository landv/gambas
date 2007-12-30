#ifndef __GDIALOG_H
#define __GDIALOG_H

#include "gcolor.h"

class gDialog
{
public:
	static void exit();

	static gColor color();
	static char** filter(int *nfilter);
	static char** paths();
	static char* path();
	static char* title();
	static gFont* font();

	static void setColor(gColor col);
	static void setFilter(char **filter, int nfilter);
	static void setPath(char *vl);
	static void setTitle(char *title);
	static void setFont(gFont *ft);

	static bool selectColor();
	static bool selectFolder();
	static bool selectFont();
	static bool openFile(bool multi=false);
	static bool saveFile();

private:
  static GPtrArray *_filter;
};

#endif
