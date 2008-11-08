/***************************************************************************

  exec_enum.c

  Enumeration management

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

#include "gb_common.h"
#include "gb_limit.h"
#include "gbx_c_enum.h"
#include "gbx_exec.h"


/* EXEC_object() ne doit pas faire d'auto-create, car sinon
   il renvoie un objet référencé */

void EXEC_enum_first(PCODE code)
{
  OBJECT *object;
  CLASS *class;
  boolean defined;
  VALUE *local;
  CENUM *old = EXEC_enum;
  CENUM *cenum;

  local = &BP[code & 0xFF];

  EXEC_object(local, &class, &object, &defined);

	if (!object && class->auto_create && !class->enum_static)
		object = EXEC_auto_create(class, FALSE);

  cenum = CENUM_create(object ? (void *)object : (void *)class);

  local++;
  RELEASE(local);
  local->_object.class = OBJECT_class(cenum);
  local->_object.object = cenum;
  OBJECT_REF(EXEC_enum, "EXEC_enum_first");

 	EXEC_enum = cenum;
  EXEC_special(SPEC_FIRST, class, object, 0, TRUE);
	EXEC_enum = old;
}


bool EXEC_enum_next(PCODE code)
{
  OBJECT *object;
  CLASS *class;
  boolean defined;
  VALUE *local;
  bool drop = (code & 0xFF);
  bool err;
  CENUM *old = EXEC_enum;
  CENUM *cenum;

  local = &BP[PC[-1] & 0xFF];

  EXEC_object(local, &class, &object, &defined);
  cenum = (CENUM *)local[1]._object.object;

  if (!cenum->stop)
  {
		EXEC_enum = cenum;
    err = EXEC_special(SPEC_NEXT, class, object, 0, FALSE);
    EXEC_enum = old;
    if (err)
      THROW(E_ENUM);
      
    if (!defined && !drop)
    	VALUE_conv(&SP[-1], T_VARIANT);
  }

  if (drop || cenum->stop)
    POP();

  return cenum->stop;
}
