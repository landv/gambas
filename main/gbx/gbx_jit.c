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

static bool _component_loaded = FALSE;
static GB_FUNCTION _jit_compile_func;

static bool _no_jit = FALSE;
static void *_jit_library = NULL;

static JIT_FUNCTION *_jit_func = NULL;

void JIT_exit(void)
{
	ARRAY_delete(&_jit_func);
}

bool JIT_compile(ARCHIVE *arch)
{
	GB_VALUE *ret;
	char *path;
	void *lib;
	void **iface;
	
	if (_no_jit)
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
		char *var = getenv("GB_NO_JIT");
		
		_component_loaded = TRUE;
		
		if (var && var[0] && !(var[0] == '0' && var[1] == 0))
		{
			_no_jit = TRUE;
			return TRUE;
		}
		
		fprintf(stderr, "gbx3: loading gb.jit component\n");
		COMPONENT_load(COMPONENT_create("gb.jit"));
		if (GB_GetFunction(&_jit_compile_func, CLASS_find_global("_Jit"), "Compile", "", "s"))
			ERROR_panic("Unable to find _Jit.Compile method");
	}
	
	ret = GB_Call(&_jit_compile_func, 0, FALSE);
	path = GB_ToZeroString((GB_STRING *)ret);
	if (!*path)
		ERROR_panic("Unable to compile jit source file");
	
	//fprintf(stderr, "gbx3: shared jit library is: %s\n", path);

	lib = dlopen(path, RTLD_NOW);
	if (!lib)
		ERROR_panic("Unable to load jit library: %s", dlerror());
	
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

void JIT_create_function(ARCHIVE *arch, CLASS *class, int index)
{
	FUNCTION *func;
	JIT_FUNCTION *jit;
	void *lib;
	void *addr;
	int i;
	int len;
	
	func = &class->load->func[index];
	
	if (!arch)
		lib = _jit_library;
	else
		lib = arch->jit_library;
	
	len = sprintf(COMMON_buffer, "%s_%d", class->name, index);
	
	for (i = 0; i < len; i++)
		COMMON_buffer[i] = tolower(COMMON_buffer[i]);
	
	addr = dlsym(lib, COMMON_buffer);
	if (!addr)
	{
		func->fast = FALSE;
		return;
	}
	
	if (func->debug)
		fprintf(stderr, "gbx3: loading jit function: %s.%s\n", class->name, func->debug->name);

	if (!_jit_func)
		ARRAY_create(&_jit_func);
	
	jit = (JIT_FUNCTION *)ARRAY_add(&_jit_func);
	
	jit->addr = addr;
	jit->code = func->code;
	
	func->code = (PCODE *)jit;
}


void JIT_exec(void)
{
	CLASS *class = CP;
	void *object = OP;
	VALUE *sp = SP;
	JIT_FUNCTION *jit;
	char nparam = EXEC.nparam;
	
	CP = EXEC.class;
	OP = (void *)EXEC.object;
	
	jit = (JIT_FUNCTION *)(EXEC.func->code);
	PC = jit->code;
	
	(*(jit->addr))(nparam);
	
	if (SP != sp)
		fprintf(stderr, "SP: %+ld (%ld) !\n", (SP - sp) / sizeof(VALUE), SP - sp);
	
	RELEASE_MANY(SP, nparam);
	
	CP = class;
	OP = object;
}

void *JIT_get_dynamic_addr(int index)
{
	CLASS_VAR *var = &CP->load->dyn[index];
	return &OP[var->pos];
}

void *JIT_get_static_addr(int index)
{
	CLASS_VAR *var = &CP->load->stat[index];
	return &((char *)CP->stat)[var->pos];
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
