/***************************************************************************

  CPictureBox.cpp

  The PictureBox class

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

#define __CPICTUREBOX_CPP



#include <qapplication.h>
#include <qlabel.h>
#include <qpixmap.h>
#include <qframe.h>

#include "gambas.h"

#include "CConst.h"
#include "CPicture.h"
#include "CPictureBox.h"

/*#define DEBUG_CBUTTON*/

#if 0
MyLabel::MyLabel(QWidget *parent)
: QLabel(parent)
{
}

MyLabel::~MyLabel()
{
}


void MyLabel::resizeEvent( QResizeEvent* e )
{
  QFrame::resizeEvent(e);
}
#endif

BEGIN_METHOD(CPICTUREBOX_new, GB_OBJECT parent)

  QLabel *wid = new QLabel(QCONTAINER(VARG(parent)));

  THIS->picture = NULL;

  CWIDGET_new(wid, (void *)_object, "Image");

  wid->setAlignment(Qt::AlignLeft | Qt::AlignTop);
  wid->show();

END_METHOD


BEGIN_METHOD_VOID(CPICTUREBOX_free)

  CLEAR_PICTURE(&(THIS->picture));

  //CALL_METHOD_VOID(CWIDGET_delete);

END_METHOD


BEGIN_PROPERTY(CPICTUREBOX_picture)

  if (READ_PROPERTY)
    GB.ReturnObject(THIS->picture);
  else
  {
    SET_PICTURE(WIDGET->setPixmap, WIDGET->setPicture, &(THIS->picture), PROP(GB_OBJECT));

    /*
    if (wid->pixmap() != 0)
      if (!wid->hasScaledContents())
        wid->resize(wid->pixmap()->width(), wid->pixmap()->height());
    */
  }

END_PROPERTY


BEGIN_PROPERTY(CPICTUREBOX_stretch)

  if (READ_PROPERTY)
    GB.ReturnBoolean(WIDGET->hasScaledContents());
  else
    WIDGET->setScaledContents(VPROP(GB_BOOLEAN));

END_PROPERTY


/*
BEGIN_PROPERTY(CPICTUREBOX_border)

  if (READ_PROPERTY)
    GB.ReturnBoolean(QLABEL(_object)->frameStyle() != QFrame::NoFrame);
  else
  {
    if (PROPERTY(char) != 0)
      QLABEL(_object)->setFrameStyle(QFrame::StyledPanel | QFrame::Sunken);
    else
      QLABEL(_object)->setFrameStyle(QFrame::NoFrame);
  }

END_PROPERTY
*/

BEGIN_PROPERTY(CPICTUREBOX_alignment)

  if (READ_PROPERTY)
    GB.ReturnInteger(CCONST_alignment(WIDGET->alignment() & ALIGN_MASK, ALIGN_TOP_LEFT, false));
  else
    WIDGET->setAlignment(CCONST_alignment(VPROP(GB_INTEGER), ALIGN_TOP_LEFT, true));

END_PROPERTY
/*
BEGIN_PROPERTY(CPICTUREBOX_transparent)

  bool trans;

  if (READ_PROPERTY)
    GB.ReturnBoolean(THIS->transparent);
  else
  {
    trans = PROPERTY(GB_BOOLEAN);

    if (THIS->transparent == trans)
      return;

    THIS->transparent = trans;



  }

END_PROPERTY
*/


GB_DESC CPictureBoxDesc[] =
{
  GB_DECLARE("PictureBox", sizeof(CPICTUREBOX)), GB_INHERITS("Control"),

  GB_METHOD("_new", NULL, CPICTUREBOX_new, "(Parent)Container;"),
  GB_METHOD("_free", NULL, CPICTUREBOX_free, NULL),

  GB_PROPERTY("Picture", "Picture", CPICTUREBOX_picture),
  GB_PROPERTY("Stretch", "b", CPICTUREBOX_stretch),
  //GB_PROPERTY("Transparent", "b", CPICTUREBOX_transparent),
  GB_PROPERTY("Border", "i", CWIDGET_border_full),
  GB_PROPERTY("Alignment", "i", CPICTUREBOX_alignment),

	PICTUREBOX_DESCRIPTION,

  GB_END_DECLARE
};


