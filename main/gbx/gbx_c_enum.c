/***************************************************************************

  gbx_c_enum.c

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

#define __GBX_C_ENUM_C

#include "gbx_info.h"

#ifndef GBX_INFO

#include "gb_common.h"
#include "gbx_class.h"

#include "gambas.h"
#include "gbx_api.h"

#include "gbx_exec.h"
#include "gbx_object.h"
#include "gbx_c_enum.h"

//#define DEBUG_ME

static CENUM *_enum_list = NULL;


CENUM *CENUM_create(void *enum_object)
{
  CENUM *_object;
  
  OBJECT_new((void **)(void *)&_object, CLASS_Enum, NULL, NULL);
  OBJECT_UNREF_KEEP(_object, "CENUM_create");
  
  THIS->enum_object = enum_object;
  OBJECT_REF(enum_object, "CENUM_create");

  LIST_insert(&_enum_list, THIS, &THIS->list);
  
  #ifdef DEBUG_ME
  fprintf(stderr, "CENUM_create: %p <%p>\n", THIS, enum_object);
  #endif
  
  return THIS;
}

CENUM *CENUM_get_next(CENUM *cenum)
{
  if (!cenum)
    return _enum_list;
  else
    return cenum->list.next;
}


BEGIN_METHOD_VOID(CENUM_free)

  #ifdef DEBUG_ME
  fprintf(stderr, "CENUM_free: %p <%p>\n", THIS, THIS->enum_object);
  #endif

  LIST_remove(&_enum_list, THIS, &THIS->list);
  OBJECT_UNREF(THIS->enum_object, "CENUM_free");
  
  if (THIS->variant)
    GB_StoreVariant(NULL, THIS->data);

END_METHOD


BEGIN_METHOD_VOID(CENUM_stop)

  #ifdef DEBUG_ME
  fprintf(stderr, "CENUM_stop: %p <%p>\n", EXEC_enum, EXEC_enum->enum_object);
  #endif
  
  EXEC_enum->stop = TRUE;

END_METHOD


BEGIN_PROPERTY(CENUM_index)

  _object = EXEC_enum;

  if (READ_PROPERTY)
  {
    if (!THIS->variant)
    {
      #ifdef DEBUG_ME
      fprintf(stderr, "CENUM_index: %p <%p>: -> NULL\n", THIS, THIS->enum_object);
      #endif  
      GB_ReturnNull();
    }
    else
    {
      #ifdef DEBUG_ME
      fprintf(stderr, "CENUM_index: %p <%p>: -> value (%d)\n", THIS, THIS->enum_object, ((VARIANT *)THIS->data)->type);
      #endif  
      GB_ReturnVariant((GB_VARIANT_VALUE *)THIS->data);
     }
  }
  else
  {
    #ifdef DEBUG_ME
    fprintf(stderr, "CENUM_index: %p <%p>: set: variant = %d\n", THIS, THIS->enum_object, THIS->variant);
    #endif  
    if (!THIS->variant)
      ((VARIANT *)THIS->data)->type = T_NULL;
      
    GB_StoreVariant(PROP(GB_VARIANT), THIS->data);
    THIS->variant = TRUE;
  }

END_PROPERTY


BEGIN_METHOD_VOID(CENUM_next)

  void *enum_object = OP ? (void *)OP : (void *)CP;
  CENUM **pcenum = (CENUM **)GB_GetEnum();
  CENUM *cenum;

  if (*pcenum == NULL)
    cenum = _enum_list;
  else
    cenum = *pcenum;
  
  for(;;)
  {
    if (!cenum)
    {
      GB_StopEnum();
      return;
    }
    
    if (cenum->enum_object == enum_object)
      break;
      
    cenum = cenum->list.next;
  }

  *pcenum = cenum->list.next;
  EXEC_enum = cenum;

END_METHOD


#endif

GB_DESC NATIVE_Enum[] =
{
  GB_DECLARE("Enum", sizeof(CENUM)), GB_NOT_CREATABLE(),

  GB_METHOD("_free", NULL, CENUM_free, NULL),
  
  GB_STATIC_PROPERTY("Index", "v", CENUM_index),
  GB_STATIC_METHOD("Stop", NULL, CENUM_stop, NULL),
  
  GB_STATIC_METHOD("_next", NULL, CENUM_next, NULL),

  GB_END_DECLARE
};


