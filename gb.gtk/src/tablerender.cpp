#include <gtk/gtk.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "gmemory.h"
#include "ggridview.h"
#include "tablerender.h"

//***********************************
// gTableData
//***********************************
gTableData::gTableData()
{	
	text = NULL;
	picture = NULL;
	font = NULL;
	bg = COLOR_DEFAULT;
	fg = COLOR_DEFAULT;
	padding = 0;
	alignment = ALIGN_NORMAL;
}

gTableData::~gTableData()
{
	if (text) g_free(text);
	gFont::assign(&font);
	gPicture::assign(&picture);
}

void gTableData::setText(char *vl)
{
	if (text) 
		g_free(text);
	if (vl)
		text = g_strdup(vl);
	else
		text = NULL;
}

void gTableData::clear()
{
	if (text) g_free(text);
	gFont::assign(&font);
	gPicture::assign(&picture);
	
	text=NULL;
	picture=NULL;
	font=NULL;
	
	bg = COLOR_DEFAULT;
	fg = COLOR_DEFAULT;
	padding = 0;	
	alignment = ALIGN_NORMAL;
}

//***********************************
// gTable
//***********************************
gboolean gTable_ecol(gTablePair *key,gTableData *data,int number)
{
	if ( (key->row == number) && (key->col!=-1) ) return true;
	return false;
}

gboolean gTable_remove_row(gTablePair *key,gTableData *data,int number)
{
	if (key->row >= number) return true;
	return false;
}

gboolean gTable_remove_col(gTablePair *key,gTableData *data,int number)
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
	colpos = colsize = NULL;
	rowpos = rowsize = NULL;
	columns=0;
	rows=0;
	doNotInvalidate = false;
	seldata=g_hash_table_new_full((GHashFunc)g_int_hash,(GEqualFunc)gTable_equal,
                         (GDestroyNotify)gTable_ekey,NULL);
	data=g_hash_table_new_full((GHashFunc)g_int_hash,(GEqualFunc)gTable_equal,
                         (GDestroyNotify)gTable_ekey,(GDestroyNotify)gTable_edata);
}

gTable::~gTable()
{
	g_hash_table_destroy(data);
	g_hash_table_destroy(seldata);
	g_free(rowsize);
	g_free(rowpos);
	g_free(colsize);
	g_free(colpos);
}


int gTable::columnCount()
{
	return columns;
}

int gTable::rowCount()
{
	return rows;
}

void gTable::setRowCount(int number)
{
	int bc;

	if (number<0) number=0;
	if (number==rows) return; 

	if (number>rows)
	{
		if (!rows)
		{
			rowsize = g_new(int, number);
			rowpos = g_new(int, number);
		}
		else
		{
			rowsize = g_renew(int, rowsize, number);
			rowpos = g_renew(int, rowpos, number);
		}

		for (bc=rows; bc<number; bc++)
		{
			rowpos[bc] = -1;
			rowsize[bc] = 20;
		}
		rowpos[0] = 0;
	}
	else
	{
		if (!number) 
		{ 
			g_free(rowsize); rowsize = NULL;
			g_free(rowpos); rowpos = NULL; 
		}
		else
		{
			rowsize = g_renew(int, rowsize, number);
			rowpos = g_renew(int, rowpos, number);
		}
		
		g_hash_table_foreach_remove (data,(GHRFunc)gTable_remove_row,(gpointer)number);
		g_hash_table_foreach_remove (seldata,(GHRFunc)gTable_remove_row,(gpointer)number);
	}
	
	rows = number;
}


void gTable::setColumnCount(int number)
{
	int bc;

	if (number<0) number=0;
	if (number==columns) return; 

	if (number>columns)
	{
		if (!columns) 
		{
			colsize = g_new(int, number);
			colpos = g_new(int, number);
		}
		else
		{
			colsize = g_renew(int, colsize, number);
			colpos = g_renew(int, colpos, number);
		}
		
		for (bc=columns; bc<number; bc++) 
		{
			colpos[bc] = -1;
			colsize[bc] = 8;
		}
		colpos[0] = 0;
	}
	else
	{
		if (!number) 
		{ 
			g_free(colpos); colpos = NULL;
			g_free(colsize); colsize = NULL; 
		}
		else
		{
			colsize = g_renew(int, colsize, number);
			colpos = g_renew(int, colpos, number);
		}

		g_hash_table_foreach_remove (data,(GHRFunc)gTable_remove_col,(gpointer)number);
		g_hash_table_foreach_remove (seldata,(GHRFunc)gTable_remove_col,(gpointer)number);
	}
	
	columns = number;
}

gTableData *gTable::getData(int row, int col, bool create)
{
	static gTableData cell;
	gTableData *d;
	gTablePair pair = {row, col};
	gTablePair *key;
	
	if (col < 0 || col >= columns || row < 0 || row >= rows) 
		return NULL;

	d = (gTableData*)g_hash_table_lookup(data, (gpointer)&pair);
	if (!d)
	{
		if (create)
		{
			key = (gTablePair *)g_malloc(sizeof(gTablePair));
			key->row = row;
			key->col = col;
			d = new gTableData();
			g_hash_table_insert(data, (gpointer)key, (gpointer)d);
		}
		else
		{
			cell.clear();
			if (voidCell) voidCell(&cell, row, col, userData);
			d = &cell;
		}
	}
		
	return d; 
}

void gTable::clearField (int col,int row)
{
	gTableData *ptr;
	gTablePair pair={row,col};

	if ( (col<0) || (col>=columns) ) return;
	if ( (row<0) || (row>=rows) ) return;

	ptr=(gTableData*)g_hash_table_lookup(data,(gpointer)&pair);
	if (!ptr) return;
	g_hash_table_remove(data,(gpointer)&pair);
	
}

char* gTable::getFieldText(int col,int row)
{
	gTableData *d = getData(row, col);
	return d ? d->text : NULL;
}

void gTable::setFieldText(int col,int row,char* value)
{
	gTableData *d = getData(row, col, true);
	d->setText(value);
}

gColor gTable::getFieldFg (int col,int row)
{
	gTableData *d = getData(row, col);
	return d ? d->fg : COLOR_DEFAULT;
}

void gTable::setFieldFg (int col,int row, gColor value)
{
	gTableData *d = getData(row, col, true);
	d->fg = value;
}

gColor gTable::getFieldBg (int col,int row)
{
	gTableData *d = getData(row, col);
	return d ? d->bg : COLOR_DEFAULT;
}

void gTable::setFieldBg (int col,int row, gColor value)
{
	gTableData *d = getData(row, col, true);
	d->bg = value;
}

int gTable::getFieldPadding(int col, int row)
{
	gTableData *d = getData(row, col);	
	return d ? d->padding : 0;
}

void gTable::setFieldPadding(int col, int row, int value)
{
	gTableData *d = getData(row, col, true);
	d->padding = value;
}

int gTable::getFieldAlignment(int col, int row)
{
	gTableData *d = getData(row, col);	
	return d ? d->alignment : 0;
}

void gTable::setFieldAlignment(int col, int row, int value)
{
	gTableData *d = getData(row, col, true);
	d->alignment = value;
}

gPicture *gTable::getFieldPicture(int col, int row)
{
	gTableData *d = getData(row, col);	
	return d ? d->picture : 0;
}

void gTable::setFieldPicture(int col, int row, gPicture *value)
{
	gTableData *d = getData(row, col, true);
	d->setPicture(value);
}

gFont *gTable::getFieldFont(int col, int row)
{
	gTableData *d = getData(row, col);	
	return d ? d->font : 0;
}

void gTable::setFieldFont(int col, int row, gFont *value)
{
	gTableData *d = getData(row, col, true);
	d->setFont(value);
}


bool gTable::getRowSelected(int row)
{
	gTablePair pair={row,-1};

	if ( (row<0) || (row>=rows) ) return false;

	if (g_hash_table_lookup(seldata,(gpointer)&pair)) return true;
	return false;
}

void gTable::setRowSelected(int row,bool value)
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

bool gTable::getFieldSelected (int col,int row)
{
	gTablePair pair={row,col};

	if ( (col<0) || (col>=columns) ) return false;
	if ( (row<0) || (row>=rows) ) return false;

	if (g_hash_table_lookup(seldata,(gpointer)&pair)) return true;
	pair.col=-1;
	if (g_hash_table_lookup(seldata,(gpointer)&pair)) return true;
	return false;
}

void gTable::setFieldSelected (int col,int row,bool value)
{
	gTablePair pair={row,col};
	gTablePair *key;
	int bc;

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

int gTable::getColumnPos(int index)
{
	int pos, i, dim;
	
	if (index < 0 || index >= columns)
		return -1;
		
	pos = colpos[index];
	if (pos < 0)
	{
		i = index;
		for(;;)
		{
			i--;
			if (colpos[i] >= 0)
				break;
		}
		
		pos = colpos[i];
		for(;;)
		{
			pos += colsize[i];
			i++;
			if (i > index)
				break;
			colpos[i] = pos;
		}
		pos = colpos[index];
	}
	
	return pos;
}

int gTable::getRowPos(int index)
{
	int pos, i, dim;
	
	if (index < 0 || index >= rows)
		return -1;
		
	pos = rowpos[index];
	if (pos < 0)
	{
		i = index;
		for(;;)
		{
			i--;
			if (rowpos[i] >= 0)
				break;
		}
		
		pos = rowpos[i];
		for(;;)
		{
			pos += rowsize[i];
			i++;
			if (i > index)
				break;
			rowpos[i] = pos;
		}
		pos = rowpos[index];
	}
	
	return pos;
}

int gTable::getColumnSize  (int position)
{
	if (position>=columns) return -1;
	if (position<0) return -1;

	return colsize[position];
}

void gTable::setColumnSize(int position, int value)
{
	int i;
	
	if (position < 0 || position >= columns) return;

	if (value<0) value=0;
	colsize[position]=value;
	
	if (!doNotInvalidate)
	{
		for (i = position + 1; i < columns; i++)
			colpos[i] = -1;
	}
}

int gTable::getRowSize (int position)
{
	if (position>=rows) return -1;
	if (position<0) return -1;

	return rowsize[position];
}

void gTable::setRowSize (int position,int value)
{
	int i;
	
	if (position < 0 || position >= rows) 
		return;

	if (value < 0) value = 0;
	rowsize[position] = value;

	if (!doNotInvalidate)
	{
		for (i = position + 1; i < rows; i++)
			rowpos[i] = -1;
	}
}


//***********************************
// gTableRender
//***********************************

static gboolean tbrender_expose(GtkWidget *wid,GdkEventExpose *e,gTableRender *data)
{
	data->render(&e->area);
	return FALSE;
}

gTableRender::gTableRender(gGridView *v)
{
	voidCell=NULL;
	userData=NULL;
	grid=true;
	txt=(GtkCellRendererText*)gtk_cell_renderer_text_new();
	g_object_ref_sink(txt);
	pix=(GtkCellRendererPixbuf*)gtk_cell_renderer_pixbuf_new();
	g_object_ref_sink(pix);
	view = v;
	sf = view->contents;
	offX=0;
	offY=0;
	firstRow = offRow = 0;
	firstCol = offCol = 0;
	
	g_object_ref(G_OBJECT(sf));
	g_signal_connect(G_OBJECT(sf),"expose-event",G_CALLBACK(tbrender_expose),this);
	//g_signal_connect(G_OBJECT(sf),"size-allocate",G_CALLBACK(cb_render_size_allocate),this);
}

void gTableRender::queryUpdate(int row,int col)
{
	GdkRectangle rect={-getOffsetX(),-getOffsetY(),0,0};
	int bc;

	if (row>=rowCount()) return;
	if (col>=columnCount()) return;
	if (!sf->window) return;

	for (bc=0; bc<col;bc++) rect.x+=getColumnSize(bc);
	for (bc=0; bc<row;bc++) rect.y+=getRowSize(bc);
	rect.width=getColumnSize(bc);
	rect.height=getRowSize(bc);

	if (col < 0)
	{
		rect.x = 0;
		gdk_drawable_get_size(sf->window,&rect.width,NULL);
	}
	if (row < 0)
	{
		rect.y = 0;
		gdk_drawable_get_size(sf->window,NULL,&rect.height);
	}

	gdk_window_invalidate_rect(sf->window,&rect,TRUE); 
}

gTableRender::~gTableRender()
{
	g_object_unref(G_OBJECT(pix));
	g_object_unref(G_OBJECT(txt));
	if (sf) g_object_unref(G_OBJECT(sf));	
}

bool gTableRender::drawGrid()
{
	return grid;
}

void gTableRender::setDrawGrid(bool vl)
{
	if (grid == vl) return;

	grid=vl;
	gtk_widget_queue_draw(sf);
}

int gTableRender::visibleWidth()
{
	int vl;

	if (!sf->window) return 0;
	gdk_drawable_get_size (sf->window,&vl,NULL);
	return vl;
}

int gTableRender::visibleHeight()
{
	int vl;

	if (!sf->window) return 0;
	gdk_drawable_get_size (sf->window,NULL,&vl);
	return vl;
}

int gTableRender::width()
{
	if (columnCount() <= 0)
		return 0;
	else
		return getColumnPos(columnCount() - 1) + getColumnSize(columnCount() - 1);
}

int gTableRender::height()
{
	if (rowCount() <= 0)
		return 0;
	else
		return getRowPos(rowCount() - 1) + getRowSize(rowCount() - 1);
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
	int diff;

	if (offX != vl) 
	{ 
		diff=offX-vl;
		offX=vl;
		
		/*for (offCol = 0, firstCol = 0; firstCol < columnCount(); firstCol++)
		{
			w = getColumnSize(firstCol);
			if ((offCol + w) > offX)
				break;
			offCol += w;
		}*/
		
		firstCol = findColumn(offX);
		offCol = getColumnPos(firstCol);
		
		if (!sf->window) return;
		gdk_window_scroll(sf->window,diff,0);
	}
}

void gTableRender::setOffsetY(int vl)
{
	int diff;

	if (offY != vl) 
	{ 
		diff=offY-vl;
		offY=vl; 
		
		/*for (offRow = 0, firstRow = 0; firstRow < rowCount(); firstRow++)
		{
			h = getRowSize(firstRow);
			if ((offRow + h) > offY)
				break;
			offRow += h;
		}*/
		
		firstRow = findRow(offY);
		offRow = getRowPos(firstRow);

		if (!sf->window) return;
		gdk_window_scroll(sf->window,0,diff);
	}
}

void gTableRender::renderCell(gTableData *data, GdkGC *gc, GdkRectangle *rect, bool sel)
{
	GdkColor color;
	GtkStyle *st;
	char *buf = data->text;
	double xa, ya;

	xa = gt_from_alignment(data->alignment, false);
	ya = gt_from_alignment(data->alignment, true);
	
	g_object_set(G_OBJECT(txt),
		"text", buf ? buf : "",
		"font-desc", sf->style->font_desc,
		"xalign", xa,
		"yalign", ya,
		NULL);
	
	if (sel)
	{
		st = gt_get_style("GtkEntry", GTK_TYPE_ENTRY);
		g_object_set(G_OBJECT(txt),"foreground-gdk",
																					&st->text[GTK_STATE_SELECTED],NULL);
		g_object_set(G_OBJECT(txt),"background-gdk",
																					&st->base[GTK_STATE_SELECTED],NULL);
	}
	else
	{
		g_object_set(G_OBJECT(txt),
			"foreground-set", data->fg != COLOR_DEFAULT,
			"background-set", data->bg != COLOR_DEFAULT,
			NULL);
			
		if (data->fg != COLOR_DEFAULT)
		{
			fill_gdk_color(&color, data->fg);
			g_object_set(G_OBJECT(txt),"foreground-gdk", &color, NULL);
		}
		
		if (data->bg != COLOR_DEFAULT)
		{
			fill_gdk_color(&color, data->bg);
			g_object_set(G_OBJECT(txt),"background-gdk", &color, NULL);
		}
	}
	
	gtk_cell_renderer_render(GTK_CELL_RENDERER(txt),sf->window,sf,rect,rect,rect,(GtkCellRendererState)0);
	
	if (data->picture)
	{
		if (buf && *buf)
		{
			xa = 1 - xa;
			ya = 1 - ya;
		}
		g_object_set(G_OBJECT(pix),
			"pixbuf", data->picture->getPixbuf(),
			"xalign", xa,
			"yalign", ya,
			NULL);
		gtk_cell_renderer_render(GTK_CELL_RENDERER(pix),sf->window,sf,rect,rect,rect,(GtkCellRendererState)0);
	}
}

void gTableRender::render(GdkRectangle *ar)
{
	bool  sel;
	int   pos;
	int   bx,by;
	int   bpos;
	int   cpos;
	int   pmaxX;
	int   pmaxY;
	int   bufW = 0;
	GdkGC *gc;
	GdkRectangle rect;
	gTableData *cell;
	gTableData *caux;

	if (!sf->window) return;

	gdk_window_clear_area(sf->window,ar->x,ar->y,ar->width,ar->height);

	//fprintf(stderr, "render: %d %d %d %d\n", ar->x,ar->y,ar->width,ar->height);

	if ( (!rowCount()) || (!columnCount()) ) return; 

	caux=new gTableData();

	// Prepare data
	pmaxX=width();
	pmaxY=height();
	if (pmaxX>visibleWidth()) pmaxX=visibleWidth();
	if (pmaxY>visibleHeight()) pmaxY=visibleHeight();

	gc=gdk_gc_new(sf->window);
	gdk_gc_set_background(gc,&sf->style->base[GTK_STATE_NORMAL]);
	
	gdk_gc_set_clip_origin(gc,0,0);
	gdk_gc_set_clip_rectangle(gc,ar);

	// Rendering texts and pixbufs
	bpos = offCol;
	for (bx=firstCol; bx<columnCount(); bx++)
	{
		if ((bpos + getColumnSize(bx) - offX) < ar->x) 
		{ 
			bpos += getColumnSize(bx); 
			continue; 
		}
		
		if ((bpos - offX) >= visibleWidth()) 
			break;
			 
		if ((bpos - offX) >= (ar->x + ar->width))
			break;

		cpos = offRow;
		for (by = firstRow; by < rowCount(); by++)
		{
			if ((cpos + getRowSize(by) - offY) < ar->y ) 
			{ 
				cpos += getRowSize(by); 
				continue; 
			}
			
			if ((cpos - offY) >= visibleHeight()) 
				break;	
			
			if ((cpos - offY) >= (ar->y + ar->height))
				break;
			
			rect.x=bpos-offX+bufW;
			rect.y=cpos-offY;
			rect.width=getColumnSize(bx)-bufW;
			rect.height=getRowSize(by);

			sel=getFieldSelected(bx,by);
			cell = getData(by, bx);
			gdk_gc_set_clip_rectangle(gc, &rect);
			renderCell(cell, gc, &rect, sel);
						
			cpos += getRowSize(by);
		}

		bpos += getColumnSize(bx);
	}

	delete caux;
	if (!grid) { g_object_unref(G_OBJECT(gc)); return; }

	gdk_gc_set_clip_origin(gc,0,0);
	gdk_gc_set_clip_rectangle(gc,ar);
	//gint8 dashes[2] = { 1, 1 };
	//gdk_gc_set_line_attributes(gc, 1, GDK_LINE_ON_OFF_DASH, GDK_CAP_NOT_LAST, GDK_JOIN_MITER);
	gdk_gc_set_foreground(gc, &sf->style->mid[GTK_STATE_NORMAL]);

	// Horizontal lines
	//gdk_gc_set_dashes(gc, offY & 1, dashes, 2);
	bpos = offRow;
	for (bx = firstRow; bx < rowCount(); bx++)
	{
		bpos += getRowSize(bx);
		pos = bpos - offY - 1;
		
		if (pos > visibleHeight())
			break;
		if (pos >= 0)
			gdk_draw_line(sf->window, gc, 0, pos, pmaxX - 1, pos);
	}
	
	// Vertical lines
	//gdk_gc_set_dashes(gc, offX & 1, dashes, 2);
	bpos = offCol;
	for (bx = firstCol; bx < columnCount(); bx++)
	{
		bpos += getColumnSize(bx);
		pos = bpos - offX - 1;
		
		if (pos >= visibleWidth()) break;
		if (pos >= 0)  
			gdk_draw_line(sf->window, gc, pos, 0, pos, pmaxY - 1);		
	}
	
	g_object_unref(G_OBJECT(gc));
}

void gTableRender::setRowSize(int position,int value)
{
	GdkRectangle rect={0,0,0,0};
	int pos;

	if (position<0) return;
	if (position>=rowCount()) return;

	gTable::setRowSize(position,value);
	
	if (view->locked() || !view->isVisible())
		return;
		
	if (!sf->window) return;
	gdk_drawable_get_size (sf->window,&rect.width,&rect.height);

	pos = getRowPos(position) - offY;

	if ((pos + getRowSize(position))<0) return;
	if (pos > rect.height) return;
		
	gdk_window_invalidate_rect(sf->window, &rect, TRUE);
}

void gTableRender::setColumnSize(int position,int value)
{
	GdkRectangle rect={0,0,0,0};
	int pos;

	if (position<0) return;
	if (position>=columnCount()) return;

	gTable::setColumnSize(position,value);
	
	if (view->locked() || !view->isVisible())
		return;
		
	if (!sf->window) return;
	gdk_drawable_get_size (sf->window,&rect.width,&rect.height);

	pos = getColumnPos(position) - offX;

	if ((pos + getColumnSize(position)) < 0) return;
	if (pos > rect.width) return;
		
	gdk_window_invalidate_rect(sf->window, &rect, TRUE);

}

void gTableRender::clearField (int col,int row)
{
	if (col<0) return;
	if (col>=columnCount()) return;
	if (row<0) return;
	if (row>=rowCount()) return;

	gTable::clearField(col,row);
	queryUpdate(row,col);
}

void gTableRender::setFieldText (int col,int row,char* value)
{
	if (col<0) return;
	if (col>=columnCount()) return;
	if (row<0) return;
	if (row>=rowCount()) return;

	gTable::setFieldText(col,row,value);
	queryUpdate(row,col);
}

void gTableRender::setFieldFg (int col,int row,gColor value)
{
	if (col<0) return;
	if (col>=columnCount()) return;
	if (row<0) return;
	if (row>=rowCount()) return;

	gTable::setFieldFg(col,row,value);
	queryUpdate(row,col);
}

void gTableRender::setFieldBg (int col,int row,gColor value)
{
	if (col<0) return;
	if (col>=columnCount()) return;
	if (row<0) return;
	if (row>=rowCount()) return;

	gTable::setFieldBg(col,row,value);
	queryUpdate(row,col);
}

void gTableRender::setFieldPadding(int col,int row,int value)
{
	if (col<0) return;
	if (col>=columnCount()) return;
	if (row<0) return;
	if (row>=rowCount()) return;

	gTable::setFieldPadding(col,row,value);
	queryUpdate(row,col);
}

void gTableRender::setFieldSelected(int col,int row,bool value)
{
	gTable::setFieldSelected(col,row,value);
	queryUpdate(row,-1);
}

void gTableRender::setRowSelected(int row,bool value)
{
	if (row<0) return;
	if (row>=rowCount()) return;

	gTable::setRowSelected(row,value);
	queryUpdate(row,-1);
}

void gTableRender::clearSelection()
{
	g_hash_table_destroy(seldata);
	seldata=g_hash_table_new_full((GHashFunc)g_int_hash,(GEqualFunc)gTable_equal,
                         (GDestroyNotify)gTable_ekey,NULL);
	gtk_widget_queue_draw(sf);
}

void gTableRender::selectRows(int start, int length, bool value)
{
	int end, i;
	 
	if (length < 0)
		length = rowCount();
	 
	end = start + length - 1;
	
	if (end < start)
		return;
	
	if (start < 0)
		start = 0;
		
	if (end >= rowCount())
		end = rowCount() - 1;
		
	for (i = start; i <= end; i++)
		setRowSelected(i, -1);
}

int gTableRender::findVisibleRow(int y)
{
	int pos, row, h;
	
	pos = offRow;
	y += offY;
	for (row = firstRow; row < rowCount(); row++)
	{
		h = getRowSize(row);
		if (y < (pos + h))
			return row;
		pos += h;
	}
	
	return -1;
}

int gTableRender::findVisibleColumn(int x)
{
	int pos, col, w;
	
	pos = offCol;
	x += offX;
	for (col = firstCol; col < columnCount(); col++)
	{
		w = getColumnSize(col);
		if (x < (pos + w))
			return col;
		pos += w;
	}
	
	return -1;
}

int gTableRender::findRow(int pos)
{
	int d, f, row;
	
	d = 0;
	f = rowCount();
	
	while (f > d)
	{
		row = (d + f) / 2;
		if (pos < getRowPos(row))
		{
			f = row;
			continue;
		}
		if (pos >= (getRowPos(row) + getRowSize(row)))
		{
			d = row + 1;
			continue;
		}
		return row;
	}
	
	return -1;
}

int gTableRender::findColumn(int pos)
{
	int d, f, col;
	
	d = 0;
	f = columnCount();
	
	while (f > d)
	{
		col = (d + f) / 2;
		if (pos < getColumnPos(col))
		{
			f = col;
			continue;
		}
		if (pos >= (getColumnPos(col) + getColumnSize(col)))
		{
			d = col + 1;
			continue;
		}
		return col;
	}
	
	return -1;
}

