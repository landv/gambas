#ifndef __GPICTUREBOX_H
#define __GPICTUREBOX_H

class gPictureBox : public gControl
{
public:
	gPictureBox(gContainer *parent);
	~gPictureBox();

	long foreground();
	long background();
	int alignment();
	int getBorder() { return getFrameBorder(); }
	bool stretch();
	gPicture* picture() { return _picture; }
	bool isAutoResize() { return _autoresize; }

	void setForeground(long vl);
	void setBackground(long vl);
	void setAlignment(int vl);
	void setBorder(int vl) { setFrameBorder(vl); }
	void setStretch(bool vl);
	void setPicture(gPicture *pic);
	void setAutoResize(bool);

//"Methods"
	void resize(long w,long h);

//"Private"
	void redraw();
  gPicture *_picture;
  bool _autoresize;
};

#endif
