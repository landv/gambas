/***************************************************************************

	fix_breeze.h

	(c) 2000-2017 Benoît Minisini <g4mba5@gmail.com>

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

#ifndef __FIX_BREEZE_H
#define __FIX_BREEZE_H

#include <QProxyStyle>
#include <QFontMetrics>

class FixBreezeStyle : public QProxyStyle
{
public:
		
	virtual QRect subControlRect(ComplexControl, const QStyleOptionComplex*, SubControl, const QWidget*) const;
	virtual QRect subElementRect(SubElement, const QStyleOption*, const QWidget*) const;
	
	void drawComplexControl(ComplexControl, const QStyleOptionComplex*, QPainter*, const QWidget*) const;
	void drawPrimitive(PrimitiveElement, const QStyleOption*, QPainter*, const QWidget*) const;
	void drawControl(ControlElement, const QStyleOption *, QPainter *, const QWidget *) const;

	QRect visualRect(const QStyleOption* opt, const QRect& subRect) const
	{ return QProxyStyle::visualRect(opt->direction, opt->rect, subRect); }
	
	QRect centerRect(const QRect &rect, int width, int height) const
	{ return QRect(rect.left() + (rect.width() - width)/2, rect.top() + (rect.height() - height)/2, width, height); }

private:
	static QFontMetrics *fm;
	static void fixFontMetrics(QStyleOption *);
};


#endif

 
