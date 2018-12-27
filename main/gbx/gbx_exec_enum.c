/***************************************************************************

	gbx_exec_enum.c

	(c) 2000-2017 Benoît Minisini <g4mba5@gmail.com>

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

#include "gb_common.h"
#include "gb_limit.h"
#include "gbx_c_enum.h"
#include "gbx_exec.h"


/* EXEC_object() ne doit pas faire d'auto-create, car sinon
	il renvoie un objet référencé */

void EXEC_enum_first(PCODE code, VALUE *local, VALUE *penum)
{
	OBJECT *object;
	CLASS *class;
	CENUM *old = EXEC_enum;
	CENUM *cenum;

	EXEC_object(local, &class, &object);

	if (!object && class->auto_create && !class->enum_static)
		object = EXEC_auto_create(class, FALSE);

	cenum = CENUM_create(object ? (void *)object : (void *)class);

	RELEASE(penum);
	penum->_object.class = OBJECT_class(cenum);
	penum->_object.object = cenum;
	OBJECT_REF(cenum);

	EXEC_enum = cenum;
	EXEC_special(SPEC_FIRST, class, object, 0, TRUE);
	EXEC_enum = old;
}


bool EXEC_enum_next(PCODE code, VALUE *local, VALUE *penum)
{
	OBJECT *object;
	CLASS *class;
	bool defined;
	bool drop = (code & 1);
	bool err;
	CENUM *old = EXEC_enum;
	CENUM *cenum;

	defined = EXEC_object(local, &class, &object);
	cenum = (CENUM *)penum->_object.object;
	if (!cenum)
		return TRUE;

	if (cenum->stop)
		goto __STOP;
	
	EXEC_enum = cenum;
	err = EXEC_special(SPEC_NEXT, class, object, 0, FALSE);
	EXEC_enum = old;
	
	if (err)
		THROW(E_ENUM);
		
	if (!defined && !drop && !cenum->stop)
		VALUE_conv_variant(&SP[-1]);

	if (drop || cenum->stop)
		POP();

	if (!cenum->stop)
		return FALSE;
	
__STOP:

	OBJECT_UNREF(cenum);
	penum->_object.object = NULL;
	return TRUE;
}
