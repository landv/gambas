/***************************************************************************

  gb.qt.h

  The Gambas QT Library Interface

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

#ifndef __GB_QT_H
#define __GB_QT_H

#include "gambas.h"
#include <qwidget.h>
#include <qfont.h>
#include <qstring.h>
#include <qpixmap.h>
#include <qimage.h>
#include <qmime.h>
#include <qevent.h>
#include <qpainter.h>

#define QT_INTERFACE_VERSION 1

#define TO_QSTRING(_str) (QString::fromUtf8((const char *)(_str)))

#ifdef DO_NOT_USE_QT_INTERFACE

  #define TO_UTF8(_str) QT_ToUTF8(_str)

#else

  #define TO_UTF8(_str) QT.ToUTF8(_str)

#endif

#define QSTRING_ARG(_arg) (QString::fromUtf8((const char *)(VARG(_arg).addr + VARG(_arg).start), VARG(_arg).len))
#define QSTRING_PROP() (QString::fromUtf8((const char *)(VPROP(GB_STRING).addr + VPROP(GB_STRING).start), VPROP(GB_STRING).len))

typedef
  struct {
    GB_BASE ob;
    QWidget *widget;
    unsigned short flag;
    unsigned short _reserved0;
    GB_VARIANT_VALUE _reserved1;
    char *_reserved2;
    char *_reserved3;
    void *_reserved4;
    void *_reserved5;
    void *_reserved6;
    long _reserved7;
    }
  QT_WIDGET;

typedef
  struct {
    QT_WIDGET widget;
    QWidget *container;
    long arrangement;
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
		}
	QT_DRAW_EXTRA;

typedef
  struct {
    long version;
    void (*InitEventLoop)(void);
    void (*Init)(void);
    void (*InitWidget)(QWidget *, void *);
    void *(*GetObject)(QWidget *);
    QWidget *(*GetContainer)(void *);
    void (*BorderProperty)(void *, void *);
    void (*FullBorderProperty)(void *, void *);
    void (*ScrollBarProperty)(void *, void *);
    void (*FontProperty)(void *, void *);
    QT_FONT *(*CreateFont)(QFont &);
    QMimeSourceFactory *(*MimeSourceFactory)(void);
    QPixmap *(*GetPixmap)(QT_PICTURE);
    const char *(*ToUTF8)(const QString &);
    bool (*EventFilter)(QEvent *);
    bool (*Notify)(void *, bool);
    void *(*GetDrawInterface)();
    void *_null;
    }
  QT_INTERFACE;


#define QT_WIDGET_PROPERTIES "*"

#define QT_EVENT_FIRST ((QEvent::Type)(QEvent::User + 10))

#endif

