#ifndef __GTREEVIEW_H
#define __GTREEVIEW_H

#include "gcontrol.h"
#include "gtree.h"

class gTreeView : public gControl
{
public:
	gTreeView(gContainer *parent, bool list = false);
	~gTreeView();

//"Properties"
	int count();
	
	int      itemChildren(char *key);
	char*     itemText(char *key);
	gPicture* itemPicture(char *key);
	bool      isItemSelected(char *key);
	bool      isItemExpanded(char *key);
	bool isItemEditable(char *key) { return tree->isRowEditable(key); }
	void setItemEditable(char *key, bool vl) { tree->setRowEditable(key, vl); }
	
	void      setItemExpanded(char *key, bool vl);
	void      setItemSelected(char *key, bool vl);
	void      setItemText(char *key, const char *vl);
	void      setItemPicture(char *key, gPicture *vl);
	
	int       mode();
	void      setMode(int vl);
	int      visibleWidth();
	int      visibleHeight();
	int clientWidth();
	int clientHeight();
	int      scrollBar();
	void      setScrollBar(int vl);
	bool			headers() { return tree->headers(); }
	void			setHeaders(bool vl) { tree->setHeaders(vl); }
	
	bool isEditable() { return tree->isEditable(); }
	void setEditable(bool vl) { tree->setEditable(vl); }

	bool isResizable() { return tree->isResizable(); }
	void setResizable(bool vl) { tree->setResizable(vl); }

	bool isAutoResize() { return tree->isAutoResize(); }
	void setAutoResize(bool vl) { tree->setAutoResize(vl); }

	char*     current() { return tree->cursor(); }
	void      setCurrent(char *key) { tree->setCursor(key); }
	char*     firstItem(char *parent = 0);
	char*     lastItem(char *parent = 0);
	char*     nextItem(char *vl);
	char*     prevItem(char *vl);
	char*     parentItem(char *vl);
	char*     belowItem(char *vl);
	char*     aboveItem(char *vl);
	
	void moveItemFirst(char *key);
	void moveItemLast(char *key);
	void moveItemBefore(char *key, char *before);
	void moveItemAfter(char *key, char *after);

	int columnCount() { return tree->columnCount(); }
	void setColumnCount(int ncol);
	char* columnText(int col);
	void setColumnText(int col, char *title);
	void setItemText(char *key, int col, const char *text);
	char* itemText(char *key, int col); 
	int columnAlignment(int col) { return tree->columnAlignment(col); }
	void setColumnAlignment(int col, int a) { tree->setColumnAlignment(col, a); }
	int columnWidth(int col) { return tree->columnWidth(col); }
	void setColumnWidth(int col, int w) { tree->setColumnWidth(col, w); }

	bool isSorted() { return tree->isSorted(); }
	void setSorted(bool v) { tree->setSorted(v); }
	int getSortColumn() { return tree->getSortColumn(); }
	void setSortColumn(int v) { tree->setSortColumn(v); }
	bool isSortAscending() { return tree->isSortAscending(); }
	void setSortAscending(bool v) { tree->setSortAscending(v); }

//"Methods"
	bool      add(char *key,char *text,gPicture *pic=NULL,char *after=NULL,char *parent=NULL);
	bool      remove(char *key);
	bool      exists(char *key);
	char*     find(int x, int y);
	void      clear() { tree->clear(); }
	void      clear(char *parent) { tree->clear(parent); }
	void      ensureItemVisible(char *key) { (*tree)[key]->ensureVisible(); }
	void      rectItem(char *key, int *x, int *y, int *w, int *h) { ((*tree)[key])->rect(x, y, w, h); }
	void			selectAll() { tree->selectAll(); }
	void			unselectAll() { tree->unselectAll(); }
	void startRename(char *key) { (*tree)[key]->startRename(); }
	char *intern(char *key);

//"Events"
	void (*onActivate)(gTreeView *sender, char *key);
	void (*onSelect)(gTreeView *sender);
	void (*onClick)(gTreeView *sender);
	void (*onCollapse)(gTreeView *sender, char *key);
	void (*onExpand)(gTreeView *sender, char *key);	
	void (*onRemove)(gTreeView *sender, char *key);
	void (*onRename)(gTreeView *sender, char *key);
	void (*onCancel)(gTreeView *sender, char *key);
	bool (*onCompare)(gTreeView *sender, char *keya, char *keyb, int *comp);

//"Private"
	char*     find(GtkTreePath *path) { return tree->pathToKey(path, false); }
	void refreshExpanded(char *parent, bool ex);

protected:
	gTree *tree;
	GtkWidget *treeview;
};

#endif
