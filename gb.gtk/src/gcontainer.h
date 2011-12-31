/***************************************************************************

  gcontainer.h

  (c) 2000-2012 Benoît Minisini <gambas@users.sourceforge.net>

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
	unsigned dirty : 1;
	unsigned autoresize : 1;
	unsigned indent : 4;
	unsigned _reserved: 10;
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
	int indent();
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

	virtual int childCount();
	virtual gControl* child(int index);
	gControl *find(int x, int y);
	
	gContainerArrangement *getArrangement() { return &arrangement; }
	gContainerArrangement fullArrangement() { return arrangement; }
	void setFullArrangement(gContainerArrangement &arr) { arrangement = arr; performArrange(); }
	
	virtual void performArrange();

	virtual void setBackground(gColor color = COLOR_DEFAULT);
	virtual void setForeground(gColor color = COLOR_DEFAULT);
	virtual void setFont(gFont *ft);
	
	bool hasBackground() const;
	bool hasForeground() const;
	bool hasFont() const;

	virtual void resize(int w, int h);

	virtual void setVisible(bool vl);

	gContainer *proxyContainer() { return _proxyContainer ? _proxyContainer : this; }
	void setProxyContainer(gContainer *proxy) { if (_proxyContainer != this) _proxyContainer = proxy; else _proxyContainer = 0; }

//"Signals"
	void (*onArrange)(gContainer *sender);
	void (*onBeforeArrange)(gContainer *sender);
	//void (*onInsert)(gContainer *sender, gControl *child);

//"Private"
	GtkWidget *radiogroup;
	GList *ch_list;
	int _client_x, _client_y, _client_w, _client_h;
	
	virtual void insert(gControl *child, bool realize = false);
	virtual void remove(gControl *child);
	virtual void moveChild(gControl *child, int x, int y);
	virtual GtkWidget *getContainer();
	gControl *findFirstFocus();	
	void updateFocusChain();

	static int _arrangement_level;

private:
  void initialize();
	gContainerArrangement arrangement;
  gContainer *_proxyContainer;
};

#endif
