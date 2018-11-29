/***************************************************************************

  gbx_jit.c

  (c) 2018 Benoît Minisini <g4mba5@gmail.com>

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

#define __GBX_JIT_C

#include "gb_common.h"
#include "gb_common_buffer.h"
#include "gb_common_case.h"
#include "gbx_component.h"
#include "gbx_exec.h"
#include "gbx_object.h"
#include "gbx_api.h"
#include "gbx_jit.h"

typedef
	struct {
		JIT_FUNC addr;
		PCODE *code;
	}
	JIT_FUNCTION;

bool JIT_disabled = FALSE;

static bool _component_loaded = FALSE;
static GB_FUNCTION _jit_compile_func;

static bool _jit_compiling = FALSE;
static void *_jit_library = NULL;

static JIT_FUNCTION *_jit_func = NULL;

static bool _debug = FALSE;

void JIT_exit(void)
{
	ARRAY_delete(&_jit_func);
}

bool JIT_can_compile(ARCHIVE *arch)
{
	return arch ? !arch->jit_compiling : !_jit_compiling;
}

bool JIT_compile(ARCHIVE *arch)
{
	GB_VALUE *ret;
	char *path;
	void *lib;
	void **iface;
	COMPONENT *current;
	
	if (JIT_disabled)
		return TRUE;
	
	if (arch)
	{
		if (arch->jit_library)
			return FALSE;
	}
	else
	{
		if (_jit_library)
			return FALSE;
	}
	
	if (!_component_loaded)
	{
		char *var;
		
		_component_loaded = TRUE;
		
		var =  getenv("GB_NO_JIT");
		if (var && var[0] && !(var[0] == '0' && var[1] == 0))
		{
			JIT_disabled = TRUE;
			return TRUE;
		}
		
		var = getenv("GB_JIT_DEBUG");
		if (var && var[0] && !(var[0] == '0' && var[1] == 0))
			_debug = TRUE;
		
		if (_debug)
			fprintf(stderr, "gbx3: loading gb.jit component\n");
		
		COMPONENT_load(COMPONENT_create("gb.jit"));
		if (GB_GetFunction(&_jit_compile_func, CLASS_find_global("Jit"), "_Compile", "s", "s"))
			ERROR_panic("Unable to find JIT compilation method");
	}
	
	arch ? (arch->jit_compiling = TRUE) : (_jit_compiling = TRUE);
	
	current = COMPONENT_current;
	COMPONENT_current = NULL;
	
	GB_Push(1, T_STRING, arch ? arch->name : "", -1);
	ret = GB_Call(&_jit_compile_func, 1, FALSE);
	path = GB_ToZeroString((GB_STRING *)ret);
	
	COMPONENT_current = current;
	
	if (!*path)
		ERROR_panic("Unable to compile JIT source file");
	
	arch ? (arch->jit_compiling = FALSE) : (_jit_compiling = FALSE);
	
	//fprintf(stderr, "gbx3: shared jit library is: %s\n", path);

	lib = dlopen(path, RTLD_NOW);
	if (!lib)
		ERROR_panic("Unable to load JIT library: %s", dlerror());
	
	if (arch)
		arch->jit_library = lib;
	else
		_jit_library = lib;
	
  iface = dlsym(lib, "GB_PTR");
	if (iface) *((void **)iface) = &GAMBAS_Api;
	
  iface = dlsym(lib, "JIT_PTR");
	if (iface) *((void **)iface) = &GAMBAS_JitApi;
	
	return FALSE;
}

static bool create_function(CLASS *class, int index)
{
	ARCHIVE *arch;
	FUNCTION *func;
	JIT_FUNCTION *jit;
	void *lib;
	void *addr;
	int i;
	int len;
	char *name;
	
	arch = class->component ? class->component->archive : NULL;
	
	func = &class->load->func[index];
	func->fast_linked = TRUE;
	
	if (!arch)
		lib = _jit_library;
	else
		lib = arch->jit_library;
	
	name = class->name;
	while (*name == '^')
		name++;
	
	len = sprintf(COMMON_buffer, "jit_%s_%d", name, index);
	
	for (i = 0; i < len; i++)
		COMMON_buffer[i] = tolower(COMMON_buffer[i]);
	
	addr = dlsym(lib, COMMON_buffer);
	if (!addr)
	{
		func->fast = FALSE;
		return TRUE;
	}
	
	if (_debug && func->debug)
		fprintf(stderr, "gbx3: loading jit function: %s.%s\n", class->name, func->debug->name);

	if (!_jit_func)
		ARRAY_create(&_jit_func);
	
	jit = (JIT_FUNCTION *)ARRAY_add(&_jit_func);
	
	jit->addr = addr;
	jit->code = func->code;
	
	func->code = (PCODE *)jit;
	
	return FALSE;
}


void JIT_exec(bool ret_on_stack)
{
	VALUE *sp = SP;
	JIT_FUNCTION *jit;
	CLASS *class = EXEC.class;
	char nparam = EXEC.nparam;
	VALUE ret;
	FUNCTION *func = EXEC.func;
	
	if (UNLIKELY(nparam < func->npmin))
		THROW(E_NEPARAM);
	else if (UNLIKELY(nparam > func->n_param && !func->vararg))
		THROW(E_TMPARAM);

	if (!func->fast_linked)
	{
		if (create_function(class, EXEC.index))
			return;
	}
	
	STACK_push_frame(&EXEC_current, func->stack_usage);
	
	CP = class;
	OP = (void *)EXEC.object;
	FP = func;
	EC = NULL;
	
	jit = (JIT_FUNCTION *)(func->code);
	
	PROFILE_ENTER_FUNCTION();
	
	TRY
	{
		(*(jit->addr))(nparam);
	}
	CATCH
	{
		PROFILE_LEAVE_FUNCTION();
		if (SP != sp)
			ERROR_panic("Stack mismatch in JIT function (sp %+ld)\n", SP - sp);
		RELEASE_MANY(SP, nparam);
		STACK_pop_frame(&EXEC_current);
		PROPAGATE();
	}
	END_TRY
	
	PROFILE_LEAVE_FUNCTION();
	
	if (SP != sp)
		ERROR_panic("Stack mismatch in JIT function (sp %+ld)\n", SP - sp);
	
	if (func->type != T_VOID)
	{
		ret = TEMP;
		BORROW(&ret);
	}
	else
		ret.type = T_VOID;
	
	RELEASE_MANY(SP, nparam);
	
	RET = ret;
	
	STACK_pop_frame(&EXEC_current);
	
	if (ret_on_stack)
	{
		if (SP[-1].type == T_FUNCTION)
		{
			SP--;
			OBJECT_UNREF(SP->_function.object);
		}

		*SP++ = ret;
		ret.type = T_VOID;
	}
}

PCODE *JIT_get_code(FUNCTION *func)
{
	if (func->fast_linked)
		return ((JIT_FUNCTION *)(func->code))->code;
	else
		return func->code;
}

void *JIT_get_class_ref(int index)
{
	return CP->load->class_ref[index];
}

CLASS_CONST *JIT_get_constant(int index)
{
	return &CP->load->cst[index];
}

void JIT_debug(const char *fmt, ...)
{
	va_list args;
	
	va_start(args, fmt);
	vfprintf(stderr, fmt, args);
	va_end(args);
}

typedef
	struct {
		PCODE *pc;
		VALUE **psp;
		PCODE code;
	}
	JIT_call_unknown_ERROR;

static void error_JIT_call_unknown(JIT_call_unknown_ERROR *save)
{
	save->pc[1] = (PCODE)save->code;
	*save->psp = SP;
}

void JIT_call_unknown(PCODE *pc, VALUE **psp)
{
	JIT_call_unknown_ERROR save;
	
	PC = pc;
	SP = *psp;
	
	save.pc = pc;
	save.psp = psp;
	save.code = pc[1];
	
	pc[1] = 0x140B;
	
	ON_ERROR_1(error_JIT_call_unknown, &save)
	{
		EXEC_function_loop();
	}
	END_ERROR

	error_JIT_call_unknown(&save);
}

void JIT_load_class(CLASS *class)
{
	CLASS_load(class);
}
