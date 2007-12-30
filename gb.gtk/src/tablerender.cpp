#include <gtk/gtk.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "tablerender.h"

//***********************************
// gTableData
//***********************************
gTableData::gTableData()
{	
	text=NULL;
	picture=NULL;
	font=NULL;
	bg=0xFFFFFF;
	fg=0;
	xpad=0;
	ypad=0;
}

gTableData::~gTableData()
{
	if (text) g_free(text);
}

void gTableData::setText(char *vl)
{
	if (!vl) vl="";
	if (text) g_free(text);
	text=(char*)g_malloc(sizeof(char)*strlen(vl));
	if (text) strcpy(text,vl);
}

void gTableData::clear()
{
	if (text) g_free(text);
	if (picture) g_object_unref(G_OBJECT(picture));
	if (font) delete font;
	text=NULL;
	picture=NULL;
	font=NULL;
	bg=0xFFFFFF;
	fg=0;
	xpad=0;
	ypad=0;
	
}

//***********************************
// gTable
//***********************************
gboolean gTable_ecol(gTablePair *key,gTableData *data,long number)
{
	if ( (key->row == number) && (key->col!=-1) ) return true;
	return false;
}

gboolean gTable_remove_row(gTablePair *key,gTableData *data,long number)
{
	if (key->row >= number) return true;
	return false;
}

gboolean gTable_remove_col(gTablePair *key,gTableData *data,long number)
{
	if (key->col >= number) return true;
	return false;
}

gboolean gTable_equal (gTablePair *a, gTablePair *b)
{
	if ( a->col != b->col) return false;
	if ( a->row != b->row) return false;
	return true;
}

void gTable_ekey(gTablePair *pair)
{
	if (pair) g_free(pair);
}

void gTable_edata(gTableData *data)
{
	if (data) delete data;
}

gTable::gTable()
{
	colsize=NULL;
	rowsize=NULL;
	columns=0;
	rows=0;
	seldata=g_hash_table_new_full((GHashFunc)g_int_hash,(GEqualFunc)gTable_equal,
                         (GDestroyNotify)gTable_ekey,NULL);
	data=g_hash_table_new_full((GHashFunc)g_int_hash,(GEqualFunc)gTable_equal,
                         (GDestroyNotify)gTable_ekey,(GDestroyNotify)gTable_edata);
}

gTable::~gTable()
{
	long bc;

	g_hash_table_destroy(data);
	g_hash_table_destroy(seldata);
	if (rowsize) g_free(rowsize);
	if (colsize) g_free(colsize);
}


long gTable::columnCount()
{
	return columns;
}

long gTable::rowCount()
{
	return rows;
}

void gTable::setRowCount(long number)
{
	int bc;

	if (number<0) number=0;
	if (number==rows) return; 

	if (number>rows)
	{
		if (!rows) rowsize=(int*)g_malloc(sizeof(int)*number);
		else       rowsize=(int*)g_realloc(rowsize,sizeof(int)*number);

		for (bc=rows; bc<number; bc++) rowsize[bc]=20;
		rows=number;
	}
	else
	{
		if (!number) { g_free(rowsize); rowsize=NULL; }
		else         rowsize=(int*)g_realloc(rowsize,sizeof(int)*number);

		g_hash_table_foreach_remove (data,(GHRFunc)gTable_remove_row,(gpointer)number);
		g_hash_table_foreach_remove (seldata,(GHRFunc)gTable_remove_row,(gpointer)number);
		rows=number;
	}
}


void gTable::setColumnCount(long number)
{
	int bc;

	if (number<0) number=0;
	if (number==columns) return; 

	if (number>columns)
	{
		if (!columns) colsize=(int*)g_malloc(sizeof(int)*number);
		else          colsize=(int*)g_realloc(colsize,sizeof(int)*number);

		for (bc=columns; bc<number; bc++) colsize[bc]=100;
		columns=number;
	}
	else
	{
		if (!number) { g_free(colsize); colsize=NULL; }
		else         colsize=(int*)g_realloc(colsize,sizeof(int)*number);

		g_hash_table_foreach_remove (data,(GHRFunc)gTable_remove_col,(gpointer)number);
		g_hash_table_foreach_remove (seldata,(GHRFunc)gTable_remove_col,(gpointer)number);
		columns=number;
	}
}

void gTable::clearField (long col,long row)
{
	gTableData *ptr;
	gTablePair pair={row,col};

	if ( (col<0) || (col>=columns) ) return;
	if ( (row<0) || (row>=rows) ) return;

	ptr=(gTableData*)g_hash_table_lookup(data,(gpointer)&pair);
	if (!ptr) return;
	g_hash_table_remove(data,(gpointer)&pair);
	
}

char* gTable::getField (long col,long row)
{
	gTableData *ptr;
	gTablePair pair={row,col};

	if ( (col<0) || (col>=columns) ) return NULL;
	if ( (row<0) || (row>=rows) ) return NULL;

	ptr=(gTableData*)g_hash_table_lookup(data,(gpointer)&pair);
	if (!ptr) return NULL;
	return ptr->text;
}

void gTable::setField (long col,long row,char* value)
{
	gTableData *ptr;
	gTablePair pair={row,col};
	gTablePair *key;

	if ( (col<0) || (col>=columns) ) return;
	if ( (row<0) || (row>=rows) ) return;

	if (!value) value="";

	ptr=(gTableData*)g_hash_table_lookup(data,(gpointer)&pair);
	if (ptr) 
	{
		if (ptr->text) g_free(ptr->text);
		ptr->text=(char*)g_malloc(sizeof(char)*(strlen(value)+1));
		strcpy(ptr->text,value);
		return;
	}

	key=(gTablePair*)g_malloc(sizeof(gTablePair));
	key->row=row;
	key->col=col;
	ptr=new gTableData();
	ptr->text=(char*)g_malloc(sizeof(char)*(strlen(value)+1));
	strcpy(ptr->text,value);
	g_hash_table_insert(data,(gpointer)key,(gpointer)ptr);
}

int gTable::getFieldFg (long col,long row)
{
	gTableData *ptr;
	gTablePair pair={row,col};

	if ( (col<0) || (col>=columns) ) return 0;
	if ( (row<0) || (row>=rows) ) return 0;

	ptr=(gTableData*)g_hash_table_lookup(data,(gpointer)&pair);
	if (!ptr) return 0;
	return ptr->fg;
}

void gTable::setFieldFg (long col,long row,int value)
{
	gTableData *ptr;
	gTablePair pair={row,col};
	gTablePair *key;

	if ( (col<0) || (col>=columns) ) return;
	if ( (row<0) || (row>=rows) ) return;

	ptr=(gTableData*)g_hash_table_lookup(data,(gpointer)&pair);
	if (ptr) 
	{
		ptr->fg=value;
		return;
	}

	key=(gTablePair*)g_malloc(sizeof(gTablePair));
	key->row=row;
	key->col=col;
	ptr=new gTableData();
	ptr->fg=value;
	g_hash_table_insert(data,(gpointer)key,(gpointer)ptr);
}

int gTable::getFieldBg (long col,long row)
{
	gTableData *ptr;
	gTablePair pair={row,col};

	if ( (col<0) || (col>=columns) ) return 0;
	if ( (row<0) || (row>=rows) ) return 0;

	ptr=(gTableData*)g_hash_table_lookup(data,(gpointer)&pair);
	if (!ptr) return 0xFFFFFF;
	return ptr->bg;
}

void gTable::setFieldBg (long col,long row,int value)
{
	gTableData *ptr;
	gTablePair pair={row,col};
	gTablePair *key;

	if ( (col<0) || (col>=columns) ) return;
	if ( (row<0) || (row>=rows) ) return;

	ptr=(gTableData*)g_hash_table_lookup(data,(gpointer)&pair);
	if (ptr) 
	{
		ptr->bg=value;
		return;
	}

	key=(gTablePair*)g_malloc(sizeof(gTablePair));
	key->row=row;
	key->col=col;
	ptr=new gTableData();
	ptr->bg=value;
	g_hash_table_insert(data,(gpointer)key,(gpointer)ptr);
}

int gTable::getFieldXPad (long col,long row)
{
	gTableData *ptr;
	gTablePair pair={row,col};

	if ( (col<0) || (col>=columns) ) return 0;
	if ( (row<0) || (row>=rows) ) return 0;

	ptr=(gTableData*)g_hash_table_lookup(data,(gpointer)&pair);
	if (!ptr) return 0xFFFFFF;
	return ptr->xpad;
}

void gTable::setFieldXPad (long col,long row,int value)
{
	gTableData *ptr;
	gTablePair pair={row,col};
	gTablePair *key;

	if ( (col<0) || (col>=columns) ) return;
	if ( (row<0) || (row>=rows) ) return;

	ptr=(gTableData*)g_hash_table_lookup(data,(gpointer)&pair);
	if (ptr) 
	{
		ptr->xpad=value;
		return;
	}

	key=(gTablePair*)g_malloc(sizeof(gTablePair));
	key->row=row;
	key->col=col;
	ptr=new gTableData();
	ptr->xpad=value;
	g_hash_table_insert(data,(gpointer)key,(gpointer)ptr);
}

int gTable::getFieldYPad (long col,long row)
{
	gTableData *ptr;
	gTablePair pair={row,col};

	if ( (col<0) || (col>=columns) ) return 0;
	if ( (row<0) || (row>=rows) ) return 0;

	ptr=(gTableData*)g_hash_table_lookup(data,(gpointer)&pair);
	if (!ptr) return 0xFFFFFF;
	return ptr->ypad;
}

void gTable::setFieldYPad (long col,long row,int value)
{
	gTableData *ptr;
	gTablePair pair={row,col};
	gTablePair *key;

	if ( (col<0) || (col>=columns) ) return;
	if ( (row<0) || (row>=rows) ) return;

	ptr=(gTableData*)g_hash_table_lookup(data,(gpointer)&pair);
	if (ptr) 
	{
		ptr->ypad=value;
		return;
	}

	key=(gTablePair*)g_malloc(sizeof(gTablePair));
	key->row=row;
	key->col=col;
	ptr=new gTableData();
	ptr->ypad=value;
	g_hash_table_insert(data,(gpointer)key,(gpointer)ptr);
}

bool gTable::getRowSelected(long row)
{
	gTablePair pair={row,-1};

	if ( (row<0) || (row>=rows) ) return false;

	if (g_hash_table_lookup(seldata,(gpointer)&pair)) return true;
	return false;
}

void gTable::setRowSelected(long row,bool value)
{
	gTablePair pair={row,-1};
	gTablePair *key;

	if ( (row<0) || (row>=rows) ) return;

	if (g_hash_table_lookup(seldata,(gpointer)&pair))
	{
		if (value) return;
		g_hash_table_remove(seldata,(gpointer)&pair);
	}
	
	if (value)
	{
		key=(gTablePair*)g_malloc(sizeof(gTablePair));
		key->row=row;
		key->col=-1;
		g_hash_table_insert(seldata,(gpointer)key,(gpointer)true);
		
	}
	
	if (!value) g_hash_table_foreach_remove(seldata,(GHRFunc)gTable_ecol,(gpointer)row);

}

bool gTable::getFieldSelected (long col,long row)
{
	bool value;
	gTablePair pair={row,col};

	if ( (col<0) || (col>=columns) ) return false;
	if ( (row<0) || (row>=rows) ) return false;

	if (g_hash_table_lookup(seldata,(gpointer)&pair)) return true;
	pair.col=-1;
	if (g_hash_table_lookup(seldata,(gpointer)&pair)) return true;
	return false;
}

void gTable::setFieldSelected (long col,long row,bool value)
{
	gTablePair pair={row,col};
	gTablePair *key;
	long bc;

	if ( (col<0) || (col>=columns) ) return;
	if ( (row<0) || (row>=rows) ) return;

	if (g_hash_table_lookup(seldata,(gpointer)&pair))
	{
		if (value) return;
		g_hash_table_remove(seldata,(gpointer)&pair);
		pair.col=-1;
		if (g_hash_table_lookup(seldata,(gpointer)&pair))
			g_hash_table_remove(seldata,(gpointer)&pair);
		return;
	}
	
	if (!value)
	{
		pair.col=-1;
		if (g_hash_table_lookup(seldata,(gpointer)&pair))
		{
			g_hash_table_remove(seldata,(gpointer)&pair);
			for (bc=0; bc<columnCount(); bc++)
				if (bc != col) setFieldSelected(bc,row,true);
		}
		return;
	}

	key=(gTablePair*)g_malloc(sizeof(gTablePair));
	key->row=row;
	key->col=col;
	g_hash_table_insert(seldata,(gpointer)key,(gpointer)value);

	
	
}

int gTable::getColumnSize  (long position)
{
	if (position>=columns) return -1;
	if (position<0) return -1;

	return colsize[position];
}

void gTable::setColumnSize  (long position,int value)
{
	if (position>=columns) return;
	if (position<0) return;

	if (value<0) value=0;
	colsize[position]=value;
}

int gTable::getRowSize (long position)
{
	if (position>=rows) return -1;
	if (position<0) return -1;

	return rowsize[position];
}

void gTable::setRowSize (long position,int value)
{
	if (position>=rows) return;
	if (position<0) return;

	if (value<0) value=0;
	rowsize[position]=value;
}


//***********************************
// gTableRender
//***********************************
gboolean tbrender_expose(GtkWidget *wid,GdkEventExpose *e,gTableRender *data)
{
	data->render(&e->area);
	return FALSE;
}

gTableRender::gTableRender(GtkWidget *surface)
{
	voidCell=NULL;
	userData=NULL;
	grid=true;
	txt=(GtkCellRendererText*)gtk_cell_renderer_text_new();
	pix=(GtkCellRendererPixbuf*)gtk_cell_renderer_pixbuf_new();
	g_object_set(G_OBJECT(txt),"foreground-set",TRUE,0);
	g_object_set(G_OBJECT(txt),"background-set",TRUE,0);
	sf=surface;
	offX=0;
	offY=0;
	if (sf) 
	{
		g_object_ref(G_OBJECT(sf));
		g_signal_connect(G_OBJECT(sf),"expose-event",G_CALLBACK(tbrender_expose),this);
	}
}

void gTableRender::queryUpdate(long row,long col)
{
	GdkRectangle rect={-getOffsetX(),-getOffsetY(),0,0};
	int bc;

	if (row>=rowCount()) return;
	if (col>=columnCount()) return;

	for (bc=0; bc<col;bc++) rect.x+=getColumnSize(bc);
	for (bc=0; bc<row;bc++) rect.y+=getRowSize(bc);
	rect.width=getColumnSize(bc);
	rect.height=getRowSize(bc);

	if (col==-1)
	{
		rect.x=0;
		gdk_drawable_get_size(sf->window,&rect.width,NULL);
	}

	gdk_window_invalidate_rect(sf->window,&rect,TRUE); 
}

gTableRender::~gTableRender()
{
	g_object_unref(G_OBJECT(pix));
	g_object_unref(G_OBJECT(txt));
	if (sf) g_object_unref(G_OBJECT(sf));	
}

void gTableRender::convert_color(GdkColor *resul,int color)
{
	GdkColormap *cmap=gdk_colormap_get_system();

	resul->red=0xFF + ((color & 0xFF0000)>>8);
	resul->green=0xFF + (color & 0x00FF00);
	resul->blue=0xFF + ((color & 0x0000FF)<<8);
	gdk_color_alloc(cmap,resul);
}

bool gTableRender::drawGrid()
{
	return grid;
}

void gTableRender::setDrawGrid(bool vl)
{
	GdkRectangle rect={0,0,0,0};

	if (grid == vl) return;

	grid=vl;
	gdk_drawable_get_size (sf->window,&rect.width,&rect.height);
	gdk_window_invalidate_rect(sf->window,&rect,TRUE); 
}

int gTableRender::visibleWidth()
{
	int vl;

	if (!sf) return 0;
	if (!sf->window) return 0;
	gdk_drawable_get_size (sf->window,&vl,NULL);
	return vl;
}

int gTableRender::visibleHeight()
{
	int vl;

	if (!sf) return 0;
	if (!sf->window) return 0;
	gdk_drawable_get_size (sf->window,NULL,&vl);
	return vl;
}

int gTableRender::width()
{
	int bc;
	int sum=0;

	for (bc=0; bc<columnCount(); bc++) sum+=getColumnSize(bc);
	return sum;
}

int gTableRender::height()
{
	int bc;
	int sum=0;

	for (bc=0; bc<rowCount(); bc++) sum+=getRowSize(bc);
	return sum;
}

int gTableRender::getOffsetX()
{
	return offX;
}

int gTableRender::getOffsetY()
{
	return offY;
}

void gTableRender::setOffsetX(int vl)
{
	GdkRectangle rect={0,0,0,0};
	int diff;

	if (offX != vl) 
	{ 
		diff=offX-vl;
		offX=vl; 
		if (!sf) return;
		if (!sf->window) return;
		gdk_window_scroll(sf->window,diff,0);
	}
}

void gTableRender::setOffsetY(int vl)
{
	GdkRectangle rect={0,0,0,0};
	int diff;

	if (offY != vl) 
	{ 
		diff=offY-vl;
		offY=vl; 
		if (!sf) return;
		if (!sf->window) return;
		gdk_window_scroll(sf->window,0,diff);
	}
}

void gTableRender::render_cell(gTableData *data,GdkGC *gc,GdkRectangle *rect,bool sel)
{
	GdkColor gcol;
	GtkSettings *set;
	GtkStyle *st;
	char *buf=data->text;

	if (!buf) buf="";

	g_object_set(G_OBJECT(txt),"text",buf,NULL);
	
	if (sel)
	{
		set=gtk_settings_get_default();
		st=gtk_rc_get_style_by_paths(set,NULL,"GtkEntry",GTK_TYPE_ENTRY);
		g_object_set(G_OBJECT(txt),"foreground-gdk",
                                           &st->text[GTK_STATE_SELECTED],NULL);
		g_object_set(G_OBJECT(txt),"background-gdk",
                                           &st->base[GTK_STATE_SELECTED],NULL);
	}
	else
	{
		convert_color(&gcol,data->fg);
		g_object_set(G_OBJECT(txt),"foreground-gdk",&gcol,NULL);
		convert_color(&gcol,data->bg);
		g_object_set(G_OBJECT(txt),"background-gdk",&gcol,NULL);
	}
	gtk_cell_renderer_render(GTK_CELL_RENDERER(txt),sf->window,sf,rect,rect,NULL,(GtkCellRendererState)0);
}

void gTableRender::render(GdkRectangle *ar)
{
	bool  sel;
	int   bx,by;
	int   bpos;
	int   cpos;
	int   pmaxX;
	int   pmaxY;
	int   bufW=0,bufH=0,bufY=0;
	GdkGC *gc;
	GdkRectangle rect;
	gTableData *cell;
	gTableData *caux;
	gTablePair pair;

	if (!sf) return;
	if (!sf->window) return;

	gdk_window_clear_area(sf->window,ar->x,ar->y,ar->width,ar->height);

	if ( (!rowCount()) || (!columnCount()) ) return; 

	caux=new gTableData();

	// Prepare data
	pmaxX=width();
	pmaxY=height();
	if (pmaxX>visibleWidth()) pmaxX=visibleWidth();
	if (pmaxY>visibleHeight()) pmaxY=visibleHeight();
	bpos=0;

	gc=gdk_gc_new(sf->window);
	gdk_gc_set_background(gc,&sf->style->base[GTK_STATE_NORMAL]);
	if (ar)
	{
		gdk_gc_set_clip_origin(gc,0,0);
		gdk_gc_set_clip_rectangle(gc,ar);
	}
	


	// Rendering texts and pixbufs
	for (bx=0; bx<columnCount(); bx++)
	{
		if ( (bpos+getColumnSize(bx))<offX ) { bpos+=getColumnSize(bx); continue; }
		if ( (bpos-offX)>visibleWidth() ) break; 

		cpos=0;
		for (by=0; by<rowCount(); by++)
		{
			if ( (cpos+getRowSize(by))<offY ) { cpos+=getRowSize(by); continue; }
			if ( (cpos-offY)>visibleHeight() ) break;	

			rect.x=bpos-offX+bufW;
			rect.y=cpos-offY;
			rect.width=getColumnSize(bx)-bufW;
			rect.height=getRowSize(by);

			sel=getFieldSelected(bx,by);

			pair.row=by;
			pair.col=bx;
			cell=(gTableData*)g_hash_table_lookup(data,(gpointer)&pair);

			if (!cell)
			{
				caux->clear();
				if (voidCell) voidCell(&caux,by,bx,userData);
				render_cell(caux,gc,&rect,sel);
			}
			else
				render_cell(cell,gc,&rect,sel);
			
			cpos+=getRowSize(by);
		}

		bpos+=getColumnSize(bx);
	}

	delete caux;
	if (!grid) { g_object_unref(G_OBJECT(gc)); return; }

	// Horizontal lines
	bpos=0;
	gdk_gc_set_foreground(gc,&sf->style->base[GTK_STATE_NORMAL]);
	for (bx=0; bx<rowCount(); bx++)
	{
		if ( bpos>offY )  gdk_draw_line(sf->window,gc,0,bpos-offY,pmaxX,bpos-offY);
		bpos+=getRowSize(bx);
		if ((bpos-offY)>visibleHeight()) break;
	}
	gdk_draw_line(sf->window,gc,0,bpos-offY,pmaxX,bpos-offY);

	// Vertical lines
	bpos=0;
	for (bx=0; bx<columnCount(); bx++)
	{
		if ( bpos>offX )  gdk_draw_line(sf->window,gc,bpos-offX,0,bpos-offX,pmaxY);
		bpos+=getColumnSize(bx);
		if ((bpos-offX)>visibleWidth()) break;
	}
	gdk_draw_line(sf->window,gc,bpos-offX,0,bpos-offX,pmaxY);
	g_object_unref(G_OBJECT(gc));

}

void gTableRender::setRowSize(long position,int value)
{
	GdkRectangle rect={0,0,0,0};
	int sum=-offX,bc;

	if (position<0) return;
	if (position>=rowCount()) return;

	gTable::setRowSize(position,value);
	if (!sf) return;
	if (!sf->window) return;
	gdk_drawable_get_size (sf->window,&rect.width,&rect.height);

	for (bc=0; bc<position; bc++) sum+=getRowSize(bc);

	if ((sum+getRowSize(position))<0) return;
	if (sum>rect.height) return;
		
	gdk_window_invalidate_rect(sf->window,&rect,TRUE);
}

void gTableRender::setColumnSize(long position,int value)
{
	GdkRectangle rect={0,0,0,0};
	int sum=-offY,bc;

	if (position<0) return;
	if (position>=columnCount()) return;

	gTable::setColumnSize(position,value);
	if (!sf) return;
	if (!sf->window) return;
	gdk_drawable_get_size (sf->window,&rect.width,&rect.height);

	for (bc=0; bc<position; bc++) sum+=getColumnSize(bc);

	if ((sum+getColumnSize(position))<0) return;
	if (sum>rect.width) return;
		
	gdk_window_invalidate_rect(sf->window,&rect,TRUE);
}

void gTableRender::clearField (long col,long row)
{
	GdkRectangle rect={0,0,0,0};
	int sum=-offX,bc;

	if (col<0) return;
	if (col>=columnCount()) return;
	if (row<0) return;
	if (row>=rowCount()) return;

	gTable::clearField(col,row);
	queryUpdate(row,col);
}

void gTableRender::setField (long col,long row,char* value)
{
	GdkRectangle rect={0,0,0,0};
	int sum=-offX,bc;

	if (col<0) return;
	if (col>=columnCount()) return;
	if (row<0) return;
	if (row>=rowCount()) return;

	gTable::setField(col,row,value);
	queryUpdate(row,col);
}

void gTableRender::setFieldFg (long col,long row,int value)
{
	GdkRectangle rect={0,0,0,0};
	int sum=-offX,bc;

	if (col<0) return;
	if (col>=columnCount()) return;
	if (row<0) return;
	if (row>=rowCount()) return;

	gTable::setFieldFg(col,row,value);
	queryUpdate(row,col);
}

void gTableRender::setFieldBg (long col,long row,int value)
{
	GdkRectangle rect={0,0,0,0};
	int sum=-offX,bc;

	if (col<0) return;
	if (col>=columnCount()) return;
	if (row<0) return;
	if (row>=rowCount()) return;

	gTable::setFieldBg(col,row,value);
	queryUpdate(row,col);
}

void gTableRender::setFieldXPad (long col,long row,int value)
{
	GdkRectangle rect={0,0,0,0};
	int sum=-offX,bc;

	if (col<0) return;
	if (col>=columnCount()) return;
	if (row<0) return;
	if (row>=rowCount()) return;

	gTable::setFieldXPad(col,row,value);
	queryUpdate(row,col);
}

void gTableRender::setFieldYPad (long col,long row,int value)
{
	GdkRectangle rect={0,0,0,0};
	int sum=-offX,bc;

	if (col<0) return;
	if (col>=columnCount()) return;
	if (row<0) return;
	if (row>=rowCount()) return;

	gTable::setFieldYPad(col,row,value);
	queryUpdate(row,col);
}

void gTableRender::setFieldSelected(long col,long row,bool value)
{
	gTable::setFieldSelected(col,row,value);
	queryUpdate(row,-1);
}

void gTableRender::setRowSelected(long row,bool value)
{
	if (row<0) return;
	if (row>=rowCount()) return;

	gTable::setRowSelected(row,value);
	queryUpdate(row,-1);
}

void gTableRender::clearSelection()
{
	GdkRectangle rect={0,0,0,0};

	g_hash_table_destroy(seldata);
	seldata=g_hash_table_new_full((GHashFunc)g_int_hash,(GEqualFunc)gTable_equal,
                         (GDestroyNotify)gTable_ekey,NULL);

	gdk_drawable_get_size(sf->window,&rect.width,&rect.height);
	gdk_window_invalidate_rect(sf->window,&rect,TRUE);
}
