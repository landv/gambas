/***************************************************************************

  CEmbedder.cpp

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

#define __CEMBEDDER_CPP

#include "CEmbedder.h"

DECLARE_EVENT(EVENT_Embed);
DECLARE_EVENT(EVENT_Close);
DECLARE_EVENT(EVENT_Error);

#ifdef NO_X_WINDOW

BEGIN_METHOD(CEMBEDDER_new, GB_OBJECT parent)

  QWidget *wid = new QWidget(QCONTAINER(VARG(parent)));

  //QObject::connect(wid, SIGNAL(clientIsEmbedded()), &CEmbedder::manager, SLOT(embedded()));
  //QObject::connect(wid, SIGNAL(clientClosed()), &CEmbedder::manager, SLOT(closed()));
  //QObject::connect(wid, SIGNAL(error(QX11EmbedContainer::Error)), &CEmbedder::manager, SLOT(error()));
  
  CWIDGET_new(wid, (void *)_object);
  
END_METHOD


BEGIN_PROPERTY(CEMBEDDER_client)

  GB.ReturnInteger(0);

END_PROPERTY


BEGIN_METHOD(CEMBEDDER_embed, GB_INTEGER client)

  //WIDGET->embedClient(VARG(client));

END_METHOD


BEGIN_METHOD_VOID(CEMBEDDER_discard)

  //WIDGET->discardClient();

END_METHOD

#else

BEGIN_METHOD(CEMBEDDER_new, GB_OBJECT parent)

  QX11EmbedContainer *wid = new QX11EmbedContainer(QCONTAINER(VARG(parent)));

  QObject::connect(wid, SIGNAL(clientIsEmbedded()), &CEmbedder::manager, SLOT(embedded()));
  QObject::connect(wid, SIGNAL(clientClosed()), &CEmbedder::manager, SLOT(closed()));
  QObject::connect(wid, SIGNAL(error(QX11EmbedContainer::Error)), &CEmbedder::manager, SLOT(error()));
  
  CWIDGET_new(wid, (void *)_object);
  
END_METHOD


BEGIN_PROPERTY(CEMBEDDER_client)

  GB.ReturnInteger((int)WIDGET->clientWinId());

END_PROPERTY


BEGIN_METHOD(CEMBEDDER_embed, GB_INTEGER client)

  WIDGET->embedClient(VARG(client));

END_METHOD


BEGIN_METHOD_VOID(CEMBEDDER_discard)

  WIDGET->discardClient();

END_METHOD

/*--- CEmbedder -----------------------------------------------------------------------------------------*/

CEmbedder CEmbedder::manager;

void CEmbedder::embedded()
{
  RAISE_EVENT(EVENT_Embed);
}

void CEmbedder::closed()
{
  RAISE_EVENT(EVENT_Close);
}

void CEmbedder::error()
{
  RAISE_EVENT(EVENT_Error);
}

#endif

GB_DESC CEmbedderDesc[] =
{
  GB_DECLARE("Embedder", sizeof(CEMBEDDER)), 
  GB_INHERITS("Control"),

  GB_METHOD("_new", NULL, CEMBEDDER_new, "(Parent)Container;"),

  GB_PROPERTY_READ("Client", "i", CEMBEDDER_client),
  GB_METHOD("Embed", NULL, CEMBEDDER_embed, "(Client)i"),
  GB_METHOD("Discard", NULL, CEMBEDDER_discard, NULL),

	EMBEDDER_DESCRIPTION,

  GB_EVENT("Embed", NULL, NULL, &EVENT_Embed),
  GB_EVENT("Close", NULL, NULL, &EVENT_Close),
  GB_EVENT("Error", NULL, NULL, &EVENT_Error),
  
  GB_END_DECLARE
};


