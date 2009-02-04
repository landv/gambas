#ifndef __GCONTAINER_H
#define __GCONTAINER_H

#include "gcontrol.h"

struct gContainerArrangement
{
	unsigned mode : 8;
	unsigned padding : 8;
	unsigned spacing : 8;
	unsigned locked : 1;
	unsigned user : 1;
	unsigned dirty : 1;
	unsigned autoresize : 1;
	unsigned margin : 1;
	unsigned _reserved : 3;
}; 

class gContainer : public gControl
{
public:
	gContainer();
	gContainer(gContainer *parent);
	~gContainer();

	int arrange();
	bool autoResize();
	bool isUser() { return arrangement.user; }
	int padding();
	bool spacing();
	bool margin();
	virtual int clientWidth();
	virtual int clientHeight();
	virtual int clientX();
	virtual int clientY();
	virtual int containerX();
	virtual int containerY();

	void setArrange(int vl);
	void setUser(bool vl);
	void setAutoResize(bool vl);
	void setPadding(int vl);
	void setSpacing(bool vl);
	void setMargin(bool vl);

	virtual int childCount();
	virtual gControl* child(int index);
	gControl *find(int x, int y);
	
	gContainerArrangement *getArrangement() { return &arrangement; }
	gContainerArrangement fullArrangement() { return arrangement; }
	void setFullArrangement(gContainerArrangement &arr) { arrangement = arr; performArrange(); }
	
	virtual void performArrange();

	virtual void setBackground(gColor color = COLOR_DEFAULT);
	virtual void setForeground(gColor color = COLOR_DEFAULT);

	virtual void resize(int w, int h);

	virtual void setVisible(bool vl);

	gContainer *proxy() { return _proxy ? _proxy : this; }
	void setProxy(gContainer *proxy) { if (proxy != this) _proxy = proxy; else _proxy = 0; }

//"Signals"
	void (*onArrange)(gContainer *sender);
	void (*onBeforeArrange)(gContainer *sender);
	//void (*onInsert)(gContainer *sender, gControl *child);

//"Private"
	GtkWidget *radiogroup;
	GList *ch_list;
	int _client_w, _client_h;
	
	virtual void insert(gControl *child, int x, int y) { insert(child); move(x, y); }
	virtual void insert(gControl *child);
	virtual void remove(gControl *child);
	virtual GtkWidget *getContainer();
	gControl *findFirstFocus();	
	void updateFocusChain();

	static int _arrangement_level;

private:
  void initialize();
	gContainerArrangement arrangement;
  gContainer *_proxy;
};

#endif
