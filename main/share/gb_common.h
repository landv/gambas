/***************************************************************************

  common.h

  Common useful macros

  Copyright (c) 2000-2007 Benoit Minisini <gambas@freesurf.fr>

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

#ifndef __GB_COMMON_H
#define __GB_COMMON_H

#include "config.h"

#ifdef _GNU_SOURCE
#undef _GNU_SOURCE
#endif
#define _GNU_SOURCE 500
#define _FILE_OFFSET_BITS 64

#include <math.h>
#ifdef OS_FREEBSD
#include <mathl.h>
#endif
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

#ifdef OS_CYGWIN

typedef void (*sighandler_t) (int);

#endif

#if defined(OS_FREEBSD) || defined(OS_OPENBSD)

  /* Get definition for PATH_MAX */
  #include <limits.h>
  /* sighandler_t is replaced by sig_t */
  #define sighandler_t sig_t

  /* modfl is not yet implemented */
  # define modfl(a,b)	modf(a,b)

#endif

#ifdef OS_FREEBSD
  /* finite is replaced by isfinite */
  #define finite isfinite
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
  /* Get definition for PATH_MAX */
  #include <limits.h>
  /* Get definition for index() */
  #include <strings.h>

#endif

#if !defined(__cplusplus)

  #ifndef FALSE
    enum
    {
      FALSE = 0,
      TRUE = 1
    };
  #endif

  typedef
    char boolean;

  typedef
    char bool;

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

#define CLEAR(s) (memset(s, 0, sizeof(*s)))

/* Workaround spurious gcc warnings */
#define NO_WARNING(var) var = var

#ifndef offsetof
  #define offsetof(_type, _arg) ((size_t)&(((_type *)0)->_arg))
#endif

#define Max(a, b) ({ int _a = (a), _b = (b); _a > _b ? _a : _b; })
#define Min(a, b) ({ int _a = (a), _b = (b); _a < _b ? _a : _b; })
#define MinMax(v, a, b) ({ int _v = (v), _a = (a), _b = (b); _v < _a ? _a : (_v > _b ? _b : _v); })

#if (defined (__i386__) || defined (__x86_64__)) && defined (__GNUC__) && __GNUC__ >= 2
#  define BREAKPOINT()	{ __asm__ __volatile__ ("int $03"); }
#elif (defined (_MSC_VER) || defined (__DMC__)) && defined (_M_IX86)
#  define BREAKPOINT()	{ __asm int 3h }G_STMT_END
#elif defined (__alpha__) && !defined(__osf__) && defined (__GNUC__) && __GNUC__ >= 2
#  define BREAKPOINT()	{ __asm__ __volatile__ ("bpt"); }
#else	/* !__i386__ && !__alpha__ */
#  define BREAKPOINT()	{ raise (SIGTRAP); }
#endif	/* __i386__ */


#define COPYRIGHT "(c) 2000-2007 Benoit Minisini\n\n" \
  "This program is free software; you can redistribute it and/or \n" \
  "modify it under the terms of the GNU General Public License as \n" \
  "published by the Free Software Foundation; either version 2, or \n" \
  "(at your option) any later version.\n\n"

#endif /* __COMMON_H */
