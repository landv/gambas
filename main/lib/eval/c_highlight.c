/***************************************************************************

  c_highlight.c

  (c) 2000-2017 Benoît Minisini <g4mba5@gmail.com>

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

#define __C_HIGHLIGHT_CPP

#include "gb_common.h"
#include "main.h"
#include "gb.eval.h"
#include "eval_analyze.h"

#include "c_highlight.h"

static void *_analyze_symbol = 0;
static void *_analyze_type = 0;
static void *_analyze_pos = 0;
static char *_analyze_text = 0;

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
	int lc, ls;
	bool in_comment = FALSE;
	char wait = 0;
	char *r = NULL;
	
	for (i = 0; i < len_s; i += ls)
	{
		c = s[i];
		ls = lc = get_char_length((unsigned char)c);

		switch(wait)
		{
			case 0:

				if (in_comment)
				{
					if (!comment)
						c = ' ', lc = 1;
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
							r = GB.AddChar(r, c);
							//r += c;
						i++;
						c = s[i];
						lc = get_char_length((unsigned char)c);
					}
					else
					{
						i++;
						if (i < len_s)
							r = GB.AddChar(r, ' ');
							//r += ' ';
						c = ' ', lc = 1;
					}
				}
				else
				{
					if (!string)
						c = ' ', lc = 1;
				}
				break;
		}

		if (lc == 1)
			r = GB.AddChar(r, c);
		else
			r = GB.AddString(r, &s[i], lc);
	}

	GB.FreeString(&_purged_line);
	_purged_line = r;
	
	return r;
}


static int convState(int state)
{
	switch(state)
	{
		case RT_END: return HIGHLIGHT_NORMAL;
		case RT_RESERVED: return HIGHLIGHT_KEYWORD;
		case RT_IDENTIFIER: return HIGHLIGHT_SYMBOL;
		case RT_CLASS: return HIGHLIGHT_DATATYPE;
		case RT_NUMBER: return HIGHLIGHT_NUMBER;
		case RT_STRING: return HIGHLIGHT_STRING;
		case RT_SUBR: return HIGHLIGHT_SUBR;
		case RT_COMMENT: return HIGHLIGHT_COMMENT;
		case RT_OPERATOR: return HIGHLIGHT_OPERATOR;
		case RT_DATATYPE: return HIGHLIGHT_DATATYPE;
		case RT_ERROR: return HIGHLIGHT_ERROR;
		case RT_HELP: return HIGHLIGHT_HELP;
		case RT_PREPROCESSOR: return HIGHLIGHT_PREPROCESSOR;
		case RT_ESCAPE: return HIGHLIGHT_ESCAPE;
		case RT_CONSTANT: return HIGHLIGHT_CONSTANT;
		case RT_LABEL: return HIGHLIGHT_LABEL;
		default: return HIGHLIGHT_NORMAL;
	}
}

static void analyze(const char *src, int len_src, bool rewrite, int state)
{
	GB_ARRAY garray, tarray, parray;
	int i, n, pos, len, p, upos, ulen, l;
	char *str;
	EVAL_ANALYZE result;

	EVAL_analyze(src, len_src, state == HIGHLIGHT_COMMENT ? RT_COMMENT : RT_END, &result, rewrite);
	
	n = 0;
	for (i = 0; i < result.len; i++)
	{
		if (result.color[i].state != RT_END)
			n++;
	}

	GB.Array.New(&garray, GB_T_STRING, n);
	GB.Array.New(&tarray, GB_T_INTEGER, n);
	GB.Array.New(&parray, GB_T_INTEGER, n);

	pos = 0;
	upos = 0;
	i = 0;
	for (p = 0; p < result.len; p++)
	{
		len = result.color[p].len;

		ulen = 0;
		for (l = 0; l < len; l++)
			ulen += get_char_length(result.str[upos + ulen]);
		
		if (result.color[p].state != RT_END)
		{
			str = GB.NewString(&result.str[upos], ulen);
			*((char **)GB.Array.Get(garray, i)) = str;
			*((int *)GB.Array.Get(tarray, i)) = convState(result.color[p].state);
			*((int *)GB.Array.Get(parray, i)) = pos;
			i++;
		}

		pos += len;
		upos += ulen;
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
	
	GB.FreeString(&_analyze_text);
	_analyze_text = result.str;
}

BEGIN_METHOD_VOID(Highlight_exit)

	GB.Unref(&_analyze_symbol);
	GB.Unref(&_analyze_type);
	GB.Unref(&_analyze_pos);
	GB.FreeString(&_analyze_text);
	GB.FreeString(&_purged_line);

END_METHOD



BEGIN_METHOD(Highlight_Purge, GB_STRING text; GB_BOOLEAN comment; GB_BOOLEAN string)

	bool comment = VARGOPT(comment, FALSE);
	bool string = VARGOPT(string, FALSE);
	
	if (comment && string)	
		GB.ReturnNewString(STRING(text), LENGTH(text));
	else
		GB.ReturnString(purge(STRING(text), LENGTH(text), comment, string));

END_METHOD

BEGIN_METHOD(Highlight_Analyze, GB_STRING text; GB_BOOLEAN rewrite; GB_INTEGER state)

	analyze(STRING(text), LENGTH(text), VARGOPT(rewrite, FALSE), VARGOPT(state, HIGHLIGHT_NORMAL));
	GB.ReturnObject(_analyze_symbol);

END_METHOD

BEGIN_PROPERTY(Highlight_Symbols)

	GB.ReturnObject(_analyze_symbol);

END_PROPERTY

BEGIN_PROPERTY(Highlight_Types)

	GB.ReturnObject(_analyze_type);

END_PROPERTY

BEGIN_PROPERTY(Highlight_Positions)

	GB.ReturnObject(_analyze_pos);

END_PROPERTY

BEGIN_PROPERTY(Highlight_TextAfter)

	GB.ReturnString(_analyze_text);

END_PROPERTY

GB_DESC CHighlightDesc[] =
{
	GB_DECLARE("Highlight", 0), GB_VIRTUAL_CLASS(),

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
	GB_CONSTANT("Help", "i", HIGHLIGHT_HELP),
	GB_CONSTANT("Preprocessor", "i", HIGHLIGHT_PREPROCESSOR),
	GB_CONSTANT("Escape", "i", HIGHLIGHT_ESCAPE),
	GB_CONSTANT("Label", "i", HIGHLIGHT_LABEL),
	GB_CONSTANT("Constant", "i", HIGHLIGHT_CONSTANT),
	GB_CONSTANT("Alternate", "i", HIGHLIGHT_ALTERNATE),
	GB_CONSTANT("Custom", "i", HIGHLIGHT_NUM_COLOR),
	
	GB_STATIC_METHOD("_exit", NULL, Highlight_exit, NULL),
	GB_STATIC_METHOD("Analyze", "String[]", Highlight_Analyze, "(Code)s[(Rewrite)b(State)i]"),
	GB_STATIC_PROPERTY_READ("Symbols", "String[]", Highlight_Symbols),
	GB_STATIC_PROPERTY_READ("Types", "Integer[]", Highlight_Types),
	GB_STATIC_PROPERTY_READ("Positions", "Integer[]", Highlight_Positions),
	GB_STATIC_PROPERTY_READ("TextAfter", "s", Highlight_TextAfter),
	GB_STATIC_METHOD("Purge", "s", Highlight_Purge, "(Code)s[(Comment)b(String)b]"),

	GB_END_DECLARE
};
