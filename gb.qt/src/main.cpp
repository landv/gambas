/***************************************************************************

  main.cpp

  The interface between the QT plug-in and the Gambas interpreter

  (c) 2000-2006 Benoît Minisini <gambas@users.sourceforge.net>

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
#include "CDrawing.h"
#include "CClipboard.h"
#include "CDraw.h"
#include "CWatch.h"
#include "CScrollView.h"
#include "CDrawingArea.h"
#include "CProgress.h"
#include "CMessage.h"
#include "CPrinter.h"
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
  extensions.replace("htm", "text/html;charset=iso8859-1");
  extensions.replace("html", "text/html;charset=iso8859-1");
  extensions.replace("txt", "text/plain");
  extensions.replace("xml", "text/xml;charset=UTF-8");
  extensions.replace("jpg", "image/jpeg");
  extensions.replace("png", "image/png");
  extensions.replace("gif", "image/gif");
}


const QMimeSource* MyMimeSourceFactory::data(const QString& abs_name) const
{
  char *addr;
  long len;
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

  ptr = &CWIDGET_destroy_list;

  for(;;)
  {
    ob = *ptr;
    if (!ob)
      break;

    if (MAIN_loop_level <= ob->level)
    {
      //qDebug(">> delete %p (%p) :%p:%ld", ob, ob->widget, ob->ob.klass, ob->ob.ref);
      //*ptr = ob->next;
      delete ob->widget;
      //GB.Unref((void **)&ob);
      //qDebug("   delete %p (%p) :%p:%ld #2", ob, ob->widget, ob->ob.klass, ob->ob.ref);
      //qDebug("<< delete %p (%p)", ob, ob->widget);
    }
    else
    {
      ptr = &ob->next;
    }
  }

  return ret;
}


/** MyApplication **********************************************************/

MyApplication::MyApplication(int &argc, char **argv)
: QApplication(argc, argv)
{
}

bool MyApplication::eventFilter(QObject *o, QEvent *e)
{
	if (o->isWidgetType() && e->type() == QEvent::KeyPress)
	{
    QKeyEvent *kevent = (QKeyEvent *)e;
    GB_VALUE *result;
    bool cancel;

		CKEY_clear(true);

		GB.FreeString(&CKEY_info.text);
		GB.NewString(&CKEY_info.text, QT_ToUTF8(kevent->text()), 0);
		CKEY_info.state = kevent->state();
		CKEY_info.code = kevent->key();

		cancel = false;
		result = GB.Call(&_application_keypress_func, 0, FALSE);
	  if (result->type != GB_T_VOID)
	  {
	  	if (!GB.Conv(result, GB_T_BOOLEAN))
	  		if (((GB_BOOLEAN *)result)->value)
	  			cancel = true;
	  }

		CKEY_clear(false);

		if (cancel)
			return true;
	}

	return QObject::eventFilter(o, e);
}

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


static void hook_wait(long duration)
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
		timer->id = (long)(new MyTimer(timer));
}

static void hook_watch(int fd, int type, void *callback, long param)
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
  CWatch::stop();
  qApp->exit();

  msg = "This application has raised an unexpected\nerror and must abort.\n\n[%1] %2.\n%3";
  msg = msg.arg(code).arg(error).arg(where);

  QMessageBox::critical(0, TO_QSTRING(GB.Application.Name()), msg);
}


static void convert_image_data(void *dst, void *src, int w, int h, int format)
{
	int i;
	char *s;
	char *d;

	switch (format)
	{
		case GB_IMAGE_BGRA: case GB_IMAGE_BGRX:
			memcpy(dst, src, w * h * 4);
			break;

		case GB_IMAGE_RGBA: case GB_IMAGE_RGBX:
			s = (char *)src;
			d = (char *)dst;
			for (i = 0; i < (w * h); i++)
			{
				d[0] = s[2];
				d[1] = s[1];
				d[2] = s[0];
				d[3] = s[3];
				s += 4;
				d += 4;
			}
			break;

		case GB_IMAGE_ARGB: case GB_IMAGE_XRGB:
			s = (char *)src;
			d = (char *)dst;
			for (i = 0; i < (w * h); i++)
			{
				d[0] = s[3];
				d[1] = s[2];
				d[2] = s[1];
				d[3] = s[0];
				s += 4;
				d += 4;
			}
			break;

		case GB_IMAGE_BGR:
			s = (char *)src;
			d = (char *)dst;
			for (i = 0; i < (w * h); i++)
			{
				d[0] = s[0];
				d[1] = s[1];
				d[2] = s[2];
				d[3] = 0xFF;
				s += 3;
				d += 4;
			}
			break;

		case GB_IMAGE_RGB:
			s = (char *)src;
			d = (char *)dst;
			for (i = 0; i < (w * h); i++)
			{
				d[0] = s[2];
				d[1] = s[1];
				d[2] = s[0];
				d[3] = 0xFF;
				s += 3;
				d += 4;
			}
			break;
	}
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
  		convert_image_data(img->bits(), info->data, info->width, info->height, info->format);

		GB.New((void **)&image, GB.FindClass("Image"), NULL, NULL);
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
		if (info->format == GB_IMAGE_BGRA)
			img = new QImage((uchar *)info->data, info->width, info->height, 32, NULL, 0, QImage::LittleEndian);
		else
		{
			img = new QImage(info->width, info->height, 32);
			convert_image_data(img->bits(), info->data, info->width, info->height, info->format);
		}

		img->setAlphaBuffer(info->format == GB_IMAGE_BGRA || info->format == GB_IMAGE_ARGB);

		GB.New((void **)&picture, GB.FindClass("Picture"), NULL, NULL);
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

static QPixmap *QT_GetPixmap(CPICTURE *pict)
{
  return pict->pixmap;
}

static QMimeSourceFactory *QT_MimeSourceFactory(void)
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

/*void QT_RegisterAction(void *object, const char *key, int on)
{
	CACTION_register((CWIDGET *)object, key, on);
}

void QT_RaiseAction(const char *key)
{
	CACTION_raise(key);
}*/

extern "C" {

GB_DESC *GB_CLASSES[] EXPORT =
{
  CBorderDesc, CColorDesc, CColorInfoDesc,
  CAlignDesc, CArrangeDesc, CScrollDesc, CKeyDesc, CLineDesc, CFillDesc, CSelectDesc,
  CMessageDesc,
  CPictureDesc, CImageDesc, CDrawingDesc,
  CFontDesc, CFontsDesc,
  CMouseDesc, CCursorDesc,
  CClipboardDesc, CDragDesc,
  CDrawClipDesc, CDrawDesc, // apr� CFont !
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
  CPrinterDesc,
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
  (void *)CFONT_create,
  (void *)QT_MimeSourceFactory,
  (void *)QT_GetPixmap,
  (void *)QT_ToUTF8,
  //(void *)QT_RegisterAction,
  //(void *)QT_RaiseAction,
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

}


/* class MyPostCheck */

bool MyPostCheck::in_check = false;

void MyPostCheck::check(void)
{
  //qDebug("MyPostCheck::check");
  in_check = false;
  GB.CheckPost();
}

