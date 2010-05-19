/***************************************************************************

  gbx_subr_extern.c

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

#include <sys/wait.h>

#include "gb_common.h"
#include "gb_alloc.h"
#include "gb_common_check.h"
#include "gb_pcode.h"

#include "gbx_subr.h"


void SUBR_alloc(void)
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
  
  ALLOC(&ptr, size * count, "SUBR_alloc");
  
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
  
  FREE(&ptr, "SUBR_free");
  
  SUBR_LEAVE_VOID();
}


void SUBR_realloc(void)
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
  
  REALLOC(&ptr, size * count, "SUBR_realloc");
  
  RETURN->type = T_POINTER;
  RETURN->_pointer.value = ptr;
  
  SUBR_LEAVE();
}


void SUBR_strptr(void)
{
  char *ptr;
  size_t len = 0;
  
  SUBR_ENTER_PARAM(1);
  
  ptr = (char *)SUBR_get_pointer(PARAM);
  
  if (CHECK_strlen(ptr, &len))
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


void SUBR_varptr(void)
{
	ushort op;
	void *ptr;
	VALUE *val;
  CLASS_VAR *var;
	
	SUBR_ENTER_PARAM(1);
	
	op = (ushort)SUBR_get_integer(PARAM);
	
	if ((op & 0xFF00) == C_PUSH_LOCAL)
	{
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
			case T_FLOAT:
				ptr = &val->_float.value;
				break;
			
			case T_STRING:
			case T_CSTRING:
				ptr = val->_string.addr + val->_string.start;
				break;
				
			default:
			  THROW(E_TYPE, "Number", TYPE_get_name(val->type));
		}
	}
	else if ((op & 0xF800) == C_PUSH_DYNAMIC)
	{
    var = &CP->load->dyn[op & 0x7FF];

    if (OP == NULL)
      THROW(E_ILLEGAL);

    ptr = &OP[var->pos];
  }
	else if ((op & 0xF800) == C_PUSH_STATIC)
	{
    var = &CP->load->stat[op & 0x7FF];
    ptr = (char *)CP->stat + var->pos;
	}
	else
		THROW(E_ILLEGAL);

  RETURN->type = T_POINTER;
  RETURN->_pointer.value = ptr;
	
	SUBR_LEAVE();
}
