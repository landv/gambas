/***************************************************************************

  CSplitter.cpp

  The Splitter class

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

#define __CSPLITTER_CPP

#include "main.h"

#include <qstringlist.h>
#include <qsplitter.h>
//Added by qt3to4:
#include <QChildEvent>
#include <QEvent>

#include "CConst.h"
#include "CContainer.h"
#include "CSplitter.h"

//#define DEBUG_ME

//DECLARE_EVENT(EVENT_Change);
DECLARE_EVENT(EVENT_Resize);


/*BEGIN_METHOD(CSPLITTER_new, GB_OBJECT parent)

  QSplitter *wid = new MySplitter(QCONTAINER(VARG(parent)));

  QT.InitWidget(wid, (void *)_object);

  THIS->widget.container = wid;

  //wid->setFrameStyle(QFrame::NoFrame);
  wid->setOpaqueResize(true);
  wid->show();

END_METHOD*/


BEGIN_METHOD(CHSPLIT_new, GB_OBJECT parent)

  QSplitter *wid = new MySplitter(QCONTAINER(VARG(parent)));

  wid->setOrientation(Qt::Horizontal);
  wid->setOpaqueResize(true);

  CWIDGET_new(wid, (void *)_object);

  THIS->container = wid;

END_METHOD


BEGIN_METHOD(CVSPLIT_new, GB_OBJECT parent)

  QSplitter *wid = new MySplitter(QCONTAINER(VARG(parent)));

  CWIDGET_new(wid, (void *)_object);

  THIS->container = wid;

  wid->setOrientation(Qt::Vertical);
  wid->setOpaqueResize(true);
  wid->show();

END_METHOD


BEGIN_PROPERTY(CSPLITTER_orientation)

  if (READ_PROPERTY)
    GB.ReturnInteger(WIDGET->orientation());
  else
    WIDGET->setOrientation((Qt::Orientation)VPROP(GB_INTEGER));

END_PROPERTY


BEGIN_PROPERTY(CSPLITTER_layout)

  int i;
  QList<int> list;
  char buffer[16];
  int pos;

  if (READ_PROPERTY)
  {
    list = WIDGET->sizes();
    QString s;

    for (i = 0; i < list.count(); i++)
    {
      pos = list.at(i);
      if (pos <= 1)
        pos = 0;
      sprintf(buffer, "%d", pos);
      if (i > 0)
        s += ',';
      s += buffer;
    }

    #ifdef DEBUG_ME
    qDebug("Splitter.Layout -> %s", s.latin1());
    #endif

    GB.ReturnNewZeroString(s.toLatin1());
  }
  else
  {
    QString s = QSTRING_PROP();
    QStringList sl = s.split(',');
    int sum;
    int dim;

    if (s.length() == 0)
      return;

    #ifdef DEBUG_ME
    qDebug("Splitter.Layout = %s", s.latin1());
    #endif

    dim = WIDGET->orientation() == Qt::Horizontal ? WIDGET->width() : WIDGET->height();
    dim -= WIDGET->handleWidth() * WIDGET->handleCount();

    for (i = 0, sum = 0; i < sl.count(); i++)
    {
      pos = sl[i].toInt();
      if (pos < 1) // why <= before ?
        pos = 0;
      sum += pos;
    }

    for (i = 0; i < sl.count(); i++)
    {
      pos = sl[i].toInt();
      if (pos < 1) // why <= before ?
        pos = 0;
      else
        pos = pos * dim / sum;
      #ifdef DEBUG_ME
      qDebug("Splitter.Layout[%ld] = %ld  dim = %d  sum = %d  pos = %d", i, sl[i].toInt(), dim, sum, pos);
      #endif
      list.append(pos);
    }

    WIDGET->setSizes(list);
  }

END_PROPERTY

#if 0
BEGIN_PROPERTY(CSPLITTER_design)

  CWIDGET_design(_object, _param);

  if (READ_PROPERTY)
    return;

  WIDGET->setDesign(CWIDGET_test_flag(_object, WF_DESIGN));

END_PROPERTY


BEGIN_PROPERTY(CSPLITTER_pos)

  if (READ_PROPERTY)
    GB.ReturnFloat(WIDGET->pos());
  else
    WIDGET->setPos(VPROP(GB_FLOAT));

END_PROPERTY


BEGIN_PROPERTY(CSPLITTER_last_pos)

  GB.ReturnFloat(WIDGET->lastPos());

END_PROPERTY


BEGIN_PROPERTY(CSPLITTER_border)

  if (READ_PROPERTY)
    GB.ReturnBoolean(WIDGET->frameStyle() != Q3Frame::NoFrame);
  else
  {
    if (VPROP(GB_BOOLEAN))
    {
      WIDGET->setFrameStyle(Q3Frame::Panel + Q3Frame::Raised);
      WIDGET->setLineWidth(1);
    }
    else
      WIDGET->setFrameStyle(Q3Frame::NoFrame);
  }

END_PROPERTY
#endif

GB_DESC CHSplitDesc[] =
{
  GB_DECLARE("HSplit", sizeof(CSPLITTER)), GB_INHERITS("Container"),

  GB_METHOD("_new", NULL, CHSPLIT_new, "(Parent)Container;"),

  GB_PROPERTY("Layout", "s", CSPLITTER_layout),
  GB_PROPERTY("Settings", "s", CSPLITTER_layout),

  GB_EVENT("Resize", NULL, NULL, &EVENT_Resize),

	HSPLIT_DESCRIPTION,

  GB_END_DECLARE
};


GB_DESC CVSplitDesc[] =
{
  GB_DECLARE("VSplit", sizeof(CSPLITTER)), GB_INHERITS("Container"),

  GB_METHOD("_new", NULL, CVSPLIT_new, "(Parent)Container;"),

  GB_PROPERTY("Layout", "s", CSPLITTER_layout),
  GB_PROPERTY("Settings", "s", CSPLITTER_layout),

  GB_EVENT("Resize", NULL, NULL, &EVENT_Resize),

	VSPLIT_DESCRIPTION,

  GB_END_DECLARE
};



/***************************************************************************

  MySplitter

***************************************************************************/


MySplitter::MySplitter(QWidget *parent) :
  QSplitter(parent)
{
  _event = false;
  installEventFilter(this);
  //setAttribute(Qt::WA_NoSystemBackground, true);
}

static void send_event(QT_WIDGET *ob)
{
  if (!ob->widget)
    return;

  GB.Raise(ob, EVENT_Resize, 0);
  ((MySplitter *)ob->widget)->_event = false;
}

bool MySplitter::eventFilter(QObject *o, QEvent *e)
{
  if (o == this)
  {
    if (e->type() == QEvent::ChildAdded)
    {
      QChildEvent *ce = (QChildEvent *)e;

      if (ce->child()->isWidgetType())
      {
        //setResizeMode((QWidget *)ce->child(), QSplitter::Stretch);
        ce->child()->installEventFilter(this);
      }
    }
    else if (e->type() == QEvent::ChildRemoved)
    {
      QChildEvent *ce = (QChildEvent *)e;

      if (ce->child()->isWidgetType())
        ce->child()->removeEventFilter(this);
    }
  }
  else if (e->type() == QEvent::Resize && !_event)
  {
    _event = true;
    GB.Post((void (*)())send_event, (intptr_t)CWidget::get(this));
  }

  return QObject::eventFilter(o, e);
}

int MySplitter::handleCount()
{
  QObjectList list = children();
  CWIDGET *ob;
  int i, nh = -1;
  
  for (i = 0; i < list.count(); i++)
  {
  	ob = CWidget::getReal(list.at(i));
  	if (ob && ob->widget && !ob->widget->isHidden())
  		nh++;
  }
  		
  return nh;
}
