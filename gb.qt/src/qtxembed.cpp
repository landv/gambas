/***************************************************************************
** This file may be distributed under the terms of the Q Public License
** as defined by Trolltech AS of Norway and appearing in the file
** LICENSE.QPL included in the packaging of this file.
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.
*/
#include <unistd.h>
#include "qtxembed.h"
#include <qlayout.h>
#include <qmap.h>
#include <qapplication.h>
#include <qpushbutton.h>
#include <qstyle.h>
#include <qdatetime.h>
#include <qvbox.h>
#include <qpainter.h>    // to draw in container

#include <qapplication.h>
#include <qdatetime.h>
#include <qptrlist.h>
#include <qptrdict.h>
#include <qguardedptr.h>
#include <qwhatsthis.h>
#include <qfocusdata.h>

#include <qaccel.h>      // accelerators
#include <qobjectlist.h> // accelerators

#define XK_MISCELLANY
#define XK_LATIN1
#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <X11/Xutil.h>
#include <X11/keysymdef.h>


#ifndef XK_ISO_Left_Tab
#define XK_ISO_Left_Tab 0xFE20
#endif

const int XFocusOut = FocusOut;
const int XFocusIn = FocusIn;
const int XKeyPress = KeyPress;
const int XKeyRelease = KeyRelease;
const int XButtonPress = ButtonPress;
const int XButtonRelease = ButtonRelease;
#undef KeyRelease
#undef KeyPress
#undef FocusOut
#undef FocusIn
#undef ButtonPress
#undef ButtonRelease

#include <qevent.h>

/*! \class QtXEmbedClient qtxembed.h

    \brief The QtXEmbedClient class provides an XEmbed client widget.

    \ingroup solutions-widgets

    XEmbed is an X11 protocol that supports the embedding of a widget
    from one application into another application.

    An XEmbed \e client is a window that is embedded into a \e
    container. A container is the graphical location that embeds (or
    \e swallows) an external client.

    QtXEmbedClient is a widget used for writing XEmbed applets or
    plugins. When it has been embedded and the container receives tab
    focus, focus is passed on to the client. When the client reaches
    the end of its focus chain, focus is passed back to the
    container. Window activation, accelerator support, modality and
    drag and drop (XDND) are also handled.

    The client and container can both initiate the embedding. If the
    client is the initiator, the X11 window ID of the container that
    it wants to embed itself into must be known. If the client calls
    embedInto() passing this window ID, it will be embedded.

    If the container initiates the embedding, the window ID of the
    client must be known. The container calls embed(), passing the
    window ID, to embed the client.

    This example shows an application where the client calls
    embedInto() with the window ID passed as a command line argument:

    \code
        int main(int argc, char *argv[])
	{
	    QApplication app(argc, argv);

	    if (app.argc() != 2) {
              // Error - expected window id as argument.
	      return 1;
	    }

	    QtXEmbedClient client(0);
	    app.setMainWindow(&client);

	    client.embedInto(app.argv()[1]);
	    client.show();

	    return app.exec();
	}
    \endcode

    The problem of obtaining the window IDs is often solved by the
    container invoking the client (as a panel invokes a docked applet)
    as a seperate process, passing its window ID to the client as a
    command line argument. The client can then call embedInto() with
    the container's window ID, as shown in the example code above.
    Similarily, the client can report its window ID to the container
    through IPC, in which case the container can embed the client.

    When the client has been embedded, it emits the signal
    embedded(). If it is closed by the container, the client emits
    containerClosed(). If an error occurs when embedding, error() is
    emitted.

    There are XEmbed widgets available for KDE and GTK+. The GTK+
    equivalent of QtXEmbedClient is GtkPlug. The KDE widget is called
    QXEmbed.

    \sa QtXEmbedContainer, http://www.freedesktop.org/standards/xembed-spec/
*/

/*! \class QtXEmbedContainer qtxembed.h

    \brief The QtXEmbedContainer class provides an XEmbed container
    widget.

    \ingroup solutions-widgets

    XEmbed is an X11 protocol that supports the embedding of a widget
    from one application into another application.

    An XEmbed \e container is the graphical location that embeds an
    external \e client. A client is a window that is embedded into a
    container.

    When a client has been embedded and the container receives tab
    focus, focus is passed on to the client. When the client reaches
    the end of his focus chain, focus is passed back to the
    container. Window activation, accelerator support, modality and
    drag and drop (XDND) are also handled.

    QtXEmbedContainer is commonly used for writing panels or toolbars
    that hold applets, or for \e swallowing X11 applications. When
    writing a panel application, one container widget is created on
    the toolbar, and it can then either swallow another widget using
    embed(), or allow an XEmbed client to be embedded into itself. The
    container's X11 window ID, which is retrieved with winId(), must
    then be known to the client. After embedding, the client's window
    ID can be retrieved with clientWinId().

    In the following example, a container widget is created as the
    main widget. It then invokes an application called "playmovie",
    passing its window ID as a command line argument. The "playmovie"
    program is an XEmbed client. The client embeds itself into the
    container using the container's window ID.

    \code
        int main(int argc, char *argv[])
	{
	    QApplication app(argc, argv);

            QtXEmbedContainer container(0);
	    app.setMainWidget(&container);
	    container.show();

	    QProcess proc(&container);
	    proc.addArgument("/usr/bin/playvideo");
	    proc.addArgument(QString::number(container.winId()));
	    if (!proc.start()) {
                // An error occurred
		return 1;
	    }

	    return app.exec();
	}
    \endcode

    When the client is embedded, the container emits the signal
    clientIsEmbedded(). The signal clientClosed() is emitted when a
    client closes.

    It is possible for QtXEmbedContainer to embed XEmbed clients from
    toolkits other than Qt, such as GTK+. Non-XEmbed clients can also
    be embedded, but the XEmbed specific features such as window
    activation and focus handling are then lost.

    The GTK+ equivalent of QtXEmbedContainer is GtkSocket. The KDE
    widget is called QXEmbed.

    \sa QtXEmbedClient, http://www.freedesktop.org/standards/xembed-spec/
*/

/*! \fn QtXEmbedClient::embedded()

    This signal is emitted by the client that has been embedded by an
    XEmbed container.
*/

/*! \fn QtXEmbedClient::containerClosed()

    This signal is emitted by the client when the container closes the
    client. This can happen if the container itself closes, or if the
    client is rejected.

    The container can reject a client for any reason, but the most
    common cause of a rejection is when an attempt is made to  embed a
    client into a container that already has an embedded client.
*/

/*! \fn QtXEmbedContainer::clientIsEmbedded()

    This signal is emitted by the container when a client has been
    embedded.
*/

/*! \fn QtXEmbedContainer::clientClosed()

    This signal is emitted by the container when the client closes.
*/

/*! \fn QtXEmbedClient::error(int)

    This signal is emitted if an error occurred as a result of
    embedding into or communicating with a container.

    \sa QtXEmbedClient::Errors
*/

/*! \fn QtXEmbedContainer::error(int)

    This signal is emitted if an error occurred when embedding or
    communicating with a client.

    \sa QtXEmbedContainer::Errors
*/

/*! \enum QtXEmbedClient::Errors

    \value Unknown An unrecognized error occurred.

    \value InvalidWindowID The X11 window ID of the container was
        invalid. This error is usually triggered by passing an invalid
        window ID to embedInto().
*/

/*! \enum QtXEmbedContainer::Errors

    \value Unknown An unrecognized error occurred.

    \value InvalidWindowID The X11 window ID of the container was
        invalid. This error is usually triggered by passing an invalid
        window ID to embed().
*/

// From qapplication_x11.cpp
typedef int (*QX11EventFilter) (XEvent*);
extern QX11EventFilter qt_set_x11_event_filter (QX11EventFilter filter);
extern Time qt_x_time;
extern void qt_x11_intern_atom(const char *, Atom *);
extern Atom qt_wm_protocols;
extern Atom qt_wm_delete_window;
extern Atom qt_wm_take_focus;
extern Atom qt_wm_state;
static XKeyEvent lastKeyEvent;

// This is a hack to move topData() out from protected to public.  We
// need to to inspect topLevelWidget()'s embedded state.
class HackWidget : public QWidget
{
public:
    QTLWExtra *topData() { return QWidget::topData(); }
};

static unsigned int XEMBED_VERSION = 0;
static Atom _XEMBED = None;
static Atom _XEMBED_INFO = None;

static QX11EventFilter oldX11EventFilter = 0;

enum QtXEmbedMessageType {
    XEMBED_EMBEDDED_NOTIFY = 0,
    XEMBED_WINDOW_ACTIVATE = 1,
    XEMBED_WINDOW_DEACTIVATE = 2,
    XEMBED_REQUEST_FOCUS = 3,
    XEMBED_FOCUS_IN = 4,
    XEMBED_FOCUS_OUT = 5,
    XEMBED_FOCUS_NEXT = 6,
    XEMBED_FOCUS_PREV = 7,
    XEMBED_MODALITY_ON = 10,
    XEMBED_MODALITY_OFF = 11,
    XEMBED_REGISTER_ACCELERATOR = 12,
    XEMBED_UNREGISTER_ACCELERATOR = 13,
    XEMBED_ACTIVATE_ACCELERATOR = 14
};

enum QtXEmbedFocusInDetail {
    XEMBED_FOCUS_CURRENT = 0,
    XEMBED_FOCUS_FIRST = 1,
    XEMBED_FOCUS_LAST = 2
};

enum QtXEmbedFocusInFlags {
    XEMBED_FOCUS_OTHER = (0 << 0),
    XEMBED_FOCUS_WRAPAROUND = (1 << 0)
};

enum QtXEmbedInfoFlags {
    XEMBED_MAPPED = (1 << 0)
};

enum QtXEmbedAccelModifiers {
    XEMBED_MODIFIER_SHIFT = (1 << 0),
    XEMBED_MODIFIER_CONTROL = (1 << 1),
    XEMBED_MODIFIER_ALT = (1 << 2),
    XEMBED_MODIFIER_SUPER = (1 << 3),
    XEMBED_MODIFIER_HYPER = (1 << 4)
};

enum QtXEmbedAccelFlags {
    XEMBED_ACCELERATOR_OVERLOADED = (1 << 0)
};

// Silence the default X11 error handler.
static int x11ErrorHandler(Display *, XErrorEvent *)
{
    return 0;
}

// Initializes X11 atoms
static void initXEmbedAtoms(Display *d)
{
    if (_XEMBED == None)
	_XEMBED = XInternAtom(d, "_XEMBED", false);

    if (_XEMBED_INFO == None)
	_XEMBED_INFO = XInternAtom(d, "_XEMBED_INFO", false);
}

// Returns the X11 timestamp. Maintained mainly by qapplication
// internals, but also updated by the XEmbed widgets.
static Time x11Time()
{
    return qt_x_time;
}

// Gives the version and flags of the supported XEmbed protocol.
static unsigned int XEmbedVersion()
{
    return 0;
}

// Sends an XEmbed message.
static void sendXEmbedMessage(WId window, Display *display, long message,
			      long detail = 0, long data1 = 0, long data2 = 0)
{
    XClientMessageEvent c;
    memset(&c, 0, sizeof(c));
    c.type = ClientMessage;
    c.message_type = _XEMBED;
    c.format = 32;
    c.display = display;
    c.window = window;

    c.data.l[0] = x11Time();
    c.data.l[1] = message;
    c.data.l[2] = detail;
    c.data.l[3] = data1;
    c.data.l[4] = data2;

    XSendEvent(display, window, false, NoEventMask, (XEvent *) &c);
}

// The purpose of this global x11 filter is for one to capture the key
// events in their original state, but most importantly this is the
// only way to get the WM_TAKE_FOCUS message from WM_PROTOCOLS.
static int x11EventFilter(XEvent *event)
{
    if (event->type == XKeyPress || event->type == XKeyRelease)
	lastKeyEvent = event->xkey;

    // Update qt_x_time when a focusin is received. If the widget that
    // receives this event is active, send a WindowActivate
    // event. This will cause that widget to call XSetInputFocus
    // immediately, in which case the timestamp will be correct.
    if (event->type == ClientMessage && event->xclient.message_type == qt_wm_protocols) {
	QWidget *w = QWidget::find(event->xclient.window);
	if (w) {
	    Atom a = event->xclient.data.l[0];
	    if (a == qt_wm_take_focus) {
		if ((ulong) event->xclient.data.l[1] > qt_x_time )
		    qt_x_time = event->xclient.data.l[1];

		if (w->isActiveWindow()) {
		    QEvent e(QEvent::WindowActivate);
		    QApplication::sendEvent(w, &e);
		}
	    }
	}
    }

    if (oldX11EventFilter && oldX11EventFilter != &x11EventFilter)
	return oldX11EventFilter(event);
    else
	return false;
}

/*!
    Constructs a QtXEmbedClient object. The \a parent and \a name
    arguments are passed on to QWidget's constructor.
*/
QtXEmbedClient::QtXEmbedClient(QWidget *parent, const char *name)
    : QWidget(parent, name), container(0)
{
    XSetErrorHandler(x11ErrorHandler);
    initXEmbedAtoms(x11Display());

    XSelectInput(qt_xdisplay(), winId(),
    						 EnterWindowMask | LeaveWindowMask |
                 KeyPressMask | KeyReleaseMask | ButtonPressMask
		 | ButtonReleaseMask
                 | KeymapStateMask | ButtonMotionMask | PointerMotionMask
                 | FocusChangeMask
		 | ExposureMask | StructureNotifyMask
		 | SubstructureNotifyMask | PropertyChangeMask);

    unsigned int data[] = {XEMBED_VERSION, XEMBED_MAPPED};
    XChangeProperty(x11Display(), winId(), _XEMBED_INFO,
		    XA_CARDINAL, 32, PropModeReplace,
		    (unsigned char*) data, 2);

    setFocusPolicy(StrongFocus);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    qApp->installEventFilter(this);
}

/*!
    Destructs the QtXEmbedClient object. If the client is embedded
    when deleted, it is hidden and then detached from its container,
    so that the container is free to embed a new client.
*/
QtXEmbedClient::~QtXEmbedClient()
{
    if (container) {
	XUnmapWindow(x11Display(), winId());
	XReparentWindow(x11Display(), winId(), qt_xrootwin(), 0, 0);
    }
}

/*!
    When this function is called, the client embeds itself into the
    container whose window ID is \a id.

    If \a id is \e not the window ID of a container this function will
    behave unpredictably.
*/
void QtXEmbedClient::embedInto(WId id)
{
    container = id;
    switch (XReparentWindow(x11Display(), winId(), container, 0, 0)) {
    case BadWindow:
	emit error(InvalidWindowID);
	break;
    case BadMatch:
	emit error(Internal);
	break;
    case Success:
    default:
	break;
    }
}

/*! \internal

    Sets the embedded flag on the topLevelWidget.
*/
void QtXEmbedClient::setEmbedded()
{
    ((HackWidget *)topLevelWidget())->topData()->embedded = true;
}

/*! \internal

    Handles WindowActivate and FocusIn events for the client.
*/
bool QtXEmbedClient::eventFilter(QObject *o, QEvent *event)
{
    switch (event->type()) {
    case QEvent::FocusIn:
	switch (((QFocusEvent *)event)->reason()) {
	case QFocusEvent::Mouse:
	    // If the user clicks into one of the client widget's
	    // children and we didn't have focus already, we request
	    // focus from our container.
	    if (o != topLevelWidget()) {
		QWidget *w = reinterpret_cast<QWidget *>(o);
		if (currentFocus.isNull())
		    sendXEmbedMessage(container, x11Display(), XEMBED_REQUEST_FOCUS);

		currentFocus = w;
	    }
	    break;
	case QFocusEvent::Tab:
            // If the topLevelWidget receives a focus event because of
	    // a Tab, then we are at the end of our focus chain and we
	    // ask the container to move to its next focus widget.
	    if (o == topLevelWidget()) {
		// Setting focus on the client itself removes Qt's
		// logical focus rectangle. We can't just do a
		// clearFocus here, because when we send the synthetic
		// FocusIn to ourselves on activation, Qt will set
		// focus on focusWidget() again. This way, we "hide"
		// focus rather than clearing it.
		if (!topLevelWidget()->hasFocus()) {
		    QFocusEvent::setReason(QFocusEvent::Other);
		    topLevelWidget()->setFocus();
		    QFocusEvent::resetReason();
		}

		sendXEmbedMessage(container, x11Display(), XEMBED_FOCUS_NEXT);

		currentFocus = 0;
		return true;
	    } else {
		// We're listening on events from qApp, so in order
		// for us to know who to set focus on if we receive an
		// activation event, we note the widget that got the
		// focusin last.
		currentFocus = (QWidget *)o;
	    }
	    break;
	case QFocusEvent::Backtab:
	    // If the topLevelWidget receives a focus event because of
	    // a Backtab, then we are at the start of our focus chain
	    // and we ask the container to move to its previous focus
	    // widget.
	    if (o == topLevelWidget()) {

		// See comment for Tab.
		if (!topLevelWidget()->hasFocus()) {
		    QFocusEvent::setReason(QFocusEvent::Other);
		    topLevelWidget()->setFocus();
		    QFocusEvent::resetReason();
		}

		sendXEmbedMessage(container, x11Display(), XEMBED_FOCUS_PREV);

		// If we receive a XEMBED_FOCUS_IN
		// XEMBED_FOCUS_CURRENT, we will set focus in
		// currentFocus. To avoid that in this case, we reset
		// currentFocus.
		currentFocus = 0;
		return true;
	    } else {
		currentFocus = (QWidget *)o;
	    }
	    break;
	case QFocusEvent::ActiveWindow:
	    // This FocusIn is received when the topLevelWidget is activated.
	    if (!currentFocus.isNull()) {
		if (!currentFocus->hasFocus()) {
		    QFocusEvent::setReason(QFocusEvent::ActiveWindow);
		    currentFocus->setFocus();
		    QFocusEvent::resetReason();
		}
	    } else {
		if (!topLevelWidget()->hasFocus()) {
		    QFocusEvent::setReason(QFocusEvent::Other);
		    topLevelWidget()->setFocus();
		    QFocusEvent::resetReason();
		}
		return true;
	    }

	    break;
	case QFocusEvent::Popup:
	case QFocusEvent::Shortcut:
	case QFocusEvent::Other:
	    // If focus is received to any child widget because of any
	    // other reason, remember the widget so that we can give
	    // it focus again if we're activated.
	    currentFocus = (QWidget *)o;
	    break;
	default:
	    break;
	}
	break;
    default:
	break;
    }

    return QWidget::eventFilter(o, event);
}

/*! \internal

    Handles some notification events and client messages. Client side
    XEmbed message receiving is also handled here.
*/
bool QtXEmbedClient::x11Event(XEvent *event)
{
    switch (event->type) {
    case DestroyNotify:
	// If the container window is destroyed, X11 will also destroy
	// the client window. We signal this to the user.
	emit containerClosed();
        break;
    case ReparentNotify:
	// If the container shuts down, we will be reparented to the
	// root window. We must also consider the case that we may be
	// reparented from one container to another.
	if (event->xreparent.parent == qt_xrootwin()) {
	    emit containerClosed();
	    return true;
	} else {
	    container = event->xreparent.parent;
	}
	break;
    case UnmapNotify:
	// Mapping and unmapping are handled by changes to the
	// _XEMBED_INFO property. Any default map/unmap requests are
	// ignored.
	return true;
    case PropertyNotify:
	// The container sends us map/unmap messages through the
	// _XEMBED_INFO property. We adhere to the XEMBED_MAPPED bit in
	// data2.
	if (event->xproperty.atom != _XEMBED_INFO) {
	    Atom actual_type_return;
	    int actual_format_return;
	    unsigned long nitems_return;
	    unsigned long bytes_after_return;
	    unsigned char *prop_return = 0;
	    if (XGetWindowProperty(x11Display(), winId(), _XEMBED_INFO, 0, 2,
				   false, XA_CARDINAL, &actual_type_return,
				   &actual_format_return, &nitems_return,
				   &bytes_after_return, &prop_return) == Success) {
		if (nitems_return > 1) {
		    if (((int * )prop_return)[1] & XEMBED_MAPPED)
			XMapWindow(x11Display(), winId());
		    else
			XUnmapWindow(x11Display(), winId());
		}
	    }
	}

	break;
    case ClientMessage:
	// XEMBED messages have message_type _XEMBED
	if (event->xclient.message_type == _XEMBED) {
	    // Discard XEMBED messages not to ourselves. (### dead code?)
	    if (event->xclient.window != winId())
		break;

	    // Update qt_x_time if necessary
	    Time msgtime = (Time) event->xclient.data.l[0];
	    if (msgtime > qt_x_time)
		qt_x_time = msgtime;

	    switch (event->xclient.data.l[1]) {
	    case XEMBED_WINDOW_ACTIVATE: {
		// When we receive an XEMBED_WINDOW_ACTIVATE, we need
		// to send ourselves a synthetic FocusIn X11 event for
		// Qt to activate us.
		XEvent ev;
                memset(&ev, 0, sizeof(ev));
                ev.xfocus.display = x11Display();
                ev.xfocus.type = XFocusIn;
                ev.xfocus.window = topLevelWidget()->winId();
                ev.xfocus.mode = NotifyNormal;
                ev.xfocus.detail = NotifyNonlinear;
                qApp->x11ProcessEvent(&ev);
	    }
		break;
	    case XEMBED_WINDOW_DEACTIVATE: {
		// When we receive an XEMBED_WINDOW_DEACTIVATE, we
		// need to send ourselves a synthetic FocusOut event
		// for Qt to deativate us.
                XEvent ev;
                memset(&ev, 0, sizeof(ev));
                ev.xfocus.display = x11Display();
                ev.xfocus.type = XFocusOut;
                ev.xfocus.window = topLevelWidget()->winId();
                ev.xfocus.mode = NotifyNormal;
                ev.xfocus.detail = NotifyNonlinear;
                qApp->x11ProcessEvent( &ev );
	    }
		break;
	    case XEMBED_EMBEDDED_NOTIFY: {
		// In this message's l[2] we have the max version
		// supported by both the client and the
		// container. QtXEmbedClient does not use this field.

		// We have been embedded, so we set our
		// topLevelWidget's embedded flag.
		setEmbedded();
		emit embedded();
	    }
		break;
	    case XEMBED_FOCUS_IN:
		switch (event->xclient.data.l[2]) {
		case XEMBED_FOCUS_CURRENT:
		    // The container sends us this message if it wants
		    // us to focus on the widget that last had focus.
		    // This is the reply when XEMBED_REQUEST_FOCUS is
		    // sent to the container.
		    if (!currentFocus.isNull()) {
			if (!currentFocus->hasFocus()) {
			    // ### necessary?
			    while (focusData()->next() != currentFocus)
				;

			    QFocusEvent event(QEvent::FocusIn);
			    QFocusEvent::setReason(QFocusEvent::Other);
			    qApp->sendEvent(currentFocus, &event);
			    QFocusEvent::resetReason();
			}
		    } else {
			// No widget currently has focus. We set focus
			// on the first widget next to the
			// topLevelWidget.
			while (focusData()->next() != topLevelWidget())
			    ;

			QFocusEvent::setReason(QFocusEvent::Other);
			focusData()->next()->setFocus();
			QFocusEvent::resetReason();
		    }

		    // This is used to determine wether the focus
		    // wraps around (if the client has nothing to
		    // focus on).
		    focusOriginator = XEMBED_FOCUS_CURRENT;
		    break;
		case XEMBED_FOCUS_FIRST:
		    // The container sends this message when it wants
		    // us to focus on the first widget in our focus
		    // chain (typically because of a tab).
		    while (focusData()->next() != topLevelWidget())
			;

		    QFocusEvent::setReason(QFocusEvent::Tab);
		    focusData()->next()->setFocus();
		    QFocusEvent::resetReason();

		    focusOriginator = XEMBED_FOCUS_FIRST;
		    break;
		case XEMBED_FOCUS_LAST:
		    // The container sends this message when it wants
		    // us to focus on the last widget in our focus
		    // chain (typically because of a backtab).
		    while (focusData()->next() != topLevelWidget())
			;

		    QFocusEvent::setReason(QFocusEvent::Backtab);
		    focusData()->prev()->setFocus();
		    QFocusEvent::resetReason();

		    focusOriginator = XEMBED_FOCUS_LAST;
		    break;
		default:
		    // Ignore any other XEMBED_FOCUS_IN details.
		    break;
		}
		break;
	    case XEMBED_FOCUS_OUT:
		// The container sends us this message when it wants us
		// to lose focus and forget about the widget that last
		// had focus. Typically sent by the container when it
		// loses focus because of mouse or tab activity. We do
		// then not want to set focus on anything if we're
		// activated.
		if (!topLevelWidget()->hasFocus()) {
		    QFocusEvent::setReason(QFocusEvent::Other);
		    topLevelWidget()->setFocus();
		    QFocusEvent::resetReason();
		}

		currentFocus = 0;

		break;
	    default:
		// Ignore any other XEMBED messages.
		break;
	    };
	} else {
	    // Non-XEMBED client messages are not interesting.
	}

	break;
    default:
	// Ignore all other x11 events.
	break;
    }

    // Allow default handling.
    return QWidget::x11Event(event);
}

/*!
    If the client is embedded, returns the window ID of the container;
    otherwize returns 0.
*/
WId QtXEmbedClient::containerWinId() const
{
    return container;
}

/*!
    Creates a QtXEmbedContainer object. The \a parent and \a name
    arguments are passed on to QWidget.
*/
QtXEmbedContainer::QtXEmbedContainer(QWidget *parent, const char *name)
    : QWidget(parent, name), client(0), focusProxy(0), clientIsXEmbed(false)
{
    XSetErrorHandler(x11ErrorHandler);
    initXEmbedAtoms(x11Display());

    setFocusPolicy(StrongFocus);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    setKeyCompression(false);
    setAcceptDrops(true);
    setEnabled(false);

    // Everybody gets a focus proxy, but only one toplevel container's
    // focus proxy is actually in use.
    focusProxy = new QWidget(this, "QtXEmbedContainer focus proxy");
    focusProxy->setGeometry(-1, -1, 1, 1);

    // We need events from the topLevelWidget (activation status) and
    // from qApp (keypress/release).
    qApp->installEventFilter(this);
    topLevelWidget()->installEventFilter(this);

    // Install X11 event filter.
    if (!oldX11EventFilter)
	oldX11EventFilter = qt_set_x11_event_filter(x11EventFilter);

    XSelectInput(qt_xdisplay(), winId(),
                 KeyPressMask | KeyReleaseMask
                 | ButtonPressMask | ButtonReleaseMask | ButtonMotionMask
                 | KeymapStateMask
                 | PointerMotionMask
                 | EnterWindowMask | LeaveWindowMask
                 | FocusChangeMask
                 | ExposureMask
                 | StructureNotifyMask
                 | SubstructureNotifyMask);

    // Move input to our focusProxy if this widget is active, and not
    // shaded by a modal dialog (in which case isActiveWindow() would
    // still return true, but where we must not move input focus).
    if (qApp->activeWindow() == topLevelWidget() && !isEmbedded())
	moveInputToProxy();
}

/*!
    Destructs a QtXEmbedContainer.
*/
QtXEmbedContainer::~QtXEmbedContainer()
{
    if (client) {
	XUnmapWindow(x11Display(), client);
	XReparentWindow(x11Display(), client, qt_xrootwin(), 0, 0);
    }

    if (xgrab)
	XUngrabButton(x11Display(), AnyButton, AnyModifier, winId());
}

/*! \internal

    Returns wether or not the topLevelWidgets's embedded flag is set.
*/
bool QtXEmbedContainer::isEmbedded() const
{
    return ((HackWidget *)topLevelWidget())->topData()->embedded != 0;
}

/*! \internal

    Returns the parentWinId of the topLevelWidget.
*/
WId QtXEmbedContainer::topLevelParentWinId() const
{
    return ((HackWidget *)topLevelWidget())->topData()->parentWinId;
}

/*!
    If the container has a client, this function returns the X11 window
    ID of the client; otherwise it returns 0.
*/
WId QtXEmbedContainer::clientWinId() const
{
    return client;
}

/*!
    Instructs the container to embed the X11 window with window ID \a
    id. The client window will then move on top of the container
    window and be resized to fit into the container.

    The \a id should be the ID of a window controlled by an XEmbed
    enabled application, but this is not mandatory. If \a id does not
    belong to an XEmbed client, then focus handling, activation,
    accelerators and other features will not work properly.
*/
void QtXEmbedContainer::embed(WId id, bool xEmbedClient)
{
    if (id == 0) {
	emit error(InvalidWindowID);
	return;
    }

    // Walk up the tree of parent windows to prevent embedding of ancestors.
    WId thisId = winId();
    Window rootReturn;
    Window parentReturn;
    Window *childrenReturn = 0;
    unsigned int nchildrenReturn;
    do {
        if (XQueryTree(x11Display(), thisId, &rootReturn,
                       &parentReturn, &childrenReturn, &nchildrenReturn) == 0) {
            emit error(InvalidWindowID);
            return;
        }
        if (childrenReturn) {
            XFree(childrenReturn);
            childrenReturn = 0;
        }

        thisId = parentReturn;
        if (id == thisId) {
            emit error(InvalidWindowID);
            return;
        }
    } while (thisId != rootReturn);

    clientIsXEmbed = xEmbedClient;

    // watch for property notify events (see below)
    XGrabServer(x11Display());
    XWindowAttributes attrib;
    if (!XGetWindowAttributes(x11Display(), id, &attrib)) {
        XUngrabServer(x11Display());
	emit error(InvalidWindowID);
	return;
    }
    XSelectInput(x11Display(), id, attrib.your_event_mask | PropertyChangeMask);
    XUngrabServer(x11Display());

    // Put the window into WithdrawnState
    XUnmapWindow(x11Display(), id);
    XSync(x11Display(), False); // make sure the window is hidden

    /*
      Wait for notification from the window manager that the window is
      not in withdrawn state.  According to the ICCCM section 4.1.3.1,
      we should wait for the WM_STATE property to either be deleted or
      set to WithdrawnState.

      For safety, we will not wait more than 500ms, so that we can
      preemptively workaround buggy window managers.
    */
    QTime t;
    t.start();
    for (;;) {
	if (t.elapsed() > 500) // time-out after 500ms
	    break;

	XEvent event;
	if (!XCheckTypedWindowEvent(x11Display(), id, PropertyNotify, &event)) {
	    XSync(x11Display(), False);
	    continue;
	}
	if (event.xproperty.atom != qt_wm_state) {
	    qApp->x11ProcessEvent(&event);
	    continue;
	}

	if (event.xproperty.state == PropertyDelete)
	    break;

	Atom ret;
	int format, status;
	long *state;
	unsigned long nitems, after;
	status = XGetWindowProperty(x11Display(), id, qt_wm_state, 0, 2, False, qt_wm_state,
				    &ret, &format, &nitems, &after, (unsigned char **) &state );
	if (status == Success && ret == qt_wm_state && format == 32 && nitems > 0) {
	    if (state[0] == WithdrawnState)
		break;
	}
    }

    // restore the event mask
    XSelectInput(x11Display(), id, attrib.your_event_mask);

    switch (XReparentWindow(x11Display(), id, winId(), 0, 0)) {
    case BadWindow:
    case BadMatch:
	emit error(InvalidWindowID);
	break;
    default:
	break;
    }
}

/*! \internal

    Handles key, activation and focus events for the container.
*/
bool QtXEmbedContainer::eventFilter(QObject *o, QEvent *event)
{
    switch (event->type()) {
    case QEvent::KeyPress:
        // Forward any keypresses to our client.
	if (o == this && client) {
	    lastKeyEvent.window = client;
	    XSendEvent(x11Display(), client, false, KeyPressMask, (XEvent *) &lastKeyEvent);
	    return true;
	}
	break;
    case QEvent::KeyRelease:
	// Forward any keyreleases to our client.
	if (o == this && client) {
	    lastKeyEvent.window = client;
	    XSendEvent(x11Display(), client, false, KeyReleaseMask, (XEvent *) &lastKeyEvent);
	    return true;
	}
	break;

    case QEvent::WindowActivate:
	// When our container window is activated, we pass the
	// activation message on to our client. Note that X input
	// focus is set to our focus proxy. We want to intercept all
	// keypresses.
	if (o == topLevelWidget() && client) {
	    if (!isEmbedded())
		moveInputToProxy();

	    if (clientIsXEmbed) {
		sendXEmbedMessage(client, x11Display(), XEMBED_WINDOW_ACTIVATE);
	    } else {
		checkGrab();
                if (hasFocus())
                    XSetInputFocus(x11Display(), client, RevertToParent, qt_x_time);
            }
	}
	break;
    case QEvent::WindowDeactivate:
	// When our container window is deactivated, we pass the
	// deactivation message to our client.
	if (o == topLevelWidget() && client) {
	    if (clientIsXEmbed)
		sendXEmbedMessage(client, x11Display(), XEMBED_WINDOW_DEACTIVATE);
	    else
		checkGrab();
	}
	break;
    case QEvent::FocusIn:
        // When receiving FocusIn events generated by Tab or Backtab,
	// we pass focus on to our client. Any mouse activity is sent
	// directly to the client, and it will ask us for focus with
	// XEMBED_REQUEST_FOCUS.
	if (o == this && client) {
	    if (clientIsXEmbed) {
                if (!isEmbedded())
                    moveInputToProxy();

		QFocusEvent *fe = (QFocusEvent *)event;
		switch (fe->reason()) {
		case QFocusEvent::Tab:
		    sendXEmbedMessage(client, x11Display(), XEMBED_FOCUS_IN, XEMBED_FOCUS_FIRST);
		    break;
		case QFocusEvent::Backtab:
		    sendXEmbedMessage(client, x11Display(), XEMBED_FOCUS_IN, XEMBED_FOCUS_LAST);
		    break;
		default:
		    break;
		}
	    } else {
		checkGrab();
		XSetInputFocus(x11Display(), client, RevertToParent, qt_x_time);
	    }
	}

	break;
    case QEvent::FocusOut: {
	// When receiving a FocusOut, we ask our client to remove its
	// focus.
	if (o == this && client) {

	    if (!isEmbedded())
		moveInputToProxy();

	    if (clientIsXEmbed) {
		QFocusEvent *fe = (QFocusEvent *)event;
		if (o == this && client && fe->reason() != QFocusEvent::ActiveWindow)
		    sendXEmbedMessage(client, x11Display(), XEMBED_FOCUS_OUT);
	    } else {
		checkGrab();
	    }
	}
    }
	break;

    case QEvent::Close: {
	if (o == this && client) {
	    // Unmap the client and reparent it to the root window.
	    // Wait until the messages have been processed. Then ask
	    // the window manager to delete the window.
	    XUnmapWindow(x11Display(), client);
	    XReparentWindow(x11Display(), client, qt_xrootwin(), 0, 0);
	    XSync(x11Display(), false);

	    XEvent ev;
	    memset(&ev, 0, sizeof(ev));
	    ev.xclient.type = ClientMessage;
	    ev.xclient.window = client;
	    ev.xclient.message_type = qt_wm_protocols;
	    ev.xclient.format = 32;
	    ev.xclient.data.s[0] = qt_wm_delete_window;
	    XSendEvent(x11Display(), client, false, NoEventMask, &ev);

	    XFlush(x11Display());
	    client = 0;
	    clientIsXEmbed = false;
            wmMinimumSizeHint = QSize();
            updateGeometry();
            setEnabled(false);
	    update();

	    emit clientClosed();
	}
    }
    default:
	break;
    }

    return QObject::eventFilter(o, event);
}

/*! \internal

    Handles X11 events for the container.
*/
bool QtXEmbedContainer::x11Event(XEvent *event)
{
    switch (event->type) {
    case CreateNotify:
	// The client created an embedded window.
	if (client)
	    rejectClient(event->xcreatewindow.window);
	else
	    acceptClient(event->xcreatewindow.window);
      break;
    case DestroyNotify:
	if (event->xdestroywindow.window == client) {
	    // The client died.
	    client = 0;
	    clientIsXEmbed = false;
            wmMinimumSizeHint = QSize();
            updateGeometry();
            setEnabled(false);
	    update();
	    emit clientClosed();
	}
        break;
    case ReparentNotify:
	// The client sends us this if it reparents itself out of our
	// widget.
	if (event->xreparent.window == client && event->xreparent.parent != winId()) {
	    client = 0;
	    clientIsXEmbed = false;
            wmMinimumSizeHint = QSize();
            updateGeometry();
            setEnabled(false);
	    update();
	    emit clientClosed();
	} else if (event->xreparent.parent == winId()) {
	    // The client reparented itself into this window.
	    if (client)
		rejectClient(event->xreparent.window);
	    else
		acceptClient(event->xreparent.window);
	}

	break;
    case ClientMessage: {
	if (event->xclient.message_type == _XEMBED) {
	    // Ignore XEMBED messages not to ourselves
	    if (event->xclient.window != winId())
		break;

	    // Receiving an XEmbed message means the client
	    // is an XEmbed client.
	    clientIsXEmbed = true;

	    Time msgtime = (Time) event->xclient.data.l[0];
	    if (msgtime > qt_x_time)
		qt_x_time = msgtime;

	    switch (event->xclient.data.l[1]) {
	    case XEMBED_REQUEST_FOCUS: {
		// This typically happens when the client gets focus
		// because of a mouse click.
		if (!hasFocus()) {
		    QFocusEvent::setReason(QFocusEvent::Other);
		    setFocus();
		    QFocusEvent::resetReason();
		}

		// The message is passed along to the topmost container
		// that eventually responds with a XEMBED_FOCUS_IN
		// message. The focus in message is passed all the way
		// back until it reaches the original focus
		// requestor. In the end, not only the original client
		// has focus, but also all its ancestor containers.
		if (isEmbedded()) {
                    // If our topLevelWidget's embedded flag is set, then
		    // that suggests that we are part of a client. The
		    // parentWinId will then point to an container to whom
		    // we must pass this message.
		    sendXEmbedMessage(topLevelParentWinId(), x11Display(), XEMBED_REQUEST_FOCUS);
		} else {
                    // Our topLevelWidget's embedded flag is not set,
		    // so we are the topmost container. We respond to
		    // the focus request message with a focus in
		    // message. This message will pass on from client
		    // to container to client until it reaches the
		    // originator of the XEMBED_REQUEST_FOCUS message.
		    sendXEmbedMessage(client, x11Display(), XEMBED_FOCUS_IN, XEMBED_FOCUS_CURRENT);
		}

		break;
	    }
	    case XEMBED_FOCUS_NEXT:
		// Client sends this event when it received a tab
		// forward and was at the end of its focus chain. If
		// we are the only widget in the focus chain, we send
		// ourselves a FocusIn event.
		if (focusData()->count() > 1) {
		    QFocusEvent::setReason(QFocusEvent::Tab);
		    focusNextPrevChild(true);
		    QFocusEvent::resetReason();
		} else {
		    QFocusEvent event(QEvent::FocusIn);
		    QFocusEvent::setReason(QFocusEvent::Tab);
		    qApp->sendEvent(this, &event);
		    QFocusEvent::resetReason();
		}

		break;
	    case XEMBED_FOCUS_PREV:
		// Client sends this event when it received a backtab
		// and was at the start of its focus chain. If we are
		// the only widget in the focus chain, we send
		// ourselves a FocusIn event.
		if (focusData()->count() > 1) {
		    QFocusEvent::setReason(QFocusEvent::Backtab);
		    focusNextPrevChild(false);
		    QFocusEvent::resetReason();
		} else {
		    QFocusEvent event(QEvent::FocusIn);
		    QFocusEvent::setReason(QFocusEvent::Backtab);
		    qApp->sendEvent(this, &event);
		    QFocusEvent::resetReason();
		}

		break;
	    default:
		break;
	    }
	}
    }
	break;
    case XButtonPress:
	if (!clientIsXEmbed) {
            QFocusEvent::setReason(QFocusEvent::Mouse);
            setFocus();
            QFocusEvent::resetReason();
            XAllowEvents(x11Display(), ReplayPointer, CurrentTime);
            return TRUE;
	}
	break;
    case XButtonRelease:
	if (!clientIsXEmbed)
            XAllowEvents(x11Display(), SyncPointer, CurrentTime);
	break;
    default:
	break;
    }

    return QWidget::x11Event(event);
}

/*! \internal

    Whenever the container is resized, we need to resize our client.
*/
void QtXEmbedContainer::resizeEvent(QResizeEvent *)
{
    if (client)
	XResizeWindow(x11Display(), client, width(), height());
}

/*! \internal

    We use the QShowEvent to signal to our client that we want it to
    map itself. We do this by changing its window property
    XEMBED_INFO. The client will get an X11 PropertyNotify.
*/
void QtXEmbedContainer::showEvent(QShowEvent *)
{
    if (client) {
	unsigned int data[] = {XEMBED_VERSION, XEMBED_MAPPED};
	XChangeProperty(x11Display(), client, _XEMBED_INFO, XA_CARDINAL, 32,
			PropModeReplace, (unsigned char *) data, 2);
    }
}

/*! \internal

    We use the QHideEvent to signal to our client that we want it to
    unmap itself. We do this by changing its window property
    XEMBED_INFO. The client will get an X11 PropertyNotify.
*/
void QtXEmbedContainer::hideEvent(QHideEvent *)
{
    if (client) {
	unsigned int data[] = {XEMBED_VERSION, XEMBED_MAPPED};

	XChangeProperty(x11Display(), client, _XEMBED_INFO, XA_CARDINAL, 32,
			PropModeReplace, (unsigned char *) data, 2);
    }
}

/*! \internal

    Rejects a client window by reparenting it to the root window.  The
    client will receive a reparentnotify, and will most likely assume
    that the container has shut down. The XEmbed protocol does not
    define any way to reject a client window, but this is a clean way
    to do it.
*/
void QtXEmbedContainer::rejectClient(WId window)
{
    setEnabled(false);
    XRemoveFromSaveSet(x11Display(), window);
    XReparentWindow(x11Display(), window, qt_xrootwin(), 0, 0);
}

/*! \internal

    Accepts a client by mapping it, resizing it and optionally
    activating and giving it logical focusing through XEMBED messages.
*/
void QtXEmbedContainer::acceptClient(WId window)
{
    client = window;
    setEnabled(true);

    // This tells Qt that we wish to forward DnD messages to
    // our client.
    extraData()->xDndProxy = client;

    unsigned int version = XEmbedVersion();

    Atom actual_type_return;
    int actual_format_return;
    unsigned long nitems_return = 0;
    unsigned long bytes_after_return;
    unsigned char *prop_return = 0;
    bool useXEmbedInfo = false;
    unsigned int clientflags = 0;
    unsigned int clientversion = 0;

    // Add this client to our saveset, so if we crash, the client window
    // doesn't get destroyed. This is useful for containers that restart
    // automatically after a crash, because it can simply reembed its clients
    // without having to restart them (KDE panel).
    XAddToSaveSet(x11Display(), client);

    // XEmbed clients have an _XEMBED_INFO property in which we can
    // fetch the version
    if (XGetWindowProperty(x11Display(), client, _XEMBED_INFO, 0, 2, false, XA_CARDINAL,
			   &actual_type_return, &actual_format_return, &nitems_return,
			   &bytes_after_return, &prop_return) == Success) {

	if (actual_type_return != None && actual_format_return != 0) {
	    // Clients with the _XEMBED_INFO property are XEMBED clients.
	    clientIsXEmbed = true;

	    unsigned int *p = (unsigned int *)prop_return;
	    if (nitems_return >= 2) {
		clientversion = p[0];
		clientflags = p[1];
		useXEmbedInfo = true;
	    }
	}

	XFree(prop_return);
    }

    // Store client window's original size and placement.
    Window root;
    int x_return, y_return;
    unsigned int width_return, height_return, border_width_return, depth_return;
    XGetGeometry(x11Display(), client, &root, &x_return, &y_return,
		 &width_return, &height_return, &border_width_return, &depth_return);
    clientOriginalRect.setCoords(x_return, y_return,
				 x_return + width_return - 1,
				 y_return + height_return - 1);

    // Ask the client for its minimum size.
    XSizeHints size;
    long msize;
    if (XGetWMNormalHints(x11Display(), client, &size, &msize) && (size.flags & PMinSize)) {
        wmMinimumSizeHint = QSize(size.min_width, size.min_height);
        updateGeometry();
    }

    // The container should set the data2 field to the lowest of its
    // supported version number and that of the client (from
    // _XEMBED_INFO property).
    unsigned int minversion = version > clientversion ? clientversion : version;
    sendXEmbedMessage(client, x11Display(), XEMBED_EMBEDDED_NOTIFY, winId(), minversion);
    XMapWindow(x11Display(), client);

    // Resize it, but no smaller than its minimum size hint.
    XResizeWindow(x11Display(), client,
                  QMAX(width(), size.min_width), QMAX(height(), size.min_height));
    update();

    // Resize it.
    XResizeWindow(x11Display(), client, width(), height());

    // Not mentioned in the protocol is that if the container
    // is already active, the client must be activated to work
    // properly.
    if (topLevelWidget()->isActiveWindow())
	sendXEmbedMessage(client, x11Display(), XEMBED_WINDOW_ACTIVATE);

    // Also, if the container already has focus, then it must
    // send a focus in message to its new client.
    if (focusWidget() == this && hasFocus())
	sendXEmbedMessage(client, x11Display(), XEMBED_FOCUS_IN, XEMBED_FOCUS_FIRST);
    else
        sendXEmbedMessage(client, x11Display(), XEMBED_FOCUS_OUT);

    if (!clientIsXEmbed) {
        checkGrab();
        if (hasFocus()) {
            XSetInputFocus(x11Display(), client, RevertToParent, qt_x_time);
        } else {
            if (!isEmbedded())
                moveInputToProxy();
        }
    }

    emit clientIsEmbedded();
}

/*! \internal

    Moves X11 keyboard input focus to the focusProxy, unless the focus
    is there already. When X11 keyboard input focus is on the
    focusProxy, which is a child of the container and a sibling of the
    client, X11 keypresses and keyreleases will always go to the proxy
    and not to the client.
*/
void QtXEmbedContainer::moveInputToProxy()
{
    WId focus;
    int revert_to;
    XGetInputFocus(x11Display(), &focus, &revert_to);
    if (focus != focusProxy->winId())
	XSetInputFocus(x11Display(), focusProxy->winId(), RevertToParent, x11Time());
}

/*! \internal

    Ask the window manager to give us a default minimum size.
*/
QSize QtXEmbedContainer::minimumSizeHint() const
{
    if (!client || !wmMinimumSizeHint.isValid())
	return QWidget::minimumSizeHint();
    return wmMinimumSizeHint;
}

/*! \internal

*/
void QtXEmbedContainer::checkGrab()
{
    if (!clientIsXEmbed && isActiveWindow() && !hasFocus()) {
        if (!xgrab) {
            XGrabButton(x11Display(), AnyButton, AnyModifier, winId(),
                        true, ButtonPressMask, GrabModeSync, GrabModeAsync,
                        None, None);
        }
        xgrab = true;
    } else {
	if (xgrab)
	    XUngrabButton(x11Display(), AnyButton, AnyModifier, winId());
        xgrab = false;
    }
}

/*!
    Detaches the client from the embedder. The client will appear as a
    standalone window on the desktop.
*/
void QtXEmbedContainer::discardClient()
{
    if (client) {
	XResizeWindow(x11Display(), client, clientOriginalRect.width(),
		      clientOriginalRect.height());

	rejectClient(client);
    }
}
