/***************************************************************************

  tablerender.cpp

  (c) 2000-2009 Benoît Minisini <gambas@users.sourceforge.net>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2, or (at your option)
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
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "gmemory.h"
#include "ggridview.h"
#include "tablerender.h"

#define GET_ROW_SPAN(_span) ((int)(short)(((intptr_t)_span) & 0xFFFF))
#define GET_COL_SPAN(_span) ((int)(short)((((intptr_t)_span) >> 16) & 0xFFFF))
#define MAKE_SPAN(_rowspan, _colspan) ((gpointer)(((uint)(unsigned short)(_rowspan)) | (((uint)(unsigned short)(_colspan)) << 16)))

//***********************************
// gTableData
//***********************************
gTableData::gTableData()
{	
	text = NULL;
	richText = NULL;
	markup = NULL;
	picture = NULL;
	font = NULL;
	bg = COLOR_DEFAULT;
	fg = COLOR_DEFAULT;
	padding = 0;
	alignment = ALIGN_NORMAL;
	wordWrap = false;
}

gTableData::~gTableData()
{
	clear();
}

void gTableData::setText(const char *vl)
{
	if (text) 
		g_free(text);
	if (vl)
		text = g_strdup(vl);
	else
		text = NULL;
}

void gTableData::setRichText(const char *vl)
{
	if (richText)
	{
		g_free(richText);
		g_free(markup);
	}
	
	if (vl)
	{
		richText = g_strdup(vl);
		markup = gt_html_to_pango_string(richText, -1, true);
	}
	else
	{
		richText = NULL;
		markup = NULL;
	}
}

void gTableData::clear()
{
	if (text) g_free(text);
	if (richText) g_free(richText);
	if (markup) g_free(markup);
	gFont::assign(&font);
	gPicture::assign(&picture);
	
	text = NULL;
	richText = NULL;
	markup = NULL;
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
	columns = 0;
	rows = 0;
	doNotInvalidate = false;
	seldata = g_hash_table_new_full((GHashFunc)g_int_hash, (GEqualFunc)gTable_equal, (GDestroyNotify)gTable_ekey, NULL);
	spanHash = g_hash_table_new_full((GHashFunc)g_int_hash, (GEqualFunc)gTable_equal, (GDestroyNotify)gTable_ekey, NULL);
	data = g_hash_table_new_full((GHashFunc)g_int_hash, (GEqualFunc)gTable_equal, (GDestroyNotify)gTable_ekey, (GDestroyNotify)gTable_edata);
}

gTable::~gTable()
{
	g_hash_table_destroy(data);
	g_hash_table_destroy(seldata);
	g_hash_table_destroy(spanHash);
	g_free(rowsize);
	g_free(rowpos);
	g_free(colsize);
	g_free(colpos);
}


void gTable::clear()
{
	g_hash_table_destroy(data);
	g_hash_table_destroy(seldata);
	g_hash_table_destroy(spanHash);
	seldata = g_hash_table_new_full((GHashFunc)g_int_hash, (GEqualFunc)gTable_equal, (GDestroyNotify)gTable_ekey, NULL);
	spanHash = g_hash_table_new_full((GHashFunc)g_int_hash, (GEqualFunc)gTable_equal, (GDestroyNotify)gTable_ekey, NULL);
	data = g_hash_table_new_full((GHashFunc)g_int_hash, (GEqualFunc)gTable_equal, (GDestroyNotify)gTable_ekey, (GDestroyNotify)gTable_edata);
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
		
		g_hash_table_foreach_remove(data, (GHRFunc)gTable_remove_row, (gpointer)number);
		g_hash_table_foreach_remove(seldata, (GHRFunc)gTable_remove_row, (gpointer)number);
		g_hash_table_foreach_remove(spanHash, (GHRFunc)gTable_remove_row, (gpointer)number);
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

		g_hash_table_foreach_remove(data, (GHRFunc)gTable_remove_col, (gpointer)number);
		g_hash_table_foreach_remove(seldata, (GHRFunc)gTable_remove_col, (gpointer)number);
		g_hash_table_foreach_remove(spanHash, (GHRFunc)gTable_remove_col, (gpointer)number);
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

void gTable::moveCell(int srow, int scol, int drow, int dcol)
{
	gTableData *d;
	gTablePair pair;
	gTablePair *key;
	
	pair.row = drow;
	pair.col = dcol;
	g_hash_table_remove(data, (gpointer)&pair);
	
	pair.row = srow;
	pair.col = scol;
	if (!g_hash_table_lookup_extended(data, (gconstpointer)&pair, POINTER(&key), POINTER(&d)))
		return;
	
	g_hash_table_steal(data, &pair);
	key->row = drow;
	key->col = dcol;
	g_hash_table_insert(data, (gpointer)key, (gpointer)d);
}


void gTable::clearField (int row, int col)
{
	gTableData *ptr;
	gTablePair pair={row,col};

	if ( (col<0) || (col>=columns) ) return;
	if ( (row<0) || (row>=rows) ) return;

	ptr=(gTableData*)g_hash_table_lookup(data,(gpointer)&pair);
	if (!ptr) return;
	g_hash_table_remove(data,(gpointer)&pair);
	
}

char* gTable::getFieldText(int row, int col)
{
	gTableData *d = getData(row, col);
	return d ? d->text : NULL;
}

void gTable::setFieldText(int row, int col, const char* value)
{
	gTableData *d = getData(row, col, true);
	if (d) d->setText(value);
}

char* gTable::getFieldRichText(int row, int col)
{
	gTableData *d = getData(row, col);
	return d ? d->richText : NULL;
}

void gTable::setFieldRichText(int row, int col, const char* value)
{
	gTableData *d = getData(row, col, true);
	if (d) d->setRichText(value);
}

gColor gTable::getFieldFg (int row, int col)
{
	gTableData *d = getData(row, col);
	return d ? d->fg : COLOR_DEFAULT;
}

void gTable::setFieldFg (int row, int col, gColor value)
{
	gTableData *d = getData(row, col, true);
	if (d) d->fg = value;
}

gColor gTable::getFieldBg (int row, int col)
{
	gTableData *d = getData(row, col);
	return d ? d->bg : COLOR_DEFAULT;
}

void gTable::setFieldBg (int row, int col, gColor value)
{
	gTableData *d = getData(row, col, true);
	if (d) d->bg = value;
}

int gTable::getFieldPadding(int row, int col)
{
	gTableData *d = getData(row, col);	
	return d ? d->padding : 0;
}

void gTable::setFieldPadding(int row, int col, int value)
{
	gTableData *d = getData(row, col, true);
	if (d) d->padding = value;
}

int gTable::getFieldAlignment(int row, int col)
{
	gTableData *d = getData(row, col);	
	return d ? d->alignment : 0;
}

void gTable::setFieldAlignment(int row, int col, int value)
{
	gTableData *d = getData(row, col, true);
	if (d) d->alignment = value;
}

gPicture *gTable::getFieldPicture(int row, int col)
{
	gTableData *d = getData(row, col);	
	return d ? d->picture : 0;
}

void gTable::setFieldPicture(int row, int col, gPicture *value)
{
	gTableData *d = getData(row, col, true);
	if (d) d->setPicture(value);
}

gFont *gTable::getFieldFont(int row, int col)
{
	gTableData *d = getData(row, col);	
	return d ? d->font : 0;
}

void gTable::setFieldFont(int row, int col, gFont *value)
{
	gTableData *d = getData(row, col, true);
	if (d) d->setFont(value);
}

bool gTable::getFieldWordWrap(int row, int col)
{
	gTableData *d = getData(row, col);	
	return d ? d->wordWrap : false;
}

void gTable::setFieldWordWrap(int row, int col, bool value)
{
	gTableData *d = getData(row, col, true);
	if (d) d->wordWrap = value;
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

bool gTable::getFieldSelected (int row, int col)
{
	gTablePair pair={row,col};

	if ( (col<0) || (col>=columns) ) return false;
	if ( (row<0) || (row>=rows) ) return false;

	if (g_hash_table_lookup(seldata,(gpointer)&pair)) return true;
	pair.col=-1;
	if (g_hash_table_lookup(seldata,(gpointer)&pair)) return true;
	return false;
}

void gTable::setFieldSelected (int row, int col, bool value)
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

void gTable::getSpan(int row, int col, int *rowspan, int *colspan)
{
	gTablePair pair = { row, col };
	gpointer span;

	*colspan = 0;
	*rowspan = 0;

	if ((col < 0) || (col >= columns)) return;
	if ((row < 0) || (row >= rows)) return;

	span = g_hash_table_lookup(spanHash, (gpointer)&pair);
	if (!span)
		return;

	*colspan = GET_COL_SPAN(span);
	if (*colspan >= (columns - col - 1))
		*colspan = columns - col - 1;
	*rowspan = GET_ROW_SPAN(span);
	if (*rowspan >= (rows - row - 1))
		*rowspan = rows - row - 1;
}

void gTable::setSpan(int row, int col, int rowspan, int colspan)
{
	gTablePair pair = { row, col };
	gTablePair *key;
	int i, j;
	int oldcs, oldrs;
	gpointer span;

	if ((col < 0) || (col >= columns)) return;
	if ((row < 0) || (row >= rows)) return;

	if (rowspan < -32768 || rowspan > 32767)
		return;
	if (colspan < -32768 || colspan > 32767)
		return;

	span = g_hash_table_lookup(spanHash, (gpointer)&pair);
	
	oldcs = GET_COL_SPAN(span);
	oldrs = GET_ROW_SPAN(span);
	if ((oldcs < 0 || oldrs < 0) && (colspan != 0 || rowspan != 0))
		return;
	
	g_hash_table_remove(spanHash, (gpointer)&pair);
	
	if (oldcs > 0 || oldrs > 0)
	{
		i = 1; j = 0;
		for(;;)
		{
			if (i > oldcs)
			{
				i = 0;
				j++;
				if (j > oldrs)
					break;
			}
			setSpan(col + i, row + j, 0, 0);
			i++;
		}
	}

	if (rowspan == 0 && colspan == 0)
		return;
	
	key = (gTablePair*)g_malloc(sizeof(gTablePair));
	key->row = row;
	key->col = col;
	g_hash_table_insert(spanHash, (gpointer)key, (gpointer)MAKE_SPAN(rowspan, colspan));
	
	if (rowspan >= 0 && colspan >= 0)
	{
		i = 1; j = 0;
		for(;;)
		{
			if (i > colspan)
			{
				i = 0;
				j++;
				if (j > rowspan)
					break;
			}
			setSpan(col + i, row + j, -i, -j);
			i++;
		}
	}
}

int gTable::getColumnPos(int index)
{
	int pos, i;
	
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
	int pos, i;
	
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

void gTableRender::queryUpdate(int row, int col)
{
	GdkRectangle rect;
	int i;
	int srow, scol;
	int rowspan, colspan;

	if (row >= rowCount()) return;
	if (col >= columnCount()) return;
	if (!sf->window) return;

	if (col < 0)
	{
		rect.x = -getOffsetX();
		gdk_drawable_get_size(sf->window, &rect.width, NULL);
	}
	else
	{
		rect.x = getColumnPos(col) - getOffsetX();
		rect.width = getColumnSize(col);
	}
	
	if (row < 0)
	{
		rect.y = -getOffsetY();
		gdk_drawable_get_size(sf->window, NULL, &rect.height);
	}
	else
	{
		rect.y = getRowPos(row) - getOffsetY();
		rect.height = getRowSize(row);
	}

	if (col < 0)
	{
		gdk_window_invalidate_rect(sf->window, &rect, TRUE); 
		for (i = 0; i < columnCount(); i++)
			queryUpdate(row, i);
		return;
	}
	
	if (row < 0)
	{
		gdk_window_invalidate_rect(sf->window, &rect, TRUE); 
		for (i = 0; i < rowCount(); i++)
			queryUpdate(i, col);
		return;
	}
	
	//for (i = 0; i < col; i++) rect.x += getColumnSize(i);
	//for (i = 0; i < row; i++) rect.y += getRowSize(i);
	getSpan(col, row, &colspan, &rowspan);

	if (colspan >= 0 && rowspan >= 0)
	{
		for (i = 1; i <= colspan; i++)
			rect.width += getColumnSize(col + i);
		
		for (i = 1; i <= rowspan; i++)
			rect.height += getRowSize(row + i);
	}
	else
	{
		srow = row + colspan;
		scol = col + rowspan;
		
		for (i = scol; i < col; i++)
			rect.x -= getColumnSize(i);
		for (i = srow; i < row; i++)
			rect.y -= getRowSize(i);
		
		getSpan(scol, srow, &colspan, &rowspan);
		
		rect.width = 0;
		rect.height = 0;
		
		for (i = 0; i <= colspan; i++)
			rect.width += getColumnSize(scol + i);
		for (i = 0; i <= rowspan; i++)
			rect.height += getRowSize(srow + i);
	}

// 	if (col < 0)
// 	{
// 		rect.x = 0;
// 		gdk_drawable_get_size(sf->window,&rect.width,NULL);
// 	}
// 	if (row < 0)
// 	{
// 		rect.y = 0;
// 		gdk_drawable_get_size(sf->window,NULL,&rect.height);
// 	}

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
	char *markup = data->markup;
	char *buf = data->text;
	int padding = data->padding;
	double xa, ya;
	bool hasText = (markup && *markup) || (buf && *buf);

	if (sel)
		gdk_gc_set_foreground(gc, &sf->style->base[GTK_STATE_SELECTED]);
	else if (data->bg != COLOR_DEFAULT)
	{
		fill_gdk_color(&color, data->bg);
		gdk_gc_set_foreground(gc, &color);
	}
	else
		goto __NO_BG; //fill_gdk_color(&color, view->realBackground());
	
	gdk_draw_rectangle(sf->window, gc, TRUE, rect->x, rect->y, rect->width, rect->height);
	
__NO_BG:

	if (grid)
	{
		gdk_gc_set_foreground(gc, &sf->style->mid[GTK_STATE_NORMAL]);
		gdk_draw_line(sf->window, gc, rect->x + rect->width - 1, rect->y, rect->x + rect->width - 1, rect->y + rect->height - 1);
		gdk_draw_line(sf->window, gc, rect->x, rect->y + rect->height - 1, rect->x + rect->width - 1, rect->y + rect->height - 1);
	}

	if (padding < 1)
		padding = 1;

	rect->x += padding;
	rect->y += padding;
	rect->width -= padding * 2;
	rect->height -= padding * 2;
	
	if (rect->width < 1 || rect->height < 1)
		return;

	xa = gt_from_alignment(data->alignment, false);
	ya = gt_from_alignment(data->alignment, true);
	
	if (data->picture)
	{
		g_object_set(G_OBJECT(pix),
			"pixbuf", data->picture->getPixbuf(),
			"xalign", hasText ? 0 : xa,
			"yalign", hasText ? 0.5 : ya,
			(void *)NULL);
		gtk_cell_renderer_render(GTK_CELL_RENDERER(pix),sf->window,sf,rect,rect,rect,(GtkCellRendererState)0);
		
		rect->x += data->picture->width() + padding;
		rect->width -= data->picture->width() + padding;
		if (rect->width < 1)
			return;
	}

	g_object_set(G_OBJECT(txt),
		/*"font-desc", data->font ? data->font->desc() : sf->style->font_desc,*/
		"xalign", xa,
		"yalign", ya,
		"wrap-width", data->wordWrap ? rect->width : -1,
		"wrap-mode", PANGO_WRAP_WORD_CHAR,
		(void *)NULL);
		
	gt_set_cell_renderer_text_from_font(txt, data->font ? data->font : view->font());
	
	g_object_set(G_OBJECT(txt),
		"foreground-set", sel || data->fg != COLOR_DEFAULT,
		"background-set", FALSE,
		(void *)NULL);
			
	if (sel)
	{
		g_object_set(G_OBJECT(txt),"foreground-gdk", &sf->style->text[GTK_STATE_SELECTED],(void *)NULL);
	}
	else if (data->fg != COLOR_DEFAULT)
	{
		fill_gdk_color(&color, data->fg);
		g_object_set(G_OBJECT(txt),"foreground-gdk", &color, (void *)NULL);
	}
	
	if (markup)
		g_object_set(G_OBJECT(txt), "text", NULL, "markup", markup, (void *)NULL);
	else
		g_object_set(G_OBJECT(txt), "markup", NULL, "text", buf, (void *)NULL);
	
	gtk_cell_renderer_render(GTK_CELL_RENDERER(txt),sf->window,sf,rect,rect,rect,(GtkCellRendererState)0);
}

void gTableRender::render(GdkRectangle *ar)
{
	bool  sel;
	int   bx,by,sbx,sby;
	int   bpos;
	int   cpos;
	int   pmaxX;
	int   pmaxY;
	int colspan, rowspan;
	int i;
	GdkGC *gc;
	GdkRectangle rect;
	gTableData *cell;
	gTableData *caux;
	int fcol, frow;

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

	#if 0
	if (grid)
	{
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
	}
	#endif
	
	// Rendering texts and pixbufs
	
	fcol = frow = -1;
	
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
		if (fcol < 0) 
			fcol = bx;
		
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
			
			if (frow < 0) 
			{
				frow = by;
			}
			
			getSpan(bx, by, &rowspan, &colspan);
			
			rect.x = bpos - offX;
			rect.y = cpos - offY;
			rect.width = 0;
			rect.height = 0;

			if (colspan >= 0 && rowspan >= 0)
			{
				sel = getFieldSelected(by, bx);
				
				for (i = 0; i <= colspan; i++)
					rect.width += getColumnSize(bx + i);
				
				for (i = 0; i <= rowspan; i++)
					rect.height += getRowSize(by + i);
				
				gdk_gc_set_clip_rectangle(gc, &rect);
				cell = getData(by, bx);
				renderCell(cell, gc, &rect, sel);
			}
			else if ((bx == fcol && colspan < 0) || (by == frow && rowspan < 0))
			{
				sbx = bx + colspan;
				sby = by + rowspan;
				
				sel = getFieldSelected(sby, sbx);
				cell = getData(sby, sbx);
				
				for (i = sbx; i < bx; i++)
					rect.x -= getColumnSize(i);
				for (i = sby; i < by; i++)
					rect.y -= getRowSize(i);
				
				getSpan(sbx, sby, &colspan, &rowspan);
				
				for (i = 0; i <= colspan; i++)
					rect.width += getColumnSize(sbx + i);
				for (i = 0; i <= rowspan; i++)
					rect.height += getRowSize(sby + i);
				
				gdk_gc_set_clip_rectangle(gc, &rect);
				renderCell(cell, gc, &rect, sel);
			}
						
			cpos += getRowSize(by);
		}

		bpos += getColumnSize(bx);
	}

	delete caux;
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

#define CHECK_COORD(_row, _col) if ((_col) < 0 || (_row) < 0 || (_col) >= columnCount() || (_row) > rowCount()) return

void gTableRender::clearField (int row, int col)
{
	CHECK_COORD(row, col);
	gTable::clearField(row, col);
	queryUpdate(row,col);
}

void gTableRender::setFieldText (int row, int col, const char* value)
{
	CHECK_COORD(row, col);
	gTable::setFieldText(row, col, value);
	queryUpdate(row,col);
}

void gTableRender::setFieldRichText (int row, int col, const char* value)
{
	CHECK_COORD(row, col);
	gTable::setFieldRichText(row, col, value);
	queryUpdate(row,col);
}

void gTableRender::setFieldFg (int row, int col,gColor value)
{
	CHECK_COORD(row, col);
	gTable::setFieldFg(row, col, value);
	queryUpdate(row,col);
}

void gTableRender::setFieldBg (int row, int col,gColor value)
{
	CHECK_COORD(row, col);
	gTable::setFieldBg(row, col, value);
	queryUpdate(row,col);
}

void gTableRender::setFieldPadding(int row, int col,int value)
{
	CHECK_COORD(row, col);
	gTable::setFieldPadding(row, col, value);
	queryUpdate(row,col);
}

void gTableRender::setFieldPicture(int row, int col, gPicture *value)
{
	CHECK_COORD(row, col);
	gTable::setFieldPicture(row, col, value);
	queryUpdate(row, col);
}

void gTableRender::setFieldFont(int row, int col, gFont *value)
{
	CHECK_COORD(row, col);
	gTable::setFieldFont(row, col, value);
	queryUpdate(row, col);
}

void gTableRender::setFieldWordWrap(int row, int col, bool value)
{
	CHECK_COORD(row, col);
	gTable::setFieldWordWrap(row, col, value);
	queryUpdate(row, col);
}

void gTableRender::setFieldSelected(int row, int col,bool value)
{
	gTable::setFieldSelected(row, col, value);
	queryUpdate(row, -1);
}

void gTableRender::setRowSelected(int row, bool value)
{
	if (row<0) return;
	if (row>=rowCount()) return;
	if (value == getRowSelected(row))
		return;
		
	gTable::setRowSelected(row, value);
	queryUpdate(row, -1);
	//view->emit(SIGNAL(view->onSelect));
}

void gTableRender::clearSelection()
{
	g_hash_table_destroy(seldata);
	seldata=g_hash_table_new_full((GHashFunc)g_int_hash,(GEqualFunc)gTable_equal,
                         (GDestroyNotify)gTable_ekey,NULL);
	gtk_widget_queue_draw(sf);
	//view->emit(SIGNAL(view->onSelect));
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
		
	view->lock();
	for (i = start; i <= end; i++)
		setRowSelected(i, value);
	view->unlock();
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


void gTableRender::insertRows(int start, int length)
{
	int i, j, c;

	if (start < 0 || length <= 0 || start > rowCount())
		return;
	
	c = rowCount();
	setRowCount(rowCount() + length);
	
	for (i = c - 1; i >= start; i--)
	{
		for (j = 0; j < columnCount(); j++)
			moveCell(i, j, i + length, j);
		queryUpdate(i, -1);
		queryUpdate(i + length, -1);
	}
}

void gTableRender::removeRows(int start, int length)
{
	int i, j, dst;

	dst = start;
	
	for (i = start + length; i < rowCount(); i++)
	{
		for (j = 0; j < columnCount(); j++)
			moveCell(i, j, dst, j);
		queryUpdate(i, -1);
		queryUpdate(dst, -1);
		dst++;
	}
		
	setRowCount(rowCount() - length);
}

void gTableRender::clear()
{
	gTable::clear();
	queryUpdate(-1, -1);
}

int gTableRender::getBestHeight(int row, int col)
{
	gTableData *d = getData(row, col);
	int h = 0;
	int th;
	gFont *f;
	
	if (d)
	{
		if (d->picture)
			h = Max(h, d->picture->height());
		
		f = getFieldFont(row, col);
		if (!f) f = view->font();
		
		if (d->richText && *d->richText)
		{
			f->richTextSize(d->richText, strlen(d->richText), getColumnSize(col), NULL, &th);
			h = Max(h, th);
		}
		else if (d->text && *d->text)
		{
			th = f->height(d->text, strlen(d->text));
			h = Max(h, th);
		}
	}
	
	h += getFieldPadding(row, col) * 2 + 4; // Why + 4? No idea...
	return h;
}

int gTableRender::getBestWidth(int row, int col)
{
	gTableData *d = getData(row, col);
	int w = 0;
	int padding = getFieldPadding(row, col);
	int tw;
	gFont *f;
	
	if (d)
	{
		if (d->picture)
			w = d->picture->width();
		
		f = getFieldFont(row, col);
		if (!f) f = view->font();
		
		if (d->richText && *d->richText)
		{
			if (w) w += padding;
			f->richTextSize(d->richText, strlen(d->richText), getColumnSize(col), &tw, NULL);
			w += tw;
		}
		else if (d->text && *d->text)
		{
			tw = f->width(d->text, strlen(d->text));
			w += tw;
		}
	}
	
	w += padding * 2;
	return w;
}

