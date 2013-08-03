/***************************************************************************

  main.h

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

#ifndef __MAIN_H
#define __MAIN_H

#include "gb_common.h"
#include "gambas.h"
#include "gb.db.h"
#include "c_subcollection.h"
#include "CResult.h"

#ifndef __MAIN_C
extern GB_INTERFACE GB;
extern DB_DATABASE *DB_CurrentDatabase;
#endif

#define MAX_DRIVER 8

bool DB_Open(DB_DESC *desc, DB_DRIVER **driver, DB_DATABASE *db);
char *DB_MakeQuery(DB_DRIVER *driver, const char *pattern, int len, int narg, GB_VALUE *arg);
void DB_Format(DB_DRIVER *driver, GB_VALUE *arg, DB_FORMAT_CALLBACK func);
void DB_FormatVariant(DB_DRIVER *driver, GB_VARIANT_VALUE *arg, DB_FORMAT_CALLBACK func);
char *DB_GetQuotedTable(DB_DRIVER *driver, DB_DATABASE *db, const char *table);

void DB_LowerString(char *s);
int DB_CheckNameWith(const char *name, const char *msg, const char *more);
#define DB_CheckName(_name, _msg) DB_CheckNameWith(_name, _msg, NULL)
void DB_FreeStringArray(char ***parray);
GB_ARRAY DB_StringArrayToGambasArray(char **array);
int DB_FindStringArray(char **array, const char *elt);
void DB_SetDebug(int debug);
int DB_IsDebug(void);
void DB_TryAnother(const char *);

void q_init(void);
void q_add(const char *str);
void q_add_length(const char *str, int len);
char *q_get(void);
char *q_steal(void);
int q_length(void);

#endif /* __MAIN_H */
