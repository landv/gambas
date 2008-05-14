#ifndef __GCONTAINER_H
#define __GCONTAINER_H

#include "gcontrol.h"

struct gContainerArrangement
{
	unsigned char mode;
	unsigned char spacing;
	unsigned char padding;
	unsigned autoresize:1;
	unsigned locked:1;
	unsigned user:1;
	unsigned dirty:1;
	unsigned _reserved:4;
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
	int spacing();
	virtual int clientWidth();
	virtual int clientHeight();
	virtual int clientX();
	virtual int clientY();

	void setArrange(int vl);
	void setUser(bool vl);
	void setAutoResize(bool vl);
	void setPadding(int vl);
	void setSpacing(int vl);

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

	gContainer *proxy() { return _proxy; }
	void setProxy(gContainer *proxy) { if (proxy != this) _proxy = proxy; }

//"Signals"
	void (*onArrange)(gContainer *sender);
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
