/***************************************************************************

  gbx_c_observer.c

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

#define __GBX_C_OBSERVER_C

#include "gbx_info.h"

#ifndef GBX_INFO

#include "gbx_exec.h"
#include "gbx_api.h"
#include "gbx_event.h"
#include "gbx_c_observer.h"

//#define DEBUG_ME 1

void COBSERVER_attach(COBSERVER *this, void *parent, const char *name)
{
	#if DEBUG_ME
	fprintf(stderr, "COBSERVER_attach: %p: %s %p\n", this, parent ? OBJECT_class(parent)->name : "", parent);
	#endif
	if (this->event)
		EVENT_search(OBJECT_class(this->object), this->event, name, parent);	
}

void COBSERVER_detach(COBSERVER *this)
{
	#if DEBUG_ME
	fprintf(stderr, "COBSERVER_detach: %p\n", this);
	#endif
	
	if (this->event)
		FREE(&this->event);
}

BEGIN_METHOD(Observer_new, GB_OBJECT object; GB_BOOLEAN after)

	OBJECT *object;
	//void *proxy = NULL;
	OBJECT_EVENT *ev;
	char *name;
	CLASS *class;
	void *parent;
	
	object = (OBJECT *)VARG(object);
	if (GB_CheckObject(object))
		return;
	
	#if 0
	if (OBJECT_class(THIS) == CLASS_Proxy && OBJECT_has_events(OP))
		proxy = OP;
	#endif
	
	/*if (proxy)
	{
		//fprintf(stderr, "proxy = (%s %p) %s %s\n", GB_GetClassName(proxy), proxy, EVENT_Name, EVENT_PreviousName);
		parent = OBJECT_parent(proxy);
		name = EVENT_PreviousName;
	}
	else*/
	{
		parent = OBJECT_parent(THIS);
		name = EVENT_Name;
	}
	
	if (!parent)
		return;
	
	class = OBJECT_class(object);
	if (class->n_event == 0)
		return;
	
	if (!name || !*name)
		return;
	
	#if DEBUG_ME
	fprintf(stderr, "Observer_new: %p %d %s (%s %p)\n", THIS, OBJECT_class(object)->n_event, name, GB_GetClassName(parent), parent);
	#endif
	
	ev = OBJECT_event(object);
  
  THIS->after = VARGOPT(after, FALSE);
  
	ALLOC_ZERO(&THIS->event, sizeof(ushort) * class->n_event);

	THIS->object = object;
	OBJECT_attach((OBJECT *)THIS, parent, name);
	COBSERVER_attach(THIS, parent, name);

	LIST_insert((void **)&ev->observer, THIS, &THIS->list);
  OBJECT_REF(THIS);
	
	#if 0
	if (proxy)
	{
		// The proxy object is not referenced. It means that destroying the proxy implies
		// that the observer won't raise events anymore. IS IT SURE?
		THIS->proxy = proxy;
	}
	#endif

END_METHOD


BEGIN_METHOD_VOID(Observer_free)

	#if DEBUG_ME
	fprintf(stderr, "Observer_free: %p\n", THIS);
	#endif
	
	GB_StoreVariant(NULL, &THIS->tag);
	COBSERVER_detach(THIS);

END_METHOD

BEGIN_PROPERTY(Observer_Object)

	GB_ReturnObject(THIS->object);
	
END_PROPERTY

BEGIN_PROPERTY(Observer_Tag)

	if (READ_PROPERTY)
		GB_ReturnVariant(&THIS->tag);
	else
		GB_StoreVariant(PROP(GB_VARIANT), &THIS->tag);
	
END_PROPERTY

#endif

GB_DESC NATIVE_Observer[] =
{
  GB_DECLARE("Observer", sizeof(COBSERVER)), 

  GB_METHOD("_new", NULL, Observer_new, "(Object)o[(After)b]"),
  GB_METHOD("_free", NULL, Observer_free, NULL),
  
  GB_PROPERTY_READ("Object", "o", Observer_Object),
  GB_PROPERTY("Tag", "v", Observer_Tag),
  
  GB_END_DECLARE
};

#if 0
GB_DESC NATIVE_Proxy[] =
{
  GB_DECLARE("Proxy", sizeof(COBSERVER)), 

  GB_METHOD("_new", NULL, Observer_new, "(Object)o[(After)b]"),
  GB_METHOD("_free", NULL, Observer_free, NULL),
  
  GB_END_DECLARE
};
#endif
