/***************************************************************************

  main.cpp

  (c) 2000-2009 Benoît Minisini <gambas@users.sourceforge.net>

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
  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

***************************************************************************/

#define __MAIN_CPP

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>

#include "gambas.h"

#include <QApplication>
#include <QMessageBox>
#include <QClipboard>
#include <QString>
#include <QMap>
#include <QFileInfo>
#include <QBuffer>
#include <QWidget>
#include <QEvent>
#include <QTextCodec>
#include <QTimer>
#include <QTranslator>
#include <QTimerEvent>
#include <QKeyEvent>
#include <QPixmap>
#include <QImageReader>
#include <QEventLoop>
#include <QDesktopWidget>
#include <QX11Info>
#include <QPaintDevice>

#include "gb.image.h"
#include "gb.qt.h"
#include "gb.form.font.h"

#include "CFont.h"
#include "CScreen.h"
#include "CWidget.h"
#include "CWindow.h"
#include "CButton.h"
#include "CContainer.h"
#include "CLabel.h"
#include "CListBox.h"
#include "CTextBox.h"
#include "CTextArea.h"
#include "CPictureBox.h"
#include "CMenu.h"
#include "CPanel.h"
#include "CMouse.h"
#include "CKey.h"
#include "CColor.h"
#include "CConst.h"
#include "CCheckBox.h"
#include "CFrame.h"
#include "CRadioButton.h"
#include "CTreeView.h"
#include "CIconView.h"
#include "CGridView.h"
#include "CTabStrip.h"
#include "CDialog.h"
#include "CPicture.h"
#include "CImage.h"
#include "CClipboard.h"
#include "CDraw.h"
#include "CWatch.h"
#include "CScrollView.h"
#include "CDrawingArea.h"
#include "CProgress.h"
#include "CMessage.h"
#include "CSlider.h"
#include "CScrollBar.h"
#include "CMovieBox.h"
#include "CSpinBox.h"
#include "CSplitter.h"
#include "CWatcher.h"

#ifndef NO_X_WINDOW
#include "CEmbedder.h"
#include "CTrayIcon.h"
#include "x11.h"
#endif

#include "main.h"

/*#define DEBUG*/

extern "C" {

GB_INTERFACE GB EXPORT;
IMAGE_INTERFACE IMAGE EXPORT;

}

int MAIN_in_wait = 0;
int MAIN_loop_level = 0;
int MAIN_scale = 6;
#ifndef NO_X_WINDOW
int MAIN_x11_last_key_code = 0;
#endif

static bool in_event_loop = false;
static int _no_destroy = 0;
static QTranslator *qt = NULL;
static bool _application_keypress = false;
static GB_FUNCTION _application_keypress_func;
static QWidget *_mouseGrabber = 0;
static QWidget *_keyboardGrabber = 0;
static bool _check_quit_posted = false;

#ifndef NO_X_WINDOW
static void (*_x11_event_filter)(XEvent *) = 0;
#endif

static QHash<void *, void *> _link_map;

//static MyApplication *myApp;

/***************************************************************************

	MyMimeSourceFactory

	Create a QMimeSourceFactory to handle files stored in an archive

***************************************************************************/

#if 0
class MyMimeSourceFactory: public Q3MimeSourceFactory
{
public:

	MyMimeSourceFactory();

	virtual const QMimeSource* data(const QString& abs_name) const;

private:

	QMap<QString, QString> extensions;
};


MyMimeSourceFactory::MyMimeSourceFactory()
{
	extensions.replace("htm", "text/html;charset=UTF-8");
	extensions.replace("html", "text/html;charset=UTF-8");
	extensions.replace("txt", "text/plain");
	extensions.replace("xml", "text/xml;charset=UTF-8");
	extensions.replace("jpg", "image/jpeg");
	extensions.replace("png", "image/png");
	extensions.replace("gif", "image/gif");
}


const QMimeSource* MyMimeSourceFactory::data(const QString& abs_name) const
{
	char *addr;
	int len;
	Q3StoredDrag* sr = 0;
	char *path;

	//qDebug("MyMimeSourceFactory::data: %s", (char *)abs_name.latin1());

	path = (char *)abs_name.latin1();

	if (true) //abs_name[0] != '/')
	{
		if (GB.LoadFile(path, 0, &addr, &len))
			GB.Error(NULL);
		else
		{
			QByteArray ba;
			ba.setRawData((const char *)addr, len);

			QFileInfo fi(abs_name);
			QString e = fi.extension(FALSE);
			Q3CString mimetype = "text/html"; //"application/octet-stream";

			const char* imgfmt;

			if ( extensions.contains(e) )
				mimetype = extensions[e].latin1();
			else
			{
				QBuffer buffer(&ba);

				buffer.open(QIODevice::ReadOnly);
				if (( imgfmt = QImageReader::imageFormat( &buffer ) ) )
					mimetype = Q3CString("image/")+Q3CString(imgfmt).lower();
				buffer.close();
			}

			sr = new Q3StoredDrag( mimetype );
			sr->setEncodedData( ba );

			ba.resetRawData((const char*)addr, len);

			//qDebug("MimeSource: %s %s", abs_name.latin1(), (const char *)mimetype);

			GB.ReleaseFile(addr, len);
		}
	}

	return sr;
}

static MyMimeSourceFactory myMimeSourceFactory;
#endif

#if 0
/***************************************************************************

	MyAbstractEventDispatcher

	Manage window deletion

***************************************************************************/

class MyAbstractEventDispatcher : public QAbstractEventDispatcher
{
public:
	MyAbstractEventDispatcher();
	virtual bool processEvents(QEventLoop::ProcessEventsFlags flags);
};

MyAbstractEventDispatcher::MyAbstractEventDispatcher()
: QAbstractEventDispatcher()
{
}

bool MyAbstractEventDispatcher::processEvents(QEventLoop::ProcessEventsFlags flags)
{
	bool ret;
	CWIDGET **ptr;
	CWIDGET *ob;

	MAIN_loop_level++;
	ret = QAbstractEventDispatcher::processEvents(flags);
	MAIN_loop_level--;

	for(;;)
	{
		ptr = &CWIDGET_destroy_list;
	
		for(;;)
		{
			ob = *ptr;
			if (!ob)
				return ret;
	
			//if (MAIN_loop_level <= ob->level && !ob->flag.notified)
			if (!ob->flag.notified)
			{
				//qDebug("delete: %s %p", GB.GetClassName(ob), ob);
				//qDebug(">> delete %p (%p) :%p:%ld", ob, ob->widget, ob->ob.klass, ob->ob.ref);
				//*ptr = ob->next;
				delete ob->widget;
				break;
				//GB.Unref(POINTER(&ob));
				//qDebug("   delete %p (%p) :%p:%ld #2", ob, ob->widget, ob->ob.klass, ob->ob.ref);
				//qDebug("<< delete %p (%p)", ob, ob->widget);
			}
			else
			{
				//qDebug("cannot delete: %s %p", GB.GetClassName(ob), ob);
				ptr = &ob->next;
			}
		}
	}
	//return ret;
}
#endif

void MAIN_process_events(void)
{
	_no_destroy++;
	qApp->processEvents(QEventLoop::ExcludeUserInputEvents, 0);
	_no_destroy--;
}

/** MyApplication **********************************************************/

bool MyApplication::_tooltip_disable = false;
int MyApplication::_event_filter = 0;
QEventLoop *MyApplication::eventLoop = 0;

MyApplication::MyApplication(int &argc, char **argv)
: QApplication(argc, argv)
{
}

static bool QT_EventFilter(QEvent *e)
{
	QKeyEvent *kevent = (QKeyEvent *)e;
	bool cancel;
	
	if (!_application_keypress)
		return false;
	
	CKEY_clear(true);

	GB.FreeString(&CKEY_info.text);
	GB.NewString(&CKEY_info.text, QT_ToUTF8(kevent->text()), 0);
	CKEY_info.state = kevent->modifiers();
	CKEY_info.code = kevent->key();

	GB.Call(&_application_keypress_func, 0, FALSE);
	cancel = GB.Stopped();
	
	CKEY_clear(false);
	
	return cancel;
}

static bool QT_Notify(CWIDGET *object, bool value)
{
	bool old = object->flag.notified;
	//qDebug("QT_Notify: %s %p %d", GB.GetClassName(object), object, value);
	object->flag.notified = value;
	return old;
}

bool MyApplication::eventFilter(QObject *o, QEvent *e)
{
	if (o->isWidgetType())
	{
		if (e->type() == QEvent::KeyPress)
		{
			if (QT_EventFilter(e))
				return true;
		}
		else if (e->type() == QEvent::ToolTip)
		{
			if (_tooltip_disable)
				return true;
		}
	}

	return QApplication::eventFilter(o, e);
}

/*bool MyApplication::notify(QObject *o, QEvent *e)
{
	if (o->isWidgetType())
	{
		CWIDGET *ob = CWidget::get(o);		
		bool old, res;
	
		if (ob)
		{
			old = QT_Notify(ob, true);
			res = QApplication::notify(o, e);
			QT_Notify(ob, old);
			return res;
		}
	}
	
	return QApplication::notify(o, e);
}*/

void MyApplication::setEventFilter(bool set)
{
	if (set)
	{
		_event_filter++;
		if (_event_filter == 1)
			qApp->installEventFilter(qApp);
	}
	else
	{
		_event_filter--;
		if (_event_filter == 0)
			qApp->removeEventFilter(qApp);
	}
}

void MyApplication::setTooltipEnabled(bool b)
{
	b = !b;
	if (b == _tooltip_disable)
		return;
		
	_tooltip_disable = b;
	setEventFilter(b);
}

#ifndef NO_X_WINDOW

static void x11_set_event_filter(void (*filter)(XEvent *))
{
	_x11_event_filter = filter;
}

bool MyApplication::x11EventFilter(XEvent *e)
{
	// Workaround for input methods that void the key code of KeyRelease eventFilter
	if (e->type == XKeyPress)
		MAIN_x11_last_key_code = e->xkey.keycode;
	else if (e->type == XKeyRelease)
		MAIN_x11_last_key_code = e->xkey.keycode;
	
	if (_x11_event_filter)
		(*_x11_event_filter)(e);
	
	return false;
}
#endif

/** MyTimer ****************************************************************/

MyTimer::MyTimer(GB_TIMER *t) : QObject(0)
{
	timer = t;
	id = startTimer(t->delay);
}

MyTimer::~MyTimer()
{
	killTimer(id);
}

void MyTimer::timerEvent(QTimerEvent *e)
{
	if (timer)
		GB.RaiseTimer(timer);
}

/***************************************************************************/

static void release_grab()
{
	_mouseGrabber = QWidget::mouseGrabber();
	_keyboardGrabber = QWidget::keyboardGrabber();
	
	if (_mouseGrabber) 
	{
		//qDebug("releaseMouse");
		_mouseGrabber->releaseMouse();
	}
	if (_keyboardGrabber)
	{
		//qDebug("releaseKeyboard");
		_keyboardGrabber->releaseKeyboard();
	}
	
	#ifndef NO_X_WINDOW
	if (qApp->activePopupWidget())
	{
		XUngrabPointer(QX11Info::display(), CurrentTime);
		XFlush(QX11Info::display());
	}
	#endif
}


static void unrelease_grab()
{
	if (_mouseGrabber)
	{
		//qDebug("grabMouse");
		_mouseGrabber->grabMouse();
		_mouseGrabber = 0;
	}
	
	if (_keyboardGrabber)
	{
		//qDebug("grabKeyboard");
		_keyboardGrabber->grabKeyboard();
		_keyboardGrabber = 0;
	}
}

static bool must_quit(void)
{
	#if DEBUG_WINDOW
	qDebug("must_quit: Window = %d Watch = %d in_event_loop = %d", CWindow::count, CWatch::count, in_event_loop);
	#endif
	return CWindow::count == 0 && CWatch::count == 0 && in_event_loop;
}

static void check_quit_now(intptr_t param)
{
	if (must_quit())
	{
		#ifndef NO_X_WINDOW
			CTRAYICON_close_all();
			qApp->syncX();
		#endif
		qApp->exit();
	}
	else
		_check_quit_posted = false;
}

void MAIN_check_quit(void)
{
	if (_check_quit_posted)
		return;
	
	GB.Post((GB_POST_FUNC)check_quit_now, 0);
	_check_quit_posted = true;
}

void MAIN_update_scale(void)
{
	//QFontMetrics fm(qApp->desktop()->font());
	//MAIN_scale = GET_DESKTOP_SCALE(fm.height());
	MAIN_scale = GET_DESKTOP_SCALE(qApp->desktop()->font().pointSize(), QX11Info::appDpiY());
}

static void QT_InitEventLoop(void)
{
}

extern void qt_x11_set_global_double_buffer(bool);

static void QT_Init(void)
{
	static bool init = false;
	QFont f;

	if (init)
		return;

	//qApp->setAttribute(Qt::AA_ImmediateWidgetCreation);

	#ifndef NO_X_WINDOW
		X11_init(QX11Info::display(), QX11Info::appRootWindow());
	#endif

	/*fcntl(ConnectionNumber(qt_xdisplay()), F_SETFD, FD_CLOEXEC);*/
	
	//Q3MimeSourceFactory::addFactory(&myMimeSourceFactory);

	MAIN_update_scale();

	if (GB.GetFunction(&_application_keypress_func, (void *)GB.FindClass(GB.Application.Startup()), "Application_KeyPress", "", "") == 0)
	{
		_application_keypress = true;
		MyApplication::setEventFilter(true);
	}

	qApp->installEventFilter(&CWidget::manager);

	//qt_x11_set_global_double_buffer(false);

	//Q3StyleSheet::defaultSheet()->item("tt")->setFontFamily("Monospace");
	//Q3StyleSheet::defaultSheet()->item("pre")->setFontFamily("Monospace");

	init = true;
}

static QString _init_lang;
static bool _init_rtl;

static void init_lang(QString locale, bool rtl)
{
	qt = new QTranslator();
	qt->load(QString( "qt_") + locale, QString(getenv("QTDIR")) + "/translations");

	qApp->installTranslator(qt);
	if (rtl)
		qApp->setLayoutDirection(Qt::RightToLeft);
}

static void hook_lang(char *lang, int rtl)
{
	QString locale(lang);

	if (!qApp)
	{
		_init_lang = locale;
		_init_rtl = rtl;
		return;
	}

	init_lang(locale, rtl);

	//locale = QTextCodec::locale();
}

static void hook_main(int *argc, char **argv)
{
	//QApplication::setGraphicsSystem("raster");
	new MyApplication(*argc, argv);
	
	QT_Init();
	init_lang(_init_lang, _init_rtl);
}


static void hook_loop()
{
	//qDebug("**** ENTERING EVENT LOOP");
	
	qApp->processEvents(QEventLoop::ExcludeUserInputEvents | QEventLoop::DeferredDeletion, 0);

	in_event_loop = true;

	if (!must_quit())
		qApp->exec();
	else
		MAIN_check_quit();
}


static void hook_wait(int duration)
{
	MAIN_in_wait++;
	if (duration > 0)
		qApp->processEvents(QEventLoop::AllEvents, duration);
	else
		qApp->processEvents(QEventLoop::ExcludeUserInputEvents, duration);
	MAIN_in_wait--;
}


static void hook_timer(GB_TIMER *timer, bool on)
{
	if (timer->id)
	{
		MyTimer *t = (MyTimer *)(timer->id);
		t->clearTimer();
		t->deleteLater();
		timer->id = 0;
	}
	
	if (on)
		timer->id = (intptr_t)(new MyTimer(timer));
}


static void hook_watch(int fd, int type, void *callback, intptr_t param)
{
	CWatch::watch(fd, type, (GB_WATCH_CALLBACK)callback, param);
}


static void hook_post(void)
{
	static MyPostCheck check;

	//qDebug("hook_post ?");

	if (MyPostCheck::in_check)
		return;

	//qDebug("hook_post !");

	MyPostCheck::in_check = true;
	QTimer::singleShot(0, &check, SLOT(check()));
}


static void hook_quit()
{
	QWidgetList list;
	QWidget *w;
	int i;

	//qApp->closeAllWindows();

	list = QApplication::topLevelWidgets();

	for (i = 0; i < list.count(); i++)
	{
		w = list.at(i);
		w->close();
	}
}



static void hook_error(int code, char *error, char *where)
{
	QString msg;

	qApp->restoreOverrideCursor();
	while (qApp->activePopupWidget())
		delete qApp->activePopupWidget();
	CWatch::stop();

	msg = "<b>This application has raised an unexpected<br>error and must abort.</b><br><br>";
	
	if (code > 0)
	{
		msg = msg + "[%1] %2.<br>%3";
		msg = msg.arg(code).arg(error).arg(where);
	}
	else
	{
		msg = msg + "%1.<br>%2";
		msg = msg.arg(error).arg(where);
	}

	release_grab();
	QMessageBox::critical(0, TO_QSTRING(GB.Application.Name()), msg);
	unrelease_grab();

	//qApp->exit();
}

#if 0
static int hook_image(CIMAGE **pimage, GB_IMAGE_INFO *info) //void **pdata, int width, int height, int format)
{
	CIMAGE *image = *pimage;
	QImage *img;

	if (!image)
	{
		img = new QImage(info->width, info->height, GB_IMAGE_TRANSPARENT(info->format) ? QImage::Format_ARGB32 : QImage::Format_RGB32);

		if (info->data)
			GB.Image.Convert(img->bits(), GB_IMAGE_BGRA, info->data, info->format, info->width, info->height);

		GB.New(POINTER(&image), GB.FindClass("Image"), NULL, NULL);
		delete image->image;
		image->image = img;

		*pimage = image;
	}
	else
	{
		info->data = image->image->bits();
		info->width = image->image->width();
		info->height = image->image->height();
		info->format = image->image->hasAlphaChannel() ? GB_IMAGE_BGRA : GB_IMAGE_BGRX;
	}

	return 0;
}

static int hook_picture(CPICTURE **ppicture, GB_PICTURE_INFO *info)
{
	CPICTURE *picture = *ppicture;
	QImage *img;

	if (!picture)
	{
		if (info->format == GB_IMAGE_BGRA || info->format == GB_IMAGE_BGRX)
			img = new QImage((uchar *)info->data, info->width, info->height, info->format == GB_IMAGE_BGRA ? QImage::Format_ARGB32 : QImage::Format_RGB32);
		else
		{
			img = new QImage(info->width, info->height, GB_IMAGE_TRANSPARENT(info->format) ? QImage::Format_ARGB32 : QImage::Format_RGB32);
			GB.Image.Convert(img->bits(), GB_IMAGE_BGRA, info->data, info->format, info->width, info->height);
		}

		GB.New(POINTER(&picture), GB.FindClass("Picture"), NULL, NULL);
		delete picture->pixmap;
		*picture->pixmap = QPixmap::fromImage(*img);
		delete img;

		*ppicture = picture;
	}
	else
	{
		info->data = NULL;
		info->format = GB_IMAGE_BGRA;
		info->width = picture->pixmap->width();
		info->height = picture->pixmap->height();
	}

	return 0;
}
#endif

static void QT_InitWidget(QWidget *widget, void *object)
{
	CWIDGET_new(widget, object);
}

void *QT_GetObject(QWidget *widget)
{
	return CWidget::get((QObject *)widget);
}

static QWidget *QT_GetContainer(void *object)
{
	return QCONTAINER(object);
}

/*static bool QT_IsDestroyed(void *object)
{
	return CWIDGET_test_flag(object, WF_DELETED);
}*/

static QPixmap *QT_GetPixmap(CPICTURE *pict)
{
	return pict->pixmap;
}

const char *QT_ToUTF8(const QString &str)
{
	static QByteArray buf[UTF8_NBUF];
	static int cpt = 0;

	const char *res;

	buf[cpt] = str.toUtf8();
	res = buf[cpt].data();
	cpt++;
	if (cpt >= UTF8_NBUF)
		cpt = 0;

	return res;
}

static void *QT_CreateFont(const QFont &f, FONT_FUNC func, void *object)
{
	return CFONT_create(f, func, object);
}

static void *QT_CreatePicture(const QPixmap &p)
{
	return CPICTURE_create(&p);
}

static void *QT_GetDrawInterface()
{
	return (void *)&DRAW_Interface;
}

void MyApplication::linkDestroyed(QObject *qobject)
{
	void *object = _link_map.value(qobject, 0);
	_link_map.remove(qobject);
	if (object)
		GB.Unref(POINTER(&object));
}

void QT_Link(QObject *qobject, void *object)
{
	_link_map.insert(qobject, object);
  QObject::connect(qobject, SIGNAL(destroyed(QObject *)), qApp, SLOT(linkDestroyed(QObject *)));
	GB.Ref(object);
}

void *QT_GetLink(QObject *qobject)
{
	return _link_map.value(qobject, 0);
}

extern "C" {

GB_DESC *GB_CLASSES[] EXPORT =
{
	CBorderDesc, CColorDesc,
	CAlignDesc, CArrangeDesc, CScrollDesc, CKeyDesc, CLineDesc, CFillDesc, CSelectDesc,
	CMessageDesc,
	CImageDesc, CPictureDesc,
	CFontDesc, CFontsDesc,
	CMouseDesc, CCursorDesc,
	CClipboardDesc, CDragDesc,
	CDesktopDesc, CApplicationTooltipDesc, CApplicationDesc,
	CControlDesc, CChildrenDesc, CContainerDesc,
	CUserControlDesc, CUserContainerDesc,
	CMenuChildrenDesc, CMenuDesc,
	CLabelDesc, CTextLabelDesc, CPictureBoxDesc, CSeparatorDesc,
	CButtonDesc, CToggleButtonDesc, CToolButtonDesc,
	CCheckBoxDesc, CRadioButtonDesc,
	CTextBoxSelectionDesc, CTextBoxDesc, CComboBoxItemDesc, CComboBoxDesc,
	CTextAreaSelectionDesc, CTextAreaDesc,
	CListBoxItemDesc, CListBoxDesc,
	CListViewItemDesc, CListViewDesc,
	CTreeViewItemDesc, CTreeViewDesc,
	CColumnViewItemDesc, CColumnViewColumnDesc, CColumnViewColumnsDesc, CColumnViewDesc,
	CIconViewItemDesc, CIconViewDesc,
	CGridItemDesc, CGridRowDesc, CGridColumnDesc, CGridRowsDesc, CGridColumnsDesc, CGridViewDataDesc, CGridViewDesc,
	CFrameDesc, CPanelDesc, CHBoxDesc, CVBoxDesc, CHPanelDesc, CVPanelDesc,
	CHSplitDesc, CVSplitDesc,
	CTabChildrenDesc, CTabDesc, CTabStripDesc,
	CScrollViewDesc,
	CDrawingAreaDesc,
	CProgressDesc, CSliderDesc, CSpinBoxDesc, CMovieBoxDesc, CScrollBarDesc,
	CWindowMenusDesc, CWindowControlsDesc, CWindowTypeDesc, CWindowDesc, CWindowsDesc, CFormDesc,
	CDialogDesc,
	#ifndef NO_X_WINDOW
	CEmbedderDesc, CTrayIconDesc, CTrayIconsDesc,
	#endif
	CWatcherDesc,
	NULL
};

void *GB_QT4_1[] EXPORT = {

	(void *)1,

	(void *)QT_InitEventLoop,
	(void *)QT_Init,
	(void *)QT_InitWidget,
	(void *)QT_GetObject,
	(void *)QT_GetContainer,
	(void *)CWIDGET_border_simple,
	(void *)CWIDGET_border_full,
	(void *)CWIDGET_scrollbar,
	(void *)CCONTROL_font,
	(void *)QT_CreateFont,
	(void *)QT_CreatePicture,
	//(void *)QT_MimeSourceFactory,
	(void *)QT_GetPixmap,
	(void *)QT_ToUTF8,
	(void *)QT_EventFilter,
	(void *)QT_Notify,
	(void *)QT_GetDrawInterface,
	(void *)CCONST_alignment,
	(void *)QT_Link,
	(void *)QT_GetLink,
	NULL
};

#if 0
#if QT_VERSION >= 0x030304
static void myMessageHandler(QtMsgType type, const char *msg )
{
	if ((::strncmp(msg, "QMultiInputContext::", strlen("QMultiInputContext::")) == 0)
			|| (::strncmp(msg, "sending IM", strlen("sending IM")) == 0)
			|| (::strncmp(msg, "receiving IM", strlen("receiving IM")) == 0)
			|| (::strncmp(msg, "QInputContext: ", strlen("QInputContext: ")) == 0))
		return;

	fprintf(stderr, "%s\n", msg);
	if (type == QtFatalMsg)
		abort();
}
#endif
#endif

const char *GB_INCLUDE EXPORT = "gb.draw";

int EXPORT GB_INIT(void)
{
	#if 0
	#if QT_VERSION >= 0x030304
	qInstallMsgHandler(myMessageHandler);
	#endif
	#endif

	GB.Hook(GB_HOOK_MAIN, (void *)hook_main);
	GB.Hook(GB_HOOK_LOOP, (void *)hook_loop);
	GB.Hook(GB_HOOK_WAIT, (void *)hook_wait);
	GB.Hook(GB_HOOK_TIMER, (void *)hook_timer);
	GB.Hook(GB_HOOK_WATCH, (void *)hook_watch);
	GB.Hook(GB_HOOK_POST, (void *)hook_post);
	GB.Hook(GB_HOOK_QUIT, (void *)hook_quit);
	GB.Hook(GB_HOOK_ERROR, (void *)hook_error);
	GB.Hook(GB_HOOK_LANG, (void *)hook_lang);

	GB.LoadComponent("gb.draw");
	GB.GetInterface("gb.image", IMAGE_INTERFACE_VERSION, &IMAGE);
  IMAGE.SetDefaultFormat(GB_IMAGE_BGRA);
	DRAW_init();
	
	CLASS_Control = GB.FindClass("Control");
	CLASS_Container = GB.FindClass("Container");
	CLASS_UserControl = GB.FindClass("UserControl");
	CLASS_UserContainer = GB.FindClass("UserContainer");
	CLASS_Window = GB.FindClass("Window");
	CLASS_Menu = GB.FindClass("Menu");
	CLASS_Picture = GB.FindClass("Picture");
	CLASS_Drawing = GB.FindClass("Drawing");
	CLASS_DrawingArea = GB.FindClass("DrawingArea");
	CLASS_Printer = GB.FindClass("Printer");
	CLASS_ScrollView = GB.FindClass("ScrollView");

	QT_InitEventLoop();

	return TRUE;
}

void EXPORT GB_EXIT()
{
	#ifndef NO_X_WINDOW
		X11_exit();
	#endif
	//qApp->setStyle("windows");
	//delete qApp;
}

#ifndef NO_X_WINDOW
int EXPORT GB_INFO(const char *key, void **value)
{
	if (!strcasecmp(key, "DISPLAY"))
	{
		*value = (void *)QX11Info::display();
		return TRUE;
	}
	else if (!strcasecmp(key, "ROOT_WINDOW"))
	{
		*value = (void *)QX11Info::appRootWindow();
		return TRUE;
	}
	else if (!strcasecmp(key, "SET_EVENT_FILTER"))
	{
		*value = (void *)x11_set_event_filter;
		return TRUE;
	}
	else
		return FALSE;
}
#endif

/*#ifndef NO_X_WINDOW
extern Time	qt_x_time;
#endif*/

static void activate_main_window(int value)
{
	CWINDOW *active;
	
	active = CWINDOW_Active;
	if (!active) active = CWINDOW_LastActive;
	
	if (!active)
		return;
	
	MyMainWindow *win = (MyMainWindow *)active->widget.widget;
	if (win && !win->isWindow())
		win = (MyMainWindow *)win->window();
	if (win)
	{
		/*#ifndef NO_X_WINDOW
		qt_x_time = CurrentTime;
		#endif*/
		win->raise();
		win->activateWindow();
	}
}

void EXPORT GB_SIGNAL(int signal, void *param)
{
	if (!qApp)
		return;
	
	switch(signal)
	{
		case GB_SIGNAL_DEBUG_BREAK:
			release_grab();
			break;
			
		case GB_SIGNAL_DEBUG_FORWARD:
			//while (qApp->activePopupWidget())
			//	delete qApp->activePopupWidget();
			qApp->syncX();
			break;
			
		case GB_SIGNAL_DEBUG_CONTINUE:
			GB.Post((GB_POST_FUNC)activate_main_window, 0);
			unrelease_grab();
			break;
	}
}

} // extern "C"

/* class MyPostCheck */

bool MyPostCheck::in_check = false;

void MyPostCheck::check(void)
{
	//qDebug("MyPostCheck::check");
	in_check = false;
	GB.CheckPost();
}

