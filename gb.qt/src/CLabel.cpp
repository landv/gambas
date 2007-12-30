/***************************************************************************

  CLabel.cpp

  The Label and TextView class

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

#define __CLABEL_CPP

#include <qapplication.h>
#include <qlabel.h>
#include <qstyle.h>
#include <qbitmap.h>
#include <qpainter.h>
#include <qsimplerichtext.h>

#include "gambas.h"

#include "CConst.h"
#include "CLabel.h"

/*#define DEBUG_CBUTTON*/


/***************************************************************************/

BEGIN_METHOD(CLABEL_new, GB_OBJECT parent)

  QLabel *wid = new MyLabel(QCONTAINER(VARG(parent)));

  CWIDGET_new(wid, (void *)_object, "Label");

  wid->setTextFormat(Qt::PlainText);
  wid->setAlignment(Qt::AlignAuto + Qt::AlignVCenter); // + Qt::WordBreak);
  //wid->setLineWidth(2);
  wid->show();

END_METHOD


BEGIN_METHOD(CTEXTLABEL_new, GB_OBJECT parent)

  MyLabel *wid = new MyLabel(QCONTAINER(VARG(parent)));

  CWIDGET_new(wid, (void *)_object, "TextView");

  wid->setTextFormat(Qt::RichText);
  wid->setAlignment(Qt::AlignAuto + Qt::AlignTop + Qt::WordBreak);
  //wid->setLineWidth(2);
  //wid->setAutoResize(false);
  wid->show();

END_METHOD


BEGIN_PROPERTY(CLABEL_text)

  if (READ_PROPERTY)
    GB.ReturnNewZeroString(TO_UTF8(WIDGET->text()));
  else
    WIDGET->setText(QSTRING_PROP());

END_PROPERTY


BEGIN_PROPERTY(CLABEL_alignment)

  if (READ_PROPERTY)
    GB.ReturnInteger(CCONST_alignment(WIDGET->alignment() & ALIGN_MASK, ALIGN_NORMAL, false));
  else
    WIDGET->setAlignment(CCONST_alignment(VPROP(GB_INTEGER), ALIGN_NORMAL, true));

END_PROPERTY


BEGIN_PROPERTY(CLABEL_auto_resize)

  if (READ_PROPERTY)
    GB.ReturnInteger(WIDGET->getAutoResize());
  else
    WIDGET->setAutoResize(VPROP(GB_BOOLEAN));

END_PROPERTY

BEGIN_PROPERTY(CLABEL_margin)

	if (READ_PROPERTY)
		GB.ReturnInteger(WIDGET->margin());
	else
	{
		WIDGET->setMargin(VPROP(GB_INTEGER));
		WIDGET->update();
	}

END_PROPERTY


BEGIN_PROPERTY(CTEXTLABEL_alignment)

  if (READ_PROPERTY)
    GB.ReturnInteger(CCONST_alignment(WIDGET->alignment() & ALIGN_MASK, ALIGN_NORMAL, false));
  else
    WIDGET->setAlignment(CCONST_alignment(VPROP(GB_INTEGER), ALIGN_NORMAL, true) | Qt::WordBreak);

END_PROPERTY


BEGIN_METHOD_VOID(CLABEL_adjust)

	WIDGET->adjust();

END_METHOD

BEGIN_PROPERTY(CLABEL_transparent)

	if (READ_PROPERTY)
		GB.ReturnBoolean(WIDGET->isTransparent());
	else
		WIDGET->setTransparent(VPROP(GB_BOOLEAN));

END_PROPERTY


GB_DESC CLabelDesc[] =
{
  GB_DECLARE("Label", sizeof(CLABEL)), GB_INHERITS("Control"),

  GB_METHOD("_new", NULL, CLABEL_new, "(Parent)Container;"),

  GB_PROPERTY("Text", "s", CLABEL_text),
  GB_PROPERTY("Caption", "s", CLABEL_text),
  GB_PROPERTY("Alignment", "i", CLABEL_alignment),
  GB_PROPERTY("Border", "i", CWIDGET_border_full),
  GB_PROPERTY("AutoResize", "b", CLABEL_auto_resize),
  GB_PROPERTY("Padding", "i", CLABEL_margin),
  GB_PROPERTY("Transparent", "b", CLABEL_transparent),
  GB_METHOD("Adjust", NULL, CLABEL_adjust, NULL),

	LABEL_DESCRIPTION,

  GB_END_DECLARE
};


GB_DESC CTextLabelDesc[] =
{
  GB_DECLARE("TextLabel", sizeof(CLABEL)), GB_INHERITS("Control"),

  GB_METHOD("_new", NULL, CTEXTLABEL_new, "(Parent)Container;"),

  GB_PROPERTY("Text", "s", CLABEL_text),
  GB_PROPERTY("Caption", "s", CLABEL_text),
  GB_PROPERTY("Alignment", "i", CTEXTLABEL_alignment),
  GB_PROPERTY("Border", "i", CWIDGET_border_full),
  GB_PROPERTY("AutoResize", "b", CLABEL_auto_resize),
  GB_PROPERTY("Padding", "i", CLABEL_margin),
  GB_PROPERTY("Transparent", "b", CLABEL_transparent),
  GB_METHOD("Adjust", NULL, CLABEL_adjust, NULL),

	TEXTLABEL_DESCRIPTION,

  GB_END_DECLARE
};


/***************************************************************************/

BEGIN_METHOD(CSEPARATOR_new, GB_OBJECT parent)

  MySeparator *wid = new MySeparator(QCONTAINER(VARG(parent)));

  CWIDGET_new(wid, (void *)_object);

  wid->show();

END_METHOD


GB_DESC CSeparatorDesc[] =
{
  GB_DECLARE("Separator", sizeof(CSEPARATOR)), GB_INHERITS("Control"),

  GB_METHOD("_new", NULL, CSEPARATOR_new, "(Parent)Container;"),

	SEPARATOR_DESCRIPTION,

  GB_END_DECLARE
};


/** MyLabel *****************************************************************/

MyLabel::MyLabel(QWidget *parent) : QLabel(parent)
{
	autoResize = false;
	transparent = false;
	locked = false;
  calcMinimumHeight();
}


void MyLabel::fontChange(const QFont &font)
{
  QLabel::fontChange(font);
  calcMinimumHeight();
  updateMask();
}

void MyLabel::setText(const QString &text)
{
  QLabel::setText(text);
  calcMinimumHeight();
  updateMask();
  //qDebug("%s: %d", text.latin1(), isVisible());
}

void MyLabel::calcMinimumHeight(bool adjust)
{
	void *ob = CWidget::get(this);
	int w, h, nw, nh;

	if (!adjust && (!autoResize || CWIDGET_test_flag(ob, WF_DESIGN) || text().length() <= 0))
	{
    setMinimumSize(0, 0);
    setMaximumSize(QWIDGETSIZE_MAX, QWIDGETSIZE_MAX);
	}
	else
	{
		int f = frameWidth();
	  QRect br;
	  
		if (f > 0 && f < 4)
			f = 4;

	  if (textFormat() == Qt::RichText)
	  {
			QSimpleRichText rt(text(), font());
			w = width() - f * 2;
			
			rt.setWidth(w);
			nh = rt.height();
			nw = adjust ? rt.widthUsed() : w;
		}
	  else
	  {
		  QFontMetrics fm = fontMetrics();
  		br = fm.boundingRect(0, 0, QWIDGETSIZE_MAX, QWIDGETSIZE_MAX, alignment(), text());
  		nw = br.width();
 			nh = br.height();
			if (alignment() & Qt::AlignVCenter && (nh + f * 2) < height())
				nh = height() - f * 2;
		}
		
		w = nw + f * 2;
		h = nh + f * 2;
		

		//setMinimumSize(w, h);
		//setMaximumSize(QWIDGETSIZE_MAX, QWIDGETSIZE_MAX);
		
		//if (textFormat() != Qt::RichText && h < height())
		//	h = height();
		locked = true;
		//qDebug("%p: resize(%d, %d)", this, w, h);
		resize(w, h);
		locked = false;
	}
}

void MyLabel::frameChanged()
{
	calcMinimumHeight();
	updateMask();
}

void MyLabel::resizeEvent(QResizeEvent *e)
{
	QLabel::resizeEvent(e);
  
  if (autoResize && !locked && textFormat() == Qt::RichText && e->oldSize().width() != e->size().width())
  	calcMinimumHeight();
  	
	if ((alignment() & (AlignLeft|AlignTop) ) != (AlignLeft|AlignTop))
	{
		updateMask();
  	repaint(true);
  }
}

void MyLabel::adjust()
{
	calcMinimumHeight(true);
}

void MyLabel::setTransparent(bool t)
{
	if (transparent == t)
		return;
		
	transparent = t;
	updateMask();
}

static void make_alpha_from_white(QImage &img)
{
	uchar *p;
	int i, n;
	
	p = img.bits();
	n = img.width() * img.height();

	for (i = 0; i < n; i++, p += 4)
		p[3] = (p[2] * 11 + p[1] * 16 + p[0] * 5) / 32;
}


void MyLabel::updateMask()
{
	QString s = text();
	int bg, fg;
	CWIDGET *_object = CWidget::get(this);
	int i;
	
	if (!transparent)
	{
		clearMask();
		return;
	}
	
	fg = CWIDGET_get_foreground(_object);
	bg = CWIDGET_get_background(_object);
	CWIDGET_set_color(_object, 0, 0xFFFFFF);
	
	QPixmap *buffer = new QPixmap(width(), height());
	buffer->fill(Qt::black);
	QPainter p(buffer, this);
	//QLabel::drawFrame(&p);
	QRect rect(0, 0, width(), height());
	for (int i = 0; i < frameWidth(); i++)
		p.drawRect(rect.x() + i, rect.y() + i, rect.width() - i * 2, rect.height() - i * 2);
	QLabel::drawContents(&p);
	p.end();
  
	CWIDGET_set_color(_object, bg, fg);

  /*QPaintDevice *oldRedirect = QPainter::redirect(this);
	QPainter::redirect(this, buffer);
	//bool dblbfr = QSharedDoubleBuffer::isDisabled();
	//QSharedDoubleBuffer::setDisabled( TRUE );
	QPaintEvent e(rect(), false);
	QApplication::sendEvent(this, &e);
	//QSharedDoubleBuffer::setDisabled( dblbfr );
	QPainter::redirect(this, oldRedirect);*/

	QImage img = buffer->convertToImage();
	img.setAlphaBuffer(true);
	
	make_alpha_from_white(img);
	buffer->convertFromImage(img);
	if (buffer->mask()) setMask(*buffer->mask());
	delete buffer;	
}

/** class MySeparator ******************************************************/


MySeparator::MySeparator(QWidget *parent)
: QWidget(parent)
{
}


void MySeparator::paintEvent( QPaintEvent * )
{
	QPainter p( this );
	QStyle::SFlags flags = QStyle::Style_Default;

	if (width() < height())
		flags |= QStyle::Style_Horizontal;

	style().drawPrimitive( QStyle::PE_DockWindowSeparator, &p, rect(),
			 colorGroup(), flags );
}

