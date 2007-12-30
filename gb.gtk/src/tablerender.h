#include <gtk/gtk.h>

#ifndef TABLERENDER

#define TABLERENDER

typedef struct {
	long row;
	long col;
} gTablePair;

class gTableData
{
public:
	char *text;
	void *picture;
	void *font;
	int  alignment;
	int  xpad;
	int  ypad;
	int  bg;
	int  fg;

	void setText(char *vl);
	void clear();
	gTableData();
	~gTableData();
};


class gTable
{
	int *colsize;
	int *rowsize;
	long columns;
	long rows;
	
protected:
	GHashTable *data;
	GHashTable *seldata;

public:
	gTable();
	~gTable();

	long columnCount   ();
	long rowCount      ();
	void          setRowCount      (long number);
	void          setColumnCount   (long number);
	char*         getField         (long col,long row);
	void          setField         (long col,long row,char* value);

	int           getColumnSize    (long position);
	void          setColumnSize    (long position,int value);
	int           getRowSize       (long position);
	void          setRowSize       (long position,int value);
	bool          getRowSelected   (long position);
	void          setRowSelected   (long position,bool value);
	
	void          clearField       (long col,long row);
	int           getFieldFg       (long col,long row);
	void          setFieldFg       (long col,long row,int value);
	int           getFieldBg       (long col,long row);
	void          setFieldBg       (long col,long row,int value);
	int           getFieldXPad     (long col,long row);
	void          setFieldXPad     (long col,long row,int value);
	int           getFieldYPad     (long col,long row);
	void          setFieldYPad     (long col,long row,int value);
	bool          getFieldSelected (long col,long row);
	void          setFieldSelected (long col,long row,bool value);
};

class gTableRender : public gTable
{
	GtkWidget            *sf;
	GtkCellRendererText  *txt;
	GtkCellRendererPixbuf *pix;
	int                   offX;
	int                   offY;
	bool                  grid;

	void          convert_color (GdkColor *resul,int origin);
	void          render_cell(gTableData *data,GdkGC *gc,GdkRectangle *rect,bool sel);

public:
	gTableRender  (GtkWidget *surface);
	~gTableRender ();

	int           visibleWidth      ();
	int           visibleHeight     ();
	int           width             ();
	int           height            ();
	int           getOffsetX        ();
	int           getOffsetY        ();
	bool          drawGrid          ();
	void          setOffsetX        (int vl);
	void          setOffsetY        (int vl);
	void          render            (GdkRectangle *ar=NULL);

	void          clearSelection    ();
	void          queryUpdate       (long row,long col);
	void          setDrawGrid       (bool vl);
	void          setRowSize        (long position,int value);
	void          setRowSelected    (long position,bool value);
	void          setColumnSize     (long position,int value);
	void          clearField        (long col,long row);
	void          setField          (long col,long row,char* value);
	void          setFieldFg        (long col,long row,int value);
	void          setFieldBg        (long col,long row,int value);
	void          setFieldXPad      (long col,long row,int value);
	void          setFieldYPad      (long col,long row,int value);
	void          setFieldSelected  (long col,long row,bool value);
	void          (*voidCell)       (gTableData **fill,long row,long col,void *user);
	void          *userData;
};

#endif
