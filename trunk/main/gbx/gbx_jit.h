/***************************************************************************

  gbx_jit.c

  (c) 2012 Emil Lenngren <emil.lenngren [at] gmail.com>

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

#include "../../gb.jit/src/gb.jit.h"

#ifndef __GBX_JIT_C

EXTERN JIT_INTERFACE JIT;

#endif

bool JIT_load(void);
void JIT_default_jit_function(void);

#endif
