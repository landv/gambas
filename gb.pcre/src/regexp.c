/***************************************************************************

	regexp.c

	(c) 2004 Rob Kudla <pcre-component@kudla.org>
	(c) 2000-2011 Benoît Minisini <gambas@users.sourceforge.net>

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

/***************************************************************************

	Regexp

***************************************************************************/

static void compile(void *_object)
{
	int errptr;
	const char *errstr;

	THIS->code = pcre_compile(THIS->pattern, THIS->copts, &errstr, &errptr, NULL);

	if (!THIS->code)
		GB.Error(errstr);
}

static void exec(void *_object)
{
	if (!THIS->code) 
	{
		GB.Error("No pattern compiled yet");
		return;
	}
	
	if (!THIS->subject) 
	{
		GB.Error("No subject provided");
		return;
	}

	THIS->count = pcre_exec(THIS->code,
			NULL,
			THIS->subject,
			GB.StringLength(THIS->subject),
			0,
			THIS->eopts,
			THIS->ovector,
			99);
	
	// TODO: if count == 0, it means that ovector is too small, and count is 99/3 then.
}

static void return_match(void *_object, int index)
{
	if (index < 0 || index >= THIS->count)
	{
		GB.Error("Out of bounds");
		return;
	}
	
	index *= 2;
	GB.ReturnNewString(&THIS->subject[THIS->ovector[index]], THIS->ovector[index + 1] - THIS->ovector[index]);
}

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
	exec(THIS);

END_METHOD


BEGIN_METHOD(RegExp_new, GB_STRING subject; GB_STRING pattern; GB_INTEGER coptions; GB_INTEGER eoptions)

	GB.Alloc(POINTER(&THIS->ovector), sizeof(int) * 99);

	if (MISSING(pattern)) // the user didn't provide a pattern.
		return;

	THIS->copts = VARGOPT(coptions, 0);
	THIS->pattern = GB.NewString(STRING(pattern), LENGTH(pattern));

	compile(THIS);
	if (!THIS->code) // we didn't get a compiled pattern back.
			return;

	if (MISSING(subject)) // the user didn't specify any subject text.
		return;

	THIS->eopts = VARGOPT(eoptions, 0);
	THIS->subject = GB.NewString(STRING(subject), LENGTH(subject));

	exec(THIS);

END_METHOD


BEGIN_METHOD_VOID(RegExp_free)

	GB.FreeString(&THIS->subject);
	GB.FreeString(&THIS->pattern);
	GB.Free(POINTER(&THIS->ovector));

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
		GB.ReturnNull();
	else
		return_match(THIS, 0);

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


GB_DESC CRegexpDesc[] =
{
	GB_DECLARE("Regexp", sizeof(CREGEXP)),

	GB_METHOD("_new", NULL, RegExp_new, "[(Subject)s(Pattern)s(CompileOptions)i(ExecOptions)i]"),
	GB_METHOD("_free", NULL, RegExp_free, NULL),
	
	GB_METHOD("Compile", NULL, RegExp_Compile, "(Pattern)s[(CompileOptions)i]"),
	GB_METHOD("Exec", NULL, RegExp_Exec, "(Subject)s[(ExecOptions)i]"),
	
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

	GB_PROPERTY_SELF("SubMatches", ".RegExpSubmatches"),
	
	GB_PROPERTY_READ("Text", "s", RegExp_Text), /* this is the string matched by the entire pattern */
	GB_PROPERTY_READ("Offset", "i", RegExp_Offset), /* this is the string matched by the entire pattern */
	GB_PROPERTY_READ("Pattern", "s", RegExp_Pattern),
	GB_PROPERTY_READ("Subject", "s", RegExp_Subject),

	GB_END_DECLARE
};

GB_DESC CRegexpSubmatchesDesc[] =
{
	GB_DECLARE(".RegexpSubmatches", 0), GB_VIRTUAL_CLASS(),

	GB_METHOD("_get", ".RegexpSubmatch", RegExp_Submatches_get, "(Index)i"),
	GB_PROPERTY_READ("Count", "i", RegExp_Submatches_Count),

	GB_END_DECLARE
};

GB_DESC CRegexpSubmatchDesc[] =
{
	GB_DECLARE(".RegexpSubmatch", 0), GB_VIRTUAL_CLASS(),

	GB_PROPERTY_READ("Offset", "i", RegExp_Submatch_Offset),
	GB_PROPERTY_READ("Text", "s", RegExp_Submatch_Text),

	GB_END_DECLARE
};

