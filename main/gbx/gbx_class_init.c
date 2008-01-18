/***************************************************************************

  class_init.c

  Native class initializer

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

#define __GBX_CLASS_INIT_C

#include "gb_common.h"
#include "gb_alloc.h"
#include "gb_error.h"
#include "gb_limit.h"

#include "gbx_component.h"

#include "gbx_c_gambas.h"
#include "gbx_c_class.h"
#include "gbx_c_error.h"
#include "gbx_c_collection.h"
#include "gbx_c_file.h"
#include "gbx_c_application.h"
#include "gbx_c_array.h"
#include "gbx_c_process.h"
#include "gbx_c_subcollection.h"
#include "gbx_c_string.h"
#include "gbx_c_enum.h"
#include "gbx_c_timer.h"
#include "gbx_c_quote.h"

#include "gbx_class.h"

CLASS *CLASS_Class = NULL;
CLASS *CLASS_Collection = NULL;
CLASS *CLASS_Symbol = NULL;
CLASS *CLASS_File = NULL;
CLASS *CLASS_Stat = NULL;
CLASS *CLASS_Stream = NULL;
CLASS *CLASS_Application = NULL;
CLASS *CLASS_AppArgs = NULL;
CLASS *CLASS_AppEnv = NULL;
CLASS *CLASS_Process = NULL;
CLASS *CLASS_Component = NULL;

CLASS *CLASS_Array = NULL;
CLASS *CLASS_BooleanArray = NULL;
CLASS *CLASS_ByteArray = NULL;
CLASS *CLASS_ShortArray = NULL;
CLASS *CLASS_IntegerArray = NULL;
CLASS *CLASS_SingleArray = NULL;
CLASS *CLASS_FloatArray = NULL;
CLASS *CLASS_DateArray = NULL;
CLASS *CLASS_StringArray = NULL;
CLASS *CLASS_ObjectArray = NULL;
CLASS *CLASS_VariantArray = NULL;
CLASS *CLASS_LongArray = NULL;

CLASS *CLASS_SubCollection = NULL;
CLASS *CLASS_String = NULL;
CLASS *CLASS_Enum = NULL;

typedef
  struct {
    GB_DESC *desc;
    CLASS **class;
    }
  CLASS_INIT;

static CLASS_INIT init_list[] =
{
  { NATIVE_Gambas, NULL },
  { NATIVE_Param, NULL },
  { NATIVE_Enum, &CLASS_Enum },
  { NATIVE_Symbol, NULL },
  { NATIVE_ClassSymbols, NULL },
  { NATIVE_Class, NULL },
  { NATIVE_Classes, NULL },
  { NATIVE_Component, NULL },
  { NATIVE_Components, NULL },
  { NATIVE_Object, NULL },
  { NATIVE_Collection, &CLASS_Collection },
//  { NATIVE_List, NULL },
  { NATIVE_Error, NULL },
  { NATIVE_Stream, &CLASS_Stream },
  { NATIVE_FilePerm, NULL },
  { NATIVE_Stat, &CLASS_Stat },
  { NATIVE_File, &CLASS_File },
  { NATIVE_AppEnv, &CLASS_AppEnv },
  { NATIVE_AppArgs, &CLASS_AppArgs },
  { NATIVE_App, &CLASS_Application },
  { NATIVE_Process, &CLASS_Process },
  { NATIVE_System, NULL },
  { NATIVE_User, NULL },
  { NATIVE_String, NULL },
  { NATIVE_Timer, NULL },
  { NATIVE_Quote, NULL },
  { NATIVE_Unquote, NULL },
  { NATIVE_Observer, NULL },

  { NATIVE_ArrayBounds, NULL },
  { NATIVE_Array, &CLASS_Array },
  { NATIVE_BooleanArray, &CLASS_BooleanArray },
  { NATIVE_ByteArray, &CLASS_ByteArray },
  { NATIVE_ShortArray, &CLASS_ShortArray },
  { NATIVE_IntegerArray, &CLASS_IntegerArray },
  { NATIVE_FloatArray, &CLASS_FloatArray },
  { NATIVE_SingleArray, &CLASS_SingleArray },
  { NATIVE_DateArray, &CLASS_DateArray },
  { NATIVE_StringArray, &CLASS_StringArray },
  { NATIVE_ObjectArray, &CLASS_ObjectArray },
  { NATIVE_VariantArray, &CLASS_VariantArray },
  { NATIVE_LongArray, &CLASS_LongArray },

  { NATIVE_SubCollection, &CLASS_SubCollection },

  { NULL }
};


void CLASS_init_native(void)
{
  CLASS_INIT *init;
  CLASS *class;

  /* NOTE: The 'Class' class must be first in the global class table */
  CLASS_Class = CLASS_find("Class");
  CLASS_Symbol = CLASS_find("Symbol");
  CLASS_Component = CLASS_find("Component");

  //LIBRARY_Current = LIBRARY_create(NULL);

  for (init = init_list; init->desc; init++)
  {
    class = CLASS_register(init->desc);
    if (init->class != NULL)
      *init->class = class;
  }
}


