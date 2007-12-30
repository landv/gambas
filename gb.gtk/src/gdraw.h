#ifndef __GDRAW_H
#define __GDRAW_H

class gDrawingArea;

class gDraw
{
public:
	gDraw();
	~gDraw();
	void connect(gControl *wid);
	void connect(gPicture *wid);
	void disconnect();

//"Properties"
	gColor foreground();
	gFont* font();
	gColor background();
	gColor fillColor();
	long lineWidth();
	long lineStyle();
	long fillX();
	long fillY();
	long fillStyle();
	bool invert();
	long clipX();
	long clipY();
	long clipWidth();
	long clipHeight();
	bool clipEnabled();
	long textWidth(char *txt);
	long textHeight(char *txt);
	int width() { return _width; }
	int height() { return _height; }
	int resolution() { stub("gDraw::resolution"); return 0; }

	void setForeground(gColor vl);
	void setFont(gFont *f);
	void setBackground(gColor vl);
	void setFillColor(gColor vl);
	void setLineWidth(long vl);
	void setLineStyle(long vl);
	void setFillX(long vl);
	void setFillY(long vl);
	void setFillStyle(long vl);
	void setInvert(bool vl);
	void setClipEnabled(bool vl);

//"Methods"
	void point(long x,long y);
	void line(long x1,long y1,long x2,long y2);
	void rect(long x,long y,long width,long height);
	void ellipse(long x,long y,long w,long h,long start,long end);
	void polyline (long *vl,long nvl);
	void polygon (long *vl,long nvl);
	void startClip(long x,long y,long w,long h);
	void text(char *txt,int len,long x,long y,long w,long h,long align);

	//void image(gPicture *img,long x,long y,long Sx,long Sy,long Sw,long Sh);
	void picture(gPicture *pic,long x,long y,long Sx,long Sy,long Sw,long Sh);

//"Private"
	void pClear();
	gDrawingArea *dArea;
	gFont *ft;
	GdkRectangle clip;
	bool clip_enabled;
	GdkDrawable *dr;
	GdkPixmap *stipple;
	GdkGC *gc;
	int fill;
	long fillCol;
	long line_style;
	void *tag;
	int _width;
	int _height;
	int _resolution;
	gColor _default_fg;
	gColor _default_bg;
};

#endif
