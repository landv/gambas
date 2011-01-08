/***************************************************************************

	cscrollarea.h

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

#ifndef __CSCROLLAREA_H
#define __CSCROLLAREA_H

#include "gambas.h"

#include <QPaintEvent>
#include <QPixmap>
#include <QEvent>
//#include <QFrame>

#include "CWidget.h"
#include "CContainer.h"

#ifndef __CSCROLLAREA_CPP
extern GB_DESC CDrawingAreaDesc[];
#else

#define THIS    ((CSCROLLAREA *)_object)
#define WIDGET  ((MyDrawingArea *)((CWIDGET *)_object)->widget)

#endif

typedef
	struct {
		CWIDGET widget;
		QWidget *container;
		int arrangement;
		}
	CDRAWINGAREA;

class MyDrawingArea : public MyContainer
{
	Q_OBJECT

public:

	MyDrawingArea(QWidget *parent);
	~MyDrawingArea();

	int drawn;
	QPixmap *cache;
	
	//void setTransparent(bool);
	//bool isTransparent(void) { return transparent; }

	void updateCache();
	void setCached(bool);
	bool isCached() const { return _background != 0; }
	//QPixmap *getCache(void) { return cache; }
	//void refreshCache(void) { if (cache) setBackgroundPixmap(*cache); }

	void setBackground();
	void clearBackground();
	QPixmap *background() const { return _background; }
	void refreshBackground();
	void updateBackground();

	void setFrozen(bool f);
	bool isFrozen() const { return _frozen; }

	void setAllowFocus(bool f);
	bool isAllowFocus() const { return focusPolicy() != Qt::NoFocus; }
	
	bool isPaint() const { return _use_paint; }
	void setPaint(bool on) { _use_paint = on; }
	void redraw(QRect &r, bool frame = false);
	
	bool hasNoBackground() const { return _no_background; }
	void setNoBackground(bool on);
	void updateNoBackground();
	
	//bool isTransparent() { return _transparent; }
	//void setTransparent(bool on);

protected:

	virtual void setStaticContents(bool on);
	virtual void resizeEvent(QResizeEvent *);
	virtual void paintEvent(QPaintEvent *);
	virtual void hideEvent(QHideEvent *);
	//virtual void drawContents(QPainter *p);
	virtual void setPalette(const QPalette &);

private:

	QPixmap *_background;
	bool _frozen;
	bool _merge;
	bool _focus;
	int _event_mask;
	bool _use_paint;
	bool _set_background;
	bool _cached;
	bool _no_background;
};

#endif
