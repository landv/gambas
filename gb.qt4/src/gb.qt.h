/***************************************************************************

  gb.qt.h

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

#ifndef __GB_QT_H
#define __GB_QT_H

#include "gambas.h"
#include "gb.image.h"

#ifdef OS_MACOSX
//#define NO_X_WINDOW 1
#endif

#include <QWidget>
#include <QFont>
#include <QString>
#include <QPixmap>
#include <QImage>
#include <QEvent>
#include <QPainter>
#include <QPainterPath>
#include <QBrush>
#include <QPen>
#include <QTransform>
#include <QColor>

#define QT_INTERFACE_VERSION 1

#define TO_QSTRING(_str) (QString::fromUtf8((const char *)(_str)))

#ifdef DO_NOT_USE_QT_INTERFACE

	#define TO_UTF8(_str) QT_ToUTF8(_str)
	#define GET_SENDER() void *_object = QT_GetObject((QWidget*)sender())

#else

	#define TO_UTF8(_str) QT.ToUTF8(_str)
	#define GET_SENDER() void *_object = QT.GetObject((QWidget*)sender())

#endif

#define QSTRING_ARG(_arg) (QString::fromUtf8((const char *)(VARG(_arg).addr + VARG(_arg).start), VARG(_arg).len))
#define QSTRING_PROP() (QString::fromUtf8((const char *)(VPROP(GB_STRING).addr + VPROP(GB_STRING).start), VPROP(GB_STRING).len))

#define TO_QCOLOR(_col) QColor::fromRgba((QRgb)((_col) ^ 0xFF000000))

typedef
	struct {
		GB_BASE ob;
		QWidget *widget;
		void *_r0;
		unsigned short flag;
		unsigned short _r1;
		int _r2;
		void *_r3;
		void *_r4;
		}
	QT_WIDGET;

typedef
	struct {
		QT_WIDGET widget;
		QWidget *container;
		int arrangement;
		}
	QT_CONTAINER;

typedef
	struct {
		GB_BASE ob;
		QFont *font;
		void *func;
		void *object;    
	}
	QT_FONT;

typedef
	void *QT_PICTURE;

typedef
	struct {
		QPainter *p;
		QPainter *pm;
		QBitmap *mask;
		int fg;
		int fillColor;
		}
	QT_DRAW_EXTRA;

typedef
	void (*QT_FONT_FUNC)(QFont &, void *);

typedef
	void (*QT_COLOR_FUNC)(void *);

typedef
	struct {
		intptr_t version;
		void (*InitEventLoop)(void);
		void (*Init)(void);
		void (*InitWidget)(QWidget *, void *, int);
		void *(*GetObject)(QWidget *);
		QWidget *(*GetContainer)(void *);
		void (*BorderProperty)(void *, void *);
		void (*FullBorderProperty)(void *, void *);
		void (*ScrollBarProperty)(void *, void *);
		void (*FontProperty)(void *, void *);
		QT_FONT *(*CreateFont)(const QFont &, QT_FONT_FUNC, void *);
		void (*SetFont)(QT_FONT_FUNC, void*, void *);
		QT_PICTURE (*CreatePicture)(const QPixmap &);
		//QMimeSourceFactory *(*MimeSourceFactory)(void);
		QPixmap *(*GetPixmap)(QT_PICTURE);
		const char *(*ToUTF8)(const QString &);
		bool (*EventFilter)(QEvent *);
		bool (*Notify)(void *, bool);
		int (*Alignment)(int, int, bool);
		void (*Link)(QObject *, void *);
		void *(*GetLink)(QObject *);
		QPainter *(*GetCurrentPainter)();
		uint (*GetBackgroundColor)(void *);
		void (*MouseProperty)(void *, void *);
		QT_COLOR_FUNC (*AfterSetColor)(QT_COLOR_FUNC);
		void *_null;
		}
	QT_INTERFACE;


#define QT_WIDGET_PROPERTIES "*"

#define QT_EVENT_FIRST ((QEvent::Type)(QEvent::User + 10))

#endif

