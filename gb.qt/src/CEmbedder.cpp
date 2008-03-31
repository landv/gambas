/***************************************************************************

  CEmbedder.cpp

  The Embedder control

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

#define __CEMBEDDER_CPP

#include "CEmbedder.h"

DECLARE_EVENT(EVENT_Embed);
DECLARE_EVENT(EVENT_Close);
DECLARE_EVENT(EVENT_Error);


BEGIN_METHOD(CEMBEDDER_new, GB_OBJECT parent)

  QtXEmbedContainer *wid = new QtXEmbedContainer(QCONTAINER(VARG(parent)));

  QObject::connect(wid, SIGNAL(clientIsEmbedded()), &CEmbedder::manager, SLOT(embedded()));
  QObject::connect(wid, SIGNAL(clientClosed()), &CEmbedder::manager, SLOT(closed()));
  QObject::connect(wid, SIGNAL(error(int)), &CEmbedder::manager, SLOT(error()));
  
  CWIDGET_new(wid, (void *)_object);
  
END_METHOD


BEGIN_PROPERTY(CEMBEDDER_client)

  GB.ReturnInteger((int)WIDGET->clientWinId());

END_PROPERTY


BEGIN_METHOD(CEMBEDDER_embed, GB_INTEGER client; GB_BOOLEAN prepared)

  WIDGET->embed(VARG(client), VARGOPT(prepared, false));

END_METHOD


BEGIN_METHOD_VOID(CEMBEDDER_discard)

  WIDGET->discardClient();

END_METHOD


GB_DESC CEmbedderDesc[] =
{
  GB_DECLARE("Embedder", sizeof(CEMBEDDER)), 
  GB_INHERITS("Control"),

  GB_METHOD("_new", NULL, CEMBEDDER_new, "(Parent)Container;"),

  GB_PROPERTY_READ("Client", "i", CEMBEDDER_client),
  GB_METHOD("Embed", NULL, CEMBEDDER_embed, "(Client)i[(Prepared)b]"),
  GB_METHOD("Discard", NULL, CEMBEDDER_discard, NULL),

	EMBEDDER_DESCRIPTION,

  GB_EVENT("Embed", NULL, NULL, &EVENT_Embed),
  GB_EVENT("Close", NULL, NULL, &EVENT_Close),
  GB_EVENT("Error", NULL, NULL, &EVENT_Error),
  
  GB_END_DECLARE
};


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
