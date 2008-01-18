/***************************************************************************

  CContainer.h

  The Container class

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

#ifndef __CCONTAINER_H
#define __CCONTAINER_H

#include <qobject.h>
#if QT_VERSION >= 0x030200
#include <qobjectlist.h>
#else
#include <qobjcoll.h>
#endif
#include <qframe.h>

#include "gambas.h"

#include "CConst.h"
#include "CWidget.h"

typedef
  struct {
    unsigned char mode;
    unsigned char spacing;
    unsigned char padding;
    unsigned autoresize:1;
    unsigned locked:1;
    unsigned user:1;
    unsigned dirty:1;
    unsigned _reserved:4;
    }
  CARRANGEMENT;

#ifndef __CCONTAINER_CPP

extern GB_DESC CContainerDesc[];
extern GB_DESC CChildrenDesc[];
extern GB_DESC CUserControlDesc[];
extern GB_DESC CUserContainerDesc[];

#else

typedef
  struct {
    CWIDGET widget;
    QWidget *container;
    unsigned char mode;
    unsigned char spacing;
    unsigned char padding;
    unsigned autoresize:1;
    unsigned locked:1;
    unsigned user:1;
    unsigned dirty:1;
    unsigned _reserved:4;
    }
  CCONTAINER_ARRANGEMENT;

typedef
	struct {
		CCONTAINER parent;
		int32_t save;
		}
	CUSERCONTAINER;

#define WIDGET QWIDGET(_object)
#define THIS ((CCONTAINER *)_object)
#define CONTAINER (THIS->container)
#define THIS_ARRANGEMENT (((CCONTAINER_ARRANGEMENT *)_object))
#define THIS_USERCONTAINER (((CUSERCONTAINER *)_object))

//#define CCONTAINER_PROPERTIES CWIDGET_PROPERTIES ",Arrangement"

#endif

DECLARE_PROPERTY(CCONTAINER_arrangement);
DECLARE_PROPERTY(CCONTAINER_auto_resize);
DECLARE_PROPERTY(CCONTAINER_padding);
DECLARE_PROPERTY(CCONTAINER_spacing);

void CCONTAINER_arrange(void *_object);
void CCONTAINER_get_max_size(void *_object, int *w, int *h);

class MyContainer : public QFrame
{
  Q_OBJECT

public:

  MyContainer(QWidget *);

protected:

  virtual void frameChanged();
  virtual void resizeEvent(QResizeEvent *);
  virtual void childEvent(QChildEvent *);
  virtual void showEvent(QShowEvent *);
  virtual bool eventFilter(QObject *, QEvent *);
};

#endif
