/***************************************************************************

  CExpression.c

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

#define __CEXPRESSION_C

#include "gb_common.h"
#include "gambas.h"

#include "eval.h"
#include "main.h"

#include "CExpression.h"

/*#define DEBUG*/

static CEXPRESSION *_current;


BEGIN_METHOD_VOID(Expression_new)

  THIS->compiled = FALSE;
  CLEAR(&THIS->expr);

END_METHOD


BEGIN_METHOD_VOID(Expression_free)

  EVAL_clear(&THIS->expr, FALSE);
  GB.FreeString(&THIS->text);
  GB.FreeString(&THIS->expr.source);
  GB.Unref((void **)&THIS->env);

END_METHOD


BEGIN_PROPERTY(Expression_Text)

  if (READ_PROPERTY)
    GB.ReturnString(THIS->text);
  else
  {
    GB.StoreString(PROP(GB_STRING), &THIS->text);
    GB.FreeString(&THIS->expr.source);
    THIS->expr.source = GB.NewString(THIS->text, VPROP(GB_STRING).len);
    THIS->expr.len = VPROP(GB_STRING).len;
    THIS->compiled = FALSE;
  }

END_PROPERTY


BEGIN_PROPERTY(Expression_Environment)

  if (READ_PROPERTY)
    GB.ReturnObject(THIS->env);
  else
    GB.StoreObject(PROP(GB_OBJECT), &THIS->env);

END_PROPERTY

static void prepare(CEXPRESSION *_object)
{
  if (!THIS->compiled && (THIS->expr.len > 0))
  {
    if (!EVAL_compile(&THIS->expr, FALSE))
      THIS->compiled = TRUE;
		else
			GB.Error(THIS->expr.error);
  }
}

BEGIN_METHOD_VOID(Expression_Prepare)

	prepare(THIS);

END_METHOD


static bool get_variable(const char *sym, int len, GB_VARIANT *value)
{
  if (_current->env)
    if (!GB.Collection.Get(_current->env, sym, len, value))
      return FALSE;

  value->type = GB_T_NULL;
  return TRUE;
}

static void execute(CEXPRESSION *_object)
{
  if (!THIS->compiled)
		prepare(THIS);

  if (!THIS->compiled)
  {
    GB.ReturnVariant(NULL);
    return;
  }

  _current = THIS;
  EVAL_expression(&THIS->expr, (EVAL_FUNCTION)get_variable);
	GB.ReturnConvVariant();
}

BEGIN_PROPERTY(Expression_Value)

	execute(THIS);

END_PROPERTY

GB_DESC CExpressionDesc[] =
{
  GB_DECLARE("Expression", sizeof(CEXPRESSION)),

  //GB_STATIC_METHOD("_init", NULL, Expression_init, NULL),
  //GB_STATIC_METHOD("_exit", NULL, Expression_exit, NULL),

  GB_METHOD("_new", NULL, Expression_new, NULL),
  GB_METHOD("_free", NULL, Expression_free, NULL),

  GB_PROPERTY("Text", "s", Expression_Text),
  GB_PROPERTY("Environment", "Collection;", Expression_Environment),

  GB_METHOD("Compile", NULL, Expression_Prepare, NULL),

  GB_PROPERTY_READ("Value", "v", Expression_Value),

  GB_END_DECLARE
};

