/***************************************************************************

  gb_alloc.h

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

#ifndef __GB_ALLOC_H
#define __GB_ALLOC_H

#include "gb_common.h"

#define DEBUG_MEMORY 0
#define OPTIMIZE_MEMORY 1
//#define DO_NOT_PRINT_MEMORY

#ifndef __GB_ALLOC_C
EXTERN int MEMORY_count;
#endif

#define WHERE_AM_I MEMORY_where_am_i(__FILE__, __LINE__, __func__)

#if DEBUG_MEMORY

#undef OPTIMIZE_MEMORY
#define OPTIMIZE_MEMORY 0

typedef
  struct ALLOC {
    int _void;
    struct ALLOC *next;
    struct ALLOC *prev;
    int id;
    size_t size;
    }
  PACKED
  ALLOC;

EXTERN size_t MEMORY_size;
EXTERN FILE *MEMORY_log;

#define ALLOC(_ptr, _size)        MEMORY_alloc((void *)_ptr, _size, WHERE_AM_I)
#define ALLOC_ZERO(_ptr, _size)   MEMORY_alloc_zero((void *)_ptr, _size, WHERE_AM_I)
#define REALLOC(_ptr, _size)      MEMORY_realloc((void *)_ptr, _size, WHERE_AM_I)
#define FREE(_ptr)                MEMORY_free((void *)_ptr, WHERE_AM_I)
#define IFREE(_ptr)               FREE(&(_ptr))

#define GET_ALLOC_ID(_ptr) (((ALLOC *)((char *)(_ptr) - sizeof(ALLOC)))->id)

char *MEMORY_where_am_i(const char *file, int line, const char *func);

void MEMORY_alloc(void *p_ptr, size_t size, const char *src);
void MEMORY_alloc_zero(void *p_ptr, size_t size, const char *src);
void MEMORY_realloc(void *p_ptr, size_t size, const char *src);
void MEMORY_free(void *p_ptr, const char *src);
void MEMORY_check(void);

void MEMORY_verify(void);
void MEMORY_check_ptr(void *ptr);

#elif OPTIMIZE_MEMORY

/*#define ALLOC(_ptr, _size, _src)        (LIKELY((*(_ptr) = malloc(_size)) != 0) ? MEMORY_count++ : THROW_MEMORY())
#define ALLOC_ZERO(_ptr, _size, _src)   (LIKELY((*(_ptr) = calloc(_size, 1)) != 0) ? MEMORY_count++ : THROW_MEMORY())
#define REALLOC(_ptr, _size, _src)      (LIKELY((*(_ptr) = realloc(*(_ptr), _size)) != 0) ? 0 : THROW_MEMORY())
#define FREE(_ptr, _src)                (LIKELY(*(_ptr) != 0) ? free(*(_ptr)), *(_ptr) = NULL, MEMORY_count-- : 0)
#define IFREE(_ptr, _src)               (LIKELY(_ptr != 0) ? free(_ptr), MEMORY_count-- : 0)*/

#define ALLOC(_ptr, _size)        (*(_ptr) = my_malloc(_size))
#define ALLOC_ZERO(_ptr, _size)   (*(_ptr) = my_malloc(_size), memset(*(_ptr), 0, (_size)))
#define REALLOC(_ptr, _size)      (*(_ptr) = my_realloc(*(_ptr), (_size)))
#define FREE(_ptr)                (my_free(*(_ptr)), *(_ptr) = NULL)
#define IFREE(_ptr)               (my_free(_ptr))

void *my_malloc(size_t len);
void my_free(void *alloc);
void *my_realloc(void *alloc, size_t len);

#else

#define ALLOC(_ptr, _size)        MEMORY_alloc((void *)_ptr, _size)
#define ALLOC_ZERO(_ptr, _size)   MEMORY_alloc_zero((void *)_ptr, _size)
#define REALLOC(_ptr, _size)      MEMORY_realloc((void *)_ptr, _size)
#define FREE(_ptr)                MEMORY_free((void *)_ptr)
#define IFREE(_ptr)               FREE(&(_ptr))

void MEMORY_alloc(void *p_ptr, size_t size);
void MEMORY_alloc_zero(void *p_ptr, size_t size);
void MEMORY_realloc(void *p_ptr, size_t size);
void MEMORY_free(void *p_ptr);
void MEMORY_check(void);

#endif

void MEMORY_init(void);
void MEMORY_exit(void);
void MEMORY_clear_cache(void);
int THROW_MEMORY() NORETURN;

#endif

