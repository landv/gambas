/***************************************************************************

  CDrawingArea.h

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

#ifndef __CDRAWINGAREA_H
#define __CDRAWINGAREA_H

#include "gambas.h"

#include <QPaintEvent>
#include <QPixmap>
#include <QEvent>
//#include <QFrame>

#include "CWidget.h"
#include "CContainer.h"

#ifndef __CDRAWINGAREA_CPP
extern GB_DESC CDrawingAreaDesc[];
#else

#define THIS    ((CDRAWINGAREA *)_object)
#define WIDGET  ((MyDrawingArea *)((CWIDGET *)_object)->widget)

#endif

typedef
	struct {
		CWIDGET widget;
		QWidget *container;
		CARRANGEMENT arrangement;
		}
	CDRAWINGAREA;

class MyDrawingArea : public MyContainer
{
	Q_OBJECT

	friend class MyScrollArea;

public:

	MyDrawingArea(QWidget *parent);
	~MyDrawingArea();

	int drawn;
	QPixmap *cache;
	
	virtual void setVisible(bool visible);
	
	//void setTransparent(bool);
	//bool isTransparent(void) { return transparent; }

	void updateCache();
	void setCached(bool);
	bool isCached() const { return _cached; }
	//QPixmap *getCache(void) { return cache; }
	//void refreshCache(void) { if (cache) setBackgroundPixmap(*cache); }

	void clearBackground();
	Qt::HANDLE background() const { return _background; }
	void refreshBackground();
	void updateBackground();

	void setFrozen(bool f);
	bool isFrozen() const { return _frozen; }

	void setAllowFocus(bool f);
	bool isAllowFocus() const { return focusPolicy() != Qt::NoFocus; }
	
	void redraw(QRect &r, bool frame = false);
	
	bool hasNoBackground() const { return _no_background; }
	void setNoBackground(bool on);
	void updateNoBackground();
	
	void setDrawEvent(int event) { _draw_event = event; }
	bool inDrawEvent() const { return _in_draw_event; }
	
	void createBackground(int w, int h);
	void deleteBackground();
	
	QPixmap *getBackgroundPixmap();

public slots:
	
	void setBackground();
	//bool isTransparent() { return _transparent; }
	//void setTransparent(bool on);

protected:

	virtual void setStaticContents(bool on);
	virtual void resizeEvent(QResizeEvent *);
	virtual void paintEvent(QPaintEvent *);
	//virtual void showEvent(QShowEvent *);
	virtual void hideEvent(QHideEvent *);
	//virtual void drawContents(QPainter *p);
	virtual void setPalette(const QPalette &);
	virtual void changeEvent(QEvent *);

private:

	QPixmap _background_pixmap;
	Qt::HANDLE _background;
	int _background_w, _background_h;
	bool _frozen;
	bool _merge;
	bool _focus;
	int _event_mask;
	bool _set_background;
	bool _cached;
	bool _no_background;
	bool _in_draw_event;
	int _draw_event;
};

#endif
