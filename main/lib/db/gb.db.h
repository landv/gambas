/***************************************************************************

	gb.db.h

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
		int options;
		}
	DB_DESC;

/* LIMIT position */

#define DB_LIMIT_NONE					0
#define DB_LIMIT_AT_BEGIN     1
#define DB_LIMIT_AT_END       2

typedef
	struct {
		void *handle;                   /* Connection handle */
		int version;                    /* Version of the database system */
		char *charset;                  /* Charset used by the database */
		void *data;                     /* Can be used by the driver for storing its own private data */
		int error;                      /* Last SQL error code raise by a query */
		int timeout;                    /* Connection timeout */
		unsigned ignore_case : 1;       /* If table, field and index names are case sensitive */
		struct {
			unsigned no_table_type : 1;   /* Tables do not have types */
			unsigned no_serial : 1;       /* Serial fields are not supported */
			unsigned no_blob : 1;         /* Blob fields are not supported */
			unsigned no_seek : 1;         /* Cannot seek anywhere in a Result */
			unsigned no_nest : 1;         /* Cannot nest transactions */
			unsigned no_case : 1;         /* table, field and index names must be converted to lower case */
			unsigned schema : 1;          /* If table names can be prefixed by a schema name and a dot */
			unsigned no_collation : 1;    /* No collation support at field level */
			unsigned system : 1;          /* system database */
			}
			flags;
		struct {
			const char *keyword;          /* keyword for limiting the result of a query */
			int pos;                      /* position of 'limit' keyword */
			}
			limit;
		const char *db_name_char;       /* These characters are allowed in a database name */
		}
	DB_DATABASE;

typedef
	void *DB_RESULT;

typedef
	struct _DB_FIELD {
		struct _DB_FIELD *next;
		char *name;
		GB_TYPE type;             /* gambas field type */
		int length;               /* max length for text fields (0 = no limit) */
		GB_VARIANT_VALUE def;     /* default value */
		char *collation;          /* field collation */
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
		int length;
		int constant;
		}
	DB_BLOB;

typedef
	void (*DB_FORMAT_CALLBACK)(const char *, int);

typedef
	void (*DB_SUBST_CALLBACK)(int, char **, int *, char);

typedef
	struct {
		const char *name;

		int (*Open)(DB_DESC *desc, DB_DATABASE *db);
		void (*Close)(DB_DATABASE *db);

		int (*Format)(GB_VALUE *val, DB_FORMAT_CALLBACK add);
		void (*FormatBlob)(DB_BLOB *blob, DB_FORMAT_CALLBACK add);

		int (*Exec)(DB_DATABASE *db, const char *, DB_RESULT *result, const char *err);

		int (*Begin)(DB_DATABASE *db);
		int (*Commit)(DB_DATABASE *db);
		int (*Rollback)(DB_DATABASE *db);
		GB_ARRAY (*GetCollations)(DB_DATABASE *db);
		const char *(*GetQuote)(void);

		struct {
			void (*Init)(DB_RESULT result, DB_INFO *info, int *count);
			int (*Fill)(DB_DATABASE *db, DB_RESULT result, int pos, GB_VARIANT_VALUE *buffer, int next);
			void (*Blob)(DB_RESULT result, int pos, int field, DB_BLOB *blob);
			void (*Release)(DB_RESULT result, DB_INFO *info);
			struct {
				GB_TYPE (*Type)(DB_RESULT result, int index);
				char *(*Name)(DB_RESULT result, int index);
				int (*Index)(DB_RESULT result, const char *name, DB_DATABASE *db);
				int (*Length)(DB_RESULT result, int index);
				}
				Field;
			}
			Result;

		struct {
			int (*Exist)(DB_DATABASE *db, const char *table, const char *field);
			int (*List)(DB_DATABASE *db, const char *table, char ***fields);
			int (*Info)(DB_DATABASE *db, const char *table, const char *field, DB_FIELD *info);
			}
			Field;

		struct {
			int (*Init)(DB_DATABASE *db, const char *table, DB_INFO *info);
			int (*Index)(DB_DATABASE *db, const char *table, DB_INFO *info);
			void (*Release)(DB_DATABASE *db, DB_INFO *info);
			int (*Exist)(DB_DATABASE *db, const char *table);
			int (*List)(DB_DATABASE *db, char ***tables);
			int (*PrimaryKey)(DB_DATABASE *db, const char *table, char ***primary);
			int (*IsSystem)(DB_DATABASE *db, const char *table);
			char *(*Type)(DB_DATABASE *db, const char *table, const char *type);
			int (*Delete)(DB_DATABASE *db, const char *table);
			int (*Create)(DB_DATABASE *db, const char *table, DB_FIELD *fields, char **primary, const char *tabletype);
			}
			Table;

		struct {
			int (*Exist)(DB_DATABASE *db, const char *table, const char *index);
			int (*List)(DB_DATABASE *db, const char *table, char ***indexes);
			int (*Info)(DB_DATABASE *db, const char *table, const char *index, DB_INDEX *info);
			int (*Delete)(DB_DATABASE *db, const char *table, const char *index);
			int (*Create)(DB_DATABASE *db, const char *table, const char *index, DB_INDEX *info);
			}
			Index;

		struct {
			int (*Exist)(DB_DATABASE *db, const char *name);
			int (*List)(DB_DATABASE *db, char ***names);
			int (*IsSystem)(DB_DATABASE *db, const char *name);
			int (*Delete)(DB_DATABASE *db, const char *name);
			int (*Create)(DB_DATABASE *db, const char *name);
			}
			Database;

		struct {
			int (*Exist)(DB_DATABASE *db, const char *user);
			int (*List)(DB_DATABASE *db, char ***users);
			int (*Info)(DB_DATABASE *db, const char *user, DB_USER *info);
			int (*Delete)(DB_DATABASE *db, const char *user);
			int (*Create)(DB_DATABASE *db, const char *user, DB_USER *info);
			int (*SetPassword)(DB_DATABASE *db, const char *user, const char *password);
			}
			User;
		}
	DB_DRIVER;

typedef
	struct {
		intptr_t version;
		void (*Register)(DB_DRIVER *);
		void (*Format)(DB_DRIVER *, GB_VALUE *, DB_FORMAT_CALLBACK);
		void (*FormatVariant)(DB_DRIVER *, GB_VARIANT_VALUE *, DB_FORMAT_CALLBACK);
		int (*IsDebug)(void);
		void (*TryAnother)(const char *);
		char *(*SubstString)(const char *, int, DB_SUBST_CALLBACK);
		char *(*QuoteString)(const char *, int, char);
		char *(*UnquoteString)(const char *, int, char);
		DB_DATABASE *(*GetCurrentDatabase)();

		struct {
			void (*Init)(void);
			void (*Add)(const char *);
			void (*AddLower)(const char *);
			void (*AddLength)(const char *, int);
			char *(*Get)(void);
			char *(*GetNew)(void);
			int (*Length)(void);
			}
			Query;

		struct {
			int (*Find)(char **, const char *);
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
