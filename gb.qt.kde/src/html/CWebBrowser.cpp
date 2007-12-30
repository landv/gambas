/***************************************************************************

  CWebBrowser.cpp

  The WebBrowser class

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

#define __CWEBBROWSER_CPP

#include <khtmlview.h>
#include <kparts/partmanager.h>
#include <kparts/statusbarextension.h>
#include <qfont.h>
#include <qguardedptr.h>

#include "CWebBrowser.h"

DECLARE_EVENT(EVENT_Click);
DECLARE_EVENT(EVENT_Change);
DECLARE_EVENT(EVENT_Link);


/** class CWebBrowser *****************************************************/

CWebBrowser CWebBrowser::manager;

void CWebBrowser::storeURL(CWEBBROWSER *_object, const QString & url)
{
  GB.FreeString(&THIS->url);
  GB.NewString(&THIS->url, TO_UTF8(url), 0);
}

void CWebBrowser::click(const KURL & kurl, const KParts::URLArgs & args)
{
  void *_object = QT.GetObject(((KHTMLPart *)sender()->parent())->view());
  //QString url = kurl.url();

  storeURL(THIS, kurl.url());

  if (!GB.Raise(THIS, EVENT_Click, 0))
  {
  	THIS->part->browserExtension()->setURLArgs(args);
    THIS->part->openURL(kurl);
	}
}

void CWebBrowser::link(const QString & url)
{
  KURL kurl(url);
  KHTMLPart *part = (KHTMLPart *)sender();

  while (part->parentPart())
    part = part->parentPart();

  void *_object = QT.GetObject(part->view());

  if (KURL::isRelativeURL(url))
  {
    kurl = THIS->part->url();
    if (url.left(1) == "/")
      kurl.setPath(url);
    else
      kurl.setPath("/" + url);
  }

  storeURL(THIS, kurl.url());

  GB.Raise(THIS, EVENT_Link, 0);
}

void CWebBrowser::change()
{
  void *_object = QT.GetObject(((KHTMLPart *)sender())->view());

  GB.Raise(THIS, EVENT_Change, 0);
}


void CWebBrowser::newFrame(KParts::Part *part)
{
  QObject::connect(part,
    SIGNAL(onURL(const QString &)),
    &CWebBrowser::manager, SLOT(link(const QString &)));
}


/** WebBrowser ************************************************************/

static QGuardedPtr<KStatusBar> _status_bar = 0;

BEGIN_METHOD(CWEBBROWSER_new, GB_OBJECT parent)

	//sb = new KStatusBar(QT.GetContainer(VARG(parent)));

  //KHTMLPart *part = new KHTMLPart(QT.GetContainer(VARG(parent)));
  KHTMLPart *part = new KHTMLPart(QT.GetContainer(VARG(parent)), 0, 0, 0, KHTMLPart::BrowserViewGUI);
  KHTMLView *wid = part->view();

  QT.InitWidget(wid, _object);
  PART = part;

  QObject::connect(part->browserExtension(), SIGNAL(openURLRequest( const KURL &, const KParts::URLArgs &)),
    &CWebBrowser::manager, SLOT(click(const KURL &, const KParts::URLArgs &)));

  QObject::connect(part, SIGNAL(onURL(const QString &)),
    &CWebBrowser::manager, SLOT(link(const QString &)));

  QObject::connect(part, SIGNAL(completed()),
    &CWebBrowser::manager, SLOT(change()));

  QObject::connect(part->partManager(), SIGNAL(partAdded(KParts::Part *)),
    &CWebBrowser::manager, SLOT(newFrame(KParts::Part *)));

  wid->setMinimumSize(128, 128);

	part->setStatusMessagesEnabled(false);
  part->setJScriptEnabled(false);
  part->setJavaEnabled(false);
  part->setMetaRefreshEnabled(true);
  part->setPluginsEnabled(false);
  //part->setStandardFont(wid->font().family());
  //part->setFixedFont("monospa

  //part->openURL("http://www.kde.org");

	// Workaround a crash in KHTML deletion when there is no status bar

  KParts::StatusBarExtension *sb = KParts::StatusBarExtension::childObject(part);
  if (!_status_bar)
  	_status_bar = new KStatusBar(0);
  sb->setStatusBar(_status_bar);

  wid->show();

  //qDebug("THIS: %p", THIS);
  //qDebug("THIS->part: %p", PART);

END_METHOD


BEGIN_METHOD_VOID(CWEBBROWSER_free)

  GB.FreeString(&THIS->url);
  PART = 0;

END_METHOD


BEGIN_PROPERTY(CWEBBROWSER_path)

  if (READ_PROPERTY)
  {
    KURL kurl = PART->url();
    QString url = kurl.url();

    GB.ReturnNewZeroString(TO_UTF8(url));
  }
  else
  {
    KURL kurl(QSTRING_PROP());
    PART->openURL(kurl);
  }

END_PROPERTY


/*BEGIN_PROPERTY(CWEBBROWSER_text)

  if (READ_PROPERTY)
    GB.ReturnNewZeroString(TO_UTF8(PART->url().url()));
  else
  {
    PART->begin();
    PART->write(QSTRING_PROP());
    PART->end();
  }

END_PROPERTY*/


BEGIN_PROPERTY(CWEBBROWSER_font)

  if (READ_PROPERTY)
    QT.FontProperty(_object, _param);
  else
  {
    QT_FONT *font = (QT_FONT *)VPROP(GB_OBJECT);

    if (font == 0)
      WIDGET->unsetFont();
    else
      WIDGET->setFont(*(font->font));

		PART->setStandardFont(WIDGET->font().family());
	}

END_PROPERTY


BEGIN_PROPERTY(CWEBBROWSER_java)

  if (READ_PROPERTY)
    GB.ReturnBoolean(PART->javaEnabled());
  else
    PART->setJavaEnabled(VPROP(GB_BOOLEAN));

END_PROPERTY


BEGIN_PROPERTY(CWEBBROWSER_javascript)

  if (READ_PROPERTY)
    GB.ReturnBoolean(PART->jScriptEnabled());
  else
    PART->setJScriptEnabled(VPROP(GB_BOOLEAN));

END_PROPERTY


BEGIN_PROPERTY(CWEBBROWSER_plugins)

  if (READ_PROPERTY)
    GB.ReturnBoolean(PART->pluginsEnabled());
  else
    PART->setPluginsEnabled(VPROP(GB_BOOLEAN));

END_PROPERTY


BEGIN_PROPERTY(CWEBBROWSER_zoom)

  if (READ_PROPERTY)
    GB.ReturnInteger(PART->zoomFactor());
  else
    PART->setZoomFactor(VPROP(GB_INTEGER));

END_PROPERTY


BEGIN_PROPERTY(CWEBBROWSER_link)

  GB.ReturnString(THIS->url);

END_PROPERTY


BEGIN_METHOD_VOID(CWEBBROWSER_stop)

  PART->closeURL();

END_METHOD


BEGIN_METHOD(CWEBBROWSER_print, GB_BOOLEAN dialog)

  WIDGET->print(!VARG(dialog));

END_METHOD


static KHTMLPart *get_active_part(void *_object)
{
  KHTMLPart *part = ACTIVE_PART;
  
  if (part)
    return part;
  else
    return THIS->part;
}

BEGIN_PROPERTY(CWEBBROWSER_selected)

	GB.ReturnBoolean(get_active_part(THIS)->hasSelection());

END_PROPERTY


BEGIN_METHOD_VOID(CWEBBROWSER_sel_all)
	  
  get_active_part(THIS)->selectAll();

END_METHOD


BEGIN_PROPERTY(CWEBBROWSER_sel_text)

  GB.ReturnNewZeroString(TO_UTF8(get_active_part(THIS)->selectedText()));
  
END_PROPERTY


BEGIN_PROPERTY(CWEBBROWSER_sel_html)

  GB.ReturnNewZeroString(TO_UTF8(get_active_part(THIS)->selectedTextAsHTML()));

END_PROPERTY


BEGIN_METHOD_VOID(CWEBBROWSER_find)

  get_active_part(THIS)->findText();

END_METHOD

/**************************************************************************/

GB_DESC CWebBrowserSelectionDesc[] =
{
  GB_DECLARE(".WebBrowserSelection", 0), GB_VIRTUAL_CLASS(),
  
  GB_PROPERTY_READ("Text", "s", CWEBBROWSER_sel_text),
  GB_PROPERTY_READ("HTML", "s", CWEBBROWSER_sel_html),
  
  GB_END_DECLARE
};

GB_DESC CWebBrowserDesc[] =
{
  GB_DECLARE("WebBrowser", sizeof(CWEBBROWSER)), GB_INHERITS("Control"),

  GB_METHOD("_new", NULL, CWEBBROWSER_new, "(Parent)Container;"),
  GB_METHOD("_free", NULL, CWEBBROWSER_free, NULL),
  //GB_STATIC_METHOD("_exit", NULL, CWEBBROWSER_exit, NULL),

  GB_PROPERTY("Path", "s", CWEBBROWSER_path),
  GB_PROPERTY_READ("Link", "s", CWEBBROWSER_link),
  //GB_PROPERTY("Text", "s", CWEBBROWSER_text),

  //GB_PROPERTY("Status", "b", CWEBBROWSER_status),
  GB_PROPERTY("Java", "b", CWEBBROWSER_java),
  GB_PROPERTY("JavaScript", "b", CWEBBROWSER_javascript),
  GB_PROPERTY("Zoom", "i", CWEBBROWSER_zoom),
  GB_PROPERTY("Font", "Font", CWEBBROWSER_font),
  GB_PROPERTY("Plugins", "b", CWEBBROWSER_plugins),
  //GB_PROPERTY("LoadImages", "b", CWEBBROWSER_load_images),

  GB_METHOD("Stop", NULL, CWEBBROWSER_stop, NULL),
  GB_METHOD("Print", NULL, CWEBBROWSER_print, "[(Dialog)b]"),
  GB_METHOD("Find", NULL, CWEBBROWSER_find, NULL),

	GB_PROPERTY_SELF("Selection", ".WebBrowserSelection"),
	GB_PROPERTY_READ("Selected", "b", CWEBBROWSER_selected),
  //GB_METHOD("SelectAll", NULL, CWEBBROWSER_sel_all, NULL),

  GB_CONSTANT("_Properties", "s", CWEBBROWSER_PROPERTIES),
  GB_CONSTANT("_DefaultEvent", "s", "Click"),

  GB_EVENT("Click", NULL, NULL, &EVENT_Click),
  GB_EVENT("Change", NULL, NULL, &EVENT_Change),
  GB_EVENT("Link", NULL, NULL, &EVENT_Link),

  GB_END_DECLARE
};

