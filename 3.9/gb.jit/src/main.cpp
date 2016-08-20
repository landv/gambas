/***************************************************************************

  main.c

  gb.jit component

  (c) 2012 Emil Lenngren <emil.lenngren [at] gmail.com>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 1, or (at your option)
  any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
  MA 02110-1301, USA.

***************************************************************************/

#define __MAIN_C

#include "gambas.h"
#include "main.h"
// Don't use <config.h> if we just need the version
#include <llvm/Config/llvm-config.h>

extern "C" {
	GB_INTERFACE GB EXPORT;
	bool MAIN_debug = false;
}

void *GB_JIT_1[] EXPORT = {
	(void *)1,
	(void *)JIT_init,
	(void *)JIT_compile_and_execute,
	(void *)JIT_load_class,
	NULL
};


GB_DESC *GB_CLASSES[] EXPORT =
{
  NULL
};


extern "C" int EXPORT GB_INIT(void)
{
	char *env;

	env = getenv("GB_JIT_DEBUG");
	if (!env || !*env)
		env = getenv("GB_JIT");
	
	if (env && env[0] && strcmp(env, "0") != 0)
		MAIN_debug = true;
	
	if (LLVM_VERSION_MAJOR < 3 || (LLVM_VERSION_MAJOR == 3 && LLVM_VERSION_MINOR < 1))
		fprintf(stderr, "gb.jit: warning: LLVM %d.%d is not supported.\n", LLVM_VERSION_MAJOR, LLVM_VERSION_MINOR);
	else
	{
		if (MAIN_debug)
			fprintf(stderr, "gb.jit: using LLVM %d.%d.\n", LLVM_VERSION_MAJOR, LLVM_VERSION_MINOR);
	}
	
  return 0;
}

extern "C" void EXPORT GB_EXIT()
{
  JIT_end();
}
