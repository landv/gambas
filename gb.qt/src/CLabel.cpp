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
#include <qpainter.h>
#include <qsimplerichtext.h>

#include "gambas.h"

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
    GB.ReturnInteger(WIDGET->alignment() & ALIGN_MASK);
  else
    WIDGET->setAlignment((VPROP(GB_INTEGER) & ALIGN_MASK));

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
    GB.ReturnInteger(WIDGET->alignment() & ALIGN_MASK);
  else
    WIDGET->setAlignment((VPROP(GB_INTEGER) & ALIGN_MASK) | Qt::WordBreak);

END_PROPERTY


BEGIN_METHOD_VOID(CLABEL_adjust)

	WIDGET->adjust();

END_METHOD


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
  GB_METHOD("Adjust", NULL, CLABEL_adjust, NULL),

  GB_CONSTANT("_Properties", "s", "*,Padding{Range:0;64},AutoResize,Text,Alignment{Align.*}=Normal,Border{Border.*}"),
  GB_CONSTANT("_DefaultSize", "s", "24,3"),

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
  GB_METHOD("Adjust", NULL, CLABEL_adjust, NULL),

  GB_CONSTANT("_Properties", "s", "*,Padding{Range:0;64},AutoResize,Text,Alignment{Align.*}=TopNormal,Border{Border.*}"),
  GB_CONSTANT("_DefaultSize", "s", "24,6"),

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

  GB_CONSTANT("_Properties", "s", "*"),
  GB_CONSTANT("_DefaultSize", "s", "1,3"),

  GB_END_DECLARE
};


/** MyLabel *****************************************************************/

MyLabel::MyLabel(QWidget *parent) : QLabel(parent)
{
	autoResize = false;
  calcMinimumHeight();
}


void MyLabel::fontChange(const QFont &font)
{
  QLabel::fontChange(font);
  calcMinimumHeight();
}

void MyLabel::setText(const QString &text)
{
  QLabel::setText(text);
  calcMinimumHeight();
  //qDebug("%s: %d", text.latin1(), isVisible());
}

void MyLabel::calcMinimumHeight()
{
	void *ob = CWidget::get(this);
	int w, h, nw, nh;

	if (!autoResize || CWIDGET_test_flag(ob, WF_DESIGN) || text().length() <= 0)
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
			nw = rt.widthUsed(); // The return value is sometimes buggy, but I didn't find any workaround
			
			//qDebug("richText: width = %d / rt.width = %d / rt.height = %d / rt.widthUsed = %d /%s", width(), rt.width(), rt.height(), rt.widthUsed(),text().latin1());
			/*while (w > 4)
			{
				rt.setWidth(w - 4);
				if (rt.height() != nh)
					break;
				w -= 4;
			}
			
			if (w > 4)
				nw = w;*/
			//br = fm.boundingRect(0, 0, width(), QWIDGETSIZE_MAX, alignment(), text());
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
		
  	resize(w, h);
	}
}

void MyLabel::frameChanged()
{
	calcMinimumHeight();
}

void MyLabel::resizeEvent(QResizeEvent *e)
{
	QLabel::resizeEvent(e);
  //if (testWFlags(WResizeNoErase))
  //if (textFormat() == Qt::RichText && e->oldSize().width() != e->size().width())
  //	calcMinimumHeight();
	if ((alignment() & (AlignLeft|AlignTop) ) != (AlignLeft|AlignTop))
  	repaint(true);
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

