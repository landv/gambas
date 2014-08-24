/***************************************************************************

  CDraw.cpp

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

#define __CDRAW_CPP

#ifdef OS_SOLARIS
/* Make math.h define M_PI and a few other things */
#define __EXTENSIONS__
/* Get definition for finite() */
#include <ieeefp.h>
#endif
#include <math.h>

#include <qpainter.h>
#include <qpen.h>
#include <qbrush.h>
#include <qapplication.h>
#include <qpixmap.h>
#include <qbitmap.h>
#include <qpalette.h>
#include <qstyle.h>
#include <qdrawutil.h>
//Added by qt3to4:
#include <QStyle>
#include <QStyleOption>
#include <QVector>
#include <QTextDocument>
#include <QColor>
#include <QPen>
#include <QBrush>

#ifndef NO_X_WINDOW
#include <QX11Info>
#endif

#include "gambas.h"

#include "CConst.h"
#include "CFont.h"
#include "CWidget.h"
#include "CWindow.h"
#include "CPicture.h"
#include "CImage.h"
#include "CDrawingArea.h"
#include "CColor.h"
#include "CDraw.h"

typedef
	QT_DRAW_EXTRA GB_DRAW_EXTRA;

#define EXTRA(d) ((GB_DRAW_EXTRA *)(d->extra))
#define DP(d) (EXTRA(d)->p)
#define DPM(d) (EXTRA(d)->pm)

#define COLOR_TO_INT(color) ((color).rgba() ^ 0xFF000000)
#define MASK_COLOR(col) ((col & 0xFF000000) ? Qt::color0 : Qt::color1)

DRAW_INTERFACE DRAW EXPORT;

/*static void set_background(GB_DRAW *d, int col);
static void set_foreground(GB_DRAW *d, int col);
static void set_fill_color(GB_DRAW *d, int col);*/

void DRAW_init()
{
	GB.GetInterface("gb.draw", DRAW_INTERFACE_VERSION, &DRAW);
}

static Qt::Alignment get_horizontal_alignment(Qt::Alignment align)
{
	align &= Qt::AlignHorizontal_Mask;
	switch (align)
	{
		case Qt::AlignLeft:
			if (QApplication::isRightToLeft())
				return Qt::AlignRight;
			break;
			
		case Qt::AlignRight:
			if (QApplication::isRightToLeft())
				return Qt::AlignLeft;
			break;
			
		default:
			break;
	}

	return align & ~Qt::AlignAbsolute;
}

static QStringList text_sl;
static QVector<int> text_w;
static int text_line;

static int get_text_width(QPainter *dp, QString &s)
{
	int w, width = 0;
	int i;

	text_sl = s.split('\n', QString::KeepEmptyParts);

	text_w.resize(text_sl.count());

	for (i = 0; i < (int)text_sl.count(); i++)
	{
		w = dp->fontMetrics().width(text_sl[i]);
		if (w > width) width = w;
		text_w[i] = w;
	}

	return width;
}

static int get_text_height(QPainter *dp, QString &s)
{
	text_line = dp->fontMetrics().height();
	return text_line * (1 + s.count('\n'));
}

void DRAW_text(QPainter *p, const QString &text, float x, float y, float w, float h, int align, QPainter *p2)
{
	QPen pen, penm;
	QString t = text;
	int xx, ww;
	int tw, th;
	int i;

	tw = get_text_width(p, t);
	th = get_text_height(p, t);

	if (w < 0) w = tw;
	if (h < 0) h = th;
	
	y += p->fontMetrics().ascent();

	switch(align & Qt::AlignVertical_Mask)
	{
		case Qt::AlignBottom: y += h - th; break;
		case Qt::AlignVCenter: y += (h - th) / 2; break;
		default: break;
	}

	align = get_horizontal_alignment((Qt::Alignment)align);
	
	for (i = 0; i < (int)text_sl.count(); i++)
	{
		t = text_sl[i];
		ww = text_w[i];

		switch(align)
		{
			case Qt::AlignRight: xx = x + w - ww; break;
			case Qt::AlignHCenter: xx = x + (w - ww) / 2; break;
			default: xx = x; break;
		}

		//(*callback)(xx, y, t);
		p->drawText(xx, y, t);
		if (p2) 
			p2->drawText(xx, y, t);

		y += text_line;
	}
}

void DRAW_rich_text(QPainter *p, const QString &text, float x, float y, float w, float h, int align, QPainter *p2)
{
	static QTextDocument *doc = NULL;

	QString a;
	float tw, th;
	QColor fg = p->pen().color();
	QString t = "<font color=\"" + fg.name() + "\">" + text + "</font>";
	qreal opacity = 1.0;
	bool hasAlpha = fg.alpha() < 255;
	int margin;

	switch(get_horizontal_alignment((Qt::Alignment)align))
	{
		case Qt::AlignRight: a = "right"; break;
		case Qt::AlignHCenter: a = "center"; break;
		case Qt::AlignJustify: a = "justify"; break;
	}
	
	if (a.length())
		t = "<div align=\"" + a + "\">" + t + "</div>";
	
	if (!doc)
	{
		doc = new QTextDocument;
		doc->setDocumentMargin(0);
	}

	doc->setDefaultFont(p->font());
	margin = p->font().pointSize() * p->device()->physicalDpiY() / 96;
	doc->setDefaultStyleSheet(QString("p { margin-bottom: %1px; } h1,h2,h3,h4,h5,h6 { margin-bottom: %1px; }").arg(margin));
	doc->setHtml(t);

	if (w > 0)
		doc->setTextWidth(w);
		
	tw = doc->idealWidth();
	th = doc->size().height();

	if (w < 0) w = tw;
	if (h < 0) h = th;
	
	switch(align & Qt::AlignVertical_Mask)
	{
		case Qt::AlignBottom: y += h - th; break;
		case Qt::AlignVCenter: y += (h - th) / 2; break;
		default: break;
	}

	if (hasAlpha)
	{
		opacity = p->opacity();
		p->setOpacity(p->opacity() * fg.alpha() / 255.0);
	}
	
	p->translate(x, y);
	doc->drawContents(p);
	p->translate(-x, -y);
	
	if (hasAlpha)
		p->setOpacity(opacity);
	
	if (p2) 
	{
		p2->translate(x, y);
		doc->drawContents(p2);
		p2->translate(-x, -y);
	}
}

void DRAW_aligned_pixmap(QPainter *p, const QPixmap &pix, int x, int y, int w, int h, int align)
{
	int xp, yp;
	
	if (pix.isNull() || pix.width() == 0 || pix.height() == 0)
		return;
	
	xp = x;
	switch(get_horizontal_alignment((Qt::Alignment)align))
	{
		case Qt::AlignRight: xp += w - pix.width(); break;
		case Qt::AlignHCenter: xp += (w - pix.width()) / 2; break;
		default: break;
	}
	
	yp = y;
	switch(align & Qt::AlignVertical_Mask)
	{
		case Qt::AlignBottom: yp += h - pix.height(); break;
		case Qt::AlignVCenter: yp += (h - pix.height()) / 2; break;
		default: break;
	}
	
	p->drawPixmap(xp, yp, pix);
}

