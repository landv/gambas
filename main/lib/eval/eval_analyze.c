/***************************************************************************

  eval_analyze.c

  (c) 2000-2009 Benoît Minisini <gambas@users.sourceforge.net>

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
  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

***************************************************************************/

#define __EVAL_ANALYZE_C

#include "gambas.h"
#include "gb_common.h"
#include "eval_analyze.h"

#include "CSystem.h"
/*#define DEBUG*/

static EVAL_COLOR colors[EVAL_MAX_COLOR];
static int colors_len;

static int get_type(PATTERN *pattern)
{
  int type = PATTERN_type(*pattern);
  int index = PATTERN_index(*pattern);

  if (type == EVAL_TYPE_RESERVED)
  {
    if (index >= RS_COLON)
    {
      if (!((index == RS_AND || index == RS_OR) && PATTERN_is(pattern[1], RS_IF)))
        type = EVAL_TYPE_OPERATOR;
    }
    else if (RES_is_type(index))
      type = EVAL_TYPE_DATATYPE;
		else if (index == RS_WITH && pattern > EVAL->pattern)
		{
			index = PATTERN_index(pattern[-1]);
			if (index == RS_BEGINS || index == RS_ENDS)
				type = EVAL_TYPE_OPERATOR;
		}
  }

  return type;
}

static int is_me_last(PATTERN pattern)
{
  return PATTERN_is(pattern, RS_ME)
         || PATTERN_is(pattern, RS_SUPER)
         || PATTERN_is(pattern, RS_LAST)
         || PATTERN_is(pattern, RS_OPTIONAL)
         || PATTERN_is(pattern, RS_TRUE)
         || PATTERN_is(pattern, RS_FALSE)
         || PATTERN_is(pattern, RS_NULL)
         || PATTERN_is(pattern, RS_BYREF);
}

static void get_symbol(PATTERN pattern, const char **symbol, int *len)
{
  static char keyword[16];
  int i;
  SYMBOL *sym;
  int type = PATTERN_type(pattern);
  int index = PATTERN_index(pattern);

  switch(type)
  {
    case RT_RESERVED:
			*symbol = COMP_res_info[index].name;
			*len = strlen(*symbol);
			if (!EVAL->rewrite)
			{
				memcpy(keyword, *symbol, *len);
				for (i = 0; i < *len; i++)
					keyword[i] = toupper(keyword[i]);
				*symbol = keyword;
			}
      return;
    case RT_NUMBER:
    case RT_IDENTIFIER:
    case RT_CLASS:
      sym = TABLE_get_symbol(EVAL->table, index);
      break;
    case RT_STRING:
    case RT_TSTRING:
    case RT_COMMENT:
    case EVAL_TYPE_ERROR:
      sym = TABLE_get_symbol(EVAL->string, index);
      break;
    case RT_SUBR:
      *symbol = COMP_subr_info[index].name;
      *len = strlen(*symbol);
      return;
    default:
      *symbol = NULL;
      *len = 0;
      return;
  }

  *symbol = sym->name;
  *len = sym->len;
  if (*len > EVAL_COLOR_MAX_LEN)
    *len = EVAL_COLOR_MAX_LEN;
}


static void add_data(int state, int len)
{
  EVAL_COLOR *color;

  if (colors_len >= EVAL_MAX_COLOR || len == 0)
    return;

  //printf("[%d] %d %d\n", colors_len, state, len);

  color = &colors[colors_len];
  color->state = state;
  color->len = len;
	color->alternate = FALSE;
  colors_len++;
}


static int is_proc(void)
{
  PATTERN pattern;
  int i;

  if (!EVAL->pattern)
    return FALSE;

  for (i = 0;; i++)
  {
    pattern = EVAL->pattern[i];
    if (PATTERN_is_end(pattern))
      return FALSE;

    if (PATTERN_is(pattern, RS_PRIVATE) || PATTERN_is(pattern, RS_PUBLIC) || PATTERN_is(pattern, RS_STATIC))
      continue;

    return (PATTERN_is(pattern, RS_SUB) || PATTERN_is(pattern, RS_PROCEDURE) || PATTERN_is(pattern, RS_FUNCTION));
  }
}

static int get_indent(bool *empty)
{
  int i;
  unsigned char c;

  *empty = TRUE;

  for (i = 0; i < (int)EVAL->len; i++)
  {
    c = EVAL->source[i];
    if (c > ' ')
    {
      *empty = FALSE;
      break;
    }
  }

  return i;
}

static int get_symbol_indent(const char *symbol, int len)
{
	int i;
	
	for (i = 0; i < len; i++)
	{
		if (!(symbol[i] > 0 && symbol[i] < 33))
			return i;
	}
	
	return len;
}


static int get_utf8_length(const char *s, int l)
{
  int len;
  int i;

  for (i = 0, len = 0; i < l; i++)
  {
    if ((s[i] & 0xC0) != 0x80)
      len++;
  }

  return len;
}


static void analyze(EVAL_ANALYZE *result)
{
  PATTERN *pattern;
  const char *src;
  int nspace;
  bool empty;
  int type, old_type, next_type;
  const char *symbol;
  bool space_before, space_after, in_quote;
  //bool me = FALSE;
  int len, i;

  pattern = EVAL->pattern;
  src = EVAL->source;
  colors_len = 0;

  if (EVAL->len <= 0)
    return;

	if (!EVAL->comment)
	{
		nspace = get_indent(&empty);
		add_data(EVAL_TYPE_END, nspace);
	}

  if (empty)
    return;

  if (!EVAL->pattern)
    return;

  if (nspace)
    GB.AddString(&result->str, EVAL->source, nspace);

  type = EVAL->comment ? EVAL_TYPE_COMMENT : EVAL_TYPE_END;
  next_type = EVAL_TYPE_END;
  space_after = FALSE;
  in_quote = FALSE;

  for(;;)
  {
    old_type = next_type;
    type = get_type(pattern);
    next_type = type;
    get_symbol(*pattern, &symbol, &len);

    space_before = space_after;
    space_after = FALSE;

    if (type == EVAL_TYPE_END)
      break;

		if (in_quote && (type == EVAL_TYPE_RESERVED || type == EVAL_TYPE_DATATYPE || type == EVAL_TYPE_SUBR))
			type = EVAL_TYPE_IDENTIFIER;

    switch(type)
    {
      case EVAL_TYPE_RESERVED:
        //state = Keyword;
        //if (old_type != EVAL_TYPE_OPERATOR)
        //me = is_me_last(*pattern);
        if (!is_me_last(*pattern))
          space_before = TRUE;
        else
        {
          next_type = EVAL_TYPE_IDENTIFIER;
          if (old_type != EVAL_TYPE_OPERATOR)
            space_before = TRUE;
        }

        /*if (PATTERN_index(*pattern) >= RS_COLON)
        {
          int i;

          for (i = 0; i < len; i++)
            usym[i] = GB.toupper(symbol[i]);
          usym[len] = 0;

          symbol = usym;
        }*/

        break;

      case EVAL_TYPE_DATATYPE:
        //state = Datatype;
        if (old_type != EVAL_TYPE_OPERATOR)
          space_before = TRUE;
        break;

      case EVAL_TYPE_IDENTIFIER:
      case EVAL_TYPE_CLASS:
        //state = Symbol;
        if (old_type != EVAL_TYPE_OPERATOR)
          space_before = TRUE;
        break;

      case EVAL_TYPE_NUMBER:
        //state = Number;
        if (old_type != EVAL_TYPE_OPERATOR)
          space_before = TRUE;
        break;

      case EVAL_TYPE_STRING:
        //state = String;
        if (old_type != EVAL_TYPE_OPERATOR)
          space_before = TRUE;
        break;

      case EVAL_TYPE_SUBR:
        //state = Subr;
        if (old_type != EVAL_TYPE_OPERATOR)
          space_before = TRUE;
        break;

      case EVAL_TYPE_COMMENT:
        //state = Commentary;
        space_before = *symbol != ' ';
				i = get_symbol_indent(symbol, len);
				if (i <= (len - 2) && symbol[i + 1] == '\'')
					type = EVAL_TYPE_HELP;
        break;

      case EVAL_TYPE_OPERATOR:

        if (index("([)]@", *symbol))
        {
          space_after = FALSE;
        }
        else if (index(":;,", *symbol))
        {
          space_before = FALSE;
          space_after = TRUE;
        }
        else if (index("#{", *symbol))
        {
          if (old_type != EVAL_TYPE_OPERATOR)
            space_before = TRUE;
          space_after = FALSE;
          //in_quote = *symbol == '{';
        }
        else if (index("}", *symbol))
        {
          space_before = FALSE;
          space_after = FALSE;
          //in_quote = FALSE;
        }
        else if (index(".!", *symbol)) //symbol[0] == '.' && symbol[1] == 0)
        {
          //space_before = FALSE;
          space_after = FALSE;
        }
        else if (PATTERN_is(*pattern, RS_NOT))
        {
        	space_after = TRUE;
        }
        else if (*symbol == '-')
        {
					if (old_type == EVAL_TYPE_OPERATOR && (PATTERN_is(pattern[-1], RS_LBRA) || PATTERN_is(pattern[-1],RS_LSQR)))
        		space_before = FALSE;
					else
  	        space_before = TRUE;

      		space_after = TRUE;
				}
        else
        {
          space_before = TRUE;
          space_after = TRUE;
        }

        if (old_type == EVAL_TYPE_RESERVED)
          space_before = TRUE;

        break;


      case EVAL_TYPE_ERROR:
        space_before = TRUE;
        break;
    }

    if (space_before && old_type != EVAL_TYPE_END)
    {
      GB.AddString(&result->str, " ", 1);
      add_data(EVAL_TYPE_END, 1);
    }

    if (type == EVAL_TYPE_STRING)
      GB.AddString(&result->str, "\"", 1);

    //len = strlen(symbol);

    /*if (type == EVAL_TYPE_IDENTIFIER && len >= 2 &&
       islower(symbol[0]) && islower(symbol[1]))
    {
      char c = toupper(symbol[0]);
      GB.AddString(&result->str, &c, 1);
      GB.AddString(&result->str, &symbol[1], len - 1);
    }
    else*/
    if (len)
    {
      GB.AddString(&result->str, symbol, len);
      //printf("add: %.*s\n", len, symbol);
      len = get_utf8_length(symbol, len);
    }

    if (type == EVAL_TYPE_STRING)
    {
      GB.AddString(&result->str, "\"", 1);
      len += 2;
    }

    add_data(type, len);
    //printf("add_data: %.d (%d)\n", type, len);

    pattern++;
  }

  result->color = colors;
  result->len = colors_len;

  //fprintf(stderr, "analyze: %d %s\n", strlen(result->str), result->str);
}


#define add_pattern(_type, _index) EVAL->pattern[EVAL->pattern_count++] = PATTERN_make((_type), (_index));

static void add_end_pattern(void)
{
  int index;
  int len;

  len = EVAL->len - (READ_source_ptr - EVAL->source);
  if (len > 0)
  {
    TABLE_add_symbol(EVAL->string, READ_source_ptr, len, &index);
    add_pattern(EVAL_TYPE_ERROR, index);
  }
  
  add_pattern(EVAL_TYPE_END, 0);
  //get_symbol(PATTERN_make(EVAL_TYPE_ERROR, index), &sym, &len);
}


PUBLIC void EVAL_analyze(const char *src, int len, int state, EVAL_ANALYZE *result, bool rewrite)
{
	int nspace = 0;

  #ifdef DEBUG
  printf("EVAL: %*.s\n", expr->len, expr->source);
  #endif

  CLEAR(result);

	while (len > 0 && src[len - 1] == ' ')
	{
		len--;
		nspace++;
	}

  result->len = 0;
  result->str = NULL;

  if (len > 0)
  {
		EVAL = &EVAL_read_expr;

		EVAL_clear(EVAL);
		
		GB.NewString(&EVAL->source, src, len);
		GB.AddString(&EVAL->source, "\0\0", 2);
		EVAL->len = len;
		
		EVAL->analyze = TRUE;
		EVAL->rewrite = rewrite;
		EVAL->comment = state == EVAL_TYPE_COMMENT;
		
		//fprintf(stderr, "EVAL_analyze: [%d] %.*s\n", EVAL->comment, len, src);

    EVAL_start(EVAL);

		TRY
		{
			EVAL_read();
		}
		CATCH
		{
			add_end_pattern();
		}
		END_TRY

	  analyze(result);
		result->proc = is_proc();
		result->state = EVAL->comment ? EVAL_TYPE_COMMENT : EVAL_TYPE_END;

		//fprintf(stderr, "--> [%d]\n", EVAL->comment);
		
	  GB.FreeString(&EVAL->source);
	}
	else
	{
		result->proc = FALSE;
	}

  while (nspace > 0)
  {
   	GB.AddString(&result->str, "        ", nspace > 8 ? 8 : nspace);
    nspace -= 8;
	}
}



