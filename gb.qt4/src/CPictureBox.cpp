/***************************************************************************

  CPictureBox.cpp

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

#define __CPICTUREBOX_CPP

#include <qapplication.h>
#include <qlabel.h>
#include <qpixmap.h>

#include "gambas.h"

#include "CConst.h"
#include "CPicture.h"
#include "CPictureBox.h"


MyPictureBox::MyPictureBox(QWidget *parent)
: QLabel(parent)
{
	_autoresize = false;
}

void MyPictureBox::updateBackground()
{
	/*if (pixmap() && pixmap()->hasAlpha())
	{
		clearWFlags(Qt::WNoAutoErase);
	}
	else
	{
		setWFlags(Qt::WNoAutoErase);
	}*/
}

void MyPictureBox::setPalette(const QPalette &pal)
{
	QLabel::setPalette(pal);
	repaint();
}

void MyPictureBox::setAutoResize(bool v)
{
	_autoresize = v;
	updateSize();
}

void MyPictureBox::updateSize()
{
	const QPixmap *p;
	
	if (hasScaledContents() || !_autoresize)
		return;
		
	p = pixmap();
	if (p && !p->isNull())
	{
		resize(p->width() + frameWidth() * 2, p->height() + frameWidth() * 2);
	}
}

void MyPictureBox::resizeEvent(QResizeEvent* e)
{
	QLabel::resizeEvent(e);
	updateBackground();
  if (e->size() == e->oldSize()) // margin has changed (frameChanged() before)
  {
  	if (isAutoResize())
  		adjustSize();
  }
}

void MyPictureBox::adjustSize()
{
	const QPixmap *p = pixmap();
	QRect r;
	
	if (p && !p->isNull())
	{
		r = contentsRect();
		resize(p->width() + width() - r.width(), p->height() + height() - r.height());
	}
}


BEGIN_METHOD(CPICTUREBOX_new, GB_OBJECT parent)

	QLabel *wid = new MyPictureBox(QCONTAINER(VARG(parent)));

	THIS->picture = NULL;

	wid->setAlignment(Qt::AlignLeft | Qt::AlignTop);

	CWIDGET_new(wid, (void *)_object);

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

		WIDGET->updateBackground();
		WIDGET->updateSize();
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


BEGIN_PROPERTY(CPICTUREBOX_auto_resize)

	if (READ_PROPERTY)
		GB.ReturnBoolean(WIDGET->isAutoResize());
	else
		WIDGET->setAutoResize(VPROP(GB_BOOLEAN));

END_PROPERTY


BEGIN_PROPERTY(CPICTUREBOX_alignment)

	if (READ_PROPERTY)
		GB.ReturnInteger(CCONST_alignment(WIDGET->alignment() & ALIGN_MASK, ALIGN_TOP_LEFT, false));
	else
		WIDGET->setAlignment((Qt::Alignment)CCONST_alignment(VPROP(GB_INTEGER), ALIGN_TOP_LEFT, true));

END_PROPERTY


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
	GB_PROPERTY("AutoResize", "b", CPICTUREBOX_auto_resize),

	PICTUREBOX_DESCRIPTION,

	GB_END_DECLARE
};


