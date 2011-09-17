/***************************************************************************

  gb.debug.h

  (c) 2000-2007 Benoit Minisini <gambas@freesurf.fr>

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

#ifndef __GB_DEBUG_H
#define __GB_DEBUG_H

#include <stdlib.h>
#include "gambas.h"
#include "gbx_class.h"
#include "gbx_value.h"
#include "gb_pcode.h"

#define DEBUG_INTERFACE_VERSION 1

typedef
  struct {
    FUNCTION *fp;
    VALUE *bp;
    void *op;
    CLASS *cp;
    PCODE *ec;
    VALUE *ep;
    unsigned stop : 1;
    unsigned leave : 1;
		unsigned _reserved : 30;
    }
  DEBUG_INFO;

typedef
	struct {
		void *(*GetExec)(void);
		void *(*GetStack)(int frame);
		void (*PrintError)(FILE *where, bool msgonly, bool newline);
		void (*SaveError)(void *);
		void (*RestoreError)(void *);
		void (*ToString)(GB_VALUE *value, char **addr, int *len);
		bool (*FormatDate)(GB_DATE_SERIAL *date, int fmt_type, const char *fmt, int len_fmt, char **str, int *len_str);
		bool (*FormatNumber)(double number, int fmt_type, const char *fmt, int len_fmt, char **str, int *len_str, bool local);
		bool (*GetValue)(const char *sym, int len, GB_VARIANT *ret);
		void (*GetArrayValue)(GB_ARRAY array, int index, GB_VALUE *value);
		bool (*EnumCollection)(GB_COLLECTION col, GB_VARIANT *value, char **key, int *len);
		void *(*GetNextSortedSymbol)(void *klass, int *index);
		bool (*CanBeUsedLikeAnArray)(void *object, CLASS *klass);
		GB_CLASS (*FindClass)(const char *name);
		}
	GB_DEBUG_INTERFACE;

typedef
	struct {
		intptr_t version;
		DEBUG_INFO *(*Init)(GB_DEBUG_INTERFACE *debug, int fifo);
		void (*Exit)(void);
		void (*Welcome)(void);
		void (*Main)(int error);
		void (*Where)(void);
		void (*Backtrace)(FILE *out);
		void (*Breakpoint)(int id);
		void (*BreakOnNextLine)(void);
		const char *(*GetPosition)(void *klass, void *func, void *pcode);
		const char *(*GetCurrentPosition)(void);
		void (*InitBreakpoints)(void *klass);
		void *_null;
		}
	DEBUG_INTERFACE;

#endif
