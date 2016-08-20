/***************************************************************************

  jit_runtime.h

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

#ifndef __JIT_RUNTIME_H
#define __JIT_RUNTIME_H

#include "gambas.h"

#ifdef __cplusplus
extern "C" {
#endif
void JR_release_variant(long vtype, char* data);
void JR_borrow_variant(long vtype, char* data);
void JR_aq_variant(int add);
void JR_variant_equal(void);
void JR_variant_compi_less_than(void);
void JR_add(ushort code);
void JR_sub(ushort code);
void JR_mul(ushort code);
void JR_call(int nparam);

void JR_push_unknown_property_unknown(const char *name, int name_id, CLASS *klass, void *object);
void JR_pop_unknown_property_unknown(CLASS *klass, void *object, const char *name);

void* JR_extern_dispatch_object(OBJECT* object, int index);

void JR_exec_enter_quick(CLASS* klass, void* object, int index);
void JR_exec_enter(CLASS* klass, void* object, int index);
OBJECT* JR_object_cast(OBJECT* object, CLASS* target_class);

void JR_EXEC_jit_execute_function(void);

void* JR_try(ERROR_CONTEXT* err);
void JR_end_try(ERROR_CONTEXT* err);
void JR_try_unwind(VALUE* stack_start);

CLASS_DESC_METHOD *JR_CLASS_get_special_desc(CLASS *klass, int spec);
#ifdef __cplusplus
}
#endif

#endif /* __JIT_RUNTIME_H */
