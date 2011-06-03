/***************************************************************************

  CLabel.cpp

  (c) 2000-2011 Benoît Minisini <gambas@users.sourceforge.net>

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

#define __CLABEL_CPP

#include <qapplication.h>
#include <qlabel.h>
#include <qstyle.h>
#include <qbitmap.h>
#include <qpainter.h>
//Added by qt3to4:
#include <QPaintEvent>
#include <QResizeEvent>
#include <QPixmap>
#include <QStyleOption>
#include <QTextDocument>

#include "gambas.h"

#include "CConst.h"
#include "CColor.h"
#include "CLabel.h"

/*#define DEBUG_CBUTTON*/


/***************************************************************************/

BEGIN_METHOD(CLABEL_new, GB_OBJECT parent)

  QLabel *wid = new MyLabel(QCONTAINER(VARG(parent)));

  wid->setTextFormat(Qt::PlainText);
  wid->setAlignment(Qt::AlignLeft | Qt::AlignVCenter); // + Qt::WordBreak);
	//THIS->widget.flag.fillBackground = TRUE;
	
  CWIDGET_new(wid, (void *)_object);

END_METHOD


BEGIN_METHOD(CTEXTLABEL_new, GB_OBJECT parent)

  MyLabel *wid = new MyLabel(QCONTAINER(VARG(parent)));

  wid->setTextFormat(Qt::RichText);
  wid->setAlignment(Qt::AlignLeft | Qt::AlignTop);
  wid->setWordWrap(true);
	//THIS->widget.flag.fillBackground = TRUE;

  CWIDGET_new(wid, (void *)_object);

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
    WIDGET->setAlignment((Qt::Alignment)CCONST_alignment(VPROP(GB_INTEGER), ALIGN_NORMAL, true));

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
		WIDGET->calcMinimumHeight();
		WIDGET->update();
	}

END_PROPERTY


BEGIN_PROPERTY(CTEXTLABEL_alignment)

  if (READ_PROPERTY)
    GB.ReturnInteger(CCONST_alignment(WIDGET->alignment() & ALIGN_MASK, ALIGN_NORMAL, false));
  else
    WIDGET->setAlignment((Qt::Alignment)CCONST_alignment(VPROP(GB_INTEGER), ALIGN_NORMAL, true));

END_PROPERTY


BEGIN_METHOD_VOID(CLABEL_adjust)

	WIDGET->adjust();

END_METHOD

BEGIN_PROPERTY(CLABEL_transparent)

	if (READ_PROPERTY)
		GB.ReturnBoolean(THIS->transparent);
	else
		THIS->transparent = VPROP(GB_BOOLEAN);

END_PROPERTY

BEGIN_PROPERTY(Label_Wrap)

	if (READ_PROPERTY)
		GB.ReturnBoolean(WIDGET->wordWrap());
	else
		WIDGET->setWordWrap(VPROP(GB_BOOLEAN));

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
  GB_PROPERTY("Wrap", "b", Label_Wrap),
  GB_METHOD("Adjust", NULL, CLABEL_adjust, NULL),

	TEXTLABEL_DESCRIPTION,

  GB_END_DECLARE
};


/***************************************************************************/

BEGIN_METHOD(CSEPARATOR_new, GB_OBJECT parent)

  MySeparator *wid = new MySeparator(QCONTAINER(VARG(parent)));

  CWIDGET_new(wid, (void *)_object);

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
	locked = false;
	setIndent(0);
  calcMinimumHeight();
}


void MyLabel::changeEvent(QEvent *e)
{
  QLabel::changeEvent(e);
	if (e->type() == QEvent::FontChange || e->type() == QEvent::StyleChange)
		calcMinimumHeight();
}

void MyLabel::setText(const QString &text)
{
  QLabel::setText(text);
  calcMinimumHeight();
  //qDebug("%s: %d", text.latin1(), isVisible());
}

void MyLabel::calcMinimumHeight(bool adjust)
{
	void *_object = CWidget::getReal(this);

	if (!THIS || (!autoResize && !adjust) || CWIDGET_test_flag(THIS, WF_DESIGN) || text().length() <= 0)
		return;
	
	//qDebug("calcMinimumHeight: %p %s", ob, ((CWIDGET *)ob)->name);
	
	int w, h, nw, nh;
	int f = frameWidth() + margin();
	QRect br;
	
	//if (f > 0 && f < 4)
	//	f = 4;

	//qDebug("calcMinimumHeight: f = %d", f);
	
	if (textFormat() == Qt::RichText)
	{
		QTextDocument doc;
		
		doc.setDefaultFont(font());
		doc.setDocumentMargin(0);
		doc.setHtml(text());
		
		if (wordWrap())
		{
			w = width() - f * 2;
			doc.setTextWidth(w);
			
			if (adjust)
			{
				//doc.adjustSize();
				nw = doc.idealWidth();
				doc.setTextWidth(nw);
				nh = doc.size().height(); // Why (- 6) ? Don't know...
				nw = doc.size().width();
				
			}
			else
			{
				nh = doc.size().height(); // Why (- 6) ? Don't know...
				nw = w;
			}
		}
		else
		{
			nh = doc.size().height(); // Why (- 6) ? Don't know...
			nw = doc.size().width();
		}
	}
	else
	{
		QFontMetrics fm = fontMetrics();
		br = fm.boundingRect(0, 0, QWIDGETSIZE_MAX, QWIDGETSIZE_MAX, alignment(), text());
		nw = br.width();
		nh = br.height();
	}
	
	w = nw + f * 2;
	h = nh + f * 2;

	if (alignment() & Qt::AlignVCenter && nh < height())
		nh = height();
	
	locked = true;
	CWIDGET_resize(THIS, w, h);
	locked = false;
}

#if 0
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
			QTextDocument doc;
			
			doc.setHtml(text());
			doc.setDefaultFont(font());
			
			w = width() - f * 2;
			
			doc.setTextWidth(w);
			nh = doc.size().height();
			nw = adjust ? doc.idealWidth() : w;
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
		if (!noresize)
		{
			locked = true;
			//qDebug("%p: resize(%d, %d)", this, w, h);
			resize(w, h);
			locked = false;
		}
	}
}
#endif 

void MyLabel::resizeEvent(QResizeEvent *e)
{
	QLabel::resizeEvent(e);
  
  if (autoResize && !locked && textFormat() == Qt::RichText && e->oldSize().width() != e->size().width())
  	calcMinimumHeight();
}

void MyLabel::adjust()
{
	calcMinimumHeight(true);
}

/** class MySeparator ******************************************************/


MySeparator::MySeparator(QWidget *parent)
: QWidget(parent)
{
}

void MySeparator::paintEvent( QPaintEvent * )
{
	QPainter p(this);
	
	if (width() == 1 || height() == 1)
	{
		p.setPen(CCOLOR_light_foreground());
		if (width() >= height())
			p.drawLine(0, height() / 2, width() - 1, height() / 2);
		else
			p.drawLine(width() / 2, 0, width() / 2, height() - 1);
	}
	else
	{
		QStyleOption opt;
		
		opt.rect = rect();
		opt.palette = palette();
		opt.state |= QStyle::State_Enabled;
		
		if (width() < height())
			opt.state |= QStyle::State_Horizontal;

		style()->drawPrimitive(QStyle::PE_IndicatorToolBarSeparator, &opt, &p);
	}
}

