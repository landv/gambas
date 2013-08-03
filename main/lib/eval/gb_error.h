/***************************************************************************

  gb_error.h

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

#ifndef __GB_ERROR_H
#define __GB_ERROR_H

#include <errno.h>
#include <setjmp.h>

#include "gb_limit.h"

#define NO_ERROR_HANDLER

#include "gb_error_common.h"

#define E_MEMORY    "Out of memory"
#define E_SYNTAX    "Syntax error"

#ifndef __GB_ERROR_C
EXTERN ERROR_CONTEXT *ERROR_current;
#endif

void ERROR_clear(void);
char *ERROR_get(void);
void ERROR_reset(ERROR_INFO *info);

//void ERROR_enter(ERROR_CONTEXT *err);
//void ERROR_leave(ERROR_CONTEXT *err);

void PROPAGATE() NORETURN;
void THROW(const char *msg) NORETURN;
void THROW2(const char *pattern, const char *msg) NORETURN;

void ERROR_panic(const char *error, ...) NORETURN;

void ERROR_print(void);
void ERROR_print_at(FILE *where);

#endif
