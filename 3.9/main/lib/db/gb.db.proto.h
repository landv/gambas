/***************************************************************************

  gb.db.proto.h

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

#ifndef __GB_DB_PROTO_H
#define __GB_DB_PROTO_H

#include "gb.db.h"

static const char *get_quote(void);
static int open_database(DB_DESC *desc,  DB_DATABASE *db);
static void close_database(DB_DATABASE *db);
static GB_ARRAY get_collations(DB_DATABASE *db);
static int format_value(GB_VALUE *arg, DB_FORMAT_CALLBACK add);
static void format_blob(DB_BLOB *blob, DB_FORMAT_CALLBACK add);
static int exec_query(DB_DATABASE *db, const char *query, DB_RESULT *result, const char *err);
static void query_init(DB_RESULT result, DB_INFO *info, int *count);
static void query_release(DB_RESULT result, DB_INFO *info);
static int query_fill(DB_DATABASE *db, DB_RESULT result, int pos, GB_VARIANT_VALUE *buffer, int next);
static void blob_read(DB_RESULT result, int pos, int field, DB_BLOB *blob);
static char *field_name(DB_RESULT result, int field);
static int field_index(DB_RESULT result, const char *name, DB_DATABASE *db);
static GB_TYPE field_type(DB_RESULT result, int field);
static int field_length(DB_RESULT result, int field);
static int begin_transaction(DB_DATABASE *db);
static int commit_transaction(DB_DATABASE *db);
static int rollback_transaction(DB_DATABASE *db);
static int field_info(DB_DATABASE *db, const char *table, const char *field, DB_FIELD *info);
static int table_init(DB_DATABASE *db, const char *table, DB_INFO *info);
static int table_index(DB_DATABASE *db, const char *table, DB_INFO *info);
static void table_release(DB_DATABASE *db, DB_INFO *info);
static int table_exist(DB_DATABASE *db, const char *table);
static int table_list(DB_DATABASE *db, char ***tables);
static int table_primary_key(DB_DATABASE *db, const char *table, char ***primary);
static int table_is_system(DB_DATABASE *db, const char *table);
static char *table_type(DB_DATABASE *db, const char *table, const char *type);
static int table_delete(DB_DATABASE *db, const char *table);
static int table_create(DB_DATABASE *db, const char *table, DB_FIELD *fields, char **primary, const char *type);
static int field_exist(DB_DATABASE *db, const char *table, const char *field);
static int field_list(DB_DATABASE *db, const char *table, char ***fields);
static int field_info(DB_DATABASE *db, const char *table, const char *field, DB_FIELD *info);
static int index_exist(DB_DATABASE *db, const char *table, const char *index);
static int index_list(DB_DATABASE *db, const char *table, char ***indexes);
static int index_info(DB_DATABASE *db, const char *table, const char *index, DB_INDEX *info);
static int index_delete(DB_DATABASE *db, const char *table, const char *index);
static int index_create(DB_DATABASE *db, const char *table, const char *index, DB_INDEX *info);
static int database_exist(DB_DATABASE *db, const char *name);
static int database_list(DB_DATABASE *db, char ***databases);
static int database_is_system(DB_DATABASE *db, const char *name);
static int database_delete(DB_DATABASE *db, const char *name);
static int database_create(DB_DATABASE *db, const char *name);
static int user_exist(DB_DATABASE *db, const char *name);
static int user_list(DB_DATABASE *db, char ***users);
static int user_info(DB_DATABASE *db, const char *name, DB_USER *info );
static int user_delete(DB_DATABASE *db, const char *name);
static int user_create(DB_DATABASE *db, const char *name, DB_USER *info);
static int user_set_password(DB_DATABASE *db, const char *name, const char *password);

#define DECLARE_DRIVER(_driver, _name) \
static DB_DRIVER _driver = \
{ \
  _name, \
  open_database, \
  close_database, \
  format_value, \
  format_blob, \
  exec_query, \
  begin_transaction, \
  commit_transaction, \
  rollback_transaction, \
  get_collations, \
  get_quote, \
  { \
    query_init, \
    query_fill, \
    blob_read, \
    query_release, \
    { \
      field_type, \
      field_name, \
      field_index, \
      field_length, \
    }, \
  }, \
  { \
    field_exist, \
    field_list, \
    field_info, \
  }, \
  { \
    table_init, \
    table_index, \
    table_release, \
    table_exist, \
    table_list, \
    table_primary_key, \
    table_is_system, \
    table_type, \
    table_delete, \
    table_create, \
  }, \
  { \
    index_exist, \
    index_list, \
    index_info, \
    index_delete, \
    index_create, \
  }, \
  { \
    database_exist, \
    database_list, \
    database_is_system, \
    database_delete, \
    database_create, \
  }, \
  { \
    user_exist, \
    user_list, \
    user_info, \
    user_delete, \
    user_create, \
    user_set_password \
  } \
};


#endif
