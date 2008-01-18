/***************************************************************************

  CExpression.c

  (c) 2000-2007 Benoit Minisini <gambas@freesurf.fr>

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

#define __CEXPRESSION_C

#include "gb_common.h"
#include "gambas.h"

#include "eval.h"
#include "main.h"

#include "CExpression.h"

/*#define DEBUG*/

static CEXPRESSION *_current;


BEGIN_METHOD_VOID(CEXPRESSION_new)

  THIS->compiled = FALSE;
  CLEAR(&THIS->expr);

END_METHOD


BEGIN_METHOD_VOID(CEXPRESSION_free)

  EVAL_clear(&THIS->expr);
  GB.FreeString(&THIS->text);
  GB.FreeString(&THIS->expr.source);
  GB.Unref((void **)&THIS->env);

END_METHOD


BEGIN_PROPERTY(CEXPRESSION_text)

  if (READ_PROPERTY)
    GB.ReturnString(THIS->text);
  else
  {
    GB.StoreString(PROP(GB_STRING), &THIS->text);
    GB.FreeString(&THIS->expr.source);
    GB.NewString(&THIS->expr.source, THIS->text, VPROP(GB_STRING).len);
    THIS->expr.len = VPROP(GB_STRING).len;
    THIS->compiled = FALSE;
  }

END_PROPERTY


BEGIN_PROPERTY(CEXPRESSION_environment)

  if (READ_PROPERTY)
    GB.ReturnObject(THIS->env);
  else
    GB.StoreObject(PROP(GB_OBJECT), &THIS->env);

END_PROPERTY


BEGIN_METHOD_VOID(CEXPRESSION_prepare)

  if (!THIS->compiled && (THIS->expr.len > 0))
  {
    if (!EVAL_compile(&THIS->expr))
      THIS->compiled = TRUE;
  }

END_METHOD


static bool get_variable(const char *sym, int len, GB_VARIANT *value)
{
  if (_current->env)
    if (!GB.Collection.Get(_current->env, sym, len, value))
      return FALSE;

  value->type = GB_T_NULL;
  return TRUE;
}


BEGIN_PROPERTY(CEXPRESSION_value)

  if (!THIS->compiled)
    CALL_METHOD_VOID(CEXPRESSION_prepare);

  if (!THIS->compiled)
  {
    GB.ReturnNull();
    return;
  }

  _current = THIS;
  EVAL_expression(&THIS->expr, (EVAL_FUNCTION)get_variable);

END_PROPERTY


GB_DESC CExpressionDesc[] =
{
  GB_DECLARE("Expression", sizeof(CEXPRESSION)),

  //GB_STATIC_METHOD("_init", NULL, CEXPRESSION_init, NULL),
  //GB_STATIC_METHOD("_exit", NULL, CEXPRESSION_exit, NULL),

  GB_METHOD("_new", NULL, CEXPRESSION_new, NULL),
  GB_METHOD("_free", NULL, CEXPRESSION_free, NULL),

  GB_PROPERTY("Text", "s", CEXPRESSION_text),
  GB_PROPERTY("Environment", "Collection;", CEXPRESSION_environment),

  GB_METHOD("Compile", NULL, CEXPRESSION_prepare, NULL),

  GB_PROPERTY_READ("Value", "v", CEXPRESSION_value),

  GB_END_DECLARE
};

