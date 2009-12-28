/***************************************************************************

  CDrawingArea.h

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

  void setCached(bool);
  bool isCached(void) { return _background != 0; }
  //QPixmap *getCache(void) { return cache; }
  //void refreshCache(void) { if (cache) setBackgroundPixmap(*cache); }

  void setBackground(void);
  void clearBackground(void);
  QPixmap *background() const { return _background; }
  void refreshBackground();

  void setFrozen(bool f);
  bool isFrozen() { return _frozen; }

  void setMerge(bool m);
  bool isMerge() { return _merge; }

  void setAllowFocus(bool f);
  bool isAllowFocus() { return focusPolicy() != Qt::NoFocus; }
	
	bool isPaint() { return _use_paint; }
	void setPaint(bool on) { _use_paint = on; }

protected:

  virtual void resizeEvent(QResizeEvent *);
  virtual void paintEvent(QPaintEvent *);
  //virtual void drawContents(QPainter *p);
	virtual void setPalette(const QPalette &);

private:

  QPixmap *_background;
  bool _frozen;
  bool _merge;
  bool _focus;
  int _event_mask;
	bool _use_paint;

  bool doResize();
};

#endif
