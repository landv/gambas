/***************************************************************************

  gbx_jit.h

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

#ifndef __GBX_JIT_H
#define __GBX_JIT_H

#include "gbx_class.h"
#include "gbx_type.h"
#include "gbx_value.h"
#include "gbx_stack.h"
#include "gbx_object.h"
#include "gbx_exec.h"

typedef
	void (*JIT_FUNC)(uchar nparam);

bool JIT_compile(ARCHIVE *arch);
void *JIT_get_function(ARCHIVE *arch, CLASS *class, int index);
void JIT_debug(const char *fmt, ...);
void JIT_exec(void);
void *JIT_get_static_addr(int index);
void *JIT_get_dynamic_addr(int index);
CLASS_CONST *JIT_get_constant(int index);

#endif
