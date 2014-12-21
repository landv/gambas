/***************************************************************************

  regexp.c

  (c) 2004 Rob Kudla <pcre-component@kudla.org>
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

#define __REGEXP_C

#include "gb_common.h"

#include "regexp.h"
#include "main.h"

//#define DEBUG_REPLACE 1

#define OVECSIZE_INC 99

DECLARE_METHOD(RegExp_free);

//---------------------------------------------------------------------------

static void compile(void *_object)
{
	int errptr;
	const char *errstr;

	if (!THIS->pattern) {
		GB.Error("No pattern provided");
		return;
	}

	if (THIS->code)
		free(THIS->code);

	THIS->code = pcre_compile(THIS->pattern, THIS->copts, &errstr, &errptr, NULL);

	if (!THIS->code)
	{
		THIS->error = errptr;
		GB.Error(errstr);
	}
}

static void exec(void *_object, int lsubject)
{
	int ret;
	char code[8];
	
	if (!THIS->code) 
	{
		GB.Error("No pattern compiled yet");
		return;
	}
	
	if (lsubject < 0)
		lsubject = GB.StringLength(THIS->subject);

	if (!THIS->subject)
	{
		GB.Error("No subject provided");
		return;
	}

	for(;;)
	{
		ret = pcre_exec(THIS->code,
				NULL,
				THIS->subject,
				lsubject,
				0,
				THIS->eopts,
				THIS->ovector,
				THIS->ovecsize);
		
		if (ret > 0)
		{
			THIS->error = 0;
			THIS->count = ret;
			break;
		}
		else if (ret < 0)
		{
			THIS->error = ret;
			
			switch (ret)
			{
				case PCRE_ERROR_NOMATCH:
					THIS->count = 0; return;
				case PCRE_ERROR_NULL:
					GB.Error("Pattern or subject is null"); return;
				case PCRE_ERROR_BADOPTION:
					GB.Error("Unknown option"); return;
				case PCRE_ERROR_BADMAGIC:
				case PCRE_ERROR_UNKNOWN_OPCODE:
					GB.Error("Incorrect PCRE bytecode"); return;
				case PCRE_ERROR_NOMEMORY:
					GB.Error("Out of memory"); return;
				case PCRE_ERROR_BADUTF8:
				#ifdef PCRE_ERROR_SHORTUTF8
				case PCRE_ERROR_SHORTUTF8:
				#endif
					GB.Error("Bad UTF-8 string"); return;
				#ifdef PCRE_ERROR_BADUTF8_OFFSET
				case PCRE_ERROR_BADUTF8_OFFSET:
					GB.Error("Bad UTF-8 offset"); return;
				#endif
				case PCRE_ERROR_INTERNAL:
					GB.Error("Unexpected internal error"); return;
				case PCRE_ERROR_BADNEWLINE:
					GB.Error("Invalid combination of newline options"); return;
				//case PCRE_ERROR_RECURSELOOP:
				//	GB.Error("Recursion loop detected"); return;
				//case PCRE_ERROR_JIT_STACKLIMIT:
				//	GB.Error("JIT stack limit reached"); return;
				default:
					sprintf(code, "%d", -ret);
					GB.Error("Unable to run regular expression: error #&1", code);
					return;
			}
		}
		
		THIS->ovecsize += OVECSIZE_INC;
		GB.Realloc(POINTER(&THIS->ovector), THIS->ovecsize * sizeof(int));
	}
}

static void return_match(void *_object, int index)
{
	int len;

	if (index < 0 || index >= THIS->count)
	{
		GB.Error("Out of bounds");
		return;
	}
	
	index *= 2;
	len = THIS->ovector[index + 1] - THIS->ovector[index];
	if (len <= 0)
		GB.ReturnVoidString();
	else
		GB.ReturnNewString(&THIS->subject[THIS->ovector[index]], len);
}

bool REGEXP_match(const char *subject, int lsubject, const char *pattern, int lpattern, int coptions, int eoptions)
{
	/*
	 * The gb.pcre internal routines don't require the GB_BASE to be
	 * initialised by Gambas!
	 */
	
	CREGEXP tmp;
	bool ret = FALSE;

	CLEAR(&tmp);
	tmp.ovecsize = OVECSIZE_INC;
	GB.Alloc(POINTER(&tmp.ovector), sizeof(int) * tmp.ovecsize);
	tmp.copts = coptions;
	tmp.pattern = GB.NewString(pattern, lpattern);

	compile(&tmp);
	
	if (tmp.code) 
	{
		tmp.eopts = eoptions;
		tmp.subject = GB.NewString(subject, lsubject);

		exec(&tmp, -1);
		ret = (tmp.ovector[0] != -1);
	}

	RegExp_free(&tmp, NULL);
	
	return ret;
}

//---------------------------------------------------------------------------

BEGIN_METHOD(RegExp_Compile, GB_STRING pattern; GB_INTEGER coptions)

	THIS->copts = VARGOPT(coptions, 0);

	GB.FreeString(&THIS->pattern);
	THIS->pattern = GB.NewString(STRING(pattern), LENGTH(pattern));
	compile(THIS);

END_METHOD


BEGIN_METHOD(RegExp_Exec, GB_STRING subject; GB_INTEGER eoptions)

	THIS->eopts = VARGOPT(eoptions, 0);
	
	GB.FreeString(&THIS->subject);
	THIS->subject = GB.NewString(STRING(subject), LENGTH(subject));
	exec(THIS, -1);

END_METHOD


BEGIN_METHOD(RegExp_new, GB_STRING subject; GB_STRING pattern; GB_INTEGER coptions; GB_INTEGER eoptions)

	THIS->ovecsize = OVECSIZE_INC;
	GB.Alloc(POINTER(&THIS->ovector), sizeof(int) * THIS->ovecsize);

	if (MISSING(pattern)) // the user didn't provide a pattern.
		return;

	THIS->copts = VARGOPT(coptions, 0);
	THIS->pattern = GB.NewString(STRING(pattern), LENGTH(pattern));
	THIS->code = NULL;

	compile(THIS);
	if (!THIS->code) // we didn't get a compiled pattern back.
			return;

	if (MISSING(subject)) // the user didn't specify any subject text.
		return;

	THIS->eopts = VARGOPT(eoptions, 0);
	THIS->subject = GB.NewString(STRING(subject), LENGTH(subject));

	exec(THIS, -1);

END_METHOD


BEGIN_METHOD_VOID(RegExp_free)

	if (THIS->code)
		free(THIS->code);
	GB.FreeString(&THIS->subject);
	GB.FreeString(&THIS->pattern);
	GB.Free(POINTER(&THIS->ovector));

END_METHOD


BEGIN_METHOD(RegExp_Match, GB_STRING subject; GB_STRING pattern; GB_INTEGER coptions; GB_INTEGER eoptions)

	GB.ReturnBoolean(REGEXP_match(STRING(subject), LENGTH(subject), STRING(pattern), LENGTH(pattern), VARGOPT(coptions, 0), VARGOPT(eoptions, 0)));

END_METHOD


BEGIN_PROPERTY(RegExp_Pattern)

	GB.ReturnString(THIS->pattern);

END_PROPERTY


BEGIN_PROPERTY(RegExp_Subject)

	GB.ReturnString(THIS->subject);

END_PROPERTY


BEGIN_PROPERTY(RegExp_Offset)

	GB.ReturnInteger(THIS->ovector[0]);

END_PROPERTY


BEGIN_PROPERTY(RegExp_Text)

	if (THIS->count == 0)
		GB.ReturnVoidString();
	else
		return_match(THIS, 0);

END_PROPERTY


BEGIN_PROPERTY(RegExp_Error)

	GB.ReturnInteger(THIS->error);

END_PROPERTY


BEGIN_PROPERTY(RegExp_Submatches)

	GB.Deprecated("gb.pcre", "Regexp.Submatches", NULL);
	GB.ReturnSelf(THIS);

END_PROPERTY


BEGIN_PROPERTY(RegExp_Submatches_Count)

	GB.ReturnInteger(THIS->count - 1);

END_PROPERTY


BEGIN_METHOD(RegExp_Submatches_get, GB_INTEGER index)

	int index = VARG(index);

	if (index < 0 || index >= THIS->count)
	{
		GB.Error("Out of bounds");
		return;
	}
	
	THIS->_submatch = index;
	RETURN_SELF();

END_METHOD


BEGIN_PROPERTY(RegExp_Submatch_Text)

	return_match(THIS, THIS->_submatch);

END_PROPERTY


BEGIN_PROPERTY(RegExp_Submatch_Offset)

	GB.ReturnInteger(THIS->ovector[2 * THIS->_submatch]);

END_PROPERTY


static CREGEXP *_subst_regexp = NULL;

static void subst_get_submatch(int index, const char **p, int *lp)
{
	if (index <= 0 || index >= _subst_regexp->count)
	{
		*p = NULL;
		*lp = 0;
	}
	else
	{
		index *= 2;
		*p = &_subst_regexp->subject[_subst_regexp->ovector[index]];
		*lp = _subst_regexp->ovector[index + 1] - _subst_regexp->ovector[index];
	}
}

BEGIN_METHOD(RegExp_Replace, GB_STRING subject; GB_STRING pattern; GB_STRING replace; GB_INTEGER coptions; GB_INTEGER eoptions)

	CREGEXP r;
	char *replace;
	char *result = NULL;
	char *subject;
	int offset;

	CLEAR(&r);
	r.ovecsize = OVECSIZE_INC;
	GB.Alloc(POINTER(&r.ovector), sizeof(int) * r.ovecsize);
	r.copts = VARGOPT(coptions, 0) | PCRE_UNGREEDY;
	r.pattern = GB.NewString(STRING(pattern), LENGTH(pattern));

	compile(&r);

	if (r.code)
	{
		r.eopts = VARGOPT(eoptions, 0);
		subject = GB.NewString(STRING(subject), LENGTH(subject));

		offset = 0;

		while (offset < LENGTH(subject))
		{
			r.subject = &subject[offset];
			#if DEBUG_REPLACE
			fprintf(stderr, "\nsubject: (%d) %s\n", offset, r.subject);
			#endif
			exec(&r, GB.StringLength(subject) - offset);

			if (r.ovector[0] < 0)
				break;

			_subst_regexp = &r;

			if (r.ovector[0] > 0)
			{
			#if DEBUG_REPLACE
				fprintf(stderr, "add: (%d) %.*s\n", r.ovector[0], r.ovector[0], r.subject);
			#endif
				result = GB.AddString(result, r.subject, r.ovector[0]);
			#if DEBUG_REPLACE
				fprintf(stderr, "result = %s\n", result);
			#endif
			}

			replace = GB.SubstString(STRING(replace), LENGTH(replace), (GB_SUBST_CALLBACK)subst_get_submatch);
			#if DEBUG_REPLACE
			fprintf(stderr, "replace = %s\n", replace);
			#endif
			result = GB.AddString(result, replace, GB.StringLength(replace));
			#if DEBUG_REPLACE
			fprintf(stderr, "result = %s\n", result);
			#endif

			offset += r.ovector[1];

			if (*r.pattern == '^')
				break;
		}

		if (offset < LENGTH(subject))
			result = GB.AddString(result, &subject[offset], LENGTH(subject) - offset);

		_subst_regexp = NULL;

		GB.FreeStringLater(result);
		#if DEBUG_REPLACE
		fprintf(stderr, "result = %s\n", result);
		#endif
		r.subject = subject;
	}

	RegExp_free(&r, NULL);

	GB.ReturnString(result);

END_METHOD

//---------------------------------------------------------------------------

GB_DESC CRegexpDesc[] =
{
	GB_DECLARE("RegExp", sizeof(CREGEXP)),

	GB_METHOD("_new", NULL, RegExp_new, "[(Subject)s(Pattern)s(CompileOptions)i(ExecOptions)i]"),
	GB_METHOD("_free", NULL, RegExp_free, NULL),
	
	GB_METHOD("Compile", NULL, RegExp_Compile, "(Pattern)s[(CompileOptions)i]"),
	GB_METHOD("Exec", NULL, RegExp_Exec, "(Subject)s[(ExecOptions)i]"),

	GB_STATIC_METHOD("Match", "b", RegExp_Match, "(Subject)s(Pattern)s[(CompileOptions)i(ExecOptions)i]"),
	GB_STATIC_METHOD("Replace", "s", RegExp_Replace, "(Subject)s(Pattern)s(Replace)s[(CompileOptions)i(ExecOptions)i]"),

	GB_CONSTANT("Caseless", "i", PCRE_CASELESS),
	GB_CONSTANT("MultiLine", "i", PCRE_MULTILINE),
	GB_CONSTANT("DotAll", "i", PCRE_DOTALL),
	GB_CONSTANT("Extended", "i", PCRE_EXTENDED),
	GB_CONSTANT("Anchored", "i", PCRE_ANCHORED),
	GB_CONSTANT("DollarEndOnly", "i", PCRE_DOLLAR_ENDONLY),
	GB_CONSTANT("Extra", "i", PCRE_EXTRA),
	GB_CONSTANT("NotBOL", "i", PCRE_NOTBOL),
	GB_CONSTANT("NotEOL", "i", PCRE_NOTEOL),
	GB_CONSTANT("Ungreedy", "i", PCRE_UNGREEDY),
	GB_CONSTANT("NotEmpty", "i", PCRE_NOTEMPTY),
	GB_CONSTANT("UTF8", "i", PCRE_UTF8),
	GB_CONSTANT("NoAutoCapture", "i", PCRE_NO_AUTO_CAPTURE),
	GB_CONSTANT("NoUTF8Check", "i", PCRE_NO_UTF8_CHECK),
	GB_CONSTANT("NoMatch", "i", PCRE_ERROR_NOMATCH),
	GB_CONSTANT("Null", "i", PCRE_ERROR_NULL),
	GB_CONSTANT("BadOption", "i", PCRE_ERROR_BADOPTION),
	GB_CONSTANT("BadMagic", "i", PCRE_ERROR_BADMAGIC),
	GB_CONSTANT("UnknownNode", "i", PCRE_ERROR_UNKNOWN_NODE),
	GB_CONSTANT("NoMemory", "i", PCRE_ERROR_NOMEMORY),
	GB_CONSTANT("NoSubstring", "i", PCRE_ERROR_NOSUBSTRING),
	GB_CONSTANT("MatchLimit", "i", PCRE_ERROR_MATCHLIMIT),
	GB_CONSTANT("Callout", "i", PCRE_ERROR_CALLOUT),
	GB_CONSTANT("BadUTF8", "i", PCRE_ERROR_BADUTF8),
#if (((PCRE_MAJOR == 4) && (PCRE_MINOR < 5)) || (PCRE_MAJOR < 4))
	GB_CONSTANT("BadUTF8Offset", "i", 65535), /* PCRE_ERROR_BADUTF8_OFFSET not defined < 4.5 */
#else
	GB_CONSTANT("BadUTF8Offset", "i", PCRE_ERROR_BADUTF8_OFFSET),
#endif

	GB_PROPERTY_READ("SubMatches", ".Regexp.Submatches", RegExp_Submatches),
	
	GB_PROPERTY_READ("Text", "s", RegExp_Text), /* this is the string matched by the entire pattern */
	GB_PROPERTY_READ("Offset", "i", RegExp_Offset), /* this is the string matched by the entire pattern */
	GB_PROPERTY_READ("Pattern", "s", RegExp_Pattern),
	GB_PROPERTY_READ("Subject", "s", RegExp_Subject),
	GB_PROPERTY_READ("Error", "i", RegExp_Error),

	GB_METHOD("_get", ".Regexp.Submatch", RegExp_Submatches_get, "(Index)i"),
	GB_PROPERTY_READ("Count", "i", RegExp_Submatches_Count),

	GB_END_DECLARE
};

GB_DESC CRegexpSubmatchesDesc[] =
{
	GB_DECLARE(".Regexp.Submatches", 0), GB_VIRTUAL_CLASS(),

	GB_METHOD("_get", ".Regexp.Submatch", RegExp_Submatches_get, "(Index)i"),
	GB_PROPERTY_READ("Count", "i", RegExp_Submatches_Count),

	GB_END_DECLARE
};

GB_DESC CRegexpSubmatchDesc[] =
{
	GB_DECLARE(".Regexp.Submatch", 0), GB_VIRTUAL_CLASS(),

	GB_PROPERTY_READ("Offset", "i", RegExp_Submatch_Offset),
	GB_PROPERTY_READ("Text", "s", RegExp_Submatch_Text),

	GB_END_DECLARE
};

