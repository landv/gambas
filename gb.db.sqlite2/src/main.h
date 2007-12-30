/***************************************************************************

  main.h

  MySQL driver

  Hacked by Nigel Gerrard from original code by
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

#ifndef __MAIN_H
#define __MAIN_H

extern "C" {
#include "gambas.h"
#include "gb_common.h"
#include "../gb.db.h"

#ifndef __MAIN_C
extern GB_INTERFACE GB;
extern DB_INTERFACE DB;
#endif
}

#define QUOTE_STRING "'"

#define MAX_PATH 132 /* MAX LENGTH OF FILNAME PATH */
#define TRUE 1
#define FALSE 0

/* Prototypes Required to allow cpp compilation */
static char *get_quote(void);
static int open_database(DB_DESC *desc, DB_DATABASE *db);
static void close_database(DB_DATABASE *db);
static int format_value(GB_VALUE *arg, DB_FORMAT_CALLBACK add);
static void format_blob(DB_BLOB *blob, DB_FORMAT_CALLBACK add);
static int exec_query(DB_DATABASE *db, char *query, DB_RESULT *result, char *err);
static void query_init(DB_RESULT result, DB_INFO *info, long *count);
static void query_release(DB_RESULT result, DB_INFO *info);
static int query_fill(DB_RESULT result, int pos, GB_VARIANT_VALUE *buffer, int next);
static void blob_read(DB_RESULT result, int pos, int field, DB_BLOB *blob);
static char *field_name(DB_RESULT result, int field);
static int field_index(DB_RESULT result, char *name, DB_DATABASE *db);
static GB_TYPE field_type(DB_RESULT result, int field);
static int field_length(DB_RESULT result, int field);
static int begin_transaction(DB_DATABASE *db);
static int commit_transaction(DB_DATABASE *db);
static int rollback_transaction(DB_DATABASE *db);
static int table_init(DB_DATABASE *db, char *table, DB_INFO *info);
static int table_index(DB_DATABASE *db, char *table, DB_INFO *info);
static void table_release(DB_DATABASE *db, DB_INFO *info);
static int table_exist(DB_DATABASE *db, char *table);
static long table_list(DB_DATABASE *db, char ***tables);
static int table_primary_key(DB_DATABASE *db, char *table, char ***primary);
static int table_is_system(DB_DATABASE *db, char *table);
static char *table_type(DB_DATABASE *db, char *table, char *type);
static int table_delete(DB_DATABASE *db, char *table);
static int table_create(DB_DATABASE *db, char *table, DB_FIELD *fields, char **primary, char *not_used);
static int field_exist(DB_DATABASE *db, char *table, char *field);
static long field_list(DB_DATABASE *db, char *table, char ***fields);
static int field_info(DB_DATABASE *db, char *table, char *field, DB_FIELD *info);
static int index_exist(DB_DATABASE *db, char *table, char *index);
static long index_list(DB_DATABASE *db, char *table, char ***indexes);
static int index_info(DB_DATABASE *db, char *table, char *index, DB_INDEX *info);
static int index_delete(DB_DATABASE *db, char *table, char *index);
static int index_create(DB_DATABASE *db, char *table, char *index, DB_INDEX *info);
static int database_exist(DB_DATABASE *db, char *name);
static long database_list(DB_DATABASE *db, char ***databases);
static int database_is_system(DB_DATABASE *db, char *name);
static int database_delete(DB_DATABASE *db, char *name);
static int database_create(DB_DATABASE *db, char *name);
static int user_exist(DB_DATABASE *db, char *name);
static long user_list(DB_DATABASE *db, char ***users);
static int user_info(DB_DATABASE *db, char *name, DB_USER *info );
static int user_delete(DB_DATABASE *db, char *name);
static int user_create(DB_DATABASE *db, char *name, DB_USER *info);
static int user_set_password(DB_DATABASE *db, char *name, char *password);
#endif /* __MAIN_H */
