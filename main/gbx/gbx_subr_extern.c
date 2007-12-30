/***************************************************************************

  subr_extern.c

  Extern help subroutines

  (c) 2000-2005 Beno�t Minisini <gambas@users.sourceforge.net>

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

#include <sys/wait.h>

#include "gb_common.h"
#include "gb_alloc.h"
#include "gb_common_check.h"

#include "gbx_subr.h"


PUBLIC void SUBR_alloc(void)
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
  
  RETURN->type = T_INTEGER;
  RETURN->_integer.value = (int)ptr;
  
  SUBR_LEAVE();
}


PUBLIC void SUBR_free(void)
{
  void *ptr;

  SUBR_ENTER_PARAM(1);

  ptr = (void *)SUBR_get_integer(PARAM);
  
  FREE(&ptr, "SUBR_free");
  
  SUBR_LEAVE_VOID();
}


PUBLIC void SUBR_realloc(void)
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
  
  ptr = (void *)SUBR_get_integer(&PARAM[0]);
  
  REALLOC(&ptr, size * count, "SUBR_realloc");
  
  RETURN->type = T_INTEGER;
  RETURN->_integer.value = (int)ptr;
  
  SUBR_LEAVE();
}


PUBLIC void SUBR_strptr(void)
{
  char *ptr;
  int len = 0;
  
  SUBR_ENTER_PARAM(1);
  
  ptr = (char *)SUBR_get_integer(PARAM);
  
  if (CHECK_strlen(ptr, &len))
  {
    RETURN->type = T_NULL;
  }
  else
  { 
    RETURN->type = T_CSTRING;
    RETURN->_string.addr = ptr;
    RETURN->_string.start = 0;
    RETURN->_string.len = len;
  }
    
  SUBR_LEAVE();
}
