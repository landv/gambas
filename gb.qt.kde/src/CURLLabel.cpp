/***************************************************************************

  CURLLabel.cpp

  The KDE URL Label

  (c) 2000-2005 Benoît Minisini <gambas@users.sourceforge.net>

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

#define __CURLLABEL_CPP


#include <kurllabel.h>

#include "gambas.h"
#include "main.h"
#include "CURLLabel.h"

DECLARE_EVENT(EVENT_Click);

/***************************************************************************

  class CUrlLabel

***************************************************************************/

CUrlLabel CUrlLabel::manager;

void CUrlLabel::clicked()
{
  void *object = QT.GetObject((QWidget *)sender());
  GB.Raise(object, EVENT_Click, 0);
}


/***************************************************************************

  URLLabel

***************************************************************************/

BEGIN_METHOD(CURLLABEL_new, GB_OBJECT parent)

  KURLLabel *wid = new KURLLabel(QT.GetContainer(VARG(parent)));

  QT.InitWidget(wid, _object);

  QObject::connect(wid, SIGNAL(leftClickedURL(const QString &)), &CUrlLabel::manager, SLOT(clicked()));

  wid->show();

END_METHOD


BEGIN_PROPERTY(CURLLABEL_text)

  if (READ_PROPERTY)
    GB.ReturnNewZeroString(TO_UTF8(WIDGET->text()));
  else
    WIDGET->setText(QSTRING_PROP());

END_PROPERTY


BEGIN_PROPERTY(CURLLABEL_url)

  if (READ_PROPERTY)
    GB.ReturnNewZeroString(TO_UTF8(WIDGET->url()));
  else
    WIDGET->setURL(QSTRING_PROP());

END_PROPERTY


GB_DESC CURLLabelDesc[] =
{
  GB_DECLARE("URLLabel", sizeof(CURLLABEL)), GB_INHERITS("Control"),

  GB_METHOD("_new", NULL, CURLLABEL_new, "(Parent)Container;"),

  GB_PROPERTY("Text", "s", CURLLABEL_text),
  GB_PROPERTY("URL", "s", CURLLABEL_url),

  GB_EVENT("Click", NULL, NULL, &EVENT_Click),
  GB_CONSTANT("_DefaultEvent", "s", "Click"),

  GB_CONSTANT("_Properties", "s", CURLLABEL_PROPERTIES),

  GB_END_DECLARE
};

