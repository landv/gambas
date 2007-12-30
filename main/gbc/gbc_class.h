/***************************************************************************

  class.h

  Class management

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

#ifndef __GBC_CLASS_H
#define __GBC_CLASS_H

#include "gbc_type.h"
#include "gb_table.h"
#include "gbc_trans.h"

#define FUNC_INIT_STATIC    0
#define FUNC_INIT_DYNAMIC   1

typedef
  struct {
    TYPE type;
    long value;
    }
  PACKED
  CLASS_SYMBOL_INFO;

typedef
  struct {
    SYMBOL symbol;
    CLASS_SYMBOL_INFO global;
    CLASS_SYMBOL_INFO local;
    long class;
    long unknown;
    unsigned used : 1;
    unsigned _reserved : 31;
    }
  PACKED
  CLASS_SYMBOL;

typedef
  TRANS_PARAM PARAM;

typedef
  struct {
    TYPE type;
    long index;
    long pos;
    long size;
    }
  VARIABLE;

typedef
  struct {
    long num;
    long param;
    }
  VARIABLE_INIT;

typedef
  struct {
    TYPE type;
    long index;
    long value;
    long line;
    long long lvalue;
    }
  CONSTANT;

typedef
  struct {
    TYPE type;            // Return value datatype
    long name;            /* index du nom de la fonction dans la table des symboles de la classe */

    char nparam;         /* nombre de param�res */
    char npmin;          /* nombre de param�res obligatoires */
    char vararg;          /* if this function accepts extra arguments */
    char _reserved;
    short nlocal;         /* nombre de variables locales */
    short nctrl;          /* nombre de variables locales pour les structures de contr�e */
    PARAM *local;          /* Liste des variables locales */
    PARAM *param;          /* Liste des param�res */

    PATTERN *start;        /* Position d'o d�arrer la compilation de la fonction */
    long line;	            /* A quelle ligne cette position correspond ? */
    ushort *code;          /* Code compil�*/
    ushort ncode;          // Number of instruction

    short last_code;      /* position de la derni�e instruction compil� */
    short stack;          /* consommation de pile */
    short finally;        /* position de l'instruction finally */
    short catch;         /* position de l'instruction catch */

    short *pos_line;      /* position de chaque ligne de code de la fonction */
    short _reserved2[3];
    }
  PACKED
  FUNCTION;

typedef
  struct {
    TYPE type;            /* type de la valeur de retour */
    long name;           /* index du nom de la fonction dans la table des symboles de la classe */
    PARAM *param;         /* Liste des param�res */
    short nparam;        /* nombre de param�res */
    short _reserved;
    }
  PACKED
  EVENT;

typedef
  struct {
    TYPE type;            /* type de la valeur de retour */
    long name;            /* function name index */
    PARAM *param;         /* Liste des param�res */
    short nparam;         /* nombre de param�res */
    short _reserved;
    long library;         /* library name index */
    long alias;           /* library function name index */
    }
  PACKED
  EXTFUNC;

typedef
  struct {
    TYPE type;            /* property type */
    long name;            /* property name */
    long line;            /* the line where the property is declared */
    long comment;         /* property string description, added to datatype */
    short read;           /* read function */
    short write;          /* write function */
    }
  PACKED
  PROPERTY;

typedef
  struct {
    TYPE type;                /* Type du tableau */
    long ndim;               /* nombre de dimensions */
    long dim[MAX_ARRAY_DIM];  /* dimensions du tableau */
    }
  CLASS_ARRAY;

typedef
  struct {
    long nfield;              /* nombre de champs dans la structure */
    VARIABLE *field;          /* champs de la structure */
    }
  CLASS_STRUCT;

typedef
	struct {
		long index;
		bool used;
		}
	CLASS_REF;

typedef
  struct {
    TABLE *table;             /* symbol table */
    TABLE *string;            /* strings table */
    char *name;               /* class name */
    short parent;             /* parent class */
    unsigned exported : 1;    /* class is exported */
    unsigned autocreate : 1;  /* class is auto-creatable */
    unsigned optional : 1;    /* class is optional */
    unsigned nocreate : 1;    /* class cannot be instanciated */
    unsigned _reserved : 12;
    VARIABLE *stat;           /* static variables */
    VARIABLE *dyn;            /* dynamic variables */
    CONSTANT *constant;       /* constants */
    CLASS_REF *class;         /* classes */
    long *unknown;            /* unknown symbols */
    FUNCTION *function;       /* functions */
    long size_stat;           /* static variables total size */
    long size_dyn;            /* dynamic variables total size */
    EVENT *event;             /* events */
    PROPERTY *prop;           /* properties */
    EXTFUNC *ext_func;        /* extern functions */
    CLASS_ARRAY *array;       /* array definitions */
    CLASS_STRUCT *structure;  /* structs definitions */
    }
  CLASS;


#define CLASS_get_symbol(class, ind) ((CLASS_SYMBOL *)TABLE_get_symbol((class)->table, ind))


#define FUNCTION_is_procedure(func)  (TYPE_get_id((func)->type) == T_VOID)
#define FUNCTION_is_static(func)     (TYPE_is_static((func)->type))

PUBLIC void CLASS_create(CLASS **result);
PUBLIC void CLASS_delete(CLASS **class);

PUBLIC CLASS_SYMBOL *CLASS_declare(CLASS *class, long index, boolean global);

PUBLIC void CLASS_add_function(CLASS *class, TRANS_FUNC *decl);
PUBLIC void CLASS_add_event(CLASS *class, TRANS_EVENT *decl);
PUBLIC void CLASS_add_property(CLASS *class, TRANS_PROPERTY *prop);
PUBLIC void CLASS_add_extern(CLASS *class, TRANS_EXTERN *decl);
PUBLIC void CLASS_add_declaration(CLASS *class, TRANS_DECL *decl);
PUBLIC long CLASS_add_constant(CLASS *class, TRANS_DECL *decl);
PUBLIC long CLASS_add_class(CLASS *class, long index);
PUBLIC long CLASS_add_class_unused(CLASS *class, long index);
PUBLIC bool CLASS_exist_class(CLASS *class, long index);
PUBLIC long CLASS_add_unknown(CLASS *class, long index);
PUBLIC long CLASS_add_array(CLASS *class, TRANS_ARRAY *array);

PUBLIC long CLASS_get_array_class(CLASS *class, long type);

PUBLIC void FUNCTION_add_pos_line(void);
PUBLIC char *FUNCTION_get_fake_name(int func);

PUBLIC long CLASS_add_symbol(CLASS *class, const char *name);

PUBLIC void CLASS_sort_declaration(CLASS *class);
PUBLIC void CLASS_check_properties(CLASS *class);

/* gbc_dump.c */

PUBLIC void CLASS_dump(void);
PUBLIC void CLASS_export(void);
PUBLIC void CLASS_exit_export(void);

/*
PUBLIC long CLASS_add_symbol_string(CLASS *class, char *str);
*/

#endif
