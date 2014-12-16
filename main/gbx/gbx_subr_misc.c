/***************************************************************************

  gbx_subr_misc.c

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

static void init_eval()
{
	static bool init = FALSE;
	
	if (init)
		return;
		
	COMPONENT_load(COMPONENT_create("gb.eval"));
	LIBRARY_get_interface_by_name("gb.eval", EVAL_INTERFACE_VERSION, &EVAL);
	init = TRUE;
}

void SUBR_error(void)
{
	SP->type = T_BOOLEAN;
	SP->_boolean.value = EXEC_got_error ? -1 : 0;
	SP++;
}


void SUBR_wait(ushort code)
{
	SUBR_ENTER();

	EXEC_set_native_error(FALSE);

	if (NPARAM == 0)
		GB_Wait(0);
	else
		GB_Wait((int)(SUBR_get_float(PARAM) * 1000 + 0.5));

	if (EXEC_has_native_error())
	{
		EXEC_set_native_error(FALSE);
		PROPAGATE();
	}
	
	SUBR_LEAVE_VOID();
}


void SUBR_sleep(ushort code)
{
	SUBR_ENTER_PARAM(1);

	switch(code & 0x3F)
	{
		case 0: // Sleep
		{
			double wait;
			struct timespec rem;

			wait = SUBR_get_float(PARAM);

			rem.tv_sec = (time_t)(int)wait;
			rem.tv_nsec = (int)(frac(wait) * 1E9);

			while (nanosleep(&rem, &rem) < 0);
			break;
		}
		case 1: // Use
		{
			char *name = SUBR_get_string(PARAM);
			COMPONENT *comp = COMPONENT_find(name);
			if (!comp)
				comp = COMPONENT_create(name);

			COMPONENT_load(comp);
			break;
		}
		case 2: // CheckExec
		{
			CPROCESS_check(PARAM->_object.object);
			break;
		}
	}
	
	SUBR_LEAVE();
}

static CPROCESS *_error_subr_exec_process;

static void error_subr_exec()
{
	OBJECT_UNREF(_error_subr_exec_process);
}

void SUBR_exec(ushort code)
{
	void *cmd;
	bool wait;
	int mode;
	CPROCESS *process;
	bool ret;
	bool shell;
	char *name;
	CARRAY *env;

	SUBR_ENTER_PARAM(4);

	shell = (code & 0x1F) != 0;

	if (shell)
		cmd = (void *)SUBR_get_string(PARAM);
	else
	{
		VALUE_conv_object(PARAM, (TYPE)CLASS_StringArray);
		cmd = (void *)(PARAM->_object.object);
	}

	if (!cmd)
		THROW(E_ARG);

	if (VALUE_is_null(&PARAM[1]))
		env = NULL;
	else
	{
		VALUE_conv_object(&PARAM[1], (TYPE)CLASS_StringArray);
		env = (PARAM[1]._object.object);
	}
	
	VALUE_conv_integer(&PARAM[2]);
	mode = PARAM[2]._integer.value;
	wait = mode & PM_WAIT;

	name = SUBR_get_string(&PARAM[3]);

	ret = TRUE; // !PCODE_is_void(code);

	if (shell)
		mode |= PM_SHELL;

	//STRING_ref(name); ## This should not be needed
	process = CPROCESS_create(mode, cmd, name, env);
	//STRING_unref(&name);

	if (wait)
	{
		OBJECT_REF(process);

		_error_subr_exec_process = process;
		ON_ERROR(error_subr_exec)
		{
			CPROCESS_wait_for(process, 0);
		}
		END_ERROR

		if (!ret)
		{
			OBJECT_UNREF(process);
		}
		else if (!process->to_string)
		{
			OBJECT_UNREF_KEEP(process);
		}
	}

	if (ret)
	{
		if (process->to_string)
		{
			char *result = process->result;
			process->result = NULL;
			
			RELEASE_MANY(SP, NPARAM);
			
			SP->type = T_STRING;
			SP->_string.addr = result;
			SP->_string.start = 0;
			SP->_string.len = STRING_length(result);
			SP++;

			OBJECT_UNREF(process);
		}
		else
		{
			RETURN->_object.class = CLASS_Process;
			RETURN->_object.object = process;
			SUBR_LEAVE();
		}
	}
	else
	{
		SUBR_LEAVE_VOID();
	}
}


static bool get_value(const char *sym, int len, GB_VARIANT *value)
{
	if (eval_env)
		if (!GB_CollectionGet(eval_env, sym, len, value))
			return FALSE;

	value->type = GB_T_NULL;
	return TRUE;
}

void EVAL_string(char *expr)
{
	int len;
	EXPRESSION *eval;

	init_eval();
	len = strlen(expr);
	eval_env = NULL;
	
	EVAL.New((void **)(void *)&eval, expr, len);

	if (EVAL.Compile(eval, FALSE))
	{
		GB_Error(eval->error);
		goto _ERROR;
	}

	if (!EVAL.Run(eval, get_value))
		goto _ERROR;

	goto _FREE;

_ERROR:
	EVAL.Free((void **)(void *)&eval);
	PROPAGATE();

_FREE:
	EVAL.Free((void **)(void *)&eval);

	VALUE_to_string(RETURN, &expr, &len);
	STREAM_write(CSTREAM_stream(CFILE_out), expr, len);
	STREAM_write_eol(CSTREAM_stream(CFILE_out));
	STREAM_flush(CSTREAM_stream(CFILE_out));
}

void SUBR_eval(ushort code)
{
	char *expr;
	int len;
	EXPRESSION *eval;

	SUBR_ENTER();

	init_eval();
	SUBR_get_string_len(PARAM, &expr, &len);

	if (NPARAM == 2)
	{
		VALUE_conv_object(&PARAM[1], (TYPE)CLASS_Collection);
		eval_env = (CCOLLECTION *)(PARAM[1]._object.object);
	}
	else
		eval_env = NULL;

	EVAL.New((void **)(void *)&eval, expr, len);

	if (EVAL.Compile(eval, FALSE))
	{
		GB_Error(eval->error);
		goto _ERROR;
	}

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

void SUBR_array(ushort code)
{
	static bool reuse = FALSE;

	TYPE type;
	int i, j;
	CARRAY *array;
	bool next_reuse;

	SUBR_ENTER();

	next_reuse = code & CODE_CALL_VARIANT;

	if (reuse)
	{
		array = (CARRAY *)(PARAM[-1]._object.object);
		type = array->type;
	}
	else
	{
		type = SUBR_check_good_type(PARAM, NPARAM);

		if (type == T_NULL)
			type = T_OBJECT;
	}

	for (i = 0; i < NPARAM; i++)
		VALUE_conv(&PARAM[i], type);

	if (reuse)
	{
		j = array->count;
		CARRAY_resize(array, j + NPARAM);
	}
	else
	{
		j = 0;
		GB_ArrayNew(POINTER(&array), type, NPARAM);
		OBJECT_REF(array);
	}

	for (i = 0; i < NPARAM; i++, j++)
	{
		GB_Store(type, (GB_VALUE *)&PARAM[i], GB_ArrayGet(array, j));
		RELEASE(&PARAM[i]);
	}

	if (reuse)
	{
		SP = PARAM;
	}
	else
	{
		PARAM->_object.class = OBJECT_class(array); //CLASS_Array;
		PARAM->_object.object = array;
		SP = PARAM + 1;
	}

	reuse = next_reuse;
}

void SUBR_collection(ushort code)
{
	static bool reuse = FALSE;

	int i;
	GB_COLLECTION col;
	char *key;
	int len;
	VALUE *vkey, *vval;
	bool next_reuse;

	SUBR_ENTER();

	next_reuse = code & CODE_CALL_VARIANT;

	if (reuse)
		col = (GB_COLLECTION)(PARAM[-1]._object.object);
	else
	{
		GB_CollectionNew(&col, GB_COMP_BINARY);
		OBJECT_REF(col);
	}

	for (i = 0; i < NPARAM; i += 2)
	{
		vkey = &PARAM[i];
		vval = vkey + 1;
		SUBR_get_string_len(vkey, &key, &len);
		VALUE_conv_variant(vval);
		if (GB_CollectionSet(col, key, len, (GB_VARIANT *)vval))
		{
			OBJECT_UNREF(col);
			THROW(E_VKEY);
		}
		RELEASE_STRING(&PARAM[i]);
		RELEASE(&PARAM[i + 1]);
	}

	if (reuse)
	{
		SP = PARAM;
	}
	else
	{
		PARAM->_object.class = OBJECT_class(col); //CLASS_Array;
		PARAM->_object.object = col;
		SP = PARAM + 1;
	}

	reuse = next_reuse;
}


