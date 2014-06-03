/***************************************************************************

  systemtrayicon.cpp

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

#define __SYSTEMTRAYICON_CPP

#include "gb.qt.h"

#ifndef NO_X_WINDOW

#include <QLabel>
#include <QX11Info>
#include <QPaintEngine>
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
Atom SystemTrayIcon::NET_SYSTEM_TRAY_SELECTION = None;
Atom SystemTrayIcon::NET_SYSTEM_TRAY_VISUAL = None;
XVisualInfo SystemTrayIcon::sysTrayVisual = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

// Locate the system tray
Window SystemTrayIcon::locateSystemTray()
{
    Display *display = QX11Info::display();
    if (NET_SYSTEM_TRAY_SELECTION == None) 
		{
        int screen = QX11Info::appScreen();
        QString net_sys_tray = QString::fromLatin1("_NET_SYSTEM_TRAY_S%1").arg(screen);
        NET_SYSTEM_TRAY_SELECTION = XInternAtom(display, net_sys_tray.toLatin1(), False);
				NET_SYSTEM_TRAY_VISUAL = XInternAtom(display, "_NET_SYSTEM_TRAY_VISUAL", False);
    }

    return XGetSelectionOwner(QX11Info::display(), NET_SYSTEM_TRAY_SELECTION);
}

XVisualInfo* SystemTrayIcon::getSysTrayVisualInfo()
{
    Display *display = QX11Info::display();

    if (!sysTrayVisual.visual) {
        Window win = locateSystemTray();
        if (win != None) {
            Atom actual_type;
            int actual_format;
            ulong nitems, bytes_remaining;
            uchar *data = 0;
            int result = XGetWindowProperty(display, win, NET_SYSTEM_TRAY_VISUAL, 0, 1,
                                            False, XA_VISUALID, &actual_type,
                                            &actual_format, &nitems, &bytes_remaining, &data);
            VisualID vid = 0;
            if (result == Success && data && actual_type == XA_VISUALID && actual_format == 32 &&
                nitems == 1 && bytes_remaining == 0)
                vid = *(VisualID*)data;
            if (data)
                XFree(data);
            if (vid == 0)
                return 0;

            uint mask = VisualIDMask;
            XVisualInfo *vi, rvi;
            int count;
            rvi.visualid = vid;
            vi = XGetVisualInfo(display, mask, &rvi, &count);
            if (vi) {
                sysTrayVisual = vi[0];
                XFree((char*)vi);
            }
            if (sysTrayVisual.depth != 32)
                memset(&sysTrayVisual, 0, sizeof(sysTrayVisual));
        }
    }

    return sysTrayVisual.visual ? &sysTrayVisual : 0;
}

#if 0
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
        if ((cm->message_type == manager_atom) && ((Atom)cm->data.l[1] == NET_SYSTEM_TRAY_SELECTION)) {
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
#endif

bool SystemTrayIcon::sysTrayTracker(void *message, long *result)
{
    bool retval = false;
    if (SystemTrayIcon::oldEventFilter)
        retval = SystemTrayIcon::oldEventFilter(message, result);

    if (trayIcons.isEmpty())
        return retval;

    Display *display = QX11Info::display();
    XEvent *ev = (XEvent *)message;
    if  (ev->type == DestroyNotify && ev->xany.window == sysTrayWindow) {
	sysTrayWindow = locateSystemTray();
        memset(&sysTrayVisual, 0, sizeof(sysTrayVisual));
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
        if ((cm->message_type == manager_atom) && ((Atom)cm->data.l[1] == NET_SYSTEM_TRAY_SELECTION)) {
	    sysTrayWindow = cm->data.l[2];
            memset(&sysTrayVisual, 0, sizeof(sysTrayVisual));
	    XSelectInput(display, sysTrayWindow, StructureNotifyMask);
            for (int i = 0; i < trayIcons.count(); i++) {
                trayIcons[i]->addToTray();
            }
            retval = true;
        }
    } else if (ev->type == PropertyNotify && ev->xproperty.atom == NET_SYSTEM_TRAY_VISUAL &&
               ev->xproperty.window == sysTrayWindow) {
        memset(&sysTrayVisual, 0, sizeof(sysTrayVisual));
        for (int i = 0; i < trayIcons.count(); i++) {
            trayIcons[i]->addToTray();
        }
    }

    return retval;
}

SystemTrayIcon::SystemTrayIcon()
    : QWidget(0, Qt::FramelessWindowHint | Qt::X11BypassWindowManagerHint),
    colormap(0)
{
    setAttribute(Qt::WA_AlwaysShowToolTips);
    setAttribute(Qt::WA_QuitOnClose, false);
    setAttribute(Qt::WA_NoSystemBackground, true);
    setAttribute(Qt::WA_PaintOnScreen);

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
		resize(22, 22);
//#ifndef QT_NO_TOOLTIP
//    setToolTip(q->toolTip());
//#endif
    /*if (sysTrayWindow != None)
        addToTray();*/
}


SystemTrayIcon::~SystemTrayIcon()
{
	trayIcons.removeAt(trayIcons.indexOf(this));
	Display *display = QX11Info::display();
	if (trayIcons.isEmpty()) 
	{
		if (sysTrayWindow == None)
			return;
		if (display)
			XSelectInput(display, sysTrayWindow, 0); // stop tracking the tray
		sysTrayWindow = None;
	}
	if (colormap)
		XFreeColormap(display, colormap);
}

#if 0
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
#endif

void SystemTrayIcon::addToTray()
{
    Q_ASSERT(sysTrayWindow != None);
    Display *display = QX11Info::display();

    XVisualInfo *vi = getSysTrayVisualInfo();
    if (vi && vi->visual) {
        Window root = RootWindow(display, vi->screen);
        Window p = root;
        if (QWidget *pw = parentWidget())
            p = pw->effectiveWinId();
        colormap = XCreateColormap(display, root, vi->visual, AllocNone);
        XSetWindowAttributes wsa;
        wsa.background_pixmap = 0;
        wsa.colormap = colormap;
        wsa.background_pixel = 0;
        wsa.border_pixel = 0;
        Window wid = XCreateWindow(display, p, -1, -1, 1, 1,
                                   0, vi->depth, InputOutput, vi->visual,
                                   CWBackPixmap|CWBackPixel|CWBorderPixel|CWColormap, &wsa);
        create(wid);
    } else {
        XSetWindowBackgroundPixmap(display, winId(), ParentRelative);
    }

    // GNOME, NET WM Specification
    static Atom netwm_tray_atom = XInternAtom(display, "_NET_SYSTEM_TRAY_OPCODE", False);
    long l[5] = { CurrentTime, SYSTEM_TRAY_REQUEST_DOCK, (long)winId(), 0, 0 };
    XEvent ev;
    memset(&ev, 0, sizeof(ev));
    ev.xclient.type = ClientMessage;
    ev.xclient.window = sysTrayWindow;
    ev.xclient.message_type = netwm_tray_atom;
    ev.xclient.format = 32;
    memcpy((char *)&ev.xclient.data, (const char *) l, sizeof(l));
    XSendEvent(display, sysTrayWindow, False, 0, &ev);
		//qDebug("addToTray: %d %d", width(), height());
    setMinimumSize(width(), height()); // required at least on GNOME
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

void SystemTrayIcon::drawContents(QPainter *p)
{
}

void SystemTrayIcon::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    if (!getSysTrayVisualInfo()) {
        const QRegion oldSystemClip = p.paintEngine()->systemClip();
        const QRect clearedRect = oldSystemClip.boundingRect();
        XClearArea(QX11Info::display(), winId(), clearedRect.x(), clearedRect.y(),
                   clearedRect.width(), clearedRect.height(), False);
        QPaintEngine *pe = p.paintEngine();
        pe->setSystemClip(clearedRect);
				drawContents(&p);
        //q->icon().paint(&p, rect());
        pe->setSystemClip(oldSystemClip);
    } else {
        p.setCompositionMode(QPainter::CompositionMode_Source);
        p.fillRect(rect(), Qt::transparent);
        p.setCompositionMode(QPainter::CompositionMode_SourceOver);
        //q->icon().paint(&p, rect());
				drawContents(&p);
    }
}

bool SystemTrayIcon::x11Event(XEvent *event)
{
    if (event->type == ReparentNotify)
        show();
    /*else if (event->type == ConfigureNotify || event->type == Expose) {
        XClearArea(QX11Info::display(), winId(), 0, 0, width(), height(), False);
        qApp->syncX();
        background = QPixmap::grabWindow(winId());
        update();
    }*/
    return QWidget::x11Event(event);
}

#endif
