/***************************************************************************

  gbx_class_init.c

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

#define __GBX_CLASS_INIT_C

#include "gb_common.h"
#include "gb_alloc.h"
#include "gb_error.h"
#include "gb_limit.h"

#include "gbx_component.h"

#include "gbx_c_gambas.h"
#include "gbx_c_observer.h"
#include "gbx_c_class.h"
#include "gbx_c_error.h"
#include "gbx_c_collection.h"
#include "gbx_c_file.h"
#include "gbx_c_system.h"
#include "gbx_c_application.h"
#include "gbx_c_array.h"
#include "gbx_c_process.h"
#include "gbx_c_string.h"
#include "gbx_c_enum.h"
#include "gbx_c_timer.h"
#include "gbx_c_task.h"

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
CLASS *CLASS_Observer = NULL;
CLASS *CLASS_Timer = NULL;

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
CLASS *CLASS_PointerArray = NULL;

CLASS *CLASS_String = NULL;
CLASS *CLASS_Enum = NULL;

typedef
  struct {
    GB_DESC *desc;
    CLASS **class;
    int array;
		TYPE array_type;
    }
  CLASS_INIT;

static const CLASS_INIT init_list[] =
{
  { NATIVE_Gambas, NULL },
  { NATIVE_Param, NULL },
  { NATIVE_Enum, &CLASS_Enum },
  { NATIVE_Symbol, NULL },
  { NATIVE_Class, NULL },
  { NATIVE_Classes, NULL },
  { NATIVE_Component, NULL },
  { NATIVE_Components, NULL },
  { NATIVE_Object, NULL },
  { NATIVE_Collection, &CLASS_Collection, CQA_COLLECTION },
  { NATIVE_Error, NULL },
  { NATIVE_StreamLines, NULL },
  { NATIVE_Stream, &CLASS_Stream },
  { NATIVE_StatPerm, NULL },
  { NATIVE_Stat, &CLASS_Stat },
  { NATIVE_File, &CLASS_File },
  { NATIVE_AppEnv, &CLASS_AppEnv },
  { NATIVE_AppArgs, &CLASS_AppArgs },
  { NATIVE_App, &CLASS_Application },
  { NATIVE_Process, &CLASS_Process },
  { NATIVE_System, NULL },
  { NATIVE_User, NULL },
  { NATIVE_String, NULL },
  { TaskDesc, NULL },
  { NATIVE_Timer, &CLASS_Timer },
  { NATIVE_Observer, &CLASS_Observer },
  //{ NATIVE_Proxy, &CLASS_Proxy },

  { NATIVE_ArrayBounds, NULL },
  { NATIVE_Array, &CLASS_Array },
  { NATIVE_BooleanArray, &CLASS_BooleanArray, CQA_ARRAY, T_BOOLEAN },
  { NATIVE_ByteArray, &CLASS_ByteArray, CQA_ARRAY, T_BYTE },
  { NATIVE_ShortArray, &CLASS_ShortArray, CQA_ARRAY, T_SHORT },
  { NATIVE_IntegerArray, &CLASS_IntegerArray, CQA_ARRAY, T_INTEGER },
  { NATIVE_FloatArray, &CLASS_FloatArray, CQA_ARRAY, T_FLOAT },
  { NATIVE_SingleArray, &CLASS_SingleArray, CQA_ARRAY, T_SINGLE },
  { NATIVE_DateArray, &CLASS_DateArray, CQA_ARRAY, T_DATE },
  { NATIVE_StringArray, &CLASS_StringArray, CQA_ARRAY, T_STRING },
  { NATIVE_ObjectArray, &CLASS_ObjectArray, CQA_ARRAY, T_OBJECT },
  { NATIVE_VariantArray, &CLASS_VariantArray, CQA_ARRAY, T_VARIANT },
  { NATIVE_LongArray, &CLASS_LongArray, CQA_ARRAY, T_LONG },
  { NATIVE_PointerArray, &CLASS_PointerArray, CQA_ARRAY, T_POINTER },

  { NULL }
};


void CLASS_init_native(void)
{
  const CLASS_INIT *init;
  CLASS *class;

  /* NOTE: The 'Class' class must be first in the global class table */
  CLASS_Class = CLASS_find("Class");
  CLASS_Symbol = CLASS_find("Symbol");
  CLASS_Component = CLASS_find("Component");
  CLASS_Stream = CLASS_find("Stream");

  //LIBRARY_Current = LIBRARY_create(NULL);

  for (init = init_list; init->desc; init++)
  {
    class = CLASS_register(init->desc);
    if (init->class != NULL)
      *init->class = class;
		if (init->array)
		{
			class->quick_array = init->array;
			class->array_type = init->array_type;
			class->is_array = init->array == CQA_ARRAY;
		}
  }
	
	CLASS_Observer->is_observer = TRUE;
	CLASS_Observer->size += sizeof(OBJECT_EVENT);
	//CLASS_Proxy->is_observer = TRUE;
	//CLASS_Proxy->size += sizeof(OBJECT_EVENT);
}
