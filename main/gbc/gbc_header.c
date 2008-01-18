/***************************************************************************

  header.c

  Analyzing class description

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

#define _TRANS_HEADER_C

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "gb_common.h"
#include "gb_error.h"
#include "gb_str.h"
#include "gb_file.h"
#include "gb_table.h"

#include "gbc_compile.h"
#include "gbc_trans.h"
#include "gbc_header.h"

/*#define DEBUG*/


static char *get_num_desc(int num)
{
  static char *num_desc[3] = { "first", "second", "third" };
  static char desc[6];

  if (num < 1)
    return NULL;

  if (num < 4)
    return num_desc[num - 1];

  snprintf(desc, sizeof(desc), "%dth", num);
  return desc;
}


static void analyze_function_desc(TRANS_FUNC *func, int flag)
{
  PATTERN *look = JOB->current;
  TRANS_PARAM *param;
  bool is_output;
  bool is_optional = FALSE;
  TRANS_DECL ttyp;

  if (!PATTERN_is_identifier(*look))
    THROW("Syntax error. Invalid identifier in function name");

  func->index = PATTERN_index(*look);
  look++;

  if (flag & HF_EVENT)
    TABLE_copy_symbol_with_prefix(JOB->class->table, func->index, ':', NULL, &func->index);

  func->nparam = 0;
  func->vararg = FALSE;

  if ((flag & HF_VOID) && PATTERN_is_newline(*look))
  {
    JOB->current = ++look;
    return;
  }

  if (!PATTERN_is(*look, RS_LBRA))
    THROW(E_UNEXPECTED, READ_get_pattern(look));
  look++;

  for(;;)
  {
    param = &func->param[func->nparam];

    if (PATTERN_is(*look, RS_RBRA))
    {
      look++;
      break;
    }

    if (func->nparam > 0)
    {
      if (!PATTERN_is(*look, RS_COMMA))
        THROW("Syntax error. Missing ',' or ')'");
      look++;
    }

    if (PATTERN_is(*look, RS_3PTS))
    {
      look++;
      if (!PATTERN_is(*look, RS_RBRA))
        THROW("Syntax error. '...' must be the last parameter"); //, get_num_desc(func->nparam + 1));
      look++;
      func->vararg = TRUE;
      break;
    }

    is_output = FALSE;

    /* Pas de param�re en sortie pour l'instant !
    if (PATTERN_is(*look, RS_AT))
    {
      is_output = TRUE;
      look++;
    }
    */

    if (!(flag & HF_NO_OPT))
    {
      if (PATTERN_is(*look, RS_OPTIONAL))
      {
        look++;
        is_optional = TRUE;
      }
    }

    if (!PATTERN_is_identifier(*look))
      THROW("Syntax error. The &1 parameter is not a valid identifier", get_num_desc(func->nparam + 1));

    param->index = PATTERN_index(*look);
    look++;
    JOB->current = look;

    if (!TRANS_type(TT_NOTHING, &ttyp))
      THROW("Syntax error. Invalid type description of &1 parameter", get_num_desc(func->nparam + 1));

    param->type = ttyp.type;

    /*
    if (is_output)
      TYPE_set_flag(&param->type, TF_OUTPUT);
    */

    look = JOB->current;

    if (is_optional)
    {
      param->optional = look;
      for(;;)
      {
        if (PATTERN_is(*look, RS_COMMA) || PATTERN_is(*look, RS_RBRA) || PATTERN_is_newline(*look))
          break;
        look++;
      }
      JOB->current = look;
    }

    func->nparam++;
  }

  JOB->current = look;
}


static void header_module_type(void)
{
  const char *ext;

  /*JOB->class->name = STR_copy(FILE_get_name(JOB->name));*/

  ext = FILE_get_ext(JOB->name);

  if (strcasecmp(ext, "module") == 0)
  {
    JOB->is_module = TRUE;
    JOB->is_form = FALSE;
  }
  else if (strcasecmp(ext, "class") == 0)
  {
    JOB->is_module = FALSE;
    JOB->is_form = FALSE;
  }
  else if (strcasecmp(ext, "form") == 0)
  {
    JOB->is_module = FALSE;
    JOB->is_form = TRUE;
  }
  else
    THROW("Unknown file extension");

  JOB->declared = TRUE;
}


static bool header_event(TRANS_EVENT *event)
{
  PATTERN *look = JOB->current;
  TRANS_DECL ttyp;

  if (!PATTERN_is(*look, RS_EVENT))
    return FALSE;

  CLEAR(event);

  if (JOB->is_module)
    THROW("A module cannot raise events");

  JOB->current++;
  analyze_function_desc((TRANS_FUNC *)event, HF_VOID);

  if (PATTERN_is(*JOB->current, RS_AS))
  {
    if (!TRANS_type(TT_CAN_SQUARE, &ttyp))
      THROW("Syntax error in return type");
    event->type = ttyp.type;
  }

  TYPE_set_kind(&event->type, TK_EVENT);
  TYPE_set_flag(&event->type, TF_PUBLIC);

  return TRUE;
}


static bool header_property(TRANS_PROPERTY *prop)
{
  TRANS_DECL ptype;
  PATTERN *look = JOB->current;
  bool is_static = FALSE;
  bool is_public = TRUE;

  CLEAR(prop);

  /* static */

  if (JOB->is_module)
  {
    is_static = TRUE;
  }
  else if (PATTERN_is(*look, RS_STATIC))
  {
    is_static = TRUE;
    look++;
  }

  /* public */

  if (PATTERN_is(*look, RS_PUBLIC) || PATTERN_is(*look, RS_PRIVATE))
  {
    is_public = PATTERN_is(*look, RS_PUBLIC);
    look++;
  }

  if (!PATTERN_is(*look, RS_PROPERTY))
    return FALSE;
  look++;
  JOB->current = look;

  if (!is_public)
    THROW("A property must be public");

  /* read-only property */

  if (PATTERN_is(*JOB->current, RS_READ))
  {
    prop->read = TRUE;
    JOB->current++;
  }
  else
    prop->read = FALSE;

  /* property name */

  if (!PATTERN_is_identifier(*JOB->current))
    THROW("Syntax error. Invalid identifier in property name");

  prop->index = PATTERN_index(*JOB->current);
  JOB->current++;

  if (!TRANS_type(TT_CAN_SQUARE, &ptype))
    THROW("Syntax error. Bad property type");

  prop->type = ptype.type;
  prop->line = JOB->line;

  TYPE_set_kind(&prop->type, TK_PROPERTY);
  if (is_static)
    TYPE_set_flag(&prop->type, TF_STATIC);
  TYPE_set_flag(&prop->type, TF_PUBLIC);

  if (PATTERN_is_string(*JOB->current))
  {
    prop->comment = PATTERN_index(*JOB->current);
    JOB->current++;
  }
  else
    prop->comment = NO_SYMBOL;

  return TRUE;
}

static bool header_extern(TRANS_EXTERN *trans)
{
  PATTERN *look = JOB->current;
  TRANS_DECL ttyp;
  int index;
  bool is_public;

  /* public ? */

  is_public = JOB->is_module && JOB->public_module;

  if (PATTERN_is(*look, RS_PUBLIC))
  {
    is_public = TRUE;
    look++;
  }
  else if (PATTERN_is(*look, RS_PRIVATE))
  {
    is_public = FALSE;
    look++;
  }

  if (!PATTERN_is(*look, RS_EXTERN))
    return FALSE;

  look++;

  CLEAR(trans);

  JOB->current = look;
  analyze_function_desc((TRANS_FUNC *)trans, HF_NO_3PTS);

  if (PATTERN_is(*JOB->current, RS_AS))
  {
    if (!TRANS_type(TT_CAN_SQUARE, &ttyp))
      THROW("Syntax error in return type");
    trans->type = ttyp.type;
  }

  if (TRANS_is(RS_IN))
  {
    if (!PATTERN_is_string(*JOB->current))
      THROW("Library name must be a string");

    index = PATTERN_index(*JOB->current);
    JOB->current++;
  }
  else
  {
    if (JOB->default_library == NO_SYMBOL)
      THROW("IN missing");

    index = JOB->default_library;
  }

  trans->library = index;

  if (TRANS_is(RS_EXEC))
  {
    if (!PATTERN_is_string(*JOB->current))
      THROW("Alias name must be a string");

    trans->alias = PATTERN_index(*JOB->current);
    JOB->current++;
  }
  else
    trans->alias = NO_SYMBOL;

  TYPE_set_kind(&trans->type, TK_EXTERN);
  TYPE_set_flag(&trans->type, TF_STATIC);
  if (is_public) TYPE_set_flag(&trans->type, TF_PUBLIC);

  return TRUE;
}

static bool header_class(TRANS_DECL *decl)
{
  if (!PATTERN_is(*JOB->current, RS_CLASS))
    return FALSE;

  JOB->current++;

  if (!PATTERN_is_identifier(*JOB->current))
    THROW("Syntax error. CLASS needs an identifier");

  CLEAR(decl);
  decl->index = PATTERN_index(*JOB->current);
  JOB->current++;

  return TRUE;
}

static bool header_declaration(TRANS_DECL *decl)
{
  PATTERN *look = JOB->current;
  PATTERN *save;
  bool is_static = FALSE;
  bool is_public = FALSE;
  bool is_const = FALSE;
  /*bool other = FALSE;*/

  /* static ! */

  if (JOB->is_module)
  {
    is_static = TRUE;
  }
  else if (PATTERN_is(*look, RS_STATIC))
  {
    is_static = TRUE;
    look++;
  }

  /* public or private ? */

  is_public = JOB->is_module && JOB->public_module;

  if (PATTERN_is(*look, RS_PUBLIC))
  {
    is_public = TRUE;
    look++;
  }
  else if (PATTERN_is(*look, RS_PRIVATE))
  {
    is_public = FALSE;
    look++;
  }

  /* const ? */

  is_const = FALSE;

  if (PATTERN_is(*look, RS_CONST))
  {
    /* static const <=> const */

    /*
    if (is_static)
      return FALSE;
    */

    is_const = TRUE;
    look++;
  }

  if (!PATTERN_is_identifier(*look))
    return FALSE;

  CLEAR(decl);

  decl->index = PATTERN_index(*look);
  look++;

  save = JOB->current;
  JOB->current = look;

  if (!TRANS_type(((!is_const && !is_public) ? (TT_CAN_SQUARE  | TT_CAN_ARRAY) : 0) | TT_CAN_NEW, decl))
  {
    JOB->current = save;
    return FALSE;
  }

  if (is_static) TYPE_set_flag(&decl->type, TF_STATIC);
  if (is_public) TYPE_set_flag(&decl->type, TF_PUBLIC);
  if (is_const)
    TYPE_set_kind(&decl->type, TK_CONST);
  else
    TYPE_set_kind(&decl->type, TK_VARIABLE);

  if (is_const)
  {
    if (!decl->init)
      THROW("Syntax error. Missing '='");

    JOB->current = decl->init;
		JOB->current = TRANS_get_constant_value(decl, JOB->current);
  }

  //JOB->current = look;

  return TRUE;
}


static bool header_function(TRANS_FUNC *func)
{
  static HEADER_SPECIAL spec[] =
  {
    { "_init",      HS_PUBLIC + HS_STATIC + HS_PROCEDURE + HS_NOPARAM },
    { "_exit",      HS_PUBLIC + HS_STATIC + HS_PROCEDURE + HS_NOPARAM},
    { "_new",       HS_PUBLIC + HS_DYNAMIC + HS_PROCEDURE },
    { "_free",      HS_PUBLIC + HS_DYNAMIC + HS_PROCEDURE + HS_NOPARAM },
    { "_call",      HS_PUBLIC },
    { "_get",       HS_PUBLIC + HS_FUNCTION },
    { "_put",       HS_PUBLIC + HS_PROCEDURE + HS_PUT },
    { "_next",      HS_PUBLIC + HS_NOPARAM },
    { "_unknown",   HS_PUBLIC + HS_UNKNOWN },
    { "_compare",   HS_PUBLIC + HS_DYNAMIC + HS_FUNCTION + HS_COMPARE },
    { NULL, 0 }
  };

  PATTERN *look = JOB->current;
  PATTERN pat;
  TRANS_DECL ttyp;
  SYMBOL *sym;
  HEADER_SPECIAL *hsp;

  bool is_proc = FALSE;
  bool is_static = FALSE;
  bool is_public = FALSE;

  /* static ? */

  if (JOB->is_module)
  {
    is_static = TRUE;
  }
  else if (PATTERN_is(*look, RS_STATIC))
  {
    is_static = TRUE;
    look++;
  }

  /* public ou static ? */

  is_public = JOB->is_module && JOB->public_module;

  if (PATTERN_is(*look, RS_PUBLIC))
  {
    is_public = TRUE;
    look++;
  }
  else if (PATTERN_is(*look, RS_PRIVATE))
  {
    is_public = FALSE;
    look++;
  }

  if (PATTERN_is(*look, RS_PROCEDURE) || PATTERN_is(*look, RS_SUB))
    is_proc = TRUE;
  else if (!PATTERN_is(*look, RS_FUNCTION))
    return FALSE;
  look++;

  CLEAR(func);

  JOB->current = look;
  analyze_function_desc(func, HF_NORMAL);

  //if (!is_proc)
  if (PATTERN_is(*JOB->current, RS_AS))
  {
    if (!TRANS_type(TT_CAN_SQUARE, &ttyp))
      THROW("Syntax error. Invalid return type");
    func->type = ttyp.type;
    is_proc = FALSE;
  }

  TYPE_set_kind(&func->type, TK_FUNCTION);
  if (is_static) TYPE_set_flag(&func->type, TF_STATIC);
  if (is_public) TYPE_set_flag(&func->type, TF_PUBLIC);

  /* On v�ifie les m�hodes sp�iales */

  sym = TABLE_get_symbol(JOB->class->table, func->index);

  if (*sym->name == '_')
  {
    for (hsp = spec; hsp->name; hsp++)
    {
      if (sym->len != strlen(hsp->name))
        continue;
      if (strncmp(sym->name, hsp->name, sym->len))
        continue;

      if (hsp->flag == HS_ERROR)
        THROW("The special method &1 cannot be implemented", hsp->name);

      if ((hsp->flag & HS_PUBLIC) && !is_public)
        THROW("The special method &1 must be public", hsp->name);

      if ((hsp->flag & HS_STATIC) && !is_static)
        THROW("The special method &1 must be static", hsp->name);

      if ((hsp->flag & HS_DYNAMIC) && is_static)
        THROW("The special method &1 cannot be static", hsp->name);

      if ((hsp->flag & HS_PROCEDURE) && !is_proc)
        THROW("The special method &1 cannot be a function", hsp->name);

      if ((hsp->flag & HS_FUNCTION) && is_proc)
        THROW("The special method &1 must be a function", hsp->name);

      if ((hsp->flag & HS_NOPARAM) && func->nparam > 0)
        THROW("The special method &1 takes no arguments", hsp->name);

      if (hsp->flag & HS_PUT)
      {
        if (func->nparam < 1)
          THROW("The special method &1 must take at least one argument", hsp->name);
      }

      if (hsp->flag & HS_UNKNOWN)
      {
        if (func->nparam > 0 || !func->vararg)
          THROW("The special method &1 must take a variable number of arguments only", hsp->name);
      }

      if (hsp->flag & HS_COMPARE)
      {
      	if (func->type.t.id != T_INTEGER)
      		THROW("The special method must return an integer");
				if (func->nparam != 1)
      		THROW("The special method must take exactly one argument");

      }

      break;
    }
  }


  /* on saute le corps de la fonction */

  if (!PATTERN_is_newline(*(JOB->current)))
    THROW("Syntax error after function declaration");

  func->line = PATTERN_index(*(JOB->current)) + 1; /* � commence �la ligne suivante ! */
  func->start = JOB->current + 1;

  for(;;)
  {
    pat = *JOB->current;
    if (PATTERN_is_end(pat))
    {
      THROW("Missing END");
    }

    if (PATTERN_is_newline(pat))
      if (PATTERN_is(JOB->current[1], RS_END))
      {
        if (PATTERN_is_newline(JOB->current[2]))
        {
          JOB->current += 2;
          break;
        }
        if (TRANS_is_end_function(is_proc, &JOB->current[2]))
        {
          JOB->current += 3;
          break;
        }
      }

    JOB->current++;
  }

  return TRUE;
}


static bool header_inherits(void)
{
  int index;

  if (!PATTERN_is(*JOB->current, RS_INHERITS))
    return FALSE;

  /*{
    if (!(PATTERN_is(JOB->current[0], RS_CLASS)
          && PATTERN_is(JOB->current[1], RS_INHERITS)))
      return FALSE;
    JOB->current++;
  }*/

  JOB->current++;

  if (!PATTERN_is_class(*JOB->current))
    THROW("Syntax error. INHERITS needs a class name");

  if (JOB->class->parent != NO_SYMBOL)
    THROW("Cannot inherit twice");

  index = PATTERN_index(*JOB->current);

  JOB->class->parent = CLASS_add_class(JOB->class, index);
  /*printf("JOB->class->parent = %d\n", JOB->class->parent);*/

  JOB->current++;
  return TRUE;
}


static bool header_option(void)
{
  if (PATTERN_is(JOB->current[0], RS_EXPORT))
  {
    JOB->current++;
    JOB->class->exported = TRUE;

		if (PATTERN_is(JOB->current[0], RS_OPTIONAL))
  	{
    	JOB->current++;
    	JOB->class->optional = TRUE;
		}

    return TRUE;
  }

  if (PATTERN_is(JOB->current[0], RS_CREATE))
  {
    JOB->current++;
    
    if (PATTERN_is_newline(JOB->current[0]) || PATTERN_is(JOB->current[0], RS_STATIC))
		{
    	JOB->class->autocreate = TRUE;
    	if (PATTERN_is(JOB->current[0], RS_STATIC))
    		JOB->current++;
    	return TRUE;
		}
    else if (PATTERN_is(JOB->current[0], RS_PRIVATE))
		{
    	JOB->class->nocreate = TRUE;
    	JOB->current++;
    	return TRUE;
		}
  }

  return FALSE;
}

static bool header_library(void)
{
  if (!TRANS_is(RS_LIBRARY))
    return FALSE;

  if (!PATTERN_is_string(*JOB->current))
    THROW("Library name must be a string");

  JOB->default_library = PATTERN_index(*JOB->current);
  JOB->current++;
  return TRUE;
}


PUBLIC void HEADER_do(void)
{
  union {
    TRANS_DECL decl;
    TRANS_FUNC func;
    TRANS_EVENT event;
    TRANS_EXTERN ext;
    TRANS_PROPERTY prop;
    } trans;

  TRANS_reset();

  header_module_type();

  while (JOB->current < JOB->end)
  {
    if (PATTERN_is_end(*JOB->current))
      break;

    if (TRANS_newline())
      continue;

    if (header_function(&trans.func))
    {
      CLASS_add_function(JOB->class, &trans.func);
      continue;
    }

    if (header_event(&trans.event))
    {
      CLASS_add_event(JOB->class, &trans.event);
      continue;
    }

    if (header_property(&trans.prop))
    {
      CLASS_add_property(JOB->class, &trans.prop);
      continue;
    }

    if (header_extern(&trans.ext))
    {
      CLASS_add_extern(JOB->class, &trans.ext);
      continue;
    }

    if (header_declaration(&trans.decl))
    {
      CLASS_add_declaration(JOB->class, &trans.decl);
      continue;
    }

    if (header_class(&trans.decl))
    {
      CLASS_add_class(JOB->class, trans.decl.index);
      continue;
    }

    if (header_inherits())
      continue;

    if (header_option())
      continue;

    if (header_library())
      continue;

    THROW(E_UNEXPECTED, READ_get_pattern(JOB->current));
  }

  /* R�rganisation des variables statiques et dynamiques
     pour �iter les probl�es d'alignement
  */

  CLASS_sort_declaration(JOB->class);

  if (JOB->verbose)
    CLASS_dump();
}
