/***************************************************************************

  CLabel.h

  (c) 2000-2007 Benoit Minisini <gambas@users.sourceforge.net>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 1, or (at your option)
  any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

***************************************************************************/

#ifndef __CLABEL_H
#define __CLABEL_H

#include "gambas.h"

#include <qlabel.h>
#include <qevent.h>

#include "CWidget.h"
#include "CPicture.h"

#ifndef __CLABEL_CPP
extern GB_DESC CLabelDesc[];
extern GB_DESC CTextLabelDesc[];
extern GB_DESC CSeparatorDesc[];
#else

#define QLABEL(object) ((QLabel *)((CWIDGET *)object)->widget)

#define THIS    ((CLABEL *)_object)
#define WIDGET  ((MyLabel *)((CWIDGET *)_object)->widget)

#endif

typedef
  struct {
    CWIDGET widget;
    }
  CLABEL;

typedef
  struct {
    CWIDGET widget;
    }
  CSEPARATOR;

class MyLabel : public QLabel
{
  Q_OBJECT

public:

  MyLabel(QWidget *parent);
  void calcMinimumHeight(bool adjust = false, bool noresize = false);
  bool getAutoResize() { return autoResize; }
  void setAutoResize(bool a) { autoResize = a; calcMinimumHeight(); }
  virtual void setText(const QString &);
  void adjust();
	bool isTransparent() { return transparent; }
	void setTransparent(bool transparent);
	
protected:

  virtual void fontChange(const QFont &);
  virtual void frameChanged();
  virtual void resizeEvent(QResizeEvent *);

private:

	void updateMask();
	
	unsigned autoResize : 1;
	unsigned transparent : 1;
	unsigned locked : 1;
};

class MySeparator : public QWidget
{
  Q_OBJECT

public:

  MySeparator(QWidget *);

protected:

  void paintEvent(QPaintEvent *);
};

#endif
