/***************************************************************************

  c_crypt.c

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

#define __C_CRYPT_C

#include <stdlib.h>
#include <errno.h>
#include <time.h>

#include "gb_common.h"

#ifndef OS_BSD
  #include <crypt.h>
#endif

#include "c_crypt.h"
#include "main.h"

enum { USE_DES, USE_MD5, USE_SHA256, USE_SHA512 };

static char *do_crypt(const char *passwd, const char *prefix, int mode)
{
  static char key_table[65] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcedefghijklmnopqrstuvwxyz./";
  static bool init = FALSE;

  int i, n;
  char key[18];
  char *result;
  char errormsg[64];

	switch(mode)
	{
		case USE_MD5:
			strcpy(key, "$1$");
			n = 8;
			break;
		case USE_SHA256:
			strcpy(key, "$5$");
			n = 13;
			break;
		case USE_SHA512:
			strcpy(key, "$6$");
			n = 13;
			break;
		case USE_DES:
		default:
			n = 2;
	}

  if (prefix)
  {
    if (strlen(prefix) != n)
    {
      snprintf(errormsg, sizeof(errormsg), "must be %d characters long", n);
      goto __BAD_PREFIX;
    }

    for (i = 0; i < n; i++)
    {
      if (strchr(key_table, prefix[i]) == NULL)
      {
        snprintf(errormsg, sizeof(errormsg), "character '%c' is not allowed" , prefix[i]);
        goto __BAD_PREFIX;
      }
    }

    if (mode == USE_DES)
      strcpy(key, prefix);
    else
      strcat(key, prefix);
  }
  else
  {
    if (!init)
    {
      srandom((unsigned int)time(NULL));
      init = TRUE;
    }

    if (mode == USE_DES)
    {
      for (i = 0; i < 2; i++)
        key[i] = key_table[(int)((random() / (RAND_MAX + 1.0)) * sizeof(key_table))];
      key[2] = 0;
    }
    else if (mode == USE_MD5)
    {
      for (i = 0; i < 8; i++)
        key[i + 3] = key_table[(int)((random() / (RAND_MAX + 1.0)) * sizeof(key_table))];
      key[11] = '$';
      key[12] = 0;
    }
    else
    {
      for (i = 0; i < 13; i++)
        key[i + 3] = key_table[(int)((random() / (RAND_MAX + 1.0)) * sizeof(key_table))];
      key[16] = '$';
      key[17] = 0;
    }
  }

  result = crypt(passwd, key);

  if (!result && errno == ENOSYS)
    GB.Error("Crypting is not supported on this system");

  return result;

__BAD_PREFIX:

  GB.Error("Bad prefix, &1",errormsg);
  return NULL;
}

static bool check_crypt(const char *passwd, const char *crypted)
{
  char *result = crypt(passwd, crypted);

  if (!result && errno == ENOSYS)
    GB.Error("Crypting is not supported on this system");

  if (!result)
    return TRUE;
  else
    return strcmp(result, crypted) != 0;
}

BEGIN_METHOD(CCRYPT_call_des, GB_STRING password; GB_STRING key)

  char *result;

  result = do_crypt(GB.ToZeroString(ARG(password)), MISSING(key) ? NULL : GB.ToZeroString(ARG(key)), USE_DES);
  if (result)
    GB.ReturnNewZeroString(result);

END_METHOD

BEGIN_METHOD(CCRYPT_call_md5, GB_STRING password; GB_STRING key)

  char *result;

  result = do_crypt(GB.ToZeroString(ARG(password)), MISSING(key) ? NULL : GB.ToZeroString(ARG(key)), USE_MD5);
  if (result)
    GB.ReturnNewZeroString(result);

END_METHOD

BEGIN_METHOD(CCRYPT_call_sha256, GB_STRING password; GB_STRING key)

  char *result;

  result = do_crypt(GB.ToZeroString(ARG(password)), MISSING(key) ? NULL : GB.ToZeroString(ARG(key)), USE_SHA256);
  if (result)
    GB.ReturnNewZeroString(result);

END_METHOD

BEGIN_METHOD(CCRYPT_call_sha512, GB_STRING password; GB_STRING key)

  char *result;

  result = do_crypt(GB.ToZeroString(ARG(password)), MISSING(key) ? NULL : GB.ToZeroString(ARG(key)), USE_SHA512);
  if (result)
    GB.ReturnNewZeroString(result);

END_METHOD

BEGIN_METHOD(CCRYPT_check, GB_STRING password; GB_STRING crypt)

  GB.ReturnBoolean(check_crypt(GB.ToZeroString(ARG(password)), GB.ToZeroString(ARG(crypt))));

END_METHOD

GB_DESC CCryptDesc[] =
{
  GB_DECLARE("Crypt", 0), GB_VIRTUAL_CLASS(),

  GB_STATIC_METHOD("_call", "s", CCRYPT_call_md5, "(Password)s[(Prefix)s]"),
  GB_STATIC_METHOD("Check", "b", CCRYPT_check, "(Password)s(Crypt)s"),
  GB_STATIC_METHOD("DES", "s", CCRYPT_call_des, "(Password)s[(Prefix)s]"),
  GB_STATIC_METHOD("MD5", "s", CCRYPT_call_md5, "(Password)s[(Prefix)s]"),
  GB_STATIC_METHOD("SHA256", "s", CCRYPT_call_sha256, "(Password)s[(Prefix)s]"),
  GB_STATIC_METHOD("SHA512", "s", CCRYPT_call_sha512, "(Password)s[(Prefix)s]"),

  GB_END_DECLARE
};
