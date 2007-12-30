/***************************************************************************

  gb.db.h

  The Gambas Database component Interface

  (c) 2000-2005 Beno� Minisini <gambas@users.sourceforge.net>

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

#ifndef __GB_DB_H
#define __GB_DB_H

#include "gambas.h"

#define DB_INTERFACE_VERSION 1

typedef
  struct {
    char *type;
    char *host;
    char *port;
    char *name;
    char *user;
    char *password;
    long options;
    }
  DB_DESC;

/* LIMIT position */

#define DB_LIMIT_NONE					0
#define DB_LIMIT_AT_BEGIN     1
#define DB_LIMIT_AT_END       2

typedef
	struct {
		void *handle;										/* Connection handle */
		long version;										/* Version of the database system */
		char *charset;									/* Charset used by the database */
		void *data;											/* Can be used by the driver for storing its own private data */
		struct {
			unsigned no_table_type : 1;		/* Tables do not have types */
			unsigned no_serial : 1;				/* Serial fields are not supported */
			unsigned no_blob : 1;					/* Blob fields are not supported */
			unsigned no_seek : 1;					/* Cannot seek anywhere in a Result */
			}
			flags;
		struct {
			char *keyword;								/* keyword for limiting the result of a query */
			int pos;											/* position of 'limit' keyword */
			}
			limit;
		}
  DB_DATABASE;

typedef
  void *DB_RESULT;

typedef
  struct _DB_FIELD {
    struct _DB_FIELD *next;
    char *name;
    GB_TYPE type;             /* gambas field type */
    long length;              /* max length for text fields (0 = no limit) */
    GB_VARIANT_VALUE def;     /* default value */
    }
  DB_FIELD;


typedef
  struct {
    char *table;
    int nfield;
    int nindex;
    DB_FIELD *field;
    int *index;
    }
  DB_INFO;

typedef
  struct {
    char *name;
    char *fields;   /* list of index fields separated by commas */
    int unique;     /* index is unique */
    int primary;    /* primary index */
    }
  DB_INDEX;

typedef
  struct {
    char *name;
    char *password;
    int admin;      /* user is a superuser */
    }
  DB_USER;

typedef
	struct {
		GB_BASE ob;
		char *data;
		long length;
		int constant;
		}
	DB_BLOB;


typedef
  void (*DB_FORMAT_CALLBACK)(char *, long);


typedef
  struct {
    char *name;

    int (*Open)(DB_DESC *desc, DB_DATABASE *db);
    void (*Close)(DB_DATABASE *db);

    int (*Format)(GB_VALUE *val, DB_FORMAT_CALLBACK add);
    void (*FormatBlob)(DB_BLOB *blob, DB_FORMAT_CALLBACK add);

    int (*Exec)(DB_DATABASE *db, char *, DB_RESULT *result, char *err);

    int (*Begin)(DB_DATABASE *db);
    int (*Commit)(DB_DATABASE *db);
    int (*Rollback)(DB_DATABASE *db);
    char *(*GetQuote)(void);

    struct {
      void (*Init)(DB_RESULT result, DB_INFO *info, long *count);
      int (*Fill)(DB_RESULT result, int pos, GB_VARIANT_VALUE *buffer, int next);
      void (*Blob)(DB_RESULT result, int pos, int field, DB_BLOB *blob);
      void (*Release)(DB_RESULT result, DB_INFO *info);
      struct {
        GB_TYPE (*Type)(DB_RESULT result, int index);
        char *(*Name)(DB_RESULT result, int index);
        int (*Index)(DB_RESULT result, char *name, DB_DATABASE *db);
        int (*Length)(DB_RESULT result, int index);
        }
        Field;
      }
      Result;

    struct {
      int (*Exist)(DB_DATABASE *db, char *table, char *field);
      long (*List)(DB_DATABASE *db, char *table, char ***fields);
      int (*Info)(DB_DATABASE *db, char *table, char *field, DB_FIELD *info);
      }
      Field;

    struct {
      int (*Init)(DB_DATABASE *db, char *table, DB_INFO *info);
      int (*Index)(DB_DATABASE *db, char *table, DB_INFO *info);
      void (*Release)(DB_DATABASE *db, DB_INFO *info);
      int (*Exist)(DB_DATABASE *db, char *table);
      long (*List)(DB_DATABASE *db, char ***tables);
      int (*PrimaryKey)(DB_DATABASE *db, char *table, char ***primary);
      int (*IsSystem)(DB_DATABASE *db, char *table);
      char *(*Type)(DB_DATABASE *db, char *table, char *type);
      int (*Delete)(DB_DATABASE *db, char *table);
      int (*Create)(DB_DATABASE *db, char *table, DB_FIELD *fields, char **primary, char *tabletype);
      }
      Table;

    struct {
      int (*Exist)(DB_DATABASE *db, char *table, char *index);
      long (*List)(DB_DATABASE *db, char *table, char ***indexes);
      int (*Info)(DB_DATABASE *db, char *table, char *index, DB_INDEX *info);
      int (*Delete)(DB_DATABASE *db, char *table, char *index);
      int (*Create)(DB_DATABASE *db, char *table, char *index, DB_INDEX *info);
      }
      Index;

    struct {
      int (*Exist)(DB_DATABASE *db, char *name);
      long (*List)(DB_DATABASE *db, char ***names);
      int (*IsSystem)(DB_DATABASE *db, char *name);
      int (*Delete)(DB_DATABASE *db, char *name);
      int (*Create)(DB_DATABASE *db, char *name);
      }
      Database;

    struct {
      int (*Exist)(DB_DATABASE *db, char *user);
      long (*List)(DB_DATABASE *db, char ***users);
      int (*Info)(DB_DATABASE *db, char *user, DB_USER *info);
      int (*Delete)(DB_DATABASE *db, char *user);
      int (*Create)(DB_DATABASE *db, char *user, DB_USER *info);
      int (*SetPassword)(DB_DATABASE *db, char *user, char *password);
      }
      User;
    }
  DB_DRIVER;

typedef
  struct {
    long version;
    void (*Register)(DB_DRIVER *);
    void (*Format)(DB_DRIVER *, GB_VALUE *, DB_FORMAT_CALLBACK);
    void (*FormatVariant)(DB_DRIVER *, GB_VARIANT_VALUE *, DB_FORMAT_CALLBACK);
    int (*IsDebug)(void);
    void (*TryAnother)(char *);

    struct {
      void (*Init)(void);
      void (*Add)(char *);
      void (*AddLength)(char *, long);
      char *(*Get)(void);
      char *(*GetNew)(void);
      long (*Length)(void);
      }
      Query;

    struct {
      int (*Find)(char **, char *);
      }
      StringArray;
    }
  DB_INTERFACE;

/* Field datatypes */

#define DB_T_SERIAL ((GB_TYPE)-1)
#define DB_T_BLOB   ((GB_TYPE)-2)

/* Field Separator Character e.g. Table.field = Table.field */

#define FLD_SEP '.'

#endif /* __MAIN_H */
