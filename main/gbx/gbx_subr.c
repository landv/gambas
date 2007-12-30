/***************************************************************************

  subr.c

  Useful routines for subroutines and miscellaneous subroutines

  (c) 2000-2005 Benoît Minisini <gambas@users.sourceforge.net>

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

#define __GBX_SUBR_C

#include "gb_common.h"
#include "gbx_subr.h"
#include "gambas.h"
#include "gbx_api.h"

/*PUBLIC int NPARAM;*/

PUBLIC void SUBR_leave_void(int nparam)
{
  while (nparam)
  {
    nparam--;
    POP();
  }
  
  SP->type = T_VOID;
  SP++;
}

PUBLIC void SUBR_leave(int nparam)
{
  BORROW(RP);

  while (nparam)
  {
    nparam--;
    POP();
  }

  *SP++ = *RP;
  RP->type = T_VOID;
}


PUBLIC boolean SUBR_check_string(VALUE *param)
{
  if (param->type == T_VARIANT)
    VARIANT_undo(param);

  if (TYPE_is_null(param->type))
    return TRUE;

  if (TYPE_is_string(param->type))
  {
    if (param->_string.len == 0)
      return TRUE;
    else
      return FALSE;
    }

  THROW(E_TYPE, TYPE_get_name(T_STRING), TYPE_get_name((param)->type));
}


PUBLIC void SUBR_check_integer(VALUE *param)
{
  if (param->type == T_VARIANT)
    VARIANT_undo(param);

  if (TYPE_is_integer(param->type))
    return;

  THROW(E_TYPE, TYPE_get_name(T_INTEGER), TYPE_get_name((param)->type));
}


PUBLIC void SUBR_check_float(VALUE *param)
{
  if (param->type == T_VARIANT)
    VARIANT_undo(param);

  if (TYPE_is_number(param->type))
  {
    VALUE_conv(param, T_FLOAT);
    return;
  }

  THROW(E_TYPE, TYPE_get_name(T_INTEGER), TYPE_get_name((param)->type));
}


PUBLIC long SUBR_get_integer(VALUE *param)
{
  SUBR_check_integer(param);
  return param->_integer.value;
}


PUBLIC double SUBR_get_float(VALUE *param)
{
  SUBR_check_float(param);
  return param->_float.value;
}


PUBLIC char *SUBR_get_string(VALUE *param)
{
  char *str;

  if (SUBR_check_string(param))
    return "";

  STRING_copy_from_value_temp(&str, param);
  return str;
}


PUBLIC char *SUBR_copy_string(VALUE *param)
{
  char *ret;

  SUBR_check_string(param);
  STRING_copy_from_value_temp(&ret, param);
  /*RELEASE_STRING(param);*/
  return ret;
}


PUBLIC void SUBR_get_string_len(VALUE *param, char **str, long *len)
{
  if (SUBR_check_string(param))
  {
    *str = NULL;
    *len = 0;
  }
  else
    VALUE_get_string(param, str, len);
}

