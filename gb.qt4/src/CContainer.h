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
#include <qframe.h>
//Added by qt3to4:
#include <QResizeEvent>
#include <QShowEvent>
#include <QChildEvent>
#include <QEvent>

#include "gambas.h"

#include "CConst.h"
#include "CWidget.h"

typedef
  struct {
    unsigned mode : 8;
    unsigned padding : 8;
    unsigned spacing : 8;
    unsigned locked : 1;
    unsigned user : 1;
    unsigned dirty : 1;
    unsigned autoresize : 1;
    unsigned margin : 1;
    unsigned _reserved : 3;
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
    unsigned mode : 8;
    unsigned padding : 8;
    unsigned spacing : 8;
    unsigned locked : 1;
    unsigned user : 1;
    unsigned dirty : 1;
    unsigned autoresize : 1;
    unsigned margin : 1;
    unsigned _reserved : 3;
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
DECLARE_PROPERTY(CCONTAINER_margin);

void CCONTAINER_arrange(void *_object);
void CCONTAINER_get_max_size(void *_object, int *w, int *h);
void CCONTAINER_insert_child(void *_object);

class MyContainer : public QFrame
{
  Q_OBJECT

public:

  MyContainer(QWidget *);

protected:

  virtual void showEvent(QShowEvent *);
  virtual void hideEvent(QHideEvent *);
};

#endif
