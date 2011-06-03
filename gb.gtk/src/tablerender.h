/***************************************************************************

  tablerender.h

  (c) 2000-2011 Benoît Minisini <gambas@users.sourceforge.net>

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
  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
	MA 02110-1301, USA.

***************************************************************************/
#ifndef __TABLERENDER_H
#define __TABLERENDER_H

#include "gpicture.h"
#include "gfont.h"
#include "gcolor.h"

class gGridView;

typedef struct {
	int row;
	int col;
} gTablePair;

class gTableData
{
public:
	char *text;
	char *richText;
	char *markup;
	gPicture *picture;
	gFont *font;
	short  alignment;
	short  padding;
	gColor  bg;
	gColor  fg;
	bool wordWrap;

	void setText(const char *vl);
	void setRichText(const char *vl);
	void setFont(gFont *f) { gFont::assign(&font, f); }
	void setPicture(gPicture *p) { gPicture::assign(&picture, p); }
	void clear();
	gTableData();
	~gTableData();
};

class gTableSpan
{
	public:
		char rowspan;
		char colspan;
};


class gTable
{
	int *colpos;
	int *colsize;
	int *rowpos;
	int *rowsize;
	int columns;
	int rows;
	
protected:
	GHashTable *data;
	GHashTable *seldata;
	GHashTable *spanHash;

public:
	gTable();
	~gTable();

	bool doNotInvalidate;

	int columnCount   ();
	int rowCount      ();
	void          setRowCount      (int number);
	void          setColumnCount   (int number);
	char*         getFieldText     (int row, int col);
	void          setFieldText     (int row, int col, const char * value);
	char*         getFieldRichText     (int row, int col);
	void          setFieldRichText     (int row, int col, const char * value);

	gTableData *getData(int row, int col, bool create = false);
	
	int getColumnPos(int index);
	int getRowPos(int index);
	int           getColumnSize    (int position);
	void          setColumnSize    (int position,int value);
	int           getRowSize       (int position);
	void          setRowSize       (int position,int value);
	bool          getRowSelected   (int position);
	void          setRowSelected   (int position,bool value);
	
	void          clearField       (int row, int col);
	gColor        getFieldFg       (int row, int col);
	void          setFieldFg       (int row, int col,gColor value);
	gColor        getFieldBg       (int row, int col);
	void          setFieldBg       (int row, int col,gColor value);
	int           getFieldPadding  (int row, int col);
	void          setFieldPadding  (int row, int col,int value);
	int           getFieldAlignment  (int row, int col);
	void          setFieldAlignment  (int row, int col,int value);
	gPicture *getFieldPicture(int row, int col);
	void setFieldPicture(int row, int col, gPicture *value);
	gFont *getFieldFont(int row, int col);
	void setFieldFont(int row, int col, gFont *value);
	void clear();
	
	bool getFieldWordWrap(int row, int col);
	void setFieldWordWrap(int row, int col, bool value);
	
	bool          getFieldSelected (int row, int col);
	void          setFieldSelected (int row, int col,bool value);
	
	void setSpan(int row, int col, int rowspan, int colspan);
	void getSpan(int row, int col, int *rowspan, int *colspan);
  
  void moveCell(int srow, int scol, int drow, int dcol);
	
	void          (*voidCell)       (gTableData *fill, int row, int col, void *user);
	void          *userData;
};

class gTableRender : public gTable
{
public:
	gGridView *view;
	GtkWidget            *sf;
	GtkCellRendererText  *txt;
	GtkCellRendererPixbuf *pix;
	int                   offX;
	int                   offY;
	bool                  grid;

	void          renderCell(gTableData *data,GdkGC *gc,GdkRectangle *rect,bool sel);

	int firstRow, offRow;
	int firstCol, offCol;
	
	gTableRender  (gGridView *v);
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
	
	int findVisibleRow(int y);
	int findVisibleColumn(int x);
	int findColumn(int pos);
	int findRow(int pos);

  void removeRows(int start, int length);
  void insertRows(int start, int length);
	void          clearSelection    ();
	void selectRows(int start, int length, bool value);
	void          queryUpdate       (int row,int col);
	void          setDrawGrid       (bool vl);
	void          setRowSize        (int position,int value);
	void          setRowSelected    (int position,bool value);
	void          setColumnSize     (int position,int value);
	void          clearField        (int row, int col);
	void          setFieldText      (int row, int col, const char* value);
	void          setFieldRichText      (int row, int col, const char* value);
	void          setFieldFg        (int row, int col,gColor value);
	void          setFieldBg        (int row, int col,gColor value);
	void          setFieldPadding   (int row, int col,int value);
	//void          setFieldYPad      (int row, int col,int value);
	void          setFieldSelected  (int row, int col,bool value);
	void setFieldPicture(int row, int col, gPicture *value);
	void setFieldFont(int row, int col, gFont *value);
	void setFieldWordWrap(int row, int col, bool value);
	void clear();
	//int getBestWidth(int row, int col);
	int getBestHeight(int row, int col);
	int getBestWidth(int row, int col);
};

#endif
