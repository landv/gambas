/***************************************************************************

  gb_common.h

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

#ifndef __GB_COMMON_H
#define __GB_COMMON_H

#include "config.h"

#ifdef _GNU_SOURCE
#undef _GNU_SOURCE
#endif
#define _GNU_SOURCE 500

#define _FILE_OFFSET_BITS 64

#define _ISOC9X_SOURCE	1
#define _ISOC99_SOURCE	1
#define __USE_ISOC99	1
#define __USE_ISOC9X	1

#include <math.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <stddef.h>
#include <stdint.h>
#include <inttypes.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/param.h>
#include <limits.h>

#if defined(__GNU_LIBRARY__) || defined(OS_FREEBSD)

#include <getopt.h>
#define HAVE_GETOPT_LONG 1

#endif

#if defined(OS_CYGWIN)

	typedef void (*sighandler_t) (int);
	typedef unsigned long ulong;

#endif

#if defined(OS_FREEBSD) || defined(OS_OPENBSD)

	/* sighandler_t is replaced by sig_t */
	#define sighandler_t sig_t

	typedef unsigned long ulong;

	#if (defined(__amd64__) || defined(__ia64__) || defined(__sparc64__))
		#define __WORDSIZE 64
	#endif

#endif

#ifdef OS_SOLARIS

/* PGS: The following #define prevents /usr/include/sys/mman.h on solaris
		from #define'ing PRIVATE to 0x20, thus breaking Gambas.
		Perhaps Gambas should use a different name?
		BM: I don't use PRIVATE anymore!
*/
	#ifdef _POSIX_C_SOURCE
	/* PGS: Stop compiler warnings when gcc on solaris does remember to define
		_POSIX_C_SOURCE, e.g. when compiling qt related files. */
		#undef _POSIX_C_SOURCE
	#endif

	#define _POSIX_C_SOURCE 3
	/* Get prototype for alloca() */
	#include <alloca.h>
	/* Get definition for index() */
	#include <strings.h>

#endif

#ifdef OS_MACOSX

#include <stdbool.h>
#undef bool

#endif

#if !defined(__cplusplus)

	#ifndef FALSE
		enum
		{
			FALSE = 0,
			TRUE = 1
		};
	#endif

	#define bool char

#endif

typedef
	unsigned char uchar;
	
typedef
	size_t offset_t;
	
#define PUBLIC
#define INLINE __inline__
#define EXTERN extern
#define PACKED __attribute__((packed))
#define NORETURN __attribute__((noreturn))
#define CONST __attribute__((const))

#if __WORDSIZE == 64
#define OS_64BITS 1
#endif

#ifndef LLONG_MAX
#define LLONG_MAX 9223372036854775807LL
#endif

#ifndef PATH_MAX
#define PATH_MAX 4096
#endif

#ifndef MAXPATHLEN
#define MAXPATHLEN 4096
#endif

#define CLEAR(s) (memset(s, 0, sizeof(*s)))

/* Workaround spurious gcc warnings */
#define NO_WARNING(var) var = var

#ifndef offsetof
	#define offsetof(_type, _arg) ((size_t)&(((_type *)0)->_arg))
#endif

#define Max(a, b) ({ __typeof__(a) _a = (a), _b = (b); _a > _b ? _a : _b; })
#define Min(a, b) ({ __typeof__(a) _a = (a), _b = (b); _a < _b ? _a : _b; })
#define MinMax(v, a, b) ({ __typeof__(v) _v = (v), _a = (a), _b = (b); _v < _a ? _a : (_v > _b ? _b : _v); })

#if (defined (__i386__) || defined (__x86_64__)) && defined (__GNUC__) && __GNUC__ >= 2
	#define BREAKPOINT()	{ __asm__ __volatile__ ("int $03"); }
#elif (defined (_MSC_VER) || defined (__DMC__)) && defined (_M_IX86)
	#define BREAKPOINT()	{ __asm int 3h }G_STMT_END
#elif defined (__alpha__) && !defined(__osf__) && defined (__GNUC__) && __GNUC__ >= 2
	#define BREAKPOINT()	{ __asm__ __volatile__ ("bpt"); }
#else	/* !__i386__ && !__alpha__ */
	#define BREAKPOINT()	{ raise(SIGTRAP); }
#endif	/* __i386__ */

#define COPYRIGHT "(c) 2000-2013 Benoît Minisini\n\n" \
	"This program is free software; you can redistribute it and/or \n" \
	"modify it under the terms of the GNU General Public License as \n" \
	"published by the Free Software Foundation; either version 2, or \n" \
	"(at your option) any later version.\n\n"

//#define LIKELY(_x) __builtin_expect((_x), 1)
//#define UNLIKELY(_x) __builtin_expect((_x), 0)
#define LIKELY(_x) (_x)
#define UNLIKELY(_x) (_x)

#define $(_x) _x

#endif /* __COMMON_H */
