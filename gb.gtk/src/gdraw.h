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
	int lineWidth();
	int lineStyle();
	int fillX();
	int fillY();
	int fillStyle();
	bool invert();
	int clipX();
	int clipY();
	int clipWidth();
	int clipHeight();
	bool clipEnabled();
	int textWidth(char *txt, int len = -1);
	int textHeight(char *txt, int len = -1);
	void richTextSize(char *txt, int len, int sw, int *w, int *h);
	int width() { return _width; }
	int height() { return _height; }
	int resolution();
	bool isTransparent();

	void setForeground(gColor vl);
	void setFont(gFont *f);
	void setBackground(gColor vl);
	void setFillColor(gColor vl);
	void setLineWidth(int vl);
	void setLineStyle(int vl);
	void setFillX(int vl);
	void setFillY(int vl);
	void setFillStyle(int vl);
	void setInvert(bool vl);
	void setClipEnabled(bool vl);
	void setTransparent(bool vl);

//"Methods"
	void point(int x,int y);
	void line(int x1,int y1,int x2,int y2);
	void rect(int x,int y,int width,int height);
	void ellipse(int x,int y,int w,int h,double start,double end);
	void polyline (int *vl,int nel);
	void polygon (int *vl,int nel);
	void setClip(int x,int y,int w,int h);
	void text(char *txt, int len, int x, int y, int w, int h, int align);
	void richText(char *txt, int len, int x, int y, int w, int h, int align);

	void picture(gPicture *pic, int x, int y, int w = -1, int h = -1, int sx = 0, int sy = 0, int sw = -1, int sh = -1);
	void tiledPicture(gPicture *pic, int x, int y, int w, int h);

//"Private"
private:
	void init();
	void clear();
	void reset();
	void startFill();
	void endFill();
	void drawLayout(PangoLayout *ly, int x, int y, int w, int h, int align);
	void initGC();

	gDrawingArea *dArea;
	gFont *ft;
	GdkRectangle clip;
	bool clip_enabled;
	bool _transparent;
	GdkDrawable *dr;
	GdkDrawable *drm;
	GdkPixmap *stipple;
	GdkGC *gc;
	GdkGC *gcm;
	int fill;
	int fillCol;
	int line_style;
	void *tag;
	int _width;
	int _height;
	int _resolution;
	gColor _default_fg;
	gColor _default_bg;
	gColor _save_fg;
};

#endif
