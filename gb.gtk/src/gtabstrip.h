#ifndef __GTABSTRIP_H
#define __GTABSTRIP_H

class gTabStripPage;

class gTabStrip : public gContainer
{
	friend class gTabStripPage;
	
public:
	gTabStrip(gContainer *parent);
	~gTabStrip();

//"Properties"
	int count() { return _pages->len; }
	int index();
	int orientation();
	void setOrientation(int vl);
	bool tabEnabled(int ind);
	gPicture* tabPicture(int ind);
	bool tabVisible(int ind);
	char *tabText(int ind);
	int tabCount(int ind);
	gControl *tabChild(int ind, int n);
	
	bool setCount(int vl);
	void setIndex(int vl);
	void setTabPicture(int ind, gPicture *pic);
	void setTabEnabled(int ind, bool vl);
	void setTabText(int ind, char *txt);
	void setTabVisible(int ind, bool vl);
	bool removeTab(int ind);

	virtual int childCount();
	virtual gControl *child(int index);

	virtual void setRealBackground(gColor color);
	virtual void setRealForeground(gColor color);

	virtual void setFont(gFont *ft);

//"Events"
	void (*onClick)(gTabStrip *sender);

//"Private"
	virtual GtkWidget *getContainer();
	
private:
	GPtrArray *_pages;
	gTabStripPage *get(int ind);
	int getRealIndex(GtkWidget *page);
	void destroyTab(int ind);
};

#endif
