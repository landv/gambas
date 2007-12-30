/***************************************************************************

  subr_misc.c

  Miscellaneous subroutine

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

#include <sys/wait.h>
#include <time.h>

#include "gb_common.h"
#include "gbx_subr.h"
#include "gbx_class.h"
#include "gambas.h"
#include "gbx_eval.h"
#include "gbx_date.h"
#include "gbx_archive.h"

#include "gbx_api.h"
#include "gbx_c_collection.h"
#include "gbx_c_process.h"
#include "gbx_debug.h"
#include "gbx_watch.h"
#include "gbx_math.h"


static EVAL_INTERFACE EVAL;
static CCOLLECTION *eval_env;


PUBLIC void SUBR_error(void)
{
  SP->type = T_BOOLEAN;
  SP->_boolean.value = ERROR_info.code != 0 ? -1 : 0;
  SP++;
}


PUBLIC void SUBR_wait(void)
{
  double wait;
  double stop, time;

  SUBR_ENTER();

  if (NPARAM == 0)
  {
    HOOK_DEFAULT(wait, WATCH_wait)(0);
  }
  else
  {
    wait = SUBR_get_float(PARAM);
    DATE_timer(&stop, FALSE);
    stop += wait;

    for(;;)
    {
      HOOK_DEFAULT(wait, WATCH_wait)((long)(wait * 1000 + 0.5));

      if (DATE_timer(&time, FALSE))
        break;

      if (time >= stop)
        break;
    }
  }

  SUBR_LEAVE_VOID();
}


PUBLIC void SUBR_sleep(void)
{
	double wait;
	struct timespec rem;

	SUBR_ENTER_PARAM(1);

  wait = SUBR_get_float(PARAM);

	rem.tv_sec = (time_t)(long)wait;
	rem.tv_nsec = (long)(frac(wait) * 1E9);

	while (nanosleep(&rem, &rem) < 0);

	SUBR_LEAVE();
}


PUBLIC void SUBR_exec(void)
{
  void *cmd;
  bool wait;
  int mode;
  CPROCESS *process;
  bool ret;
  bool shell;
  char *name;

  SUBR_ENTER_PARAM(4);

  shell = (EXEC_code & 0x1F) != 0;

  if (shell)
    cmd = (void *)SUBR_get_string(PARAM);
  else
  {
    VALUE_conv(PARAM, (TYPE)CLASS_StringArray);
    cmd = (void *)(PARAM->_object.object);
  }

  VALUE_conv(&PARAM[1], T_BOOLEAN);
  wait = PARAM[1]._boolean.value;

  VALUE_conv(&PARAM[2], T_INTEGER);
  mode = PARAM[2]._integer.value;

	name = SUBR_get_string(&PARAM[3]);

  ret = !PCODE_is_void(EXEC_code);

  if (shell)
    mode |= PM_SHELL;

  //if (!ret)
  //	mode |= PM_IGNORE;

	//STRING_ref(name); ## This should not be needed
  process = CPROCESS_create(mode, cmd, name);
  //STRING_unref(&name);

  if (wait)
  {
    OBJECT_REF(process, "subr_exec");

    CPROCESS_wait_for(process);

    if (!ret)
    {
      OBJECT_UNREF(&process, "subr_exec");
    }
    else if (!process->to_string)
    {
      OBJECT_UNREF_KEEP(&process, "subr_exec");
    }
  }

  if (ret)
  {
    if (process->to_string)
    {
      RETURN->type = T_STRING;
      RETURN->_string.addr = process->result;
      RETURN->_string.start = 0;
      RETURN->_string.len = STRING_length(process->result);

      STRING_unref_keep(&process->result);
      process->result = NULL;
      OBJECT_UNREF(&process, "subr_exec");
    }
    else
    {
      RETURN->_object.class = CLASS_Process;
      RETURN->_object.object = process;
    }
    SUBR_LEAVE();
  }
  else
  {
    SUBR_LEAVE_VOID();
  }
}


static bool get_value(const char *sym, long len, GB_VARIANT *value)
{
  if (eval_env)
    if (!GB_CollectionGet(eval_env, sym, len, value))
      return FALSE;

  value->type = GB_T_NULL;
  return TRUE;
}

PUBLIC void SUBR_eval(void)
{
  static bool init = FALSE;
  char *expr;
  long len;
  EXPRESSION *eval;

  SUBR_ENTER();

  if (!init)
  {
    COMPONENT_load(COMPONENT_create("gb.eval"));
    LIBRARY_get_interface_by_name("gb.eval", EVAL_INTERFACE_VERSION, &EVAL);
    init = TRUE;
  }

  SUBR_get_string_len(PARAM, &expr, &len);

  if (NPARAM == 2)
  {
    VALUE_conv(&PARAM[1], (TYPE)CLASS_Collection);
    eval_env = (CCOLLECTION *)(PARAM[1]._object.object);
  }
  else
    eval_env = NULL;

  EVAL.New((void **)(void *)&eval, expr, len);

  if (EVAL.Compile(eval))
    goto _ERROR;

  if (!EVAL.Run(eval, get_value))
    goto _ERROR;

  goto _FREE;

_ERROR:
  EVAL.Free((void **)(void *)&eval);
  PROPAGATE();

_FREE:
  EVAL.Free((void **)(void *)&eval);

  SUBR_LEAVE();
}


static TYPE conv_type(TYPE type)
{
  if (type <= T_BYTE)
    type = T_BYTE;
  /*else if (type <= T_INTEGER)
    ;*/
  else if (type == T_CSTRING || type == T_NULL)
    type = T_STRING;
  else if (type >= T_OBJECT)
    type = T_OBJECT;

	return type;
}

PUBLIC void SUBR_array(void)
{
  TYPE type;
  int i;
  GB_ARRAY array;

  SUBR_ENTER();

  type = conv_type(PARAM[0].type);
	if (NPARAM >= 2 && conv_type(PARAM[1].type) != type)
		type = T_VARIANT;

  GB_ArrayNew(&array, type, NPARAM);

  for (i = 0; i < NPARAM; i++)
  {
    VALUE_conv(&PARAM[i], type);
    GB_Store(type, (GB_VALUE *)&PARAM[i], GB_ArrayGet(array, i));
  }

  RETURN->_object.class = OBJECT_class(array); //CLASS_Array;
  RETURN->_object.object = array;

  SUBR_LEAVE();
}


