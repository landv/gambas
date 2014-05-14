/***************************************************************************

  gbx_subr_extern.c

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

#include <sys/wait.h>

#include "gb_common.h"
#include "gb_alloc.h"
#include "gb_common_check.h"
#include "gb_pcode.h"

#include "gbx_subr.h"


void SUBR_alloc(ushort code)
{
  int size;
  int count;
  void *ptr;
  char *copy;

  SUBR_ENTER();

  if (NPARAM == 2)
    count = SUBR_get_integer(&PARAM[1]);
  else
    count = 1;
  
  if (TYPE_is_null(PARAM->type))
  {
    size = 1;
    copy = NULL;
  }
  if (TYPE_is_string(PARAM->type))
  {
    size = PARAM->_string.len + 1;
    copy = PARAM->_string.addr + PARAM->_string.start;
  }
  else
  {
    size = SUBR_get_integer(PARAM);
    copy = NULL;
  }
  
  if (count <= 0 || size <= 0)
    THROW(E_ARG);
  
  ALLOC(&ptr, size * count);
  
  if (copy)
  {
    size--;
    memcpy(ptr, copy, size);
    ((char *)ptr)[size] = 0;
  }
  
  RETURN->type = T_POINTER;
  RETURN->_pointer.value = ptr;
  
  SUBR_LEAVE();
}


void SUBR_free(void)
{
  void *ptr;

  SUBR_ENTER_PARAM(1);

  ptr = SUBR_get_pointer(PARAM);
  
  IFREE(ptr);
  
  SUBR_LEAVE_VOID();
}


void SUBR_realloc(ushort code)
{
  int size;
  int count;
  void *ptr;

  SUBR_ENTER();

  if (NPARAM == 3)
    size = SUBR_get_integer(&PARAM[2]);
  else
    size = 1;
  
  count = SUBR_get_integer(&PARAM[1]);
  
  if (size <= 0 || count <= 0)
    THROW(E_ARG);
  
  ptr = SUBR_get_pointer(&PARAM[0]);
  
  REALLOC(&ptr, size * count);
  
  RETURN->type = T_POINTER;
  RETURN->_pointer.value = ptr;
  
  SUBR_LEAVE();
}


void SUBR_strptr(ushort code)
{
  char *ptr;
  ssize_t len = 0;
	bool err;
  
  SUBR_ENTER();
  
  ptr = (char *)SUBR_get_pointer(PARAM);
	
	if (NPARAM == 1)
	{
		err = CHECK_strlen(ptr, &len);
	}
	else
	{
		len = SUBR_get_integer(&PARAM[1]);
		err = CHECK_address(ptr, len);
	}
    
	if (err)
	{
		RETURN->type = T_NULL;
	}
	else
	{ 
		RETURN->type = T_CSTRING;
		RETURN->_string.addr = ptr;
		RETURN->_string.start = 0;
		RETURN->_string.len = (int)len;
	}
	
  SUBR_LEAVE();
}

void SUBR_varptr(ushort code)
{
	ushort op;
	void *ptr;
	VALUE *val;
  CLASS_VAR *var;
	
	SUBR_ENTER_PARAM(1);
	
	op = (ushort)SUBR_get_integer(PARAM);
	
	if ((code & 0xFF) == 1)
	{
		uint64_t optargs = BP[FP->n_local + FP->n_ctrl - 1]._long.value;

		RETURN->type = T_BOOLEAN;
		RETURN->_boolean.value = (optargs & (1 << (FP->n_param + (op & 0xFF) - 256))) ? -1 : 0;
	}
	else
	{
		if ((op & 0xFF00) == C_PUSH_LOCAL || (op & 0xFF00) == C_PUSH_PARAM)
		{
			if ((op & 0xFF00) == C_PUSH_PARAM)
				val = &PP[(signed char)(op & 0xFF)];
			else
				val = &BP[op & 0xFF];

			switch(val->type)
			{
				case T_BOOLEAN:
				case T_BYTE:
				case T_SHORT:
				case T_INTEGER:
					ptr = &val->_integer.value;
					break;

				case T_LONG:
					ptr = &val->_long.value;
					break;

				case T_SINGLE:
					ptr = &val->_single.value;
					break;

				case T_FLOAT:
					ptr = &val->_float.value;
					break;

				case T_DATE:
					ptr = &val->_date.date;
					break;

				case T_STRING:
				case T_CSTRING:
					ptr = val->_string.addr + val->_string.start;
					break;

				case T_POINTER:
					ptr = &val->_pointer.value;
					break;

				default:
					THROW(E_TYPE, "Number", TYPE_get_name(val->type));
			}
		}
		else if ((op & 0xF800) == C_PUSH_DYNAMIC)
		{
			var = &CP->load->dyn[op & 0x7FF];

			if (OP == NULL)
				THROW_ILLEGAL();

			ptr = &OP[var->pos];
		}
		else if ((op & 0xF800) == C_PUSH_STATIC)
		{
			var = &CP->load->stat[op & 0x7FF];
			ptr = (char *)CP->stat + var->pos;
		}
		else
			THROW_ILLEGAL();

		RETURN->type = T_POINTER;
		RETURN->_pointer.value = ptr;
	}
	
	SUBR_LEAVE();
}


void SUBR_ptr(ushort code)
{
  void *ptr;
  
	SUBR_ENTER_PARAM(1);
	
  ptr = SUBR_get_pointer_or_string(PARAM);
	
	CHECK_enter();
  if (sigsetjmp(CHECK_jump, TRUE) == 0)	
		VALUE_read(RETURN, ptr, code & 0xF);
	CHECK_leave();
	
	if (CHECK_got_error())
		THROW(E_ARG);

	SUBR_LEAVE();
}

void SUBR_make(ushort code)
{
  static void *jump[] = {
    &&__ERROR, &&__BOOLEAN, &&__BYTE, &&__SHORT, &&__INTEGER, &&__LONG, &&__SINGLE, &&__FLOAT, 
		&&__DATE, &&__ERROR, &&__ERROR, &&__POINTER, &&__ERROR, &&__ERROR, &&__ERROR, &&__ERROR
    };
		
	TYPE type;
	
	SUBR_ENTER_PARAM(1);
	
	type = code & 0x3F;
	VALUE_conv(PARAM, type);
	STRING_new_temp_value(RETURN, NULL, TYPE_sizeof_memory(type));
	goto *jump[type];
	
__BOOLEAN:

	*(RETURN->_string.addr) = PARAM->_boolean.value != 0;
	goto __END;

__BYTE:

	*(RETURN->_string.addr) = PARAM->_byte.value;
	goto __END;

__SHORT:

	memcpy(RETURN->_string.addr, &PARAM->_short.value, sizeof(short));
	goto __END;

__INTEGER:

	memcpy(RETURN->_string.addr, &PARAM->_integer.value, sizeof(int));
	goto __END;

__LONG:

	memcpy(RETURN->_string.addr, &PARAM->_long.value, sizeof(int64_t));
	goto __END;

__SINGLE:

	memcpy(RETURN->_string.addr, &PARAM->_single.value, sizeof(float));
	goto __END;

__FLOAT:

	memcpy(RETURN->_string.addr, &PARAM->_float.value, sizeof(double));
	goto __END;

__DATE:

	memcpy(RETURN->_string.addr, &PARAM->_date.date, sizeof(int) * 2);
	goto __END;
	
__POINTER:

	memcpy(RETURN->_string.addr, &PARAM->_pointer.value, sizeof(void *));
	goto __END;
	
__ERROR:

	THROW_ILLEGAL();

__END:

	SUBR_LEAVE();
}


