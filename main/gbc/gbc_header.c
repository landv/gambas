/***************************************************************************

  gbc_header.c

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
#include "gb_code.h"

/*#define DEBUG*/


static void check_public_private(PATTERN **look, bool *is_public)
{
	*is_public = JOB->is_module && JOB->public_module;

	if (PATTERN_is(**look, RS_PUBLIC))
	{
		*is_public = TRUE;
		(*look)++;
	}
	else if (PATTERN_is(**look, RS_PRIVATE))
	{
		*is_public = FALSE;
		(*look)++;
	}
}

static PATTERN *jump_expression(PATTERN *look)
{
	int niv = 0;
	
	for(;;)
	{
		if (PATTERN_is_newline(*look))
			break;

		if (PATTERN_is(*look, RS_LBRA) || PATTERN_is(*look, RS_LSQR))
		{
			niv++;
		}
		else if (PATTERN_is(*look, RS_RBRA) || PATTERN_is(*look, RS_RSQR))
		{
			if (niv > 0)
				niv--;
			else
				break;
		}
		else if (niv == 0)
		{
			if (PATTERN_is(*look, RS_COMMA))
				break;
		}
		
		look++;
	}
	
	return look;
}

static void analyze_function_desc(TRANS_FUNC *func, int flag)
{
	PATTERN *look = JOB->current;
	TRANS_PARAM *param;
	//bool is_output;
	bool is_optional = FALSE;
	TRANS_DECL ttyp;
	uint64_t byref_mask = 1;

	if (!PATTERN_is_identifier(*look))
		THROW("Syntax error. Invalid identifier in function name");

	func->index = PATTERN_index(*look);
	TYPE_clear(&func->type);
	look++;

	if (flag & HF_EVENT)
		TABLE_copy_symbol_with_prefix(JOB->class->table, func->index, ':', &func->index);

	func->nparam = 0;
	func->byref = 0;
	func->vararg = FALSE;

	if ((flag & HF_VOID) && PATTERN_is_newline(*look))
	{
		JOB->current = ++look;
		return;
	}

	if (!PATTERN_is(*look, RS_LBRA))
		THROW_UNEXPECTED(look);
	look++;

	for(;;)
	{
		if (func->nparam >= MAX_PARAM_FUNC)
			THROW("Too many arguments");

		param = &func->param[func->nparam];
		CLEAR(param);

		if (PATTERN_is(*look, RS_RBRA))
		{
			look++;
			break;
		}

		if (func->nparam > 0)
		{
			if (!PATTERN_is(*look, RS_COMMA))
				THROW(E_SYNTAX_MISSING, "',' or ')'");
			look++;
		}

		if (PATTERN_is(*look, RS_3PTS) && !(flag & HF_NO_3PTS))
		{
			look++;
			if (!PATTERN_is(*look, RS_RBRA))
				THROW("Syntax error. '...' must be the last argument"); //, get_num_desc(func->nparam + 1));
			look++;
			func->vararg = TRUE;
			break;
		}

		//is_output = FALSE;

		if (!(flag & HF_NO_OPT))
		{
			if (PATTERN_is(*look, RS_OPTIONAL))
			{
				look++;
				is_optional = TRUE;
			}
		}

		if (PATTERN_is(*look, RS_AT) || PATTERN_is(*look, RS_BYREF))
		{
			func->byref |= byref_mask;
			look++;
		}

		if (PATTERN_is(*look, RS_LBRA))
		{
			param->ignore = TRUE;
			look++;
		}
		
		if (!PATTERN_is_identifier(*look))
			THROW("Syntax error. The &1 argument is not a valid identifier", TRANS_get_num_desc(func->nparam + 1));
		
		param->index = PATTERN_index(*look);
		look++;
		
		if (param->ignore)
		{
			if (!PATTERN_is(*look, RS_RBRA))
				THROW(E_MISSING, "')'");
			look++;
		}

		JOB->current = look;

		if (!TRANS_type(TT_NOTHING, &ttyp))
			THROW("Syntax error. Invalid type description of &1 argument", TRANS_get_num_desc(func->nparam + 1));

		param->type = ttyp.type;

		/*
		if (is_output)
			TYPE_set_flag(&param->type, TF_OUTPUT);
		*/

		look = JOB->current;

		if (is_optional)
		{
			param->optional = look;
			look = jump_expression(look);
			JOB->current = look;
		}

		func->nparam++;
		byref_mask <<= 1;
	}

	JOB->current = look;
}


static void header_module_type(void)
{
	const char *ext;
	const FORM_FAMILY *p;

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
	else
	{
		p = COMP_form_families;
		while (p->ext)
		{
			if (strcasecmp(ext, p->ext) == 0)
			{
				JOB->is_module = FALSE;
				JOB->is_form = TRUE;
				break;
			}
			p++;
		}
		
		if (!p->ext)
			THROW("Unknown file extension");
	}

	JOB->declared = TRUE;
}


static bool header_event(TRANS_EVENT *event)
{
	PATTERN *look = JOB->current;
	//TRANS_DECL ttyp;

	if (!PATTERN_is(*look, RS_EVENT))
		return FALSE;

	CLEAR(event);

	if (JOB->is_module)
		THROW("A module cannot raise events");

	JOB->current++;
	analyze_function_desc((TRANS_FUNC *)event, HF_VOID + HF_NO_BYREF + HF_NO_3PTS + HF_NO_OPT);

	/*if (PATTERN_is(*JOB->current, RS_AS))
	{
		if (!TRANS_type(TT_CAN_SQUARE, &ttyp))
			THROW("Syntax error in return type");
		event->type = ttyp.type;
	}*/

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
	
	prop->nsynonymous = 0;
	
	while (TRANS_is(RS_COMMA))
	{
		if (prop->nsynonymous == 3)
			THROW("Too many property synonymous");
		if (!PATTERN_is_identifier(*JOB->current))
			THROW("Syntax error. Invalid identifier in property name");
		prop->synonymous[prop->nsynonymous++] = PATTERN_index(*JOB->current);
		JOB->current++;
	}

	if (!TRANS_type(TT_NOTHING, &ptype))
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

	check_public_private(&look, &is_public);
	
	if (!PATTERN_is(*look, RS_EXTERN))
		return FALSE;
	look++;

	CLEAR(trans);

	JOB->current = look;
	analyze_function_desc((TRANS_FUNC *)trans, HF_NO_BYREF);

	if (PATTERN_is(*JOB->current, RS_AS))
	{
		if (!TRANS_type(TT_NOTHING, &ttyp))
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
			THROW(E_MISSING, "IN");

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
	if (!TRANS_is(RS_CLASS))
		return FALSE;

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
	bool no_warning = FALSE;

	/* static ! */

	if (JOB->is_module)
	{
		is_static = TRUE;
	}
	else if (PATTERN_is(*look, RS_STATIC))
	{
		is_static = TRUE;
		//has_static = TRUE;
		look++;
	}

	check_public_private(&look, &is_public);

	/* const ? */

	is_const = FALSE;

	if (PATTERN_is(*look, RS_CONST))
	{
		//if (has_static)
		//	THROW("Unexpected &1", "STATIC");

		is_const = TRUE;
		look++;
	}

	if (PATTERN_is(*look, RS_LBRA))
	{
		no_warning = TRUE;
		look++;
	}

	if (!PATTERN_is_identifier(*look))
		return FALSE;

	CLEAR(decl);

	decl->index = PATTERN_index(*look);
	look++;

	if (no_warning)
	{
		if (!PATTERN_is(*look, RS_RBRA))
			return FALSE;
		look++;
		decl->no_warning = TRUE;
	}

	save = JOB->current;
	JOB->current = look;

	if (!TRANS_type((!is_const ? TT_CAN_ARRAY | TT_CAN_EMBED : 0) | TT_CAN_NEW, decl))
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
			THROW(E_SYNTAX_MISSING, "'='");

		JOB->current = decl->init;
		JOB->current = TRANS_get_constant_value(decl, JOB->current);
	}

	//JOB->current = look;

	return TRUE;
}


static bool header_enumeration(TRANS_DECL *decl)
{
	PATTERN *look = JOB->current;
	bool is_public;
	int value = 0;
	
	check_public_private(&look, &is_public);
	
	if (!PATTERN_is(*look, RS_ENUM))
		return FALSE;
	look++;

	JOB->current = look;
	
	for(;;)
	{
		TRANS_newline();

		if (!PATTERN_is_identifier(*JOB->current))
			THROW("Syntax error. Identifier expected.");
		
		CLEAR(decl);

		decl->index = PATTERN_index(*JOB->current);
		JOB->current++;
	
		decl->type = TYPE_make(T_INTEGER, -1, 0);

		if (is_public) TYPE_set_flag(&decl->type, TF_PUBLIC);
		TYPE_set_kind(&decl->type, TK_CONST);

		if (TRANS_is(RS_EQUAL))
		{
			JOB->current = TRANS_get_constant_value(decl, JOB->current);
			value = decl->value + 1;
		}
		else
		{
			decl->value = value;
			value++;
		}
		
		CLASS_add_declaration(JOB->class, decl);

		if (TRANS_newline())
			break;

		if (!PATTERN_is(*JOB->current, RS_COMMA))
			THROW_UNEXPECTED(JOB->current);
		JOB->current++;
	}	
	
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
		{ "_property",  HS_PUBLIC + HS_NOPARAM + HS_FUNCTION + HS_PROPERTY },
		{ "_unknown",   HS_PUBLIC + HS_UNKNOWN },
		{ "_compare",   HS_PUBLIC + HS_DYNAMIC + HS_FUNCTION + HS_COMPARE },
		{ "_attach",    HS_PUBLIC + HS_DYNAMIC + HS_PROCEDURE + HS_ATTACH },
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
	bool is_fast = FALSE;

	// fast ?
	
	if (PATTERN_is(*look, RS_FAST))
	{
		is_fast = TRUE;
		look++;
	}
	
	// static ?

	if (JOB->is_module)
	{
		is_static = TRUE;
	}
	else if (PATTERN_is(*look, RS_STATIC))
	{
		is_static = TRUE;
		look++;
	}

	// public ou static ?

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

	//CLEAR(func);

	JOB->current = look;
	analyze_function_desc(func, HF_NORMAL);

	if (PATTERN_is(*JOB->current, RS_AS))
	{
		if (!TRANS_type(TT_NOTHING, &ttyp))
			THROW("Syntax error. Invalid return type");
		func->type = ttyp.type;
		is_proc = FALSE;
	}

	TYPE_set_kind(&func->type, TK_FUNCTION);
	if (is_static) TYPE_set_flag(&func->type, TF_STATIC);
	if (is_public) TYPE_set_flag(&func->type, TF_PUBLIC);
	
	func->fast = is_fast;

	// Check special methods

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

			if (hsp->flag & HS_PROPERTY)
			{
				if (TYPE_get_id(func->type) != T_BOOLEAN)
					THROW("The special method &1 must return a boolean", hsp->name);
			}

			if (hsp->flag & HS_COMPARE)
			{
				if (func->type.t.id != T_INTEGER)
					THROW("The special method must return an integer");
				if (func->nparam != 1)
					THROW("The special method must take exactly one argument");
			}
			
			if (hsp->flag & HS_ATTACH)
			{
				if (func->nparam != 2)
					THROW("The special method must take exactly two arguments");
				if (func->param[0].type.t.id != T_OBJECT || func->param[1].type.t.id != T_STRING)
					THROW("The special method signature is incorrect");
			}

			break;
		}
	}


	// We ignore function body

	if (!PATTERN_is_newline(*(JOB->current)))
		THROW("Syntax error at function declaration");

	func->line = PATTERN_index(*(JOB->current)) + 1;
	func->start = JOB->current + 1;

	look = JOB->current;
	for(;;)
	{
		pat = *look;

		if (PATTERN_is_newline(pat))
		{
			JOB->line = PATTERN_index(pat) + 1;
			pat = look[1];
			
			if (PATTERN_is(pat, RS_END))
			{
				if (PATTERN_is_newline(look[2]))
				{
					look += 2;
					break;
				}
				if (TRANS_is_end_function(is_proc, &look[2]))
				{
					look += 3;
					break;
				}
				else
				{
					if (is_proc && PATTERN_is(look[2], RS_FUNCTION))
						THROW(E_EXPECTED, "END SUB");
					else if (!is_proc && PATTERN_is(look[2], RS_SUB))
						THROW(E_EXPECTED, "END FUNCTION");
				}
			}
			else if (UNLIKELY(PATTERN_is_end(pat))) // || PATTERN_is_command(pat)))
				THROW(E_MISSING, "END");
		}

		look++;
	}

	JOB->current = look;
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
	
	if (strcasecmp(TABLE_get_symbol_name(JOB->class->table, index), JOB->class->name) == 0)
		THROW("Cannot inherit itself");

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
	
	if (PATTERN_is(JOB->current[0], RS_FAST))
	{
		JOB->current++;
		JOB->class->all_fast = TRUE;
		
		return TRUE;
	}

	return FALSE;
}


static bool header_library(void)
{
	if (!TRANS_is(RS_LIBRARY))
		return FALSE;

	if (!PATTERN_is_string(*JOB->current))
		THROW("Extern library name must be a string");

	JOB->default_library = PATTERN_index(*JOB->current);
	JOB->current++;
	return TRUE;
}


static bool header_structure(void)
{
	PATTERN *look = JOB->current;
	bool is_public;
	CLASS_STRUCT *structure;
	VARIABLE *field;
	TRANS_DECL decl;
	int nfield;
	int index;

	check_public_private(&look, &is_public);
	
	if (!PATTERN_is(*look, RS_STRUCT))
		return FALSE;
	look++;
	JOB->current = look;
	
	if (!is_public)
		THROW("Structures must be public");
	
	if (!PATTERN_is_identifier(*JOB->current))
		THROW("Syntax error. STRUCT needs an identifier");

	structure = ARRAY_add_void(&JOB->class->structure);
	ARRAY_create(&structure->field);
	nfield = 0;
	
	structure->index = PATTERN_index(*JOB->current);
	index = CLASS_add_class_exported(JOB->class, structure->index);
	JOB->class->class[index].structure = TRUE;
	//fprintf(stderr, "Set structure flag to %s\n", TABLE_get_symbol_name(JOB->class->table, structure->index));

//TABLE_copy_symbol_with_prefix(JOB->class->table, structure->index, '.', NULL, &structure->index);

	JOB->current++;
	TRANS_want_newline();
	
	for(;;)
	{
		do
		{
			if (PATTERN_is_end(*JOB->current)) // || PATTERN_is_command(*JOB->current))
				THROW ("Missing END STRUCT");
		}
		while (TRANS_newline());
		
		if (PATTERN_is(*JOB->current, RS_END) && PATTERN_is(JOB->current[1], RS_STRUCT))
		{
			if (nfield == 0)
				THROW ("Syntax error. A structure must have one field at least.");
				
			JOB->current += 2;
			
			TRANS_want_newline();
			break;
		}
		
		if (!PATTERN_is_identifier(*JOB->current))
			THROW("Syntax error. The &1 field is not a valid identifier", TRANS_get_num_desc(nfield + 1));

		field = ARRAY_add(&structure->field);
		field->index = PATTERN_index(*JOB->current); 
		
		
		JOB->current++;
		
		CLEAR(&decl);
		if (!TRANS_type(TT_CAN_ARRAY | TT_CAN_EMBED, &decl))
			THROW("Syntax error. Invalid type description of &1 field", TRANS_get_num_desc(nfield + 1));

		TRANS_want_newline();
		
		field->type = decl.type;
		
		nfield++;
	}
	
	structure->nfield = nfield;
	
	return TRUE;
}

static bool header_use(void)
{
	if (!TRANS_is(RS_USE))
		return FALSE;

	CLASS_begin_init_function(JOB->class, FUNC_INIT_STATIC);

	for(;;)
	{
		if (!PATTERN_is_string(*JOB->current))
			THROW("Component name must be a string");

		TRANS_string(*JOB->current);
		TRANS_subr(TS_SUBR_USE, 1);
		CODE_drop();

		JOB->current++;

		if (!TRANS_is(RS_COMMA))
			break;
	}

	if (!PATTERN_is_newline(*JOB->current))
		THROW("Syntax error");

	return TRUE;
}


static void check_class_header()
{
	while (TRUE) //(JOB->current < JOB->end)
	{
		if (PATTERN_is_end(*JOB->current))
			break;

		if (TRANS_newline())
			continue;

		if (header_option())
			continue;
		
		if (header_inherits())
			continue;

		break;
	}
}

void HEADER_do(void)
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
	
	if (JOB->line == 1)
		check_class_header();

	while (TRUE) //JOB->current < JOB->end)
	{
		if (PATTERN_is_end(*JOB->current))
			break;
		
		if (TRANS_newline())
		{
			if (JOB->line == 1)
				check_class_header();
			
			continue;
		}

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
		
		if (header_enumeration(&trans.decl))
			continue;
		
		if (header_declaration(&trans.decl))
		{
			CLASS_add_declaration(JOB->class, &trans.decl);
			continue;
		}
		
		if (header_structure())
			continue;

		if (header_class(&trans.decl))
		{
			CLASS_add_class_exported(JOB->class, trans.decl.index);
			continue;
		}

		if (header_inherits())
			continue;

		if (header_library())
			continue;

		if (header_use())
			continue;

		/*if (PATTERN_is_command(*JOB->current))
		{
			JOB->current++;
			continue;
		}*/

		THROW_UNEXPECTED(JOB->current);
	}

	// Sort class declaration to avoid alignment problems.
	// This should be useless now, as it is done again by the interpreter
	// when loading the class.

	CLASS_sort_declaration(JOB->class);

	if (JOB->verbose)
		CLASS_dump();
}
