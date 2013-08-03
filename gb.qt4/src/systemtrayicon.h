/***************************************************************************

  systemtrayicon.h

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

/****************************************************************************
**
** Copyright (C) 2008 Nokia Corporation and/or its subsidiary(-ies).
** Contact: Qt Software Information (qt-info@nokia.com)
**
** This file is part of the QtGui module of the Qt Toolkit.
**
** Commercial Usage
** Licensees holding valid Qt Commercial licenses may use this file in
** accordance with the Qt Commercial License Agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and Nokia.
**
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License versions 2.0 or 3.0 as published by the Free
** Software Foundation and appearing in the file LICENSE.GPL included in
** the packaging of this file.  Please review the following information
** to ensure GNU General Public Licensing requirements will be met:
** http://www.fsf.org/licensing/licenses/info/GPLv2.html and
** http://www.gnu.org/copyleft/gpl.html.  In addition, as a special
** exception, Nokia gives you certain additional rights. These rights
** are described in the Nokia Qt GPL Exception version 1.3, included in
** the file GPL_EXCEPTION.txt in this package.
**
** Qt for Windows(R) Licensees
** As a special exception, Nokia, as the sole copyright holder for Qt
** Designer, grants users of the Qt/Eclipse Integration plug-in the
** right for the Qt/Eclipse Integration to link to functionality
** provided by Qt Designer and its related libraries.
**
** If you are unsure which license is appropriate for your use, please
** contact the sales department at qt-sales@nokia.com.
**
****************************************************************************/

#ifndef SYSTEMTRAYICON_H
#define SYSTEMTRAYICON_H

#include "gb.qt.h"

#ifdef NO_X_WINDOW

#define SystemTrayIcon QWidget

#else

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists for the convenience
// of a number of Qt sources files.  This header file may change from
// version to version without notice, or even be removed.
//
// We mean it.
//

#include "systemtrayicon.h"
//#include "private/qobject_p.h"

#include <QPixmap>
#include <QString>
#include <QPointer>
#include <QWidget>

#include <QCoreApplication>
#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <X11/Xutil.h>

class SystemTrayIcon : public QWidget
{
Q_OBJECT
    //friend class QSystemTrayIconPrivate;

public:
    SystemTrayIcon();
    ~SystemTrayIcon();
    enum {
        SYSTEM_TRAY_REQUEST_DOCK = 0,
        SYSTEM_TRAY_BEGIN_MESSAGE = 1,
        SYSTEM_TRAY_CANCEL_MESSAGE =2
    };

    void addToTray();
    XVisualInfo* getSysTrayVisualInfo();

    // QObject::event is public but QWidget's ::event() re-implementation
    // is protected ;(
    inline bool deliverToolTipEvent(QEvent *e)
    { return QWidget::event(e); }

    static Window sysTrayWindow;
    static QList<SystemTrayIcon *> trayIcons;
    static QCoreApplication::EventFilter oldEventFilter;
    static bool sysTrayTracker(void *message, long *result);
    static Window locateSystemTray();
    static Atom NET_SYSTEM_TRAY_SELECTION;
    static Atom NET_SYSTEM_TRAY_VISUAL;
    static XVisualInfo sysTrayVisual;

protected:
		virtual void drawContents(QPainter *p);
    void paintEvent(QPaintEvent *pe);
    //void resizeEvent(QResizeEvent *re);
    bool x11Event(XEvent *event);
    //void mousePressEvent(QMouseEvent *event);
    //void mouseDoubleClickEvent(QMouseEvent *event);
    //void wheelEvent(QWheelEvent *event);
    //bool event(QEvent *e);
private:
  QPixmap background;
  Colormap colormap;
};

#endif

#endif // SYSTEMTRAYICON_H

