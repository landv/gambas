#ifndef __GSIMPLELABEL_H
#define __GSIMPLELABEL_H

class gSimpleLabel : public gControl
{
public:
	gSimpleLabel(gContainer *parent);
	~gSimpleLabel();

	int alignment();
	int getBorder() { return getFrameBorder(); }
	char* text();
	//long background();
	//long foreground();
	bool transparent();
	bool autoResize();
	int padding() { return getFramePadding(); }

	void setAlignment(int al);
	void setBorder(int vl) { setFrameBorder(vl); }
	void setText(char *st);
	//void setBackground(long vl);
	//void setForeground(long vl);
	void setFont(gFont *ft);
	void setTransparent(bool vl);
	void setAutoResize(bool vl);
	void setPadding(int vl) { setFramePadding(vl); }

//"Methods"
	void enableMarkup(bool vl);
	virtual void resize(int w, int h);

//"Private"
	PangoLayout *layout;
	GdkBitmap *mask;
	int align,lay_x,lay_y;
	bool markup;
	bool autoresize;
	char *textdata;
};

#endif
