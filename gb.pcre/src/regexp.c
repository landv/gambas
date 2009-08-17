/***************************************************************************

  regexp.c

  (c) 2004 Rob Kudla <pcre-component@kudla.org>
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

#define __REGEXP_C

#include "gb_common.h"

#include "regexp.h"
#include "main.h"

static int init = 0;

void REGEXP_init(void)
{
  init++;
  if (init > 1)
    return;
}


void REGEXP_exit(void)
{
  init--;
  if (init > 0)
    return;
}


/***************************************************************************

  Regexp

***************************************************************************/

BEGIN_METHOD_VOID(CREGEXP_private_compile)

     // fprintf(stderr, "debug 21\n");
     int errptr;
     const char *errstr;

     // fprintf(stderr, "debug 22\n");
     // compile the pattern
     // fprintf(stderr, "debug 22\n");

     // fprintf(stderr, "debug 23\n");

     // fprintf(stderr, "debug 24\n");
     THIS->code = pcre_compile(THIS->pattern, THIS->copts, &errstr, &errptr, NULL);

     // fprintf(stderr, "debug 25\n");
     if (THIS->code) {
       THIS->compiled = 1;
     // fprintf(stderr, "compiled OK\n");
     // fprintf(stderr, "compiled is %d\n", THIS->compiled);
     } else {
       GB.Error(errstr);
     // fprintf(stderr, "didn't compile: %s\n", errstr);
     }
     // fprintf(stderr, "debug 26\n");

END_METHOD

BEGIN_METHOD_VOID(CREGEXP_private_exec)

     // fprintf(stderr, "debug 11\n");
     if (!THIS->code) {
       GB.Error("No pattern compiled yet");
     // fprintf(stderr, "No pattern, compiled is %d\n", THIS->compiled);
       return;
     }
     if (!THIS->subject) {
       GB.Error("No subject provided");
       return;
     }

     int *ovector = THIS->ovector;

     // fprintf(stderr, "debug 12\n");
     // fprintf(stderr, "debug 13\n");
     // do the actual match
     // fprintf(stderr, "debug 14\n");
       THIS->rc = pcre_exec(THIS->code,
			    NULL,
			    THIS->subject,
			    strlen(THIS->subject),
			    0,
			    THIS->eopts,
			    ovector,
			    99);

     // fprintf(stderr, "debug 15\n");

END_METHOD

BEGIN_METHOD(CREGEXP_compile, GB_STRING pattern; GB_INTEGER coptions)

	THIS->copts = VARGOPT(coptions, 0);

	GB.FreeString(&THIS->pattern);
	GB.NewString(&THIS->pattern, STRING(pattern), LENGTH(pattern));
	CALL_METHOD_VOID(CREGEXP_private_compile);

END_METHOD

BEGIN_METHOD(CREGEXP_exec, GB_STRING subject; GB_INTEGER eoptions)

	THIS->eopts = VARGOPT(eoptions, 0);
	
	GB.FreeString(&THIS->subject);
	GB.NewString(&THIS->subject, STRING(subject), LENGTH(subject));
	CALL_METHOD_VOID(CREGEXP_private_exec);

END_METHOD

BEGIN_METHOD(CREGEXP_new, GB_STRING subject; GB_STRING pattern; GB_INTEGER coptions; GB_INTEGER eoptions)

	GB.NewArray((void *) &(THIS->smcache), sizeof(*(THIS->smcache)), 0); // smcache is where i keep track of what to free later
	THIS->compiled = 0;
	THIS->ovector = NULL;
	THIS->rc = 0;

	int *ovector = NULL;
	// fprintf(stderr, "debug 11a\n");
	GB.Alloc((void *) &ovector, sizeof(int) * 99);
		THIS->ovector = ovector;

	// fprintf(stderr, "debug 1\n");
	if (MISSING(pattern)) { // the user didn't provide a pattern.
		return;
	}

	// fprintf(stderr, "debug 2\n");
	if (MISSING(coptions)) { // the user didn't provide any execute options.
		THIS->copts = 0;
	} else {
		THIS->copts = VARG(coptions);
	}

     char *tmp = NULL;
if (THIS->pattern) {
  GB.FreeString(&THIS->pattern);
}
     GB.NewString(&tmp, STRING(pattern), LENGTH(pattern));
     THIS->pattern = tmp;

     // fprintf(stderr, "debug 3\n");
     CALL_METHOD_VOID(CREGEXP_private_compile);
// CREGEXP_compile(ARG(pattern), ARG(coptions));
     // fprintf(stderr, "compiled is %d\n", THIS->compiled);

     // fprintf(stderr, "debug 4\n");
     if (!THIS->code) { // we didn't get a compiled pattern back.
       return;
     }

     // fprintf(stderr, "debug 5\n");
     if (MISSING(subject)) { // the user didn't specify any subject text.
       return;
     }

     // fprintf(stderr, "compiled is %d\n", THIS->compiled);
     // fprintf(stderr, "debug 6\n");
     if (MISSING(eoptions)) { // the user didn't provide any execute options.
       THIS->eopts = 0;
     } else {
       THIS->eopts = VARG(eoptions);
     }

if (THIS->subject) {
  GB.FreeString(&THIS->subject);
}
       GB.NewString(&tmp, STRING(subject), LENGTH(subject));
       THIS->subject = tmp;

     // fprintf(stderr, "debug 7\n");
     // fprintf(stderr, "compiled is %d\n", THIS->compiled);
// fprintf(stderr, "Subject contains %s\n", THIS->subject);
     CALL_METHOD_VOID(CREGEXP_private_exec);
     // fprintf(stderr, "debug 8\n");
     // fprintf(stderr, "debug 9\n");
     return;
     // fprintf(stderr, "debug 10\n");

END_METHOD

BEGIN_METHOD_VOID(CREGEXP_free)

  GB.FreeString(&(THIS->subject));
  GB.FreeString(&(THIS->pattern));
  int i = 0;
  for (i = 0; i < GB.Count(THIS->smcache); i++) {
    GB.FreeString(&(THIS->smcache[i]));
  }
  GB.FreeArray((void *) &(THIS->smcache));
  GB.Free((void *) &(THIS->ovector));

END_METHOD

BEGIN_PROPERTY(CREGEXP_Offset)

     GB.ReturnInteger((THIS->ovector)[0]);

END_PROPERTY

BEGIN_PROPERTY(CREGEXPSUBMATCH_Count)

     GB.ReturnInteger(THIS->rc - 1);

END_PROPERTY

BEGIN_PROPERTY(CREGEXP_Text)

     const char *substring_start = THIS->subject + (THIS->ovector)[0];
     long substring_length = (THIS->ovector)[1] - (THIS->ovector)[0];
     char *str = NULL;
     char **tmp = NULL;

     GB.NewString(&str, substring_start, substring_length);
     tmp = (char **) GB.Add((void *) &(THIS->smcache));
     *tmp = str;
     GB.ReturnString(str);

END_PROPERTY

BEGIN_PROPERTY(CREGEXPSUBMATCH_Text)

     int i = THIS->_submatch;
     int *ovector = THIS->ovector;
     int rc = THIS->rc;
     const char *str = NULL;
     char *submatch = NULL;
     char **tmp = NULL;

     if (i < rc) {
       pcre_get_substring(THIS->subject, ovector, rc, i, &str);
       GB.NewString(&submatch, str, 0);
       tmp = (char **) GB.Add((void *) &(THIS->smcache));
       *tmp = submatch;
       pcre_free_substring(str);
     } else {
       GB.Error("Submatch index out of bounds");
       return;
     }

     GB.ReturnString(submatch);
     return;

END_PROPERTY

BEGIN_PROPERTY(CREGEXPSUBMATCHES_count)

     GB.ReturnInteger(THIS->rc - 1);

END_PROPERTY

BEGIN_PROPERTY(CREGEXPSUBMATCH_Offset)

     int i = THIS->_submatch;
     int *ovector = THIS->ovector;
     int rc = THIS->rc;

     if (i < rc) {
       GB.ReturnInteger(ovector[2*i]);
     } else {
       GB.Error("Submatch index out of bounds");
       return;
     }

     return;

END_PROPERTY

BEGIN_METHOD(CREGEXPSUBMATCHES_get, GB_INTEGER index)

     int i = VARG(index);
     int rc = THIS->rc;

     if (i < rc && i >= 0) {
       THIS->_submatch = i;
       RETURN_SELF();
     } else {
       GB.Error("Submatch index out of bounds");
       return;
     }

END_METHOD

BEGIN_PROPERTY(CREGEXP_submatches)

     RETURN_SELF();

END_METHOD

GB_DESC CRegexpDesc[] =
{
  GB_DECLARE("Regexp", sizeof(CREGEXP)),

  GB_METHOD("_new", NULL, CREGEXP_new, "[(Subject)s(Pattern)s(CompileOptions)i(ExecOptions)i]"),
  GB_METHOD("_free", NULL, CREGEXP_free, NULL),
  GB_METHOD("Compile", NULL, CREGEXP_compile, "(Pattern)s[(CompileOptions)i]"),
  GB_METHOD("Exec", NULL, CREGEXP_exec, "(Subject)s[(ExecOptions)i]"),
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
  //GB_METHOD("SubMatch", ".RegexpSubmatches", CREGEXP_submatch, "(Index)i"),
  GB_PROPERTY_SELF("SubMatches", ".RegExpSubmatches"),
  GB_PROPERTY_READ("Text", "s", CREGEXP_Text), /* this is the string matched by the entire pattern */
  GB_PROPERTY_READ("Offset", "i", CREGEXP_Offset), /* this is the string matched by the entire pattern */

  GB_END_DECLARE
};

GB_DESC CRegexpSubmatchesDesc[] =
{
  GB_DECLARE(".RegexpSubmatches", 0), GB_VIRTUAL_CLASS(),

  GB_METHOD("_get", ".RegexpSubmatch", CREGEXPSUBMATCHES_get, "(Index)i"),
  GB_PROPERTY_READ("Count", "i", CREGEXPSUBMATCHES_count),

  GB_END_DECLARE
};

GB_DESC CRegexpSubmatchDesc[] =
{
  GB_DECLARE(".RegexpSubmatch", 0), GB_VIRTUAL_CLASS(),

  GB_PROPERTY_READ("Offset", "i", CREGEXPSUBMATCH_Offset),
  GB_PROPERTY_READ("Text", "s", CREGEXPSUBMATCH_Text),
  GB_PROPERTY_READ("Count", "i", CREGEXPSUBMATCH_Count),

  GB_END_DECLARE
};

