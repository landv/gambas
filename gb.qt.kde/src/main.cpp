/***************************************************************************

  main.cpp

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

#define __MAIN_C

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>

#define QT_THREAD_SUPPORT

#include <qeventloop.h>
#include <qevent.h>

#include <kapplication.h>
#include <kcmdlineargs.h>
#include <kaboutdata.h>
#include <klocale.h>
#include <kcrash.h>
#include <kfiledialog.h>

#include <X11/Xlib.h>
#undef KeyPress

#include "gambas.h"
#include "../gb.qt.h"
#include "main.h"

#include "CDialog.h"
#include "CApplication.h"
#include "CURLLabel.h"
//#include "CArrowButton.h"
#include "CColorBox.h"


extern "C" {

GB_INTERFACE GB EXPORT;
QT_INTERFACE QT;

GB_DESC *GB_CLASSES[] EXPORT =
{
  CDialogDesc,
  CKDEDCOPRefDesc,
  CKDEObjectDesc,
  CKDEApplicationDesc,
  CApplicationDesc,
  CURLLabelDesc,
  CColorBoxDesc,
  NULL
};

}

static const KCmdLineOptions _options[] =
{
	{ "!+arguments", "...", 0 },
	KCmdLineLastOption
};

static char **_args = 0;
static int _nargs = 0;

static void *old_hook_lang;

//static KApplication *app;


/** MyApplication **********************************************************/

#if 0
/* Corrige le bug de KPopupFrame en attendant KDE 3.1.5 */
bool MyApplication::eventFilter(QObject *o, QEvent *e)
{
  static int level = 0;

  if (o->isWidgetType()
      && (((QWidget *)o)->isPopup()))
  {
    if ((e->type() == QEvent::Show)
        && (o->isA("KPopupFrame")))
    {
      level = qApp->eventLoop()->loopLevel();
    }
    if ((e->type() == QEvent::Hide)
        && (o->isA("KPopupFrame")))
    {
      if (qApp->eventLoop()->loopLevel() > level)
        qApp->eventLoop()->exitLoop();
    }
  }
  return false;
}
#endif

bool MyApplication::eventFilter(QObject *o, QEvent *e)
{
	if (o->isWidgetType() && e->type() == QEvent::KeyPress)
		return QT.EventFilter(e);

	return QObject::eventFilter(o, e);
}

bool MyApplication::notify(QObject *o, QEvent *e)
{
	if (o->isWidgetType())
	{
		void *ob = QT.GetObject((QWidget *)o);		
		bool old, res;
	
		if (ob)
		{
			old = QT.Notify(ob, true);
			res = QApplication::notify(o, e);
			QT.Notify(ob, old);
			return res;
		}
	}
	
	return QApplication::notify(o, e);
}

/** Hooks ******************************************************************/

static QString _init_lang;
static bool _init_rtl;

static void init_lang(QString language, bool rtl)
{
  QString country;
  int pos;

  pos = language.find('_');
  if (pos >= 0)
  {
    country = language.mid(pos + 1);
    language = language.left(pos);
  }
  else
  {
    country = language;
  }

  if (old_hook_lang)
  	(*((void (*)(char *, int))(old_hook_lang)))((char *)language.latin1(), rtl);

  //qDebug("language = %s country = %s\n", lang.latin1(), country.latin1());

  KGlobal::locale()->setCountry(country.lower()); //QString(GB.System.Language()));
  KGlobal::locale()->setLanguage(language.lower()); //QString(GB.System.Language()));
}



static void hook_lang(char *lang, int rtl)
{
	if (!KApplication::kApplication())
	{
		_init_lang = QString(lang);
		_init_rtl = rtl;
		return;
	}

	init_lang(_init_lang, _init_rtl);
}

static void hook_main(int *argc, char **argv)
{
	QCString arg;

  //QT.InitEventLoop();

  KCmdLineArgs::init(*argc, argv, GB.Application.Name(), GB.Application.Title(), GB.Application.Title(), GB.Application.Version());
  KCmdLineArgs::addCmdLineOptions(_options);

  new MyApplication();
  
  #if QT_VERSION <= 0x030005
  qApp->unlock();
  #endif

  KCmdLineArgs *args = KCmdLineArgs::parsedArgs();

	if (args->count())
	{
		_args = new char *[args->count()];

		for(int i = 0; i < args->count(); i++)
		{
			arg = QCString(args->arg(i));
			_args[i] = new char[arg.length() + 1];
			qstrcpy(_args[i], (const char *)arg);
			argv[i] = _args[i];
		}
	}

	*argc = _nargs = args->count();

  //XSetErrorHandler(old_err_handler);

  #if QT_VERSION <= 0x030200
  qApp->installEventFilter(qApp);
  #endif

  KCrash::setCrashHandler(NULL);
  //XSetIOErrorHandler( NULL );
  //KFileDialog::getSaveFileName("", "", 0, "test");

  QT.Init();
  init_lang(_init_lang, _init_rtl);
}

extern "C" {

int EXPORT GB_INIT(void)
{
  GB.GetInterface("gb.qt", QT_INTERFACE_VERSION, &QT);

  GB.Hook(GB_HOOK_MAIN, (void *)hook_main);
  old_hook_lang = GB.Hook(GB_HOOK_LANG, (void *)hook_lang);

  return TRUE;
}

void EXPORT GB_EXIT()
{
	if (_nargs)
	{
		for (int i = 0; i < _nargs; i++)
			delete [] _args[i];
		delete[] _args;
	}
}

}


