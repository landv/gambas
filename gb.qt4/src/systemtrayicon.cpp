/***************************************************************************

  systemtrayicon.cpp

  (c) 2000-2012 Benoît Minisini <gambas@users.sourceforge.net>

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

#define __SYSTEMTRAYICON_CPP

#include "gb.qt.h"

#ifndef NO_X_WINDOW

#include <QLabel>
#include <QX11Info>
#include <QPainter>
#include <QPixmap>
#include <QBitmap>
#include <QEvent>
#include <QApplication>
#include <QList>
#include <QTimer>

#include "systemtrayicon.h"

Window SystemTrayIcon::sysTrayWindow = None;
QList<SystemTrayIcon *> SystemTrayIcon::trayIcons;
QCoreApplication::EventFilter SystemTrayIcon::oldEventFilter = 0;
Atom SystemTrayIcon::sysTraySelection = None;

// Locate the system tray
Window SystemTrayIcon::locateSystemTray()
{
    Display *display = QX11Info::display();
    if (sysTraySelection == None) {
        int screen = QX11Info::appScreen();
        QString net_sys_tray = QString::fromLatin1("_NET_SYSTEM_TRAY_S%1").arg(screen);
        sysTraySelection = XInternAtom(display, net_sys_tray.toLatin1(), False);
    }

    return XGetSelectionOwner(QX11Info::display(), sysTraySelection);
}

bool SystemTrayIcon::sysTrayTracker(void *message, long *result)
{
    bool retval = false;
    if (SystemTrayIcon::oldEventFilter)
        retval = SystemTrayIcon::oldEventFilter(message, result);

    if (trayIcons.isEmpty())
        return retval;

    Display *display = QX11Info::display();
    XAnyEvent *ev = (XAnyEvent *)message;
    if  (ev->type == DestroyNotify && ev->window == sysTrayWindow) {
	sysTrayWindow = locateSystemTray();
        for (int i = 0; i < trayIcons.count(); i++) {
            if (sysTrayWindow == None) {
	        //QBalloonTip::hideBalloon();
                trayIcons[i]->hide(); // still no luck
                trayIcons[i]->destroy();
                trayIcons[i]->create();
	    } else
                trayIcons[i]->addToTray(); // add it to the new tray
        }
        retval = true;
    } else if (ev->type == ClientMessage && sysTrayWindow == None) {
        static Atom manager_atom = XInternAtom(display, "MANAGER", False);
        XClientMessageEvent *cm = (XClientMessageEvent *)message;
        if ((cm->message_type == manager_atom) && ((Atom)cm->data.l[1] == sysTraySelection)) {
	    sysTrayWindow = cm->data.l[2];
	    XSelectInput(display, sysTrayWindow, StructureNotifyMask);
            for (int i = 0; i < trayIcons.count(); i++) {
                trayIcons[i]->addToTray();
            }
            retval = true;
        }
    }

    return retval;
}

SystemTrayIcon::SystemTrayIcon()
    : QWidget(0, Qt::FramelessWindowHint | Qt::X11BypassWindowManagerHint)
{
    setAttribute(Qt::WA_AlwaysShowToolTips);
    setAttribute(Qt::WA_QuitOnClose, false);
    //setAttribute(Qt::WA_NoSystemBackground, true);
    setAttribute(Qt::WA_OpaquePaintEvent, true);
    static bool eventFilterAdded = false;
    Display *display = QX11Info::display();
    if (!eventFilterAdded) {
        oldEventFilter = qApp->setEventFilter(sysTrayTracker);
	eventFilterAdded = true;
	Window root = QX11Info::appRootWindow();
        XWindowAttributes attr;
        XGetWindowAttributes(display, root, &attr);
        if ((attr.your_event_mask & StructureNotifyMask) != StructureNotifyMask) {
            (void) QApplication::desktop(); // lame trick to ensure our event mask is not overridden
            XSelectInput(display, root, attr.your_event_mask | StructureNotifyMask); // for MANAGER selection
        }
    }
    if (trayIcons.isEmpty()) {
        sysTrayWindow = locateSystemTray();
	if (sysTrayWindow != None)
	    XSelectInput(display, sysTrayWindow, StructureNotifyMask); // track tray events
    }
    trayIcons.append(this);
    setMouseTracking(true);
#ifndef QT_NO_TOOLTIP
    //setToolTip(q->toolTip());
#endif
    if (sysTrayWindow != None)
        addToTray();
}

SystemTrayIcon::~SystemTrayIcon()
{
    trayIcons.removeAt(trayIcons.indexOf(this));
    if (trayIcons.isEmpty()) {
        Display *display = QX11Info::display();
        if (sysTrayWindow == None)
            return;
        if (display)
            XSelectInput(display, sysTrayWindow, 0); // stop tracking the tray
        sysTrayWindow = None;
    }
}

void SystemTrayIcon::addToTray()
{
    Q_ASSERT(sysTrayWindow != None);
    Display *display = QX11Info::display();
    Window wid = winId();

    XSetWindowBackgroundPixmap(display, wid, ParentRelative);

    // GNOME, NET WM Specification
    static Atom netwm_tray_atom = XInternAtom(display, "_NET_SYSTEM_TRAY_OPCODE", False);
    unsigned long l[5] = { CurrentTime, SYSTEM_TRAY_REQUEST_DOCK, wid, 0, 0 };
    XEvent ev;
    memset(&ev, 0, sizeof(ev));
    ev.xclient.type = ClientMessage;
    ev.xclient.window = sysTrayWindow;
    ev.xclient.message_type = netwm_tray_atom;
    ev.xclient.format = 32;
    memcpy((char *)&ev.xclient.data, (const char *) l, sizeof(l));
    XSendEvent(display, sysTrayWindow, False, 0, &ev);
    setMinimumSize(22, 22); // required at least on GNOME
}

// void SystemTrayIcon::updateIcon()
// {
//    update();
// }

// void SystemTrayIcon::resizeEvent(QResizeEvent *re)
// {
//      QWidget::resizeEvent(re);
//      updateIcon();
// }
// 
void SystemTrayIcon::paintEvent(QPaintEvent *)
{
	QPainter p(this);
	p.drawPixmap(0, 0, background);
}

bool SystemTrayIcon::x11Event(XEvent *event)
{
    if (event->type == ReparentNotify)
        show();
    else if (event->type == ConfigureNotify || event->type == Expose) {
        XClearArea(QX11Info::display(), winId(), 0, 0, width(), height(), False);
        qApp->syncX();
        background = QPixmap::grabWindow(winId());
        update();
    }
    return QWidget::x11Event(event);
}

#endif
