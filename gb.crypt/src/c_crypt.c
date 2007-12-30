/***************************************************************************

  c_crypt.c

  crypt routines

  (c) 2000-2007 Benoit Minisini <gambas@users.sourceforge.net>

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

#define __C_CRYPT_C

#include <stdlib.h>
#include <crypt.h>
#include <errno.h>
#include <time.h>

#include "gb_common.h"

#include "c_crypt.h"
#include "main.h"


static char *do_crypt(const char *passwd, const char *prefix, bool use_md5)
{
  static char key_table[65] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcedefghijklmnopqrstuvwxyz./";
  static bool init = FALSE;
  
  int i, n;
  char key[16];
  char *result;
  char errormsg[35];

  if (prefix)
  {
    n = use_md5 ? 8 : 2;
    if (strlen(prefix) != n)
    { 
      snprintf(errormsg, 35, "size must be %d characters long", n);
      goto __BAD_PREFIX;
    }
    
    for (i = 0; i < n; i++)
    {
      if (strchr(key_table, prefix[i]) == NULL)
      {
        snprintf(errormsg, 35, "character '%c' not allowed" , prefix[i]);
        goto __BAD_PREFIX;
      }
    }
    
    if (use_md5)
    {
      strcpy(key, "$1$");
      strcat(key, prefix);
    }
    else
      strcpy(key, prefix);
  }
  else
  {    
    if (!init)
    {
      srandom((unsigned int)time(NULL));
      init = TRUE;
    }
    
    if (use_md5)
    {
      strcpy(key, "$1$");
      for (i = 0; i < 8; i++)
        key[i + 3] = key_table[(int)((random() / (RAND_MAX + 1.0)) * sizeof(key_table))];
      key[11] = '$';
      key[12] = 0;
    }
    else
    {
      for (i = 0; i < 2; i++)
        key[i] = key_table[(int)((random() / (RAND_MAX + 1.0)) * sizeof(key_table))];
      key[2] = 0;
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
    return strcmp(result, crypted);
}


BEGIN_METHOD(CCRYPT_call_md5, GB_STRING password; GB_STRING key)

  char *result;

  result = do_crypt(GB.ToZeroString(ARG(password)), MISSING(key) ? NULL : GB.ToZeroString(ARG(key)), TRUE);
  if (result)
    GB.ReturnNewZeroString(result);

END_METHOD


BEGIN_METHOD(CCRYPT_call_des, GB_STRING password; GB_STRING key)

  char *result;

  result = do_crypt(GB.ToZeroString(ARG(password)), MISSING(key) ? NULL : GB.ToZeroString(ARG(key)), FALSE);
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
  GB_STATIC_METHOD("MD5", "s", CCRYPT_call_md5, "(Password)s[(Prefix)s]"),
  GB_STATIC_METHOD("DES", "s", CCRYPT_call_des, "(Password)s[(Prefix)s]"),

  GB_END_DECLARE
};

