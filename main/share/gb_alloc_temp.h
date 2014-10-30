/***************************************************************************

  gb_alloc_temp.h

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

#define __GB_ALLOC_C

#include <config.h>

#ifdef HAVE_MALLOC_H
#include <malloc.h>
#endif

#include <stdlib.h>

#include "gb_common.h"
#include "gb_error.h"
#include "gb_alloc.h"

int MEMORY_count = 0;

//#define DEBUG_ME
//#define DO_NOT_PRINT_MEMORY

#if defined(DEBUG_ME) || DEBUG_MEMORY
size_t MEMORY_size = 0;
size_t MEMORY_pool_size = 0;
#endif

#if DEBUG_MEMORY

static int _id = 0;
ALLOC *_alloc = NULL;
#ifdef PROJECT_EXEC
extern char *DEBUG_get_current_position(void);
#endif
FILE *MEMORY_log;

#elif OPTIMIZE_MEMORY

#define SIZE_INC 16
#define SIZE_SHIFT 4
#define REAL_SIZE(_size) (((_size) + (SIZE_INC - 1)) & ~(SIZE_INC - 1))

#define POOL_SIZE  16
#define POOL_MAX   128

#define POOL_MAX_LEN   (POOL_SIZE * SIZE_INC)

static size_t *_pool[POOL_SIZE] = { 0 };
static int _pool_count[POOL_SIZE] = { 0 };
/*static size_t *_first_alloc = 0;
static size_t *_max_alloc = 0;*/

#endif

int THROW_MEMORY()
{
	THROW(E_MEMORY);
}

void MEMORY_init(void)
{
	#if DEBUG_MEMORY
	/*char path[256];
	sprintf(path, "/tmp/gambas-memory-%d-%d.log", getuid(), getpid());
	MEMORY_log = fopen(path, "w+");*/
	MEMORY_log = stderr;
	#endif
}

void MEMORY_clear_cache()
{
#if OPTIMIZE_MEMORY
	int i;
	void *ptr, *next;
	size_t size = 0;
	
	for (i = 0; i < POOL_SIZE; i++)
	{
		ptr = _pool[i];
		while (ptr)
		{
			next = *((void **)ptr);
			size += (i + 1) * SIZE_INC;
			#ifdef DEBUG_ME
			MEMORY_size -= (i + 1) * SIZE_INC;
			#endif
			free(ptr);
			ptr = next;
		}
		_pool[i] = NULL;
		_pool_count[i] = 0;
	}
	
	#ifdef DEBUG_ME
	fprintf(stderr, "free %ld bytes [%ld / %p]\n", size, MEMORY_size, sbrk(0));
	#endif
	
#endif
}

void MEMORY_exit(void)
{
	MEMORY_clear_cache();

#if DEBUG_MEMORY
  if (MEMORY_count)
	{
		fprintf(MEMORY_log, "\n*************************************************\n");
		fprintf(MEMORY_log, "warning: %d allocation(s) non freed.\n", MEMORY_count);
		while (_alloc)
		{
			fprintf(MEMORY_log, "<%d>\n", _alloc->id);
			_alloc = _alloc->next;
		}
	}
	fclose(MEMORY_log);
#else
  if (MEMORY_count)
		ERROR_warning("%d allocation(s) non freed.\n", MEMORY_count);
#endif
}

#if OPTIMIZE_MEMORY
#else

#if DEBUG_MEMORY
char *MEMORY_where_am_i(const char *file, int line, const char *func)
{
	static char buffer[256];
	
	snprintf(buffer, sizeof(buffer), "[%s] %s:%d", file, func, line);
	return buffer;
}

#endif

#if DEBUG_MEMORY
void MEMORY_alloc(void *p_ptr, size_t size, const char *src)
{
  ALLOC *alloc;

  alloc = (ALLOC *)malloc(sizeof(ALLOC) + size);
  if (!alloc)
    THROW(E_MEMORY);

  _id++;
  alloc->id = _id;
  alloc->prev = NULL;
  alloc->next = _alloc;
  alloc->size = size;

  if (_alloc)
    _alloc->prev = alloc;
      
  _alloc = alloc;
    
  *((void **)p_ptr) = (char *)alloc + sizeof(ALLOC);
  MEMORY_count++;
  MEMORY_size += size;

  #ifndef DO_NOT_PRINT_MEMORY
	#ifdef PROJECT_EXEC
  fprintf(MEMORY_log, "%s: ", DEBUG_get_current_position());
	#endif
  fprintf(MEMORY_log, "<%d> %s: MEMORY_alloc(%d) -> %p\n", _id, src, (int)size, (char *)alloc + sizeof(ALLOC));
  fflush(MEMORY_log);
	/*if (_id == 2700)
		BREAKPOINT();*/
  #endif
}
#else
void MEMORY_alloc(void *p_ptr, size_t size)
{
  void *alloc;

  alloc = malloc(size);

  if (!alloc)
    THROW(E_MEMORY);

  *((void **)p_ptr) = alloc;
  MEMORY_count++;
}
#endif


#if DEBUG_MEMORY
void MEMORY_alloc_zero(void *p_ptr, size_t size, const char *src)
{
  MEMORY_alloc(p_ptr, size, src);
  memset(*((void **)p_ptr), 0, size);
}
#else
void MEMORY_alloc_zero(void *p_ptr, size_t size)
{
  void *alloc;

  alloc = calloc(size, 1);

  if (!alloc)
    THROW(E_MEMORY);

  *((void **)p_ptr) = alloc;
  MEMORY_count++;
}
#endif


#if DEBUG_MEMORY
void MEMORY_realloc(void *p_ptr, size_t size, const char *src)
{
  ALLOC *alloc = (ALLOC *)(*((char **)p_ptr) - sizeof(ALLOC));
  ALLOC *old = alloc;
  
	if (size == 0)
	{
		MEMORY_free(p_ptr, src);
		return;
	}
	
  alloc = realloc(alloc, sizeof(ALLOC) + size);
  
  if (!alloc)
    THROW(E_MEMORY);

  MEMORY_size += size - alloc->size;
  
  if (_alloc == old)
    _alloc = alloc;
  
  if (alloc->prev)
    alloc->prev->next = alloc;
  if (alloc->next)
    alloc->next->prev = alloc;
  
  #ifndef DO_NOT_PRINT_MEMORY
	#ifdef PROJECT_EXEC
  fprintf(MEMORY_log, "%s: ", DEBUG_get_current_position());
	#endif
	fprintf(MEMORY_log, "<%d> %s: MEMORY_realloc(%p, %d) -> %p\n", alloc->id, src, *((void **)p_ptr), (int)size, (char *)alloc + sizeof(ALLOC));
  fflush(MEMORY_log);
  #endif
  
  *((void **)p_ptr) = (char *)alloc + sizeof(ALLOC);
}
#else
void MEMORY_realloc(void *p_ptr, size_t size)
{
  void *alloc = *((void **)p_ptr);

	if (size == 0)
	{
		MEMORY_free(p_ptr);
		return;
	}
	
  alloc = realloc(alloc, size);

  if (!alloc)
    THROW(E_MEMORY);

  *((void **)p_ptr) = alloc;
}
#endif


#if DEBUG_MEMORY
void MEMORY_free(void *p_ptr, const char *src)
{
  ALLOC *alloc = (ALLOC *)(*((char **)p_ptr) - sizeof(ALLOC));

	if (!(*((void **)p_ptr)))
		return;

  if (alloc->prev)
    alloc->prev->next = alloc->next;
  if (alloc->next)
    alloc->next->prev = alloc->prev;
    
  if (alloc == _alloc)
    _alloc = alloc->next;
  
  #ifndef DO_NOT_PRINT_MEMORY
	#ifdef PROJECT_EXEC
  fprintf(MEMORY_log, "%s: ", DEBUG_get_current_position());
	#endif
  fprintf(MEMORY_log, "<%d> %s: MEMORY_free(%p)\n", alloc->id, src, (char *)alloc + sizeof(ALLOC));
  fflush(MEMORY_log);
  #endif

  MEMORY_size -= alloc->size;
    
  //*((long *)alloc) = 0x31415926;
  free(alloc);

  *((void **)p_ptr) = NULL;
  MEMORY_count--;
}
#else
void MEMORY_free(void *p_ptr)
{
  void *alloc = *((void **)p_ptr);

	if (!alloc)
		return;
		
  free(alloc);

  *((void **)p_ptr) = NULL;
  MEMORY_count--;
}
#endif

#endif // OPTIMIZE_MEMORY

#if OPTIMIZE_MEMORY

//void DEBUG_print_current_backtrace(void);
//static bool _print_backtrace = FALSE;

void *my_malloc(size_t len)
{
	size_t *ptr;
	int size = REAL_SIZE((int)len + sizeof(size_t));
	int pool = (size / SIZE_INC) - 1;
	
	MEMORY_count++;
	
	if (pool < POOL_SIZE)
	{
		if (_pool_count[pool])
		{
			ptr = _pool[pool];
			#if defined(DEBUG_ME) && !defined(DO_NOT_PRINT_MEMORY)
			MEMORY_pool_size -= size;
			fprintf(stderr, "my_malloc: %d bytes from pool #%d -> %p (%p) [%ld / %ld]\n", size, pool, ptr + 1, sbrk(0), MEMORY_size - MEMORY_pool_size, MEMORY_size);
			#endif
			_pool[pool] = *((void **)ptr);
			_pool_count[pool]--;
			*ptr++ = size;
			return ptr;
		}
	}
	
	#ifdef DEBUG_ME
  MEMORY_size += size;
	#endif
	ptr = malloc(size);
	if (!ptr)
		THROW_MEMORY();
	#if defined(DEBUG_ME) && !defined(DO_NOT_PRINT_MEMORY)
	fprintf(stderr, "my_malloc: %d bytes from malloc -> %p (%p) [%ld / %ld]\n", size, ptr + 1, sbrk(0), MEMORY_size - MEMORY_pool_size, MEMORY_size);
	#endif

	/*if (!_first_alloc)
		_first_alloc = ptr;
	if (ptr > _max_alloc && !_print_backtrace)
	{
		_max_alloc = ptr;
		fprintf(stderr, "%ld\n", (char *)_max_alloc - (char *)_first_alloc + size);
		_print_backtrace = TRUE;
		DEBUG_print_current_backtrace();
		_print_backtrace = FALSE;
	}*/

	*ptr++ = size;
	return ptr;
}

void my_free(void *alloc)
{
	size_t *ptr;
	int size;
	int pool;

	if (!alloc)
		return;
	
	MEMORY_count--;
	
	ptr = alloc;
	ptr--;
	
	size = (int)*ptr;
	pool = (size / SIZE_INC) - 1;

	//if (ptr < (_first_alloc + (_max_alloc - _first_alloc) / 2))
	if (1)
	{
		if (pool < POOL_SIZE)
		{
			if (_pool_count[pool] < POOL_MAX)
			{
				#if defined(DEBUG_ME) && !defined(DO_NOT_PRINT_MEMORY)
				MEMORY_pool_size += size;
				fprintf(stderr, "my_free: %p (%p) -> %d bytes to pool #%d [%ld / %ld]\n", alloc, sbrk(0), size, pool, MEMORY_size - MEMORY_pool_size, MEMORY_size);
				#endif
				*((void **)ptr) = _pool[pool];
				_pool[pool] = ptr;
				_pool_count[pool]++;
				return;
			}
		}
	}

	#ifdef DEBUG_ME
  MEMORY_size -= size;
		#ifndef DO_NOT_PRINT_MEMORY
		fprintf(stderr, "my_free: %p (%p) -> %d bytes freed [%ld / %ld]\n", alloc, sbrk(0), size, MEMORY_size - MEMORY_pool_size, MEMORY_size);
		#endif
	#endif
	free(ptr);
}

void *my_realloc(void *alloc, size_t new_len)
{
	size_t *ptr;
	int size;
	int new_size;
	
	if (!alloc)
		return my_malloc(new_len);

	ptr = alloc;
	ptr--;
	size = (int)*ptr;
	new_size = REAL_SIZE(new_len + sizeof(size_t));
	
	if (size == new_size)
		return alloc;
	
	#if defined(DEBUG_ME) && !defined(DO_NOT_PRINT_MEMORY)
	fprintf(stderr, "my_realloc: %p (%p) -> %d ==> %d\n", alloc, sbrk(0), size, new_size);
	#endif

	if (new_len == 0)
	{
		my_free(alloc);
		return NULL;
	}
	else if (size > POOL_MAX_LEN && new_size > POOL_MAX_LEN)
	{
		#ifdef DEBUG_ME
		MEMORY_size += new_size - size;
			#ifndef DO_NOT_PRINT_MEMORY
			fprintf(stderr, "my_realloc: %p (%p) -> %d bytes reallocated to %d [%ld / %ld]\n", alloc, sbrk(0), size, new_size, MEMORY_size - MEMORY_pool_size, MEMORY_size);
			#endif
		#endif
		ptr = realloc(ptr, new_size);
		if (!ptr)
			THROW_MEMORY();
		*ptr++ = new_size;
		return ptr;
	}
	else
	{
		size_t *nptr = my_malloc(new_len);
		
		nptr--;
		
		if (new_size < size)
			memcpy(nptr, ptr, new_size);
		else
			memcpy(nptr, ptr, size);
		
		my_free(alloc);
		
		*nptr++ = new_size;
		return nptr;
	}
}

#endif
