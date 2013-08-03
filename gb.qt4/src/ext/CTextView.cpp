/***************************************************************************

  CTextView.cpp

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

#define __CTEXTVIEW_CPP

#include "main.h"

#include <qapplication.h>
#include <q3textbrowser.h>

#include "CTextView.h"

DECLARE_EVENT(EVENT_Link);


BEGIN_METHOD(CTEXTVIEW_new, GB_OBJECT parent)

  Q3TextBrowser *wid = new Q3TextBrowser(QT.GetContainer(VARG(parent)));

  QT.InitWidget(wid, _object);
  //QT.SetBackgroundRole(_object, QColorGroup::Base);

  QObject::connect(wid, SIGNAL(linkClicked(const QString &)), &CTextView::manager, SLOT(event_link(const QString &)));
  //QObject::connect(wid, SIGNAL(sourceChanged(const QString &)), &CTextView::manager, SLOT(event_link(const QString &)));

  wid->setTextFormat(Qt::RichText);
  wid->setMimeSourceFactory(QT.MimeSourceFactory());
  wid->show();

END_METHOD

BEGIN_METHOD_VOID(CTEXTVIEW_free)

  //GB.StoreString(NULL, &THIS->root);

END_METHOD


BEGIN_PROPERTY(CTEXTVIEW_border)

  QT.BorderProperty(_object, _param);

END_PROPERTY


BEGIN_PROPERTY(CTEXTVIEW_scrollbar)

  QT.ScrollBarProperty(_object, _param);

END_PROPERTY


BEGIN_PROPERTY(CTEXTVIEW_text)

  if (READ_PROPERTY)
    GB.ReturnNewZeroString(TO_UTF8(WIDGET->text()));
  else
  {
    WIDGET->setText(QSTRING_PROP());
    WIDGET->sync();
    THIS->change = true;
  }

END_PROPERTY


BEGIN_PROPERTY(CTEXTVIEW_path)

  if (READ_PROPERTY)
    GB.ReturnNewZeroString(TO_UTF8(WIDGET->source()));
  else
  {
    WIDGET->setSource(QSTRING_PROP());
    WIDGET->sync();
    THIS->change = true;
  }

END_PROPERTY


BEGIN_PROPERTY(CTEXTVIEW_root)

  GB.ReturnNewZeroString(TO_UTF8(WIDGET->context()));

END_PROPERTY


BEGIN_METHOD_VOID(CTEXTVIEW_clear)

  WIDGET->clear();

END_METHOD


BEGIN_PROPERTY(CTEXTVIEW_scroll_x)

  if (READ_PROPERTY)
    GB.ReturnInteger(WIDGET->contentsX());
  else
    WIDGET->setContentsPos(VPROP(GB_INTEGER), WIDGET->contentsY());

END_PROPERTY


BEGIN_PROPERTY(CTEXTVIEW_scroll_y)

  if (READ_PROPERTY)
    GB.ReturnInteger(WIDGET->contentsY());
  else
    WIDGET->setContentsPos(WIDGET->contentsX(), VPROP(GB_INTEGER));

END_PROPERTY


BEGIN_PROPERTY(CTEXTVIEW_text_width)

  if (WIDGET->paragraphs() <= 0)
    GB.ReturnInteger(0);
  else
  {
    WIDGET->sync();
    GB.ReturnInteger(WIDGET->contentsWidth());
    //QRect r = WIDGET->paragraphRect(WIDGET->paragraphs() - 1);
    //GB.ReturnInteger(r.y() + r.height());
  }

END_PROPERTY


BEGIN_PROPERTY(CTEXTVIEW_text_height)

  if (WIDGET->paragraphs() <= 0)
    GB.ReturnInteger(0);
  else
  {
    WIDGET->sync();
    GB.ReturnInteger(WIDGET->contentsHeight());
    //QRect r = WIDGET->paragraphRect(WIDGET->paragraphs() - 1);
    //GB.ReturnInteger(r.y() + r.height());
  }

END_PROPERTY

GB_DESC CTextViewDesc[] =
{
  GB_DECLARE("TextView", sizeof(CTEXTVIEW)), GB_INHERITS("Control"),

  //GB_CONSTANT("UpDownArrows", "i", QSpinBox::UpDownArrows),
  //GB_CONSTANT("PlusMinus","i", QSpinBox::PlusMinus),

  GB_METHOD("_new", NULL, CTEXTVIEW_new, "(Parent)Container;"),
  GB_METHOD("_free", NULL, CTEXTVIEW_free, NULL),

  GB_METHOD("Clear", NULL, CTEXTVIEW_clear, NULL),

  GB_PROPERTY("Text", "s", CTEXTVIEW_text),
  GB_PROPERTY("Path", "s", CTEXTVIEW_path),
  GB_PROPERTY_READ("Root", "s", CTEXTVIEW_root),
  GB_PROPERTY("Border", "b", CTEXTVIEW_border),
  GB_PROPERTY("ScrollBar", "i", CTEXTVIEW_scrollbar),

  GB_PROPERTY("ScrollX", "i", CTEXTVIEW_scroll_x),
  GB_PROPERTY("ScrollY", "i", CTEXTVIEW_scroll_y),
  
  GB_PROPERTY("TextWidth", "i", CTEXTVIEW_text_width),
  GB_PROPERTY("TextHeight", "i", CTEXTVIEW_text_height),

  GB_EVENT("Link", NULL, "(Path)s", &EVENT_Link),

  GB_CONSTANT("_Properties", "s", "*,Text,ScrollBar{Scroll.*}=Both,Border=True"),
  GB_CONSTANT("_DefaultEvent", "s", "Link"),

  GB_END_DECLARE
};


/***************************************************************************

  class CTextView

***************************************************************************/

CTextView CTextView::manager;

void CTextView::event_link(const QString &link)
{
  void *_object = QT.GetObject((QWidget *)sender());

  THIS->change = false;

  GB.Raise(_object, EVENT_Link, 1,
    GB_T_STRING, TO_UTF8(link), 0);

  if (!THIS->change)
  {
    // This cancels the click on the link (see qt source code)
    WIDGET->setSource(WIDGET->source());
  }
}
