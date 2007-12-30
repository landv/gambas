#ifndef __GCLIPBOARD_H
#define __GCLIPBOARD_H

class gPicture;

class gClipboard
{
public:
	enum
	{
		Nothing = 0,
		Text = 1,
		Image = 2
	};
	
	static void init();
	static void clear();
	static char *getFormat(int n = 0);
	static int getType();
	static void setText(char *text, char *format = 0);
	static char *getText();
	static void setImage(gPicture *image);
	static gPicture *getImage();
};

#endif
