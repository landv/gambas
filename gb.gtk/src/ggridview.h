#ifndef __GGRIDVIEW_H
#define __GGRIDVIEW_H

#include "gcontrol.h"
#include "tablerender.h"

class gGridView  : public gControl 
{
public:
	gGridView(gContainer *parent);
	~gGridView();

//"Properties"
	virtual void setFont(gFont *ft);
	int columnCount() { return render->columnCount(); }
	int rowCount() { return render->rowCount(); }
	bool    columnResizable(int index);
	bool    rowResizable(int index);
	bool    rowSelected(int index);
	int    columnWidth(int index) { return render->getColumnSize(index); }
	int minColumnWidth(int index);
	int 	  columnPos(int index) { return render->getColumnPos(index); }
	int    rowHeight(int index) { return render->getRowSize(index); }
	int minRowHeight(int index);
	int     rowPos(int index) { return render->getRowPos(index); }
	bool    drawGrid();
	bool    getBorder() { return getFrameBorder(); }
	int     selectionMode();
	void    setSelectionMode(int vl);
	void    setColumnCount(int vl);
	void    setRowCount(int vl);
	void    setColumnResizable(int index,bool vl);
	void    setRowResizable(int index,bool vl);
	void    setRowSelected(int index,bool vl);
	void    setColumnWidth(int index,int vl);
	void    setRowHeight(int index,int vl);
	void    setDrawGrid(bool vl);
	void    setBorder(bool vl) { setFrameBorder(vl ? BORDER_SUNKEN : BORDER_NONE); }
	
	void clearSelection() { render->clearSelection(); }
	void selectRows(int start, int end, bool value = true) { render->selectRows(start, end, value); }

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

	int    rowAt(int x);
	int    columnAt(int y);
	int     itemX(int col);
	int     itemY(int row);
	int     itemW(int col);
	int     itemH(int row);
	char*   itemText(int row,int col);
	gColor     itemFg(int row,int col);
	gColor     itemBg(int row,int col);
	int     itemPadding(int row,int col);
	int     itemAlignment(int row, int col);
	gPicture *itemPicture(int row, int col);
	gFont *itemFont(int row, int col);
	bool    itemSelected(int row,int col);
	void    ensureVisible(int row,int col);

	int	headersVisible();
	bool	footersVisible();
	char*   headerText(int col);
	char*   footerText(int col);
	char*   rowText(int col);
	void    setHeadersVisible(int vl);
	void    setFootersVisible(bool vl);
	void    setHeaderText(int col, const char *value);
	void    setFooterText(int col, const char *value);
	void    setRowText(int col, const char *value);
	int rowWidth();
	int lateralWidth() { return rowWidth(); }
	int footerHeight();
	int headerHeight();
	
	void    setItemPadding(int row, int col, int vl);
	void    setItemAlignment(int row, int col, int vl);
	void    setItemFg(int row,int col, gColor vl);
	void    setItemBg(int row,int col, gColor vl);
	void    setItemText(int row,int col, const char *vl);
	void setItemPicture(int row, int col, gPicture *vl);
	void setItemFont(int row, int col, gFont *vl);
	void    setItemSelected(int row,int col,bool vl);

	virtual void setBackground(gColor color = COLOR_DEFAULT);
	virtual void setForeground(gColor color = COLOR_DEFAULT);

//"Methods"
	void    setDataFunc(void *func,void *data);
	void    queryUpdate(int row,int col);
	void    clearItem(int row,int col);
	void    getCursor(int *row,int *col);
	void    setCursor(int row,int col);

//"Events"
	void      (*onChange)(gGridView *sender);
	void      (*onScroll)(gGridView *sender);
	void      (*onActivate)(gGridView *sender,int row,int col);
	void      (*onClick)(gGridView *sender,int row,int col);
	void      (*onColumnClick)(gGridView *send,int col);
	void      (*onFooterClick)(gGridView *send,int col);
	void      (*onRowClick)(gGridView *sender,int row);
	void      (*onColumnResize)(gGridView *send,int col);
	void      (*onRowResize)(gGridView *sender,int row);

//"Private"
	int sel_mode;
	int sel_row;
	int sel_current;
	int _index;
	int _last_col_width;
	int scroll;
	int cursor_col;
	int cursor_row;
	gTableRender  *render;
	GtkWidget     *header;
	GtkWidget     *footer;
	GtkWidget     *lateral;
	GtkWidget* hbar;
	GtkWidget* vbar;
	GtkWidget* contents;
	GHashTable    *hdata;
	GHashTable    *vdata;
	guint scroll_timer;
	int mouse_pos;
	
	void          calculateBars();
	void updateLateralWidth(int w);
	void updateHeaders();
	int findColumn(int pos) { return render->findColumn(pos); }
	int findRow(int pos) { return render->findRow(pos); }
	int findColumnSeparation(int pos);
	int findRowSeparation(int pos);
	void updateLastColumn();
	void startScrollTimer(GSourceFunc func);
	void stopScrollTimer();
};

#endif
