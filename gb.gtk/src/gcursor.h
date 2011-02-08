#ifndef __GCURSOR_H
#define __GCURSOR_H

#define CURSOR_DEFAULT GDK_X_CURSOR
#define CURSOR_CUSTOM GDK_CURSOR_IS_PIXMAP

class gPicture;

class gCursor
{
public:
	gCursor(gPicture *pic,int x,int y);
	gCursor(gCursor *cursor);
	~gCursor();

//"Properties"
	int left();
	int top();

//"Private"
	GdkCursor *cur;
	int x;
	int y;
};

#endif
