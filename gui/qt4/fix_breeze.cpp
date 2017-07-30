/***************************************************************************

	fix_breeze.cpp

	(c) 2000-2017 Benoît Minisini <gambas@users.sourceforge.net>

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

#define __FIX_BREEZE_CPP

#include <QRect>
#include <QStyleOptionSpinBox>
#include <QPainter>
#include <QApplication>

#include "gb_common.h"
#include "fix_breeze.h"

QFontMetrics *FixBreezeStyle::fm = NULL;

void FixBreezeStyle::fixFontMetrics(QStyleOption *option)
{
	if (!fm)
	{
		QFont f = qApp->font();
		f.setPointSize(1);
		fm = new QFontMetrics(f);
	}
		
	option->fontMetrics = *fm;
}

QRect FixBreezeStyle::subControlRect(ComplexControl element, const QStyleOptionComplex* option, SubControl subControl, const QWidget* widget) const
{
	if (element == CC_SpinBox)
	{
		const QStyleOptionSpinBox *spinBoxOption(qstyleoption_cast<const QStyleOptionSpinBox*>(option));
		const bool flat( !spinBoxOption->frame );
		QRect rect(option->rect);
		
		if (subControl == SC_SpinBoxEditField)
		{
			QRect labelRect;
			
			labelRect = QRect(
					rect.left(), rect.top(),
					rect.width() - 20, //Metrics::SpinBox_ArrowButtonWidth,
					rect.height() );

			// remove right side line editor margins
			const int frameWidth( pixelMetric( PM_SpinBoxFrameWidth, option, widget ) );
			if( !flat)
				labelRect.adjust( frameWidth, 2, 0, -2 );

			return visualRect( option, labelRect );
		}
		else if (subControl ==  SC_SpinBoxUp || subControl == SC_SpinBoxDown)
		{
			// take out frame width
			if (!flat)
				rect.adjust(2, 2, -2, -2); // = insideMargin( rect, 2); //Metrics::Frame_FrameWidth );

			QRect arrowRect;
			arrowRect = QRect(
					rect.right() - 20 /*Metrics::SpinBox_ArrowButtonWidth*/ + 1,
					rect.top(),
					20, //Metrics::SpinBox_ArrowButtonWidth,
					rect.height() );

			const int arrowHeight( qMin( rect.height(), 20)); //int(Metrics::SpinBox_ArrowButtonWidth) ) );
			arrowRect = centerRect( arrowRect, 20 /*Metrics::SpinBox_ArrowButtonWidth*/, arrowHeight );
			arrowRect.setHeight( arrowHeight/2 );
			if( subControl == SC_SpinBoxDown ) arrowRect.translate( 0, arrowHeight/2 );

			return visualRect( option, arrowRect );
		}
	}
	else if (element == CC_ComboBox)
	{
		if (subControl == SC_ComboBoxEditField)
		{
			const QStyleOptionComboBox *comboBoxOption( qstyleoption_cast<const QStyleOptionComboBox*>(option));
			const bool editable( comboBoxOption->editable );
			const bool flat( editable && !comboBoxOption->frame );
			QRect rect(option->rect);
			QRect labelRect;
			
			const int frameWidth(pixelMetric(PM_ComboBoxFrameWidth, option, widget));
			labelRect = QRect(
					rect.left(), rect.top(),
					rect.width() - 20, //Metrics::MenuButton_IndicatorWidth,
					rect.height() );

			// remove margins
			if( !flat)
				labelRect.adjust(frameWidth, 2, 0, -2 );

			return visualRect( option, labelRect );
		}
	}
	
	return QProxyStyle::subControlRect(element, option, subControl, widget);
}

QRect FixBreezeStyle::subElementRect(SubElement element, const QStyleOption* option, const QWidget* widget) const
{
	if (element == SE_LineEditContents)
	{
		const QStyleOptionFrame* frameOption(qstyleoption_cast<const QStyleOptionFrame*>(option));
		
		const bool flat( frameOption->lineWidth == 0 );
		if(flat)
			return option->rect;
		
		QRect rect( option->rect );

		const int frameWidth(pixelMetric(PM_DefaultFrameWidth, option, widget));
		rect.adjust(frameWidth, 2, -frameWidth, -2);
	
		return rect;
	}
	
	return QProxyStyle::subElementRect(element, option, widget);
}

void FixBreezeStyle::drawPrimitive( PrimitiveElement element, const QStyleOption* option, QPainter* painter, const QWidget* widget ) const
{
	QStyleOption newOption;
	
	if (element == PE_FrameLineEdit)
	{
		newOption = *option;
		fixFontMetrics(&newOption);
		option = &newOption;
		//qDebug("PE_FrameLineEdit: %d / %d", option->fontMetrics.height(), option->rect.height());
	}
		
	QProxyStyle::drawPrimitive(element, option, painter, widget);
}

void FixBreezeStyle::drawComplexControl(ComplexControl element, const QStyleOptionComplex* option, QPainter* painter, const QWidget* widget) const
{
	if (element == CC_SpinBox)
	{
		QStyleOptionSpinBox newOption;	
		const QStyleOptionSpinBox *spinBoxOption( qstyleoption_cast<const QStyleOptionSpinBox*>( option ) );
		
		if (option->subControls & SC_SpinBoxFrame)
		{
			if (spinBoxOption->frame)
			{
				if( option->subControls & SC_SpinBoxFrame )
				{
					newOption = *spinBoxOption;
					newOption.subControls &= ~SC_SpinBoxFrame;
					option = &newOption;
					
					drawPrimitive( PE_FrameLineEdit, option, painter, widget );
				}
			}
		}
		
		QProxyStyle::drawComplexControl(element, option, painter, widget);
		return;
	}
	
	if (element == CC_ComboBox)
	{
		QStyleOptionComboBox newOption;	
		const QStyleOptionComboBox* comboBoxOption( qstyleoption_cast<const QStyleOptionComboBox*>( option ) );

		if (option->subControls & SC_ComboBoxFrame)
		{
			if (comboBoxOption->editable)
			{
				if (comboBoxOption->frame)
				{
					newOption = *comboBoxOption;
					newOption.subControls &= ~SC_ComboBoxFrame;
					option = &newOption;
					
					drawPrimitive(PE_FrameLineEdit, option, painter, widget );
				}
			}
		}
		
		QProxyStyle::drawComplexControl(element, option, painter, widget);
		return;
	}
	
	QProxyStyle::drawComplexControl(element, option, painter, widget);
}

void FixBreezeStyle::drawControl(ControlElement element, const QStyleOption * option, QPainter * painter, const QWidget * widget) const
{
	QStyleOptionButton newOption;
	
	if (element == CE_PushButtonBevel)
	{
		newOption = *(QStyleOptionButton *)option;
		newOption.iconSize = QSize(0, 0);
		option = &newOption;
	}
	
	QProxyStyle::drawControl(element, option, painter, widget);
}
