/***************************************************************************

  CLabel.cpp

  (c) 2000-2013 Benoît Minisini <gambas@users.sourceforge.net>

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

BEGIN_METHOD(Label_new, GB_OBJECT parent)

  QLabel *wid = new MyLabel(QCONTAINER(VARG(parent)));

  wid->setTextFormat(Qt::PlainText);
  wid->setAlignment(Qt::AlignLeft | Qt::AlignVCenter); // + Qt::WordBreak);
	//THIS->widget.flag.fillBackground = TRUE;
	
  CWIDGET_new(wid, (void *)_object);

END_METHOD


BEGIN_METHOD(TextLabel_new, GB_OBJECT parent)

  MyLabel *wid = new MyLabel(QCONTAINER(VARG(parent)));

  wid->setTextFormat(Qt::RichText);
  wid->setAlignment(Qt::AlignLeft | Qt::AlignTop);
  wid->setWordWrap(true);
	//THIS->widget.flag.fillBackground = TRUE;

  CWIDGET_new(wid, (void *)_object);

END_METHOD


BEGIN_PROPERTY(Label_Text)

  if (READ_PROPERTY)
    GB.ReturnNewZeroString(TO_UTF8(WIDGET->text()));
  else
    WIDGET->setText(QSTRING_PROP());

END_PROPERTY


BEGIN_PROPERTY(Label_Alignment)

  if (READ_PROPERTY)
    GB.ReturnInteger(CCONST_alignment(WIDGET->alignment() & ALIGN_MASK, ALIGN_NORMAL, false));
  else
    WIDGET->setAlignment((Qt::Alignment)CCONST_alignment(VPROP(GB_INTEGER), ALIGN_NORMAL, true));

END_PROPERTY


BEGIN_PROPERTY(Label_AutoResize)

  if (READ_PROPERTY)
    GB.ReturnInteger(WIDGET->getAutoResize());
  else
    WIDGET->setAutoResize(VPROP(GB_BOOLEAN));

END_PROPERTY

BEGIN_PROPERTY(Label_Padding)

	if (READ_PROPERTY)
		GB.ReturnInteger(WIDGET->margin());
	else
	{
		WIDGET->setMargin(VPROP(GB_INTEGER));
		WIDGET->calcMinimumHeight();
		WIDGET->update();
	}

END_PROPERTY


BEGIN_PROPERTY(TextLabel_Alignment)

  if (READ_PROPERTY)
    GB.ReturnInteger(CCONST_alignment(WIDGET->alignment() & ALIGN_MASK, ALIGN_NORMAL, false));
  else
    WIDGET->setAlignment((Qt::Alignment)CCONST_alignment(VPROP(GB_INTEGER), ALIGN_NORMAL, true));

END_PROPERTY


BEGIN_METHOD_VOID(Label_Adjust)

	WIDGET->adjust();

END_METHOD

BEGIN_PROPERTY(Label_Transparent)

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

BEGIN_PROPERTY(Label_Border)

	if (READ_PROPERTY)
		GB.ReturnInteger(WIDGET->border());
	else
	{
		WIDGET->setBorder(VPROP(GB_INTEGER));
		WIDGET->calcMinimumHeight();
	}

END_PROPERTY

//---------------------------------------------------------------------------

GB_DESC CLabelDesc[] =
{
  GB_DECLARE("Label", sizeof(CLABEL)), GB_INHERITS("Control"),

  GB_METHOD("_new", NULL, Label_new, "(Parent)Container;"),

  GB_PROPERTY("Text", "s", Label_Text),
  GB_PROPERTY("Caption", "s", Label_Text),
  GB_PROPERTY("Alignment", "i", Label_Alignment),
  GB_PROPERTY("Border", "i", Label_Border),
  GB_PROPERTY("AutoResize", "b", Label_AutoResize),
  GB_PROPERTY("Padding", "i", Label_Padding),
  GB_PROPERTY("Transparent", "b", Label_Transparent),
  GB_METHOD("Adjust", NULL, Label_Adjust, NULL),

	LABEL_DESCRIPTION,

  GB_END_DECLARE
};


GB_DESC CTextLabelDesc[] =
{
  GB_DECLARE("TextLabel", sizeof(CLABEL)), GB_INHERITS("Control"),

  GB_METHOD("_new", NULL, TextLabel_new, "(Parent)Container;"),

  GB_PROPERTY("Text", "s", Label_Text),
  GB_PROPERTY("Caption", "s", Label_Text),
  GB_PROPERTY("Alignment", "i", TextLabel_Alignment),
  GB_PROPERTY("Border", "i", Label_Border),
  GB_PROPERTY("AutoResize", "b", Label_AutoResize),
  GB_PROPERTY("Padding", "i", Label_Padding),
  GB_PROPERTY("Transparent", "b", Label_Transparent),
  GB_PROPERTY("Wrap", "b", Label_Wrap),
  GB_METHOD("Adjust", NULL, Label_Adjust, NULL),

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
	_border = BORDER_NONE;
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
	int f = contentsMargins().left() + margin();
	QRect br;
	
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
				nw = doc.idealWidth();
				doc.setTextWidth(nw);
				nh = doc.size().height();
				nw = doc.size().width();
				
			}
			else
			{
				nh = doc.size().height();
				nw = w;
			}
		}
		else
		{
			nh = doc.size().height();
			nw = doc.size().width();
		}

		//nw += 2; // Why? Don't know...
		//nh += 2;
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

	int a = CCONST_alignment(WIDGET->alignment() & ALIGN_MASK, ALIGN_NORMAL, false);
	if ((a == ALIGN_CENTER || a == ALIGN_LEFT || a == ALIGN_NORMAL || a == ALIGN_RIGHT) && nh < height())
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

void MyLabel::paintEvent(QPaintEvent *e)
{
	QPainter p(this);
	CCONTAINER_draw_border(&p, _border, this);
	QLabel::paintEvent(e);
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
		void *_object = CWidget::getReal(this);
		uint color = CWIDGET_get_foreground(&THIS->widget);
		p.setPen(color == COLOR_DEFAULT ? CCOLOR_light_foreground() : TO_QCOLOR(color));
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

