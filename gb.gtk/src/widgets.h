/***************************************************************************

  widgets.h

  (c) 2004-2006 - Daniel Campos Fernández <dcamposf@gmail.com>

  Gtkmae "GTK+ made easy" classes

  Realizado para la Junta de Extremadura.
  Consejería de Educación Ciencia y Tecnología.
  Proyecto gnuLinEx

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 1, or (at your option)
  any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

***************************************************************************/

#include <gtk/gtk.h>
#ifdef GDK_WINDOWING_X11
#include <gdk/gdkx.h>
#endif

#include "tablerender.h"

#ifndef __widgets_h
#define __widgets_h

//#define GTK_DEBUG_SIGNALS
//#define GTK_DEBUG_OBJECTS

#define AlignVCenter 0x0040
#define AlignAuto    0x0000
#define AlignTop     0x0010
#define AlignBottom  0x0020
#define AlignHCenter 0x0004
#define AlignLeft    0x0001
#define AlignRight   0x0002

#define alignBottom        0x0024
#define alignBottomLeft    0x0021
#define alignBottomNormal  0x0020
#define alignBottomRight   0x0022
#define alignCenter        0x0044
#define alignLeft          0x0041
#define alignNormal        0x0040
#define alignRight         0x0042
#define alignTop           0x0014
#define alignTopLeft       0x0011
#define alignTopNormal     0x0010
#define alignTopRight      0x0012

#define FillNoBrush            0
#define FillSolidPattern       1
#define FillDense1Pattern      2
#define FillDense2Pattern      3
#define FillDense3Pattern      4
#define FillDense4Pattern      5
#define FillDense5Pattern      6
#define FillDense6Pattern      7
#define FillDense7Pattern      8
#define FillHorPattern         9
#define FillVerPattern        10
#define FillCrossPattern      11
#define FillBDiagPattern      12
#define FillFDiagPattern      13
#define FillDiagCrossPattern  14

#define Line_None         0
#define Line_Solid        1
#define Line_Dash         2
#define Line_Dot          3
#define Line_DashDot      4
#define Line_DashDotDot   5

#define BORDER_NONE   0
#define BORDER_PLAIN  1
#define BORDER_SUNKEN 2
#define BORDER_RAISED 3
#define BORDER_ETCHED 4

#define ARRANGE_NONE        0
#define ARRANGE_HORIZONTAL  1
#define ARRANGE_VERTICAL    2
#define ARRANGE_LEFT_RIGHT  3
#define ARRANGE_TOP_BOTTOM  4
#define ARRANGE_FILL        5

#define Type_gSeparator   0x0018
#define Type_gLabel       0x0001
#define Type_gTextLabel   0x0002
#define Type_gButton      0x0003
#define Type_gSplitter    0x0117
#define Type_gFrame       0x0105
#define Type_gMainWindow  0x0106
#define Type_gPictureBox  0x0007
#define Type_gPanel       0x0108
#define Type_gDrawingArea 0x0109
#define Type_gProgressBar 0x000A
#define Type_gSlider      0x000E
#define Type_gScrollBar   0x000F
#define Type_gTabStrip    0x0111
#define Type_gPlugin      0x0013
#define Type_gMovieBox    0x0014
#define Type_gScrollView  0x0115
#define Type_gSpinBox     0x0016
#define Type_gTextBox     0x1004
#define Type_gTextArea    0x100B
#define Type_gComboBox    0x100C
#define Type_gListBox     0x100D
#define Type_gListView    0x1010
#define Type_gTreeView    0x1014
#define Type_gIconView    0x1015
#define Type_gGridView    0x1016
#define Type_gColumnView  0x1012
#define Type_gEmbedWindow 0x0113

#define gEvent_MousePress     1
#define gEvent_MouseRelease   2
#define gEvent_MouseMove      3
#define gEvent_MouseWheel     4
#define gEvent_MouseDblClick  5
#define gEvent_MouseMenu      6
#define gEvent_KeyPress       7
#define gEvent_KeyRelease     8
#define gEvent_FocusIn        9
#define gEvent_FocusOut      10
#define gEvent_Enter         11
#define gEvent_Leave         12

void stub(char *function);

class gImage;
class gPicture;
class gControl;
class gMainWindow;
class gFont;
class gDrawingArea;
class alphaCache;
class gContainer;
class gTree;

int clipBoard_Type();
char* clipBoard_Format();
void clipBoard_Clear();
char* clipBoard_getText();
gImage* clipBoard_getImage();
void clipBoard_setText(char *text,char *format);
void clipBoard_setImage(gImage *image);

bool drag_IsEnabled();
int drag_Action();
int drag_Type();
char* drag_Format();
char* drag_Text();
gImage* drag_Image();
long drag_X();
long drag_Y();
gControl* drag_Widget();
void drag_setIcon(gPicture *pic);
gPicture* drag_Icon();

class gKey
{
public:
	static bool valid();
    static char *text();
    static int code();
    static int state();

	static bool alt();
	static bool control();
	static bool meta();
	static bool normal();
	static bool shift();

	static int fromString(char* str);

//"Private"
	static void disable();
	static void enable(GdkEventKey *e);
};


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


class gMouse
{
public:

//"Properties"
	static long button();
	static bool left();
	static bool right();
	static bool middle();
	static bool shift();
	static bool control();
	static bool alt();
	static bool meta();
	static bool normal();
	static long x();
	static long y();
	static long screenX();
	static long screenY();
	static long delta();
	static long orientation();
	static bool valid();

//"Methods"
	static void move(long x,long y);

//"Private"
	static void setWheel(long dt,long orn);
	static void setValid(int vl,long xv,long yv,long bt_val,long state_val,long sx,long sy);
};

class gDesktop
{
public:

	static void init();
	static void exit();

	static long buttonfgColor();
	static long buttonbgColor();
	static long fgColor();
	static long bgColor();
	static long textfgColor();
	static long textbgColor();
	static long selfgColor();
	static long selbgColor();

	static gFont* font();
	static void setFont(gFont *vl);
	static long height();
	static long width();
	static long resolution();
	static long scale();
	static gPicture* grab();
	static gMainWindow* activeWindow();
	static gControl* activeControl();

	static bool rightToLeft();
};


class gDraw
{
public:
	gDraw();
	~gDraw();
	void connect(gControl *wid);
	void connect(gPicture *wid);
	void disconnect();

//"Properties"
	long foreGround();
	gFont* font();
	long backGround();
	long fillColor();
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

	void setForeGround(long vl);
	void setFont(gFont *f);
	void setBackGround(long vl);
	void setFillColor(long vl);
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
	void text(char *txt,long x,long y,long w,long h,long align);

	void image(gImage *img,long x,long y,long Sx,long Sy,long Sw,long Sh);
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
};

class gStock
{
public:
	static gPicture* get(char *vl);

};

class gImage
{
public:
	gImage(long w,long h);
	~gImage();

	long width();
	long height();
	gPicture* getPicture();

	void fromMemory(char *addr,unsigned int len);
	int save(char *path);
	gImage* flip();
	gImage* mirror();
	gImage* rotate(double ang);
	void resize(long w,long h);
	gImage* stretch(long w,long h,bool smooth);

	void fill(long col);

	long getPixel(long x,long y);
	void putPixel(long x,long y,long col);
	void replace(long src,long dst);
	gImage* copy(long x,long y,long w,long h);
//"Private"
	GdkPixbuf *image;
};


class gPicture
{
public:
	gPicture(long w,long h,bool trans);
	~gPicture();

	long width();
	long height();
	long depth();
	bool transparent();

	void setTransparent(bool vl);

	void resize(int width,int height);
	int  save(char *path);
	void fromMemory(char *addr,unsigned int len);
	static gPicture* fromNamedIcon(char* name);
	void Fill(long col);
	gImage* getImage();
	gPicture* copy(long x,long y,long w,long h);

	void ref();
	void unref();

//"Private"
	GdkDrawable *pic;
	alphaCache *cache;
        int _transparent;
	GdkPixbuf* getPixbuf();
	static gPicture* fromPixbuf(GdkPixbuf *buf);
	long refcount;
};

class gImageCache
{
public:
	static void save(char *key,gImage *img);
	static gImage* load(char *key);
	static void flush();
};

class gPictureCache
{
public:
	static void save(char *key,gPicture *img);
	static gPicture* load(char *key);
	static void flush();
};

class gFont
{
public:
	gFont();
	gFont(char *name);
	~gFont();

	static void init();
	static void exit();
	static long count();
	static const char *familyItem(long pos);

	long ascent();
	long descent();
	bool fixed();
	bool scalable();
	char **styles();

	bool bold();
	bool italic();
	const char* name();
	long resolution();
	double size();
	bool strikeOut();
	bool underline();

	void setBold(bool vl);
	void setItalic(bool vl);
	void setName(char *nm);
	void setResolution(long vl);
	void setSize(double sz);
	void setStrikeOut(bool vl);
	void setUnderline(bool vl);

	const char * toString();
	long width(const char *text);
	long height(const char *text);

	void ref();
	void unref();

//"Private"
	gFont(GtkWidget *wg);
	void updateWidget();
	GtkWidget *wid;
	PangoContext* ct;
	bool uline;
	bool strike;
	long nref;
};


class gMessage
{
public:
	static int showDelete(char *msg,char *btn1,char *btn2,char *btn3);
	static int showError(char *msg,char *btn1,char *btn2,char *btn3);
	static int showInfo(char *msg,char *btn1);
	static int showQuestion(char *msg,char *btn1,char *btn2,char *btn3);
	static int showWarning(char *msg,char *btn1,char *btn2,char *btn3);

	static void setTitle(char *title);
};

class gDialog
{
public:
	static void exit();

	static long color();
	static char** filter(long *nfilter);
	static char** paths();
	static char* path();
	static char* title();
	static gFont* font();

	static void setColor(long col);
	static void setFilter(char **filter,long nfilter);
	static void setPath(char *vl);
	static void setTitle(char *title);
	static void setFont(gFont *ft);

	static bool selectColor();
	static bool selectFolder();
	static bool selectFont();
	static bool openFile(bool multi=false);
	static bool saveFile();
};

class gControl
{
public:
	gControl();
	gControl(gControl* parent);
	virtual ~gControl();

	void *hFree;

// "Properties"
	int getClass();
	virtual long backGround();
	gCursor* cursor();
	bool design();
	virtual bool enabled();
	bool expand();
	bool ignore();
	virtual gFont* font();
	virtual long foreGround();
	virtual long handle();
	virtual long height();
	virtual long left();
	int mouse();
	gControl *next();
	gControl *parent();
	gControl *previous();
	long screenX();
	long screenY();
	char *toolTip();
	virtual long top();
	virtual long width();
	virtual bool visible();
	bool acceptDrops();
	gControl *window();

	void setCursor(gCursor *vl);
	void setAcceptDrops(bool vl);
	void setDesign(bool vl);
	virtual void setEnabled(bool vl);
	virtual void setBackGround(long color);
	void setExpand (bool vl);
	void setIgnore (bool vl);
	virtual void setFont(gFont *ft);
	virtual void setForeGround(long color);
	virtual void setHeight(long h);
	void setLeft(long l);
	void setMouse(int m);
	void setToolTip(char *vl);
	void setTop(long t);
	virtual void setVisible(bool v);
	virtual void setWidth(long w);

// "Methods"
	void dragText(char *txt);
	void dragPicture(gPicture *pic);
	void dragImage(gImage *img);
	void reparent (gContainer *newpr);
	void hide();
	void lower();
	void moveResize(long x,long y,long w,long h);
	virtual void move(long x,long y);
	void raise();
	virtual void resize(long w,long h);
	void setFocus();
	virtual void show();
	void refresh(long x,long y,long w,long h);
	gPicture *grab();
	void destroy();

// "Signals"
	void (*onFinish)(gControl *sender); // Special
	void (*onFocusEvent)(gControl *sender,long type);
	bool (*onKeyEvent)(gControl *sender,long type);
	bool (*onMouseEvent)(gControl *sender,long type);
	void (*onEnterLeave)(gControl *sender,long type);
	void (*onDrag)(gControl *sender);
	bool (*onDragMove)(gControl *sender);
	void (*onDrop)(gControl *sender);

// "Private"
	gint bufW,bufH,bufX,bufY;
	gControl *pr;
	gCursor *curs;
	GtkWidget *widget;
	GtkWidget *border;
	int g_typ;
	int mous;
	int drops;
	bool dsg;
	bool expa;
	bool igno;
	bool font_change;
	void initSignals();
	void widgetSignals();
	void connectParent();
	void initAll(gControl *pr);
	static gControl* dragWidget();
	static void setDragWidget(gControl *ct);
	static char *dragTextBuffer();
	static GdkPixbuf *dragPictureBuffer();
	static void freeDragBuffer();
	static GList* controlList();
	static void cleanRemovedControls();
};

class gSeparator : public gControl
{
public:
	gSeparator(gControl *parent);

	long foreGround();
	long backGround();

	void setForeGround(long vl);
	void setBackGround(long vl);

};

class gTrayIcon
{
public:
//"Properties"
	gTrayIcon();

	void *hFree;

	char *key();
	gPicture* picture();
	void setPicture(gPicture *pic);
	char* toolTip();
	void setToolTip(char *txt);
	bool visible();
	void setVisible(bool vl);
	long screenX();
	long screenY();
	long width();
	long height();
//"Methods"
	void show();
	void hide();
	void destroy();

//"Events"
	void (*onDoubleClick)(gTrayIcon *sender);
	void (*onMousePress)(gTrayIcon *sender);
	void (*onMouseRelease)(gTrayIcon *sender);
	void (*onMenu)(gTrayIcon *sender);
	void (*onDestroy)(gTrayIcon *sender);
	void (*onFocusEnter)(gTrayIcon *sender);
	void (*onFocusLeave)(gTrayIcon *sender);
	void (*onEnter)(gTrayIcon *sender);
	void (*onLeave)(gTrayIcon *sender);

//"Private"
	GtkWidget *plug;
	GdkPixbuf *buficon;
	char *buftext;

	bool onHide;
};

class gPlugin : public gControl
{
public:
	gPlugin(gControl *parent);
	void plug(long id,bool prepared=true);
	void discard();
//"Properties"
	long client();
	int getBorder();
	long foreGround();
	long backGround();
	long handle();

	void setBorder(int vl);
	void setForeGround(long vl);
	void setBackGround(long vl);
//"Signals"
	void (*onPlug)(gControl *sender);
	void (*onUnplug)(gControl *sender);

};


class gProgressBar : public gControl
{
public:
	gProgressBar(gControl *parent);

	bool label();
	double value();

	void setLabel(bool vl);
	void setValue(double vl);

	void reset();

//"Private"
	bool lbl;
};

class gSimpleLabel : public gControl
{
public:
	gSimpleLabel(gControl *parent);
	~gSimpleLabel();

	int alignment();
	int getBorder();
	char* text();
	long backGround();
	long foreGround();
	bool transparent();
	bool autoResize();

	void setAlignment(int al);
	void setBorder(int vl);
	void setText(char *st);
	void setBackGround(long vl);
	void setForeGround(long vl);
	void setFont(gFont *ft);
	void setTransparent(bool vl);
	void setAutoResize(bool vl);

//"Methods"
	void enableMarkup(bool vl);
	void resize(long w,long h);

//"Private"
	PangoLayout *layout;
	GdkBitmap *mask;
	int align,lay_x,lay_y;
	bool markup;
	bool autoresize;
	char *textdata;
};


class gButton : public gControl
{
public:
	gButton(gControl *parent,int type);

	long backGround();
	bool getBorder();
	bool isCancel();
	bool isDefault();
	gFont* font();
	long foreGround();
	const char* text();
	gPicture* picture();
	bool value();
	bool getToggle();
	bool enabled();
	bool inconsistent();

	void setEnabled(bool vl);
	void setBackGround(long vl);
	void setBorder(bool vl);
	void setCancel(bool vl);
	void setDefault(bool vl);
	void setFont(gFont *ft);
	void setForeGround(long vl);
	void setText(const char *st);
	void setPicture(gPicture *pic);
	void setValue(bool vl);
	void setToggle(bool vl);
	void setInconsistent(bool vl);

//"Signals"
	void (*onClick)(gControl *sender);

//"Private"
	int type;
	bool disable;
	bool isToggle;
	char *bufText;
	GtkCellRenderer *rendtxt;
	GdkPixbuf *rendpix,*rendinc;
	bool scaled;

};

class gMovieBox : public gControl
{
public:
	gMovieBox(gControl *parent);

//"Properties"
	long foreGround();
	long backGround();
	int getBorder();
	bool playing();

	void setForeGround(long vl);
	void setBackGround(long vl);
	void setBorder(int vl);
	void setPlaying(bool vl);

//"Methods"
	bool loadMovie(char *buf,long len);

//"Private"
	bool pl;
	guint timeout;
	GdkPixbufAnimation *animation;
	GdkPixbufAnimationIter *iter;


};

class gPictureBox : public gControl
{
public:
	gPictureBox(gControl *parent);

	long foreGround();
	long backGround();
	int alignment();
	int getBorder();
	bool stretch();
	gPicture* picture();

	void setForeGround(long vl);
	void setBackGround(long vl);
	void setAlignment(int vl);
	void setBorder(int vl);
	void setStretch(bool vl);
	void setPicture(gPicture *pic);

//"Methods"
	void resize(long w,long h);

//"Private"
	GdkPixbuf *pix;
	void redraw();

};

class gColumnView : public gControl
{
public:
	gColumnView(gControl *parent);

//"Properties"
	char* columnText(long ind);
	long columnAlignment(long ind);
	long columnWidth(long ind);
	long columnsCount();
	long count();
	bool expanded(long ind);
	bool header();
	long itemCount(long ind);
	gPicture* itemPicture(long ind);
	char* itemText(long ind,long col);
	char* key(long ind);
	long keyIndex(char *key);
	long getCurrentCursor();
	bool resizable();
	long scrollBars();

	long findAt(int x,int y);
	long getFirstIndex();
	long getLastIndex();
	long getNextIndex(long ind);
	long getPreviousIndex(long ind);
	long getParentIndex(long ind);
	long getChildIndex(long ind);
	
	void setColumnAlignment(long ind,long vl);
	void setColumnText(long ind,char *vl);
	void setColumnWidth(long ind,long vl);
	void setColumnsCount(long vl);
	void setExpanded(long ind,bool vl);
	void setItemPicture(long ind,gPicture *pic);
	void setItemText(long ind,long col,char *txt);
	void setHeader(bool vl);
	void setResizable(bool vl);
	void setScrollBars(long vl);
	
//"Methods"
	int add(char *key,char *vl,gPicture *pic,char *pos,char *parent);
	void itemClear(long ind);
	void remove(long ind);
	
//"Events"
	void (*onClick)(gControl *sender);
	void (*onActivate)(gControl *sender);
	
//"Private"
	bool sort;
	bool colResize;
	long colCount;
};

class gListView : public gControl
{
public:
	gListView(gControl *parent);
	~gListView();

//"Properties"
	unsigned  long count();
	char*     itemText(char *key);
	void      itemSetText(char *key,char *vl);
	gPicture* itemPicture(char *key);
	void      itemSetPicture(char *key,gPicture *vl);
	bool      itemSelected(char *key);
	void      itemSetSelected(char *ket,bool vl);
	int       mode();
	void      setMode(int vl);
	bool       getBorder();
	void      setBorder(bool vl);
	long      visibleWidth();
	long      visibleHeight();
	long      scrollBar();
	void      setScrollBar(long vl);
	bool      sorted();
	void      setSorted(bool vl);

	char*     current();
	char*     firstItem();
	char*     lastItem();
	char*     nextItem(char *vl);
	char*     prevItem(char *vl);

//"Methods"
	bool      add(char *key,char *text,gPicture *pic=NULL,char *after=NULL);
	bool      remove(char *key);
	bool      exists(char *key);
	char*     find(long x,long y);
	void      clear();

//"Events"
	void      (*onActivate)(gListView *sender);
	void      (*onSelect)(gListView *sender);

//"Private"
	gTree *tree;
};

class gGridView : public gControl
{
public:
	gGridView(gControl *parent);
	~gGridView();

//"Properties"
	long	columnCount();
	long    rowCount();
	bool    columnResizable(long index);
	bool    rowResizable(long index);
	bool    rowSelected(long index);
	long    columnWidth(long index);
	long    rowHeight(long index);
	bool    drawGrid();
	bool    getBorder();
	int     selectionMode();
	void    setSelectionMode(int vl);
	void    setColumnCount(long vl);
	void	setRowCount(long vl);
	void    setColumnResizable(long index,bool vl);
	void    setRowResizable(long index,bool vl);
	void    setRowSelected(long index,bool vl);
	void    setColumnWidth(long index,int vl);
	void    setRowHeight(long index,int vl);
	void    setDrawGrid(bool vl);
	void    setBorder(bool vl);

	int     visibleTop();
	int     visibleLeft();
	int     visibleWidth();
	int     visibleHeight();
	int     scrollBar();
	int     scrollX();
	int     scrollY();
	void    setScrollX(int vl);
	void    setScrollY(int vl);
	void    setScrollBar(int vl);

	long    rowAt(int x);
	long    columnAt(int y);
	int     itemX(long col);
	int     itemY(long row);
	int     itemW(long col);
	int     itemH(long row);
	char*   itemText(long row,long col);
	int     itemFg(long row,long col);
	int     itemBg(long row,long col);
	int     itemYPad(long row,long col);
	int     itemXPad(long row,long col);
	bool    itemSelected(long row,long col);
	void    ensureVisible(long row,long col);
	
	bool	headersVisible();
	bool	footersVisible();
	bool	rowSeparatorVisible();
	char*   headerText(long col);
	char*   footerText(long col);
	char*   rowText(long col);
	void    setHeadersVisible(bool vl);
	void    setRowSeparatorVisible(bool vl);
	void    setFootersVisible(bool vl);
	void    setHeaderText(long col,char *value);
	void    setFooterText(long col,char *value);
	void    setRowText(long col,char *value);

	void    setItemXPad(long row,long col,int vl);
	void    setItemYPad(long row,long col,int vl);
	void    setItemFg(long row,long col,int vl);
	void    setItemBg(long row,long col,int vl);
	void    setItemText(long row,long col,char *vl);
        void    setItemSelected(long row,long col,bool vl);

//"Methods"
	void    setDataFunc(void *func,void *data);
	void    queryUpdate(long row,long col);
	void    clearItem(long row,long col);
	void    getGridCursor(long *row,long *col);
	void    setGridCursor(long row,long col);

//"Events"
	void      (*onScroll)(gGridView *sender);
	void      (*onActivate)(gGridView *sender,long row,long col);
        void      (*onClick)(gGridView *sender,long row,long col);
	void      (*onColumnClick)(gGridView *send,long col);
	void      (*onFooterClick)(gGridView *send,long col);
	void      (*onRowClick)(gGridView *sender,long row);

//"Private"
	int           sel_mode;
	int           sel_row;
	int           mouse_x;
	int           mouse_y;
	int           last_x;
	int           last_y;
	int           scroll;
	long          cursor_col;
	long          cursor_row;
	gTableRender  *render;
	GtkWidget     *header;
	GtkWidget     *footer;
	GtkWidget     *lateral;
	GHashTable    *hdata;
	GHashTable    *vdata;
	void          calculateBars();
};

class gTreeView : public gControl
{
public:
	gTreeView(gControl *parent);
	~gTreeView();

//"Properties"
	unsigned  long count();
	long      itemChildren(char *key);
	char*     itemText(char *key);
	char*     itemParent(char *key);
	void      itemSetText(char *key,char *vl);
	gPicture* itemPicture(char *key);
	void      itemSetPicture(char *key,gPicture *vl);
	bool      itemSelected(char *key);
	int       mode();
	void      setMode(int vl);
	bool       getBorder();
	void      setBorder(bool vl);
	long      visibleWidth();
	long      visibleHeight();
	long      scrollBar();
	void      setScrollBar(long vl);
	bool      sorted();
	void      setSorted(bool vl);

	char*     current();
	char*     firstItem();
	char*     lastItem();
	char*     nextItem(char *vl);
	char*     prevItem(char *vl);

//"Methods"
	bool      add(char *key,char *text,gPicture *pic=NULL,char *after=NULL,char *parent=NULL);
	bool      remove(char *key);
	bool      exists(char *key);
	char*     find(long x,long y);
	void      clear();

//"Events"
	void      (*onActivate)(gTreeView *sender);
	void      (*onSelect)(gTreeView *sender);
	void      (*onClick)(gTreeView *sender);

//"Private"
	gTree *tree;
};

class gIconView : public gControl
{
public:
	gIconView(gControl *parent);

//"Methods"
	void resize(long w,long h);
	int add(char *key,char *after);
	bool exists(char *key);
	void remove(char *key);
	void selectAll(bool vl);
	void clear();
	void sort();
	void ensureVisible(char *key);
	void moveItem(char *key,long x,long y);
	void renameItem(char *key);

//"Properties"
	long      clientWidth();
	long      clientHeight();
	int       selMode();
	void      setSelMode(int vl);
	long      backGround();
	void      setBackGround(long vl);
	char*     firstKey();
	char*     nextKey(char *vl);
	bool      getBorder();
	void      setBorder(bool vl);
	int       scrollBar();
	void      setScrollBar(int vl);
	int       arrange();
	void      setArrange(int vl);
	long      count();
	long      gridX();
	long      gridY();
	void      setGrid(long x,long y);
	char*     current();

	long      itemLeft(char *key);
	long      itemTop(char *key);
	long      itemWidth(char *key);
	long      itemHeight(char *key);
	bool      itemEditable(char *key);
	void      setItemEditable(char *key,bool vl);
	bool      itemSelected(char *key);
	void      setItemSelected(char *key,bool vl);
	char*     itemText(char *key);
	void      setItemText(char *key,char *vl);
	gPicture* itemPicture(char *key);
	void      setItemPicture(char *key,gPicture *vl);

//"Signals"
	void (*onSelect)(gIconView *sender);
	void (*onActivate)(gIconView *sender);
	void (*onClick)(gIconView *sender);
	void (*onRename)(gIconView *sender);


//"Private"
	long grx,gry;
	int  arr;
	int  smode;
	char *adding;
	char *currkey;
	GHashTable *elements;
	GtkListStore *store;

	void drawElements();
	bool iterKey(char *key, GtkTreeIter *iter);
};

class gListBox : public gControl
{
public:
	gListBox(gControl *parent);

//"Properties"
	long   foreGround();
	long   backGround();
	long   count();
	long   index();
	bool   itemSelected(long ind);
	char*  itemText(long ind);
	char** list();
	long   mode();
	bool   sorted();
	char   *text();

	void setForeGround(long vl);
	void setBackGround(long vl);
	void setIndex(long vl); //TODO
	void setItemSelected(long ind,bool vl);
	void setItemText(long ind,char *txt); //TODO
	void setList(char **vl);
	void setMode(long vl);
	void setSorted(bool vl);

//"Methods"
	void add(char *vl,long pos);
	void clear();
	long find(char *ptr);
	void remove(long pos);
//"Events"
	void (*onSelect)(gListBox *data);
	void (*onActivate)(gListBox *data);

//"Private"
	long selmode;
	bool sort;
};

class gSpinBox : public gControl
{
public:
	gSpinBox(gControl *parent);

//"Properties"
	long backGround();
	long foreGround();
	long maxValue();
	long minValue();
	long step();
	long value();
	bool wrap();

	void setBackGround(long vl);
	void setForeGround(long vl);
	void setMaxValue  (long vl);
	void setMinValue  (long vl);
	void setStep      (long vl);
	void setValue     (long vl);
	void setWrap      (bool vl);

//"Signals"
	void (*onChange)  (gSpinBox *sender);

};

class gComboBox : public gControl
{
public:
	gComboBox(gControl *parent);

	long backGround();
	long foreGround();
	long count();
	long index();
	char* itemText(long ind);
	long length();
	char** list();
	bool readOnly();
	bool sorted();
	char* text();

	void setBackGround(long vl);
	void setForeGround(long vl);
	void setIndex(long vl);
	void setItemText(long ind,char *txt);
	void setList(char **vl);
	void setReadOnly(bool vl);
	void setSorted(bool vl);
	void setText(char *vl);

//"Methods"
	void popup();
	void resize(long w,long h);
	void add(char *vl,long pos);
	void clear();
	long find(char *ptr);
	void remove(long pos);

//"Signals"
	void (*onChange)(gComboBox *sender);

//"Private"
	bool sort;
	GtkCellRenderer *cell;

};

class gTextBox : public gControl
{
public:
	gTextBox(gControl *parent);

//"Properties"
	int alignment();
	long backGround();
	long foreGround();
	bool hasBorder();
	long length();
	int maxLength();
	bool password();
	int position();
	bool readOnly();
	int selLength();
	int selStart();
	char* selText();
	char* text();

	void setAlignment(int vl);
	void setBackGround(long vl);
	void setBorder(bool vl);
	void setForeGround(long vl);
	void setMaxLength(int len);
	void setPassword(bool vl);
	void setPosition(int pos);
	void setReadOnly(bool vl);
	void setSelText(char *txt,int len);
	void setText(char *vl);

//"Methods"
	void insert(char* txt,int len);
	void selClear();
	void select(int start,int len);
	void selectAll();

//"Signals"
	void (*onChange)(gTextBox *sender);
	void (*onActivate)(gTextBox *sender);

};

class gTextArea : public gControl
{
public:
	gTextArea(gControl *parent);

//"Properties"
	bool hasBorder();
	long backGround();
	long column();
	long foreGround();
	long length();
	long line();
	long position();
	bool readOnly();
	long scrollBar();
	char* text();
	bool wrap();

	void setBackGround(long vl);
	void setBorder(bool vl);
	void setColumn(long vl);
	void setForeGround(long vl);
	void setLine(long vl);
	void setPosition(long vl);
	void setReadOnly(bool vl);
	void setScrollBar(long vl);
	void setText(char *txt);
	void setWrap(bool vl);

//"Methods"
	void copy();
	void cut();
	void ensureVisible();
	void paste();
	void insert(char *txt);
	long toLine(long pos);
	long toColumn(long pos);
	long toPosition(long line,long col);

//"Selection properties"
	long selStart();
	long selEnd();
	char* selText();

	void setSelText(char *vl);

//"Selection methods"
	void selDelete();
	void selSelect(long start,long length);
};

class gSlider : public gControl
{
public:
	gSlider(gControl *parent);

//"Properties"
	long foreGround();
	long backGround();
	long max();
	long min();
	bool tracking();
	long value();
	bool mark();
	long step();
	long pageStep();

	void setForeGround(long vl);
	void setBackGround(long vl);
	void setMax(long vl);
	void setMin(long vl);
	void setTracking(bool vl);
	void setValue(long vl);
	void setMark(bool vl);
	void setStep(long vl);
	void setPageStep(long vl);

//"Signals"
	void (*onChange)(gSlider *sender);

//"Private"
	void orientation(long w,long h);
	bool bDraw;
	long p_step;
	long p_page;
};

class gScrollBar : public gSlider
{
public:
	gScrollBar(gControl *parent);

//"Private"
	void orientation(long w,long h);
};

typedef struct
{
	int mode;
	long spacing;
	long padding;
	bool autoresize;
	bool locked;
	bool user;

} gContainerArrangement;

class gContainer : public gControl
{
public:
	gContainer();
	gContainer(gControl *parent);
	~gContainer();

	long arrange();
	bool autoSize();
	bool user();
	long padding();
	long spacing();
	virtual long clientWidth();
	virtual long clientHeight();
	virtual long clientX();
	virtual long clientY();

	void setArrange(long vl);
	void setUser(bool vl);
	void setAutoSize(bool vl);
	void setPadding(long vl);
	void setSpacing(long vl);

	virtual long childCount();
	virtual gControl* child(long index);

//"Private"
	gContainerArrangement arrangement;
	GtkWidget *radiogroup;
	virtual void performArrange();
	GList *ch_list;
};

class gSplitter : public gContainer
{
public:
	gSplitter(gControl *parent,int type); // 0->Horizontal, 1->Vertical

//"Properties"
	char* layout();
	void  setLayout(char *vl);

	long childCount();
	gControl* child(long index);

//"Signals"
	void (*onResize)(gControl *sender);

//"Private"
	GtkPaned *curr;
	void addWidget(GtkWidget *w);


};

class gScrollView : public gContainer
{
public:
	gScrollView(gControl *parent);

//"Properties"
	long clientX();
	long clientY();
	long clientWidth();
	long clientHeight();
	long scrollX();
	void setScrollX(long vl);
	long scrollY();
	void setScrollY(long vl);
	long scrollBar();
	void setScrollBar(long vl);
	long foreGround();
	long backGround();
	void setForeGround(long vl);
	void setBackGround(long vl);
	int getBorder();
	void setBorder(int vl);
	long width();
	long height();
	void setWidth(long vl);
	void setHeight(long vl);
//"Methods"
	void move(long x,long y);
	void resize(long w,long h);
	void scroll(long x,long y);
	void ensureVisible(long x,long y,long w,long h);
//"Events"
	void (*onScroll)(gScrollView *sender);
//"Private"
	void performArrange();

};

class gDrawingArea : public gContainer
{
public:
	gDrawingArea(gControl *parent);

	long foreGround();
	long backGround();
	int getBorder();
	bool cached();
	bool canFocus();
	bool getTracking();

	void setForeGround(long vl);
	void setBackGround(long vl);
	void setBorder(int vl);
	void setCached(bool vl);
	void setCanFocus(bool vl);
	void setTracking(bool vl);

//"Methods"
	void clear();
	void resize(long w,long h);
//"Events"
	void (*onExpose)(gDrawingArea *sender,long x,long y,long w,long h);

//"Private"
	GdkPixmap *buffer;
	GdkGC *gc;
	int  btype;
	bool track;
	bool berase;

};

class gTabStrip : public gContainer
{
public:
	gTabStrip(gControl *parent);

//"Properties"
	long foreGround();
	long backGround();
	long count();
	long index();
	long orientation();
	gPicture* picture(long ind);
	void setOrientation(long vl);
	bool tabEnabled(long vl);
	char *text(long ind);

	void setForeGround(long vl);
	void setBackGround(long vl);
	int setCount(long vl);
	void setIndex(long vl);
	void setPicture(long ind,gPicture *pic);
	void setTabEnabled(long ind,bool vl);
	void setText(long ind,char *txt);

//"Events"
	void (*onClick)(gTabStrip *sender);

};

class gFrame : public gContainer
{
public:
	gFrame(gControl *parent);

	long backGround();
	void setBackGround(long vl);
	long foreGround();
	void setForeGround(long vl);
	char* text();
	void setText(char* vl);
	int getBorder();
	void setBorder(int vl);

//"Methods"
	void resize(long w,long h);

};


class gMenu
{
public:
	gMenu(gMainWindow *par,bool hidden);
	gMenu(gMenu *par,bool hidden);
	~gMenu();

	void *hFree;

	static long   winChildCount(gMainWindow *win);
	static gMenu* winChildMenu(gMainWindow *par,long pos);
//"Properties"
	bool checked();
	bool enabled();
	gMenu* childMenu(long pos);
	long childCount();
	char* shortcut();
	char* text();
	bool visible();
	gPicture* picture();

	void setChecked(bool vl);
	void setEnabled(bool vl);
	void setShortcut(char *txt);
	void setText(char *vl);
	void setVisible(bool vl);
	void setPicture(gPicture *pic);

//"Methods"
	void popup();
	void destroy();

// "Signals"
	void (*onFinish)(gMenu *sender); // Special
	void (*onClick)(gMenu *sender);

//"Private"
	void *pr;
	GdkPixbuf *img_buf;
	bool chk;
	bool top_level;
	bool stop_signal;
	char *txt;
	char *scut;
	GtkAccelGroup *accel;
	GtkMenu *child;
	GtkMenuItem *menu;
	GtkWidget *lbl;
	GtkWidget *aclbl;
	GtkWidget *img;
	GtkSizeGroup *sizeGroup;
	void setImage(GdkPixbuf *buf);
};

class gMainWindow : public gContainer
{
public:
	gMainWindow(long plug);
	gMainWindow(gControl *parent);
	~gMainWindow();

//"Properties"
	int _border();
	gPicture *icon();
	gPicture *picture();
	bool mask();
	long menuCount();
	bool modal();
	const char* text();
	bool topOnly();
	bool skipTaskBar();
	bool minimized();
	bool maximized();
	bool fullscreen();
	long backGround();
	bool getSticky();
	int  getStacking();

	void setBorder(int b);
	void setIcon(gPicture *pic);
	void setMask(bool vl);
	void setPicture(gPicture *pic);
	void setText(const char *txt);
	void setTopOnly(bool vl);
	void setSkipTaskBar(bool b);
	void setMinimized(bool vl);
	void setMaximized(bool vl);
	void setFullscreen(bool vl);
	void setBackGround(long vl);
	void setSticky(bool vl);
	void setStacking(int vl);

//"Methods"
	void center();
	void showModal();
	void raise();
	void move(long x,long y);
	void show();

//"Signals"
	void (*onOpen)(gMainWindow *sender);
	void (*onShow)(gMainWindow *sender);
	void (*onHide)(gMainWindow *sender);
	void (*onMove)(gMainWindow *sender);
	void (*onResize)(gMainWindow *sender);
	bool (*onClose)(gMainWindow *sender);

//"Private"
	void drawMask();
	GdkPixbuf *buf_mask;
	bool m_mask;
	bool top_only;
	bool resize_flag;
	GtkAccelGroup *accel;
	GtkStyle *win_style;
	GtkMenuBar *menuBar;
	GtkWidget *esc_button;
	bool sticky;
	int  stack;
//	gint bufW,bufH,bufX,bufY;
};

class gApplication
{
public:
	static void init(int *argc,char ***argv);
	static void exit();
	static void iteration();

	static long controlCount();
	static long winCount();

	static gControl* controlItem(long index);
	static gControl* controlItem(GtkWidget *wid);
	static gMainWindow *winItem(long index);

//
	static void suspendEvents(bool vl);
	static void enableEvents();
	static bool userEvents();
	static bool allEvents();

	static void enableTooltips(bool vl);
	static bool toolTips();
	static gFont* toolTipsFont();
	static void setToolTipsFont(gFont *ft);

//"Private"
	static GtkTooltips *tipHandle();
};


#endif
