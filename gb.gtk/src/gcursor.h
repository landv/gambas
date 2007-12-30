#ifndef __GCURSOR_H
#define __GCURSOR_H

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
