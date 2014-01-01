/***************************************************************************

  gcontainer.h

  (c) 2000-2013 Benoît Minisini <gambas@users.sourceforge.net>

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

#ifndef __GCONTAINER_H
#define __GCONTAINER_H

#include "gcontrol.h"

struct gContainerArrangement
{
	unsigned mode : 4;
	unsigned user : 1;
	unsigned locked : 1;
	unsigned margin : 1;
	unsigned spacing : 1;
	unsigned padding : 8;
	unsigned indent : 4;
	unsigned dirty : 1;
	unsigned autoresize : 1;
	unsigned invert : 1;
	unsigned _reserved: 9;
}; 

class gContainer : public gControl
{
public:
	gContainer();
	gContainer(gContainer *parent);
	~gContainer();

	int arrange() const { return arrangement.mode; }
	bool autoResize() const { return arrangement.autoresize; }
	bool isUser() const { return arrangement.user; }
	int padding() const { return arrangement.padding; }
	bool spacing() const { return arrangement.spacing; }
	bool margin() const { return arrangement.margin; }
	int indent() const { return arrangement.indent; }
	bool invert() const { return arrangement.invert; }
	
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
	void setIndent(int vl);
	void setInvert(bool vl);

	virtual int childCount() const;
	virtual gControl *child(int index) const;
	
	int childIndex(gControl *ch) const;
	
	virtual gControl *find(int x, int y);
	
	gContainerArrangement *getArrangement() { return &arrangement; }
	gContainerArrangement fullArrangement() { return arrangement; }
	void setFullArrangement(gContainerArrangement *arr);
	
	virtual void performArrange();

#ifndef GTK3
	virtual void setBackground(gColor color = COLOR_DEFAULT);
#else
	virtual void updateColor();
#endif
	virtual void setForeground(gColor color = COLOR_DEFAULT);
	virtual void updateFont();
	
	bool hasBackground() const;
	bool hasForeground() const;

	virtual void resize(int w, int h);

	virtual void setVisible(bool vl);

	gContainer *proxyContainer() { return _proxyContainer ? _proxyContainer : this; }
	void setProxyContainer(gContainer *proxy) { if (_proxyContainer != this) _proxyContainer = proxy; else _proxyContainer = 0; }
	
	void disableArrangement();
	void enableArrangement();

//"Signals"
	void (*onArrange)(gContainer *sender);
	void (*onBeforeArrange)(gContainer *sender);
	//void (*onInsert)(gContainer *sender, gControl *child);

//"Private"
	GtkWidget *radiogroup;
	GPtrArray *_children;
	int _client_x, _client_y, _client_w, _client_h;
	
	virtual void insert(gControl *child, bool realize = false);
	virtual void remove(gControl *child);
	virtual void moveChild(gControl *child, int x, int y);
	virtual void reparent(gContainer *newpr, int x, int y);
	void hideHiddenChildren();
	virtual GtkWidget *getContainer();
	gControl *findFirstFocus();	
	void updateFocusChain();

	static int _arrangement_level;

private:
  void initialize();
	gContainerArrangement arrangement;
  gContainer *_proxyContainer;
	unsigned _did_arrangement : 1;
	unsigned _no_arrangement : 7;
};

#endif
