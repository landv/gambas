/***************************************************************************

  gb.debug.h

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
    unsigned stop : 1;
    unsigned leave : 1;
    FUNCTION *fp;
    VALUE *bp;
    VALUE *pp;
    void *op;
    CLASS *cp;
    PCODE *ec;
    VALUE *ep;
    }
  DEBUG_INFO;

enum
{
	GB_DEBUG_ACCESS_NORMAL = 0,
	GB_DEBUG_ACCESS_ARRAY = 1,
	GB_DEBUG_ACCESS_COLLECTION = 2
};
	
enum
{
	GB_DEBUG_SET_OK = 0,
	GB_DEBUG_SET_ERROR = 1,
	GB_DEBUG_SET_READ_ONLY = 2
};

typedef
	void (*GB_DEBUG_ENUM_CB)(char *key, int len);

typedef
	struct {
		void *(*GetExec)(void);
		void *(*GetStack)(int frame);
		void (*PrintError)(FILE *where, bool msgonly, bool newline);
		void (*SaveError)(void *, void *);
		void (*RestoreError)(void *, void *);
		void (*ToString)(GB_VALUE *value, char **addr, int *len);
		int (*FormatDate)(GB_DATE_SERIAL *date, int fmt_type, const char *fmt, int len_fmt, char **str, int *len_str);
		int (*FormatNumber)(double number, int fmt_type, const char *fmt, int len_fmt, char **str, int *len_str, bool local);
		bool (*GetValue)(const char *sym, int len, GB_VARIANT *ret);
		int (*SetValue)(const char *sym, int len, VALUE *value);
		void (*GetArrayValue)(GB_ARRAY array, int index, GB_VALUE *value);
		void (*EnumKeys)(void *collection, GB_DEBUG_ENUM_CB cb);
		void *(*GetNextSortedSymbol)(void *klass, int *index);
		int (*GetObjectAccessType)(void *object, CLASS *klass, int *count);
		GB_CLASS (*FindClass)(const char *name);
		int *(*GetArrayBounds)(void *array);
		void (*BreakOnError)(bool);
		void (*EnterEval)(void);
		void (*LeaveEval)(void);
		}
	GB_DEBUG_INTERFACE;

typedef
	struct {
		intptr_t version;
		DEBUG_INFO *(*Init)(GB_DEBUG_INTERFACE *debug, int fifo, const char *fifo_name);
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
		struct {
			void (*Init)(const char *path);
			void (*Add)(void *cp, void *fp, void *pc);
			void (*Exit)(void);
			void (*Begin)(void *cp, void *fp);
			void (*End)(void *cp, void *fp);
			void (*Cancel)(void);
			}
			Profile;
		void *_null;
		}
	DEBUG_INTERFACE;

#define DEBUG_OUTPUT_MAX_SIZE 65536
#define DEBUG_FIFO_PATH_MAX 64

#endif
