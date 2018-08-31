/***************************************************************************

  gb.eval.h

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

#ifndef __GB_EVAL_H
#define __GB_EVAL_H

#include "gambas.h"
#include "gbc_read_common.h"

/*#define DEBUG*/

typedef
	bool (*EVAL_FUNCTION)(const char *, int, GB_VARIANT *);

typedef
	void *EVAL_EXPRESSION;

#define EVAL_COLOR_MAX_LEN (1 << 10)	

typedef
	struct {
		unsigned state : 5;
		unsigned alternate : 1;
		unsigned len : 10;
	}
	EVAL_COLOR;
	
typedef
	EVAL_COLOR *EVAL_COLOR_ARRAY;

typedef
	struct
	{
		char *str;
		EVAL_COLOR *color;
		int len;
		int proc;
		int state;
	}
	EVAL_ANALYZE;

#define EVAL_NORMAL       0
#define EVAL_USE_CONTEXT  1

enum {
	RT_DATATYPE = 13,
	RT_ERROR = 14,
	RT_HELP = 15,
	RT_PREPROCESSOR = 16,
	RT_ESCAPE = 17,
	RT_LABEL = 18,
	RT_CONSTANT = 19
	};

typedef
	enum
	{
		HIGHLIGHT_BACKGROUND,
		HIGHLIGHT_NORMAL,
		HIGHLIGHT_KEYWORD,
		HIGHLIGHT_SUBR,
		HIGHLIGHT_OPERATOR,
		HIGHLIGHT_SYMBOL,
		HIGHLIGHT_NUMBER,
		HIGHLIGHT_STRING,
		HIGHLIGHT_COMMENT,
		HIGHLIGHT_BREAKPOINT,
		HIGHLIGHT_CURRENT,
		HIGHLIGHT_DATATYPE,
		HIGHLIGHT_SELECTION,
		HIGHLIGHT_HIGHLIGHT,
		HIGHLIGHT_LINE,
		HIGHLIGHT_ERROR,
		HIGHLIGHT_HELP,
		HIGHLIGHT_PREPROCESSOR,
		HIGHLIGHT_ESCAPE,
		HIGHLIGHT_LABEL,
		HIGHLIGHT_CONSTANT,
		HIGHLIGHT_NUM_COLOR
	}
	HIGHLIGHT_COLOR;

typedef
	struct {
		int version;
		void (*Analyze)(const char *src, int len, int state, EVAL_ANALYZE *result, bool rewrite);
		void (*New)(EVAL_EXPRESSION *expr, const char *src, int len);
		bool (*Compile)(EVAL_EXPRESSION expr, bool assign);
		GB_VALUE *(*Run)(EVAL_EXPRESSION expr, EVAL_FUNCTION func);
		void (*Free)(EVAL_EXPRESSION *expr);
		bool (*GetAssignmentSymbol)(EVAL_EXPRESSION expr, const char **sym, int *len);
		}
	EVAL_INTERFACE;

#define EVAL_INTERFACE_VERSION 2

#endif

