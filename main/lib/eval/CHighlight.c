/***************************************************************************

  CHighlight.cpp

  (c) 2000-2007 Benoit Minisini <gambas@users.sourceforge.net>

  WIDGET program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 1, or (at your option)
  any later version.

  WIDGET program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with WIDGET program; if not, write to the Free Software
  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

***************************************************************************/

#define __CHIGHLIGHT_CPP

#include "gb_common.h"
#include "main.h"
#include "gb.eval.h"
#include "eval_analyze.h"

#include "CHighlight.h"

static void *_analyze_symbol = 0;
static void *_analyze_type = 0;
static void *_analyze_pos = 0;

static char *_purged_line = NULL;

static int get_char_length(unsigned char c)
{
  int n = 1;

  if (c & 0x80)
  {
    for (;;)
    {
      c <<= 1;
      if (!(c & 0x80))
        break;
      n++;
    }
  }

  return n;
}

static char *purge(const char *s, int len_s, bool comment, bool string)
{
  char c;
  uint i;
  int lc;
  bool in_comment = FALSE;
  char wait = 0;
  char *r = NULL;
  
  for (i = 0; i < len_s; i += lc)
  {
    c = s[i];
    lc = get_char_length((unsigned char)c);

    switch(wait)
    {
      case 0:

        if (in_comment)
        {
        	if (!comment)
          	c = ' ';
				}
        else if (c == '"')
          wait = '"';
        else if (c == '\'')
          in_comment = TRUE;

        break;

      case '"':
        if (c == '"')
          wait = 0;
				else if (c == '\\')
				{
					if (string)
					{
						if (i < len_s)
							GB.AddString(&r, &c, 1);
							//r += c;
						i++;
						c = s[i];
					}
					else
					{
						i++;
						if (i < len_s)
							GB.AddString(&r, " ", 1);
							//r += ' ';
						c = ' ';
					}
				}
        else
        {
        	if (!string)
          	c = ' ';
				}
        break;
    }

    GB.AddString(&r, &c, 1);
  }

	GB.FreeString(&_purged_line);
	_purged_line = r;
	
  return r;
}


static int convState(int state)
{
  switch(state)
  {
    case EVAL_TYPE_END: return HIGHLIGHT_NORMAL;
    case EVAL_TYPE_RESERVED: return HIGHLIGHT_KEYWORD;
    case EVAL_TYPE_IDENTIFIER: return HIGHLIGHT_SYMBOL;
    case EVAL_TYPE_CLASS: return HIGHLIGHT_DATATYPE;
    case EVAL_TYPE_NUMBER: return HIGHLIGHT_NUMBER;
    case EVAL_TYPE_STRING: return HIGHLIGHT_STRING;
    case EVAL_TYPE_SUBR: return HIGHLIGHT_SUBR;
    case EVAL_TYPE_COMMENT: return HIGHLIGHT_COMMENT;
    case EVAL_TYPE_OPERATOR: return HIGHLIGHT_OPERATOR;
    case EVAL_TYPE_DATATYPE: return HIGHLIGHT_DATATYPE;
    case EVAL_TYPE_ERROR: return HIGHLIGHT_ERROR;
    default: return HIGHLIGHT_NORMAL;
  }
}

static void analyze(const char *src, int len_src)
{
  GB_ARRAY garray, tarray, parray;
  int i, n, pos, len, p;
  char *str;
  EVAL_ANALYZE result;

  EVAL_analyze(src, len_src, &result);

  n = 0;
  for (i = 0; i < result.len; i++)
  {
    if (result.color[i].state != EVAL_TYPE_END)
      n++;
  }

  GB.Array.New(&garray, GB_T_STRING, n);
  GB.Array.New(&tarray, GB_T_INTEGER, n);
  GB.Array.New(&parray, GB_T_INTEGER, n);

  pos = 0;
  i = 0;
  for (p = 0; p < result.len; p++)
  {
    len = result.color[p].len;

    if (result.color[p].state != EVAL_TYPE_END)
    {
      GB.NewString(&str, &result.str[pos], len);
      *((char **)GB.Array.Get(garray, i)) = str;
      *((int *)GB.Array.Get(tarray, i)) = convState(result.color[p].state);
      *((int *)GB.Array.Get(parray, i)) = pos;
      i++;
    }

    pos += len;
  }

  GB.Unref(&_analyze_symbol);
  _analyze_symbol = garray;
  GB.Ref(garray);

  GB.Unref(&_analyze_type);
  _analyze_type = tarray;
  GB.Ref(tarray);

  GB.Unref(&_analyze_pos);
  _analyze_pos = parray;
  GB.Ref(parray);
  
  GB.FreeString(&result.str);  
}

BEGIN_METHOD_VOID(CHIGHLIGHT_exit)

  GB.Unref(&_analyze_symbol);
  GB.Unref(&_analyze_type);
  GB.Unref(&_analyze_pos);
  GB.FreeString(&_purged_line);

END_METHOD


#if 0
BEGIN_PROPERTY(CHIGHLIGHT_state)

	if (READ_PROPERTY)
		GB.ReturnInteger(_highlight_state);
	else
		_highlight_state = VPROP(GB_INTEGER);

END_PROPERTY

BEGIN_PROPERTY(CHIGHLIGHT_tag)

	if (READ_PROPERTY)
		GB.ReturnInteger(_highlight_tag);
	else
		_highlight_tag = VPROP(GB_INTEGER);

END_PROPERTY

BEGIN_PROPERTY(CHIGHLIGHT_show_limit)

	if (READ_PROPERTY)
		GB.ReturnBoolean(_highlight_show_limit);
	else
		_highlight_show_limit = VPROP(GB_BOOLEAN);

END_PROPERTY

BEGIN_PROPERTY(CHIGHLIGHT_text)

	if (READ_PROPERTY)
		GB.ReturnNewZeroString(TO_UTF8(_highlight_text));
	else
		_highlight_text = QSTRING_PROP();

END_PROPERTY

BEGIN_METHOD(CHIGHLIGHT_add, GB_INTEGER state; GB_INTEGER len)

	GHighlight *h;

	if (!_highlight_data)
		return;

	int count = GB.Count(*_highlight_data) - 1;
	int state = VARG(state);
	int len = VARGOPT(len, 1);

	if (len < 1)
		return;

	if (count < 0 || (*_highlight_data)[count].state != (uint)state)
	{
		count++;
		h = (GHighlight *)GB.Add(_highlight_data);
		h->state = state;
		h->len = len;
	}
	else
		(*_highlight_data)[count].len += len;

END_METHOD
#endif

BEGIN_METHOD(CHIGHLIGHT_purge, GB_STRING text; GB_BOOLEAN comment; GB_BOOLEAN string)

  bool comment = VARGOPT(comment, FALSE);
  bool string = VARGOPT(string, FALSE);
  
  if (comment && string)	
  	GB.ReturnNewString(STRING(text), LENGTH(text));
	else
	{
  	GB.ReturnString(purge(STRING(text), LENGTH(text), comment, string));
  	
  }

END_METHOD

BEGIN_METHOD(CHIGHLIGHT_analyze, GB_STRING text)

  analyze(STRING(text), LENGTH(text));
  GB.ReturnObject(_analyze_symbol);

END_METHOD

BEGIN_PROPERTY(CHIGHLIGHT_analyze_symbols)

  GB.ReturnObject(_analyze_symbol);

END_PROPERTY

BEGIN_PROPERTY(CHIGHLIGHT_analyze_types)

  GB.ReturnObject(_analyze_type);

END_PROPERTY

BEGIN_PROPERTY(CHIGHLIGHT_analyze_positions)

  GB.ReturnObject(_analyze_pos);

END_PROPERTY

GB_DESC CHighlightDesc[] =
{
  GB_DECLARE("Highlight", 0), GB_VIRTUAL_CLASS(),

  /*GB_CONSTANT("None", "i", GDocument::None),
  GB_CONSTANT("Gambas", "i", GDocument::Gambas),
  GB_CONSTANT("Custom", "i", GDocument::Custom),*/

  GB_CONSTANT("Background", "i", HIGHLIGHT_BACKGROUND),
  GB_CONSTANT("Normal", "i", HIGHLIGHT_NORMAL),
  GB_CONSTANT("Keyword", "i", HIGHLIGHT_KEYWORD),
  GB_CONSTANT("Function", "i", HIGHLIGHT_SUBR),
  GB_CONSTANT("Operator", "i", HIGHLIGHT_OPERATOR),
  GB_CONSTANT("Symbol", "i", HIGHLIGHT_SYMBOL),
  GB_CONSTANT("Number", "i", HIGHLIGHT_NUMBER),
  GB_CONSTANT("String", "i", HIGHLIGHT_STRING),
  GB_CONSTANT("Comment", "i", HIGHLIGHT_COMMENT),
  GB_CONSTANT("Breakpoint", "i", HIGHLIGHT_BREAKPOINT),
  GB_CONSTANT("Current", "i", HIGHLIGHT_CURRENT),
  GB_CONSTANT("DataType", "i", HIGHLIGHT_DATATYPE),
  GB_CONSTANT("Selection", "i", HIGHLIGHT_SELECTION),
  GB_CONSTANT("Highlight", "i", HIGHLIGHT_HIGHLIGHT),
  GB_CONSTANT("CurrentLine", "i", HIGHLIGHT_LINE),
  GB_CONSTANT("Error", "i", HIGHLIGHT_ERROR),
  
  GB_STATIC_METHOD("_exit", NULL, CHIGHLIGHT_exit, NULL),
  GB_STATIC_METHOD("Analyze", "String[]", CHIGHLIGHT_analyze, "(Code)s"),
  GB_STATIC_PROPERTY_READ("Symbols", "String[]", CHIGHLIGHT_analyze_symbols),
  GB_STATIC_PROPERTY_READ("Types", "Integer[]", CHIGHLIGHT_analyze_types),
  GB_STATIC_PROPERTY_READ("Positions", "Integer[]", CHIGHLIGHT_analyze_positions),
  GB_STATIC_METHOD("Purge", "s", CHIGHLIGHT_purge, "(Code)s[(Comment)b(String)b]"),

  /*GB_STATIC_PROPERTY("State", "i", CHIGHLIGHT_state),
  GB_STATIC_PROPERTY("Tag", "i", CHIGHLIGHT_tag),
  GB_STATIC_PROPERTY("ShowLimit", "b", CHIGHLIGHT_show_limit),
  GB_STATIC_PROPERTY("Text", "s", CHIGHLIGHT_text),
  GB_STATIC_METHOD("Add", NULL, CHIGHLIGHT_add, "(State)i[(Count)i]"),*/

  GB_END_DECLARE
};
