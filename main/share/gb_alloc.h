/***************************************************************************

  alloc.h

  Memory management routines

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

#ifndef __GB_ALLOC_H
#define __GB_ALLOC_H

#define DEBUG_MEMORY 0
//#define DO_NOT_PRINT_MEMORY

#ifndef __GB_ALLOC_C
EXTERN long MEMORY_count;
#endif

#if DEBUG_MEMORY

EXTERN long MEMORY_size;

#define ALLOC(_ptr, _size, _src)        MEMORY_alloc((void *)_ptr, _size, _src)
#define ALLOC_ZERO(_ptr, _size, _src)   MEMORY_alloc_zero((void *)_ptr, _size, _src)
#define REALLOC(_ptr, _size, _src)      MEMORY_realloc((void *)_ptr, _size, _src)
#define FREE(_ptr, _src)                MEMORY_free((void *)_ptr, _src)

PUBLIC void MEMORY_alloc(void *p_ptr, size_t size, const char *src);
PUBLIC void MEMORY_alloc_zero(void *p_ptr, size_t size, const char *src);
PUBLIC void MEMORY_realloc(void *p_ptr, size_t size, const char *src);
PUBLIC void MEMORY_free(void *p_ptr, const char *src);
PUBLIC void MEMORY_check(void);

PUBLIC void MEMORY_verify(void);
PUBLIC void MEMORY_check_ptr(void *ptr);

#else

#define ALLOC(_ptr, _size, _src)        MEMORY_alloc((void *)_ptr, _size)
#define ALLOC_ZERO(_ptr, _size, _src)   MEMORY_alloc_zero((void *)_ptr, _size)
#define REALLOC(_ptr, _size, _src)      MEMORY_realloc((void *)_ptr, _size)
#define FREE(_ptr, _src)                MEMORY_free((void *)_ptr)

PUBLIC void MEMORY_alloc(void *p_ptr, size_t size);
PUBLIC void MEMORY_alloc_zero(void *p_ptr, size_t size);
PUBLIC void MEMORY_realloc(void *p_ptr, size_t size);
PUBLIC void MEMORY_free(void *p_ptr);
PUBLIC void MEMORY_check(void);

#endif

PUBLIC void MEMORY_init(void);
PUBLIC void MEMORY_exit(void);

#endif

