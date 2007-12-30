/***************************************************************************

  main.cpp

  The interface between the QT plug-in and the Gambas interpreter

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

#define __MAIN_CPP

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>

#include "gambas.h"

#define QT_THREAD_SUPPORT
#include <qapplication.h>
#include <qtooltip.h>
#include <qmessagebox.h>
#include <qclipboard.h>
#include <qstring.h>
#include <qmap.h>
#include <qmime.h>
#include <qfileinfo.h>
#include <qbuffer.h>
#include <qwidgetlist.h>
#include <qevent.h>
#include <qtextcodec.h>
#include <qpaintdevice.h>
#include <qtimer.h>
#include <qstylesheet.h>

#if QT_VERSION >= 0x030100
  #include <qeventloop.h>
#endif

#include "gb.qt.h"

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

}

int MAIN_in_wait = 0;
int MAIN_loop_level = 0;
int MAIN_scale = 8;
#ifndef NO_X_WINDOW
int MAIN_x11_last_key_code = 0;
#endif

static bool in_event_loop = false;
static QTranslator *qt = NULL;
static GB_FUNCTION _application_keypress_func;


/***************************************************************************

  MyMimeSourceFactory

  Create a QMimeSourceFactory to handle files stored in an archive

***************************************************************************/

class MyMimeSourceFactory: public QMimeSourceFactory
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
  QStoredDrag* sr = 0;
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
      QCString mimetype = "text/html"; //"application/octet-stream";

      const char* imgfmt;

      if ( extensions.contains(e) )
        mimetype = extensions[e].latin1();
      else
      {
        QBuffer buffer(ba);

        buffer.open(IO_ReadOnly);
        if (( imgfmt = QImageIO::imageFormat( &buffer ) ) )
          mimetype = QCString("image/")+QCString(imgfmt).lower();
        buffer.close();
      }

      sr = new QStoredDrag( mimetype );
      sr->setEncodedData( ba );

      ba.resetRawData((const char*)addr, len);

      //qDebug("MimeSource: %s %s", abs_name.latin1(), (const char *)mimetype);

      GB.ReleaseFile(&addr, len);
    }
  }

  return sr;
}

static MyMimeSourceFactory myMimeSourceFactory;


/***************************************************************************

  MyEventLoop

  Manage window deletion

***************************************************************************/

class MyEventLoop : public QEventLoop
{
public:
  MyEventLoop();
  virtual bool processEvents( ProcessEventsFlags flags );
};

MyEventLoop::MyEventLoop()
: QEventLoop()
{
}

bool MyEventLoop::processEvents(ProcessEventsFlags flags)
{
  bool ret;
  CWIDGET **ptr;
  CWIDGET *ob;

  MAIN_loop_level++;
  ret = QEventLoop::processEvents(flags);
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


/** MyApplication **********************************************************/

MyApplication::MyApplication(int &argc, char **argv)
: QApplication(argc, argv)
{
}

static bool QT_EventFilter(QEvent *e)
{
	QKeyEvent *kevent = (QKeyEvent *)e;
	bool cancel;

	CKEY_clear(true);

	GB.FreeString(&CKEY_info.text);
	GB.NewString(&CKEY_info.text, QT_ToUTF8(kevent->text()), 0);
	CKEY_info.state = kevent->state();
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
	if (o->isWidgetType() && e->type() == QEvent::KeyPress)
	{
		if (QT_EventFilter(e))
			return true;
	}

	return QObject::eventFilter(o, e);
}

bool MyApplication::notify(QObject *o, QEvent *e)
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
}

#ifndef NO_X_WINDOW
// Workaround for input methods that void the key code of KeyRelease eventFilter

bool MyApplication::x11EventFilter(XEvent *e)
{
	if (e->type == XKeyPress)
		MAIN_x11_last_key_code = e->xkey.keycode;
	else if (e->type == XKeyRelease)
		MAIN_x11_last_key_code = e->xkey.keycode;
	
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
  GB.RaiseTimer(timer);
}

/***************************************************************************/

static bool must_quit(void)
{
  /*
  QPtrDictIterator<CWINDOW> iter(CWindow::dict);
  int n;
  CWINDOW *win;

  n = 0;
  while ((win = iter.current()))
  {
    if (QWIDGET(win)->isVisible())
      n++;

    ++iter;
  }
  */

  //qDebug("CWindow::count = %d  CWatch::count = %d  in_event_loop = %d",
  //  CWindow::count, CWatch::count, in_event_loop);

  return CWindow::count == 0 && CWatch::count == 0 && in_event_loop;
}

void MAIN_check_quit(void)
{
  if (must_quit())
  {
  	#ifndef NO_X_WINDOW
    	CTRAYICON_close_all();
    	qApp->syncX();
		#endif
    qApp->exit();
  }
}

void MAIN_update_scale(void)
{
	QFontMetrics fm(QApplication::font());
  MAIN_scale = fm.height() / 2;
}

static void QT_InitEventLoop(void)
{
  new MyEventLoop();
}

static void QT_Init(void)
{
  static bool init = false;
  QFont f;

  if (init)
    return;

	#ifndef NO_X_WINDOW
  	X11_init(QPaintDevice::x11AppDisplay(), QPaintDevice::x11AppRootWindow());
  #endif

  /*fcntl(ConnectionNumber(qt_xdisplay()), F_SETFD, FD_CLOEXEC);*/
  QMimeSourceFactory::addFactory(&myMimeSourceFactory);

  MAIN_update_scale();

  if (GB.GetFunction(&_application_keypress_func, GB.FindClass(GB.Application.Startup()), "Application_KeyPress", "", "") == 0)
  	qApp->installEventFilter(qApp);

  qApp->installEventFilter(&CWidget::manager);

	QStyleSheet::defaultSheet()->item("tt")->setFontFamily("Monospace");
	QStyleSheet::defaultSheet()->item("pre")->setFontFamily("Monospace");

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
    qApp->setReverseLayout(true);
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
  new MyApplication(*argc, argv);
  #if QT_VERSION <= 0x030005
  qApp->unlock();
  #endif
  //qApp->setStyle("windows");
  QT_Init();
  init_lang(_init_lang, _init_rtl);
}


static void hook_loop()
{
  in_event_loop = true;

  if (!must_quit())
    qApp->exec();

  //qDebug("Exit event loop");
}


static void hook_wait(int duration)
{
  MAIN_in_wait++;
  #if QT_VERSION >= 0x030100
    if (duration > 0)
      qApp->eventLoop()->processEvents(QEventLoop::AllEvents, duration);
    else
      qApp->eventLoop()->processEvents(QEventLoop::ExcludeUserInput, duration);
  #else
    qApp->processEvents(duration);
  #endif
  MAIN_in_wait--;
}


static void hook_timer(GB_TIMER *timer, bool on)
{
	if (timer->id)
	{
		delete (MyTimer *)(timer->id);
		timer->id = 0;
	}
	
	if (on)
		timer->id = (int)(new MyTimer(timer));
}


static void hook_watch(int fd, int type, void *callback, int param)
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
  QWidgetList *list;
  QWidget *w;

  //qApp->closeAllWindows();

  list = QApplication::topLevelWidgets();

  w = list->first();

  while (w)
  {
    //QApplication::postEvent(list->first(), new QEvent(EVENT_CLOSE));
    w->close();
    w = list->next();
  }

  delete list;
}


static void hook_error(int code, char *error, char *where)
{
  QString msg;

  qApp->restoreOverrideCursor();
  while (qApp->activePopupWidget())
    delete qApp->activePopupWidget();
  CWatch::stop();
  qApp->exit();

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

  QMessageBox::critical(0, TO_QSTRING(GB.Application.Name()), msg);
}


static int hook_image(CIMAGE **pimage, GB_IMAGE_INFO *info) //void **pdata, int width, int height, int format)
{
	CIMAGE *image = *pimage;
	QImage *img;

	if (!image)
	{
  	img = new QImage(info->width, info->height, 32);
		img->setAlphaBuffer(GB_IMAGE_TRANSPARENT(info->format));

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
		info->format = image->image->hasAlphaBuffer() ? GB_IMAGE_BGRA : GB_IMAGE_BGRX;
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
			img = new QImage((uchar *)info->data, info->width, info->height, 32, NULL, 0, QImage::LittleEndian);
		else
		{
			img = new QImage(info->width, info->height, 32);
		  GB.Image.Convert(img->bits(), GB_IMAGE_BGRA, info->data, info->format, info->width, info->height);
		}

		img->setAlphaBuffer(info->format == GB_IMAGE_BGRA || info->format == GB_IMAGE_ARGB);

		GB.New(POINTER(&picture), GB.FindClass("Picture"), NULL, NULL);
		delete picture->pixmap;
		picture->pixmap = new QPixmap(*img);
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

static void QT_InitWidget(QWidget *widget, void *object)
{
  CWIDGET_new(widget, object);
}

static void *QT_GetObject(QWidget *widget)
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

QMimeSourceFactory *QT_MimeSourceFactory(void)
{
  return &myMimeSourceFactory;
}

const char *QT_ToUTF8(const QString &str)
{
  static QCString buf[UTF8_NBUF];
  static int cpt = 0;

  const char *res;

  buf[cpt] = str.utf8();
  res = (const char *)buf[cpt];
  cpt++;
  if (cpt >= UTF8_NBUF)
    cpt = 0;

  return res;
}

static void QT_CreateFont(const QFont &f, FONT_FUNC func, void *object)
{
  CFONT_create(f, func, object);
}

static void *QT_GetDrawInterface()
{
	return (void *)&DRAW_Interface;
}

extern "C" {

GB_DESC *GB_CLASSES[] EXPORT =
{
  CBorderDesc, CColorDesc, CColorInfoDesc,
  CAlignDesc, CArrangeDesc, CScrollDesc, CKeyDesc, CLineDesc, CFillDesc, CSelectDesc,
  CMessageDesc,
  CPictureDesc, CImageDesc,
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
  CWindowMenusDesc, CWindowControlsDesc, CWindowDesc, CWindowsDesc, CFormDesc,
  CDialogDesc,
  #ifndef NO_X_WINDOW
  CEmbedderDesc, CTrayIconDesc, CTrayIconsDesc,
  #endif
  CWatcherDesc,
  NULL
};

void *GB_QT_1[] EXPORT = {

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
  (void *)QT_MimeSourceFactory,
  (void *)QT_GetPixmap,
  (void *)QT_ToUTF8,
  (void *)QT_EventFilter,
  (void *)QT_Notify,
  (void *)QT_GetDrawInterface,
  (void *)CCONST_alignment,
  NULL
};

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

const char *GB_INCLUDE EXPORT = "gb.draw";

int EXPORT GB_INIT(void)
{
	#if QT_VERSION >= 0x030304
  qInstallMsgHandler(myMessageHandler);
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
  GB.Hook(GB_HOOK_IMAGE, (void *)hook_image);
  GB.Hook(GB_HOOK_PICTURE, (void *)hook_picture);

	GB.LoadComponent("gb.draw");
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

  QT_InitEventLoop();

  return TRUE;
}

void EXPORT GB_EXIT()
{
	#ifndef NO_X_WINDOW
		X11_exit();
	#endif
  //qApp->setStyle("windows");
  delete qApp;
}

#ifndef NO_X_WINDOW
int EXPORT GB_INFO(const char *key, void **value)
{
	if (!strcasecmp(key, "DISPLAY"))
	{
		*value = (void *)QPaintDevice::x11AppDisplay();
		return TRUE;
	}
	else if (!strcasecmp(key, "ROOT_WINDOW"))
	{
		*value = (void *)QPaintDevice::x11AppRootWindow();
		return TRUE;
	}
	else
		return FALSE;
}
#endif

#ifndef NO_X_WINDOW
extern Time	qt_x_time;
#endif

static void activate_main_window(int value)
{
	CWINDOW *active;
	
	active = CWINDOW_Active;
	if (!active) active = CWINDOW_LastActive;
	
	if (!active)
		return;
	
	MyMainWindow *win = (MyMainWindow *)active->widget.widget;
	if (win && !win->isTopLevel())
		win = (MyMainWindow *)win->topLevelWidget();
	if (win)
	{
		#ifndef NO_X_WINDOW
		qt_x_time = CurrentTime;
		#endif
		win->raise();
		win->setActiveWindow();
	}
}

void EXPORT GB_SIGNAL(int signal, void *param)
{
	switch(signal)
	{
		case GB_SIGNAL_DEBUG_BREAK:
		case GB_SIGNAL_DEBUG_FORWARD:
			//while (qApp->activePopupWidget())
			//	delete qApp->activePopupWidget();
			qApp->syncX();
			break;
			
		case GB_SIGNAL_DEBUG_CONTINUE:
			GB.Post((GB_POST_FUNC)activate_main_window, 0);
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

