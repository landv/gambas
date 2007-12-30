#ifndef __GCURSOR_H
#define __GCURSOR_H

class gPicture;

class gCursor
{
public:
	gCursor(gPicture *pic,long x,long y);
	gCursor(gCursor *cursor);
	~gCursor();

//"Properties"
	long left();
	long top();

//"Private"
	GdkCursor *cur;
	long x;
	long y;
};

#endif
