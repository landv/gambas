/***************************************************************************

  helper.h

  (c) 2000-2017 Benoît Minisini <g4mba5@gmail.com>

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

#ifndef __HELPER_H
#define __HELPER_H

#include "main.h"
#include "gb_buffer.h"

typedef
	struct {
		sqlite3 *handle;
		char *path;
		char *host;
		int error;
	}
	SQLITE_DATABASE;

SQLITE_DATABASE *sqlite_open_database(const char *name, const char *host);
void sqlite_close_database(SQLITE_DATABASE *db);
const char *sqlite_get_error_message(SQLITE_DATABASE *db);

typedef
	struct {
		SQLITE_DATABASE *db;
		int nrow;
		int ncol;
		char **names;
		int *types;
		int *lengths;
		char *buffer;
		int *values;
	}
	SQLITE_RESULT;

GB_TYPE sqlite_get_type(const char *type, int *length);

SQLITE_RESULT *sqlite_query_exec(SQLITE_DATABASE *db, const char *query, bool need_types);
void sqlite_query_free(SQLITE_RESULT *result);
int sqlite_query_find_field(SQLITE_RESULT *result, const char *name);

void sqlite_query_get(SQLITE_RESULT *result, int pos, int col, char **value, int *length);
char *sqlite_query_get_string(SQLITE_RESULT *result, int pos, int col);
int sqlite_query_get_int(SQLITE_RESULT *result, int pos, int col);


#endif /* __HELPER_H */

