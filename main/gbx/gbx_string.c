/***************************************************************************

  String.c

  The String management routines

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

#define __STRING_C

#include "gb_common.h"
#include "gb_common_buffer.h"
#include "gb_common_case.h"
#include "gb_error.h"
#include "gbx_value.h"
#include "gbx_debug.h"
#include "gbx_local.h"

#include <unistd.h>
#include <pwd.h>
#include <ctype.h>
#include <iconv.h>

#include "gbx_string.h"

#if DEBUG_STRING
#define DEBUG_ME
#endif

#define STRING_last_count 32
static char *STRING_last[STRING_last_count] = { 0 };

static const char _char_string[512] = 
"\x00\x00\x01\x00\x02\x00\x03\x00\x04\x00\x05\x00\x06\x00\x07\x00\x08\x00\x09\x00\x0A\x00\x0B\x00\x0C\x00\x0D\x00\x0E\x00\x0F\x00"
"\x10\x00\x11\x00\x12\x00\x13\x00\x14\x00\x15\x00\x16\x00\x17\x00\x18\x00\x19\x00\x1A\x00\x1B\x00\x1C\x00\x1D\x00\x1E\x00\x1F\x00"
"\x20\x00\x21\x00\x22\x00\x23\x00\x24\x00\x25\x00\x26\x00\x27\x00\x28\x00\x29\x00\x2A\x00\x2B\x00\x2C\x00\x2D\x00\x2E\x00\x2F\x00"
"\x30\x00\x31\x00\x32\x00\x33\x00\x34\x00\x35\x00\x36\x00\x37\x00\x38\x00\x39\x00\x3A\x00\x3B\x00\x3C\x00\x3D\x00\x3E\x00\x3F\x00"
"\x40\x00\x41\x00\x42\x00\x43\x00\x44\x00\x45\x00\x46\x00\x47\x00\x48\x00\x49\x00\x4A\x00\x4B\x00\x4C\x00\x4D\x00\x4E\x00\x4F\x00"
"\x50\x00\x51\x00\x52\x00\x53\x00\x54\x00\x55\x00\x56\x00\x57\x00\x58\x00\x59\x00\x5A\x00\x5B\x00\x5C\x00\x5D\x00\x5E\x00\x5F\x00"
"\x60\x00\x61\x00\x62\x00\x63\x00\x64\x00\x65\x00\x66\x00\x67\x00\x68\x00\x69\x00\x6A\x00\x6B\x00\x6C\x00\x6D\x00\x6E\x00\x6F\x00"
"\x70\x00\x71\x00\x72\x00\x73\x00\x74\x00\x75\x00\x76\x00\x77\x00\x78\x00\x79\x00\x7A\x00\x7B\x00\x7C\x00\x7D\x00\x7E\x00\x7F\x00"
"\x80\x00\x81\x00\x82\x00\x83\x00\x84\x00\x85\x00\x86\x00\x87\x00\x88\x00\x89\x00\x8A\x00\x8B\x00\x8C\x00\x8D\x00\x8E\x00\x8F\x00"
"\x90\x00\x91\x00\x92\x00\x93\x00\x94\x00\x95\x00\x96\x00\x97\x00\x98\x00\x99\x00\x9A\x00\x9B\x00\x9C\x00\x9D\x00\x9E\x00\x9F\x00"
"\xA0\x00\xA1\x00\xA2\x00\xA3\x00\xA4\x00\xA5\x00\xA6\x00\xA7\x00\xA8\x00\xA9\x00\xAA\x00\xAB\x00\xAC\x00\xAD\x00\xAE\x00\xAF\x00"
"\xB0\x00\xB1\x00\xB2\x00\xB3\x00\xB4\x00\xB5\x00\xB6\x00\xB7\x00\xB8\x00\xB9\x00\xBA\x00\xBB\x00\xBC\x00\xBD\x00\xBE\x00\xBF\x00"
"\xC0\x00\xC1\x00\xC2\x00\xC3\x00\xC4\x00\xC5\x00\xC6\x00\xC7\x00\xC8\x00\xC9\x00\xCA\x00\xCB\x00\xCC\x00\xCD\x00\xCE\x00\xCF\x00"
"\xD0\x00\xD1\x00\xD2\x00\xD3\x00\xD4\x00\xD5\x00\xD6\x00\xD7\x00\xD8\x00\xD9\x00\xDA\x00\xDB\x00\xDC\x00\xDD\x00\xDE\x00\xDF\x00"
"\xE0\x00\xE1\x00\xE2\x00\xE3\x00\xE4\x00\xE5\x00\xE6\x00\xE7\x00\xE8\x00\xE9\x00\xEA\x00\xEB\x00\xEC\x00\xED\x00\xEE\x00\xEF\x00"
"\xF0\x00\xF1\x00\xF2\x00\xF3\x00\xF4\x00\xF5\x00\xF6\x00\xF7\x00\xF8\x00\xF9\x00\xFA\x00\xFB\x00\xFC\x00\xFD\x00\xFE\x00\xFF\x00";

static int _index = 0;

//static HASH_TABLE *_intern = NULL;

void STRING_new(char **ptr, const char *src, int len)
{
  STRING *str;

  if (len <= 0 && src != NULL)
    len = strlen(src);

  if (len <= 0)
  {
    *ptr = NULL;
    return;
  }

  ALLOC(&str, len + 1 + sizeof(STRING), "STRING_new");

  str->len = len;
  str->ref = 1;

  if (src != NULL)
    memcpy(str->data, src, len);

  str->data[len] = 0;

  *ptr = str->data;

  #ifdef DEBUG_ME
  DEBUG_where();
  printf("STRING_new %p ( 0 ) \"%.*s\"\n", *ptr, len, src);
  fflush(NULL);
  #endif
}

static void post_free(char *ptr)
{
  /*if (NLast >= MAX_LAST_STRING)
    THROW(E_STRING);*/

  #ifdef DEBUG_ME
  if (STRING_last[_index])
  {
    DEBUG_where();
    printf("STRING: release temp: %p '%s'\n", STRING_last[_index], STRING_last[_index]);
    fflush(NULL);
  }
  #endif

  STRING_unref(&STRING_last[_index]);

  #ifdef DEBUG_ME
  printf("STRING: post temp: %p '%s'\n", ptr, ptr);
  fflush(NULL);
  #endif

  STRING_last[_index] = ptr;

  _index++;

  if (_index >= STRING_last_count)
    _index = 0;
}


int STRING_get_free_index(void)
{
  return _index;
}

void STRING_new_temp(char **ptr, const char *src, int len)
{
  STRING_new(ptr, src, len);
  if (*ptr)
    post_free(*ptr);
}


/*void STRING_init(void)
{
	HASH_TABLE_create(&_intern, 0, GB_COMP_TEXT);
}*/

void STRING_exit(void)
{
  int i;

  for (i = 0; i < STRING_last_count; i++)
  {
    /*if (STRING_last[i])
      printf("release temp %p '%s'\n", STRING_last[i], STRING_last[i]);*/
    STRING_unref(&STRING_last[i]);
    STRING_last[i] = NULL;
  }

  _index = 0;
}


void STRING_extend(char **ptr, int new_len)
{
  STRING *str;
  int len = STRING_length(*ptr);

  if (new_len == len)
    return;

  if (new_len == 0)
  {
    STRING_free(ptr);
    return;
  }

  if (len == 0)
  {
    ALLOC(&str, new_len + 1 + sizeof(STRING), "STRING_extend");
    str->ref = 1;
  }
  else
  {
    str = STRING_from_ptr(*ptr);
    REALLOC(&str, new_len + 1 + sizeof(STRING), "STRING_extend");
  }

  str->len = new_len;

  *ptr = str->data;
}


void STRING_extend_end(char **ptr)
{
  if (*ptr)
  {
    (*ptr)[STRING_length(*ptr)] = 0;
    post_free(*ptr);
  }
}


void STRING_copy_from_value_temp(char **ptr, VALUE *value)
{
  if (value->_string.len == 0)
    *ptr = NULL;
  else if (value->type == T_STRING && value->_string.start == 0 && value->_string.len == STRING_length(value->_string.addr))
    *ptr = value->_string.addr;
  else
    STRING_new_temp(ptr, &value->_string.addr[value->_string.start], value->_string.len);
}


/* Attention ! Contrairement �STRING_new, STRING_new_temp_value cr� des
   cha�es temporaires.
*/

void STRING_new_temp_value(VALUE *value, const char *src, int len)
{
  STRING_new_temp(&(value->_string.addr), src, len);

  value->_string.len = STRING_length(value->_string.addr);
  value->_string.start = 0;
  value->type = T_STRING;
}


void STRING_new_constant_value(VALUE *value, const char *src, int len)
{
  value->_string.addr = (char *)src;
  value->_string.len = ((len < 0) ? strlen(src) : len);
  value->_string.start = 0;
  value->type = T_CSTRING;
}


void STRING_void_value(VALUE *value)
{
  value->type = T_CSTRING;
  value->_string.addr = NULL;
  value->_string.start = 0;
  value->_string.len = 0;
}


void STRING_char_value(VALUE *value, uchar car)
{
  value->type = T_CSTRING;
  value->_string.addr = (char *)&_char_string[(int)car * 2];
  value->_string.start = 0;
  value->_string.len = 1;
}


void STRING_free(char **ptr)
{
  STRING *str;

  if (*ptr == NULL)
    return;

  str = STRING_from_ptr(*ptr);

  #ifdef DEBUG_ME
  DEBUG_where();
  printf("STRING_free %p %p\n", *ptr, ptr);
  fflush(NULL);
  #endif

  str->ref = 1000000000L;

  FREE(&str, "STRING_free");
  *ptr = NULL;

  #ifdef DEBUG_ME
  printf("OK\n");
  #endif
}



int STRING_comp_value(VALUE *str1, VALUE *str2)
{
  uint i;
  int len = Min(str1->_string.len, str2->_string.len);
  int diff;
  register const char *s1;
  register const char *s2;
  register char c1, c2;

  s1 = str1->_string.addr + str1->_string.start;
  s2 = str2->_string.addr + str2->_string.start;

  for (i = 0; i < len; i++)
  {
    c1 = s1[i];
    c2 = s2[i];
    if (c1 > c2) return 1;
    if (c1 < c2) return -1;
  }

  diff =  str1->_string.len - str2->_string.len;
  return (diff < 0) ? (-1) : (diff > 0) ? 1 : 0;
}


int STRING_comp_value_ignore_case(VALUE *str1, VALUE *str2)
{
  int i;
  int len = Min(str1->_string.len, str2->_string.len);
  int diff;
  register const char *s1;
  register const char *s2;
  register char c1, c2;

  s1 = str1->_string.addr + str1->_string.start;
  s2 = str2->_string.addr + str2->_string.start;

  for (i = 0; i < len; i++)
  {
    c1 = tolower(s1[i]);
    c2 = tolower(s2[i]);
    if (c1 > c2) return 1;
    if (c1 < c2) return -1;
  }

  diff =  str1->_string.len - str2->_string.len;
  return (diff < 0) ? (-1) : (diff > 0) ? 1 : 0;
}


#if DEBUG_STRING

void STRING_ref(char *ptr)
{
  STRING *str;

  if (ptr == NULL)
    return;

  str = STRING_from_ptr(ptr);


  #ifdef DEBUG_ME
  DEBUG_where();
  printf("STRING_ref %p ( %ld -> %ld )\n", ptr, str->ref, str->ref + 1);
  if (str->ref < 0 || str->ref > 10000)
    printf("*** BAD\n");
  fflush(NULL);
  #endif

  str->ref++;
}


void STRING_unref(char **ptr)
{
  STRING *str;

  if (*ptr == NULL)
    return;

  str = STRING_from_ptr(*ptr);

  #ifdef DEBUG_ME
  DEBUG_where();
  printf("STRING_unref %p ( %ld -> %ld )\n", *ptr, str->ref, str->ref - 1);
  if (str->ref < 1 || str->ref > 10000)
    printf("*** BAD\n");
  fflush(NULL);
  #endif

  if ((--str->ref) <= 0)
    STRING_free(ptr);
}

#endif

void STRING_unref_keep(char **ptr)
{
  STRING *str;

  if (*ptr == NULL)
    return;

  str = STRING_from_ptr(*ptr);
  if (str->ref > 1)
    str->ref--;
  else
    post_free(*ptr);
}


char *STRING_subst(const char *str, int len, SUBST_FUNC get_param)
{
	uint i;
	uchar c, d;
	int np;
	char *p;
	int lp;

  if (!str)
    return NULL;

  SUBST_init();

  if (len <= 0)
    len = strlen(str);

	for (i = 0; i < len; i++)
	{
		c = str[i];
		if (c == '&' && (i < (len - 1)))
		{
			d = str[i + 1];
			if (isdigit(d))
			{
				np = d - '0';
				i++;
				if (i < (len - 1))
				{
					d = str[i + 1];
					if (isdigit(d))
					{
						np = np * 10 + d - '0';
						i++;
					}
				}

      	p = NULL;
      	lp = 0;
      	(*get_param)(np, &p, &lp);

      	if (p)
        	SUBST_add(p, lp);

				continue;
			}
			else if (d == '&')
				i++;
		}

		SUBST_add_char(c);
	}

  SUBST_exit();

  return SUBST_buffer();
}

void STRING_add(char **ptr, const char *src, int len)
{
  int old_len;

  if (len <= 0 && src != NULL)
    len = strlen(src);

  if (len <= 0)
    return;

  old_len = STRING_length(*ptr);

  STRING_extend(ptr, old_len + len);
  memcpy(&((*ptr)[old_len]), src, len);
  (*ptr)[old_len + len] = 0;
}


int STRING_conv(char **result, const char *str, int len, const char *src, const char *dst, bool throw)
{
  iconv_t handle;
  bool err;
  const char *in;
  char *out;
  size_t in_len;
  size_t out_len;
  size_t ret;
  int errcode = 0;

  *result = NULL;

  in = str;
  in_len = len;

  if (len == 0)
    return errcode;

  if (!dst || *dst == 0)
    dst = "ASCII";

  if (!src || *src == 0)
    src = "ASCII";

  handle = iconv_open(dst, src);
  if (handle == (iconv_t)(-1))
  {
    if (errno == EINVAL)
    	errcode = E_UCONV;
		else
			errcode = E_CONV;
  }
  else
  {
		err = FALSE;
	
		for(;;)
		{
			out = COMMON_buffer;
			out_len = COMMON_BUF_MAX;
	
			#if defined(OS_SOLARIS) || defined(OS_FREEBSD) || defined(OS_OPENBSD)
			ret = iconv(handle, &in, &in_len, &out, &out_len);
			#else
			ret = iconv(handle, (char **)&in, &in_len, &out, &out_len);
			#endif
	
			if (ret != (size_t)(-1) || errno == E2BIG)
				STRING_add(result, COMMON_buffer, COMMON_BUF_MAX - out_len);
	
			if (ret != (size_t)(-1))
				break;
	
			if (errno != E2BIG)
			{
				err = TRUE;
				break;
			}
		}
	
		iconv_close(handle);
	
		//STRING_add(result, "\0\0\0", sizeof(wchar_t) - 1);
	
		STRING_extend_end(result);
	
		if (err)
			errcode = E_CONV;
	}
	
	if (throw && errcode)
		THROW(errcode);

	return errcode;
}


char *STRING_conv_to_UTF8(const char *name, int len)
{
  char *result = NULL;

  if (!name)
    return "";

  if (LOCAL_is_UTF8)
  {
    if (len <= 0)
      result = (char *)name;
    else
      STRING_new_temp(&result, name, len);
  }
  else
  {
    if (len <= 0)
      len = strlen(name);

    STRING_conv(&result, name, len, LOCAL_encoding, "UTF-8", TRUE);
  }

  if (result)
    return result;
  else
    return "";
}


char *STRING_conv_file_name(const char *name, int len)
{
  char *result = NULL;
  int pos;
  struct passwd *info;
  char *user;

  if (!name)
    return "";

  if (len <= 0)
    len = strlen(name);

  if (len > 0 && *name == '~')
  {
  	for (pos = 0; pos < len; pos++)
  	{
  		if (name[pos] == '/')
  			break;
  	}

		if (pos <= 1)
			info = getpwuid(getuid());
		else
		{
			STRING_new_temp(&user, &name[1], pos - 1);
			info = getpwnam(user);
		}

		if (info)
		{
			STRING_new(&user, info->pw_dir, 0);
			if (pos < len)
				STRING_add(&user, &name[pos], len - pos);
			name = user;
			len = STRING_length(name);
			post_free(user);
		}

  }

  if (LOCAL_is_UTF8)
    STRING_new_temp(&result, name, len);
  else
    STRING_conv(&result, name, len, "UTF-8", LOCAL_encoding, TRUE);

	//fprintf(stderr, "STRING_conv_file_name: %s\n", result);

  if (result)
    return result;
  else
    return "";
}


int STRING_search(const char *ps, int ls, const char *pp, int lp, int is, bool right, bool nocase)
{
  int pos = 0, ip;

  if (lp > ls)
    goto __FOUND;

  if (is < 0)
    is += ls;
  else if (is == 0)
    is = right ? ls : 1;
  
  ls = ls - lp + 1; /* Longueur du début du texte où effectuer la recherche */

  if (is > ls)
  {
  	if (!right)
    	goto __FOUND;
    else
    	is = ls;
  }
  else if (is < lp)
  {
  	if (right)
    	goto __FOUND;
    else if (is < 1)
    	is = 1;
  }

  is--;

  ps += is;

  if (right)
  {
    if (nocase)
    {
      for (; is >= 0; is--, ps--)
      {
        for (ip = 0; ip < lp; ip++)
        {
          if (tolower(ps[ip]) != tolower(pp[ip]))
            goto __NEXT_RN;
        }
  
        pos = is + 1;
        goto __FOUND;
  
    __NEXT_RN:
      ;
      }
    }
    else
    {
      for (; is >= 0; is--, ps--)
      {
        for (ip = 0; ip < lp; ip++)
        {
          if (ps[ip] != pp[ip])
            goto __NEXT_R;
        }
  
        pos = is + 1;
        goto __FOUND;
  
    __NEXT_R:
      ;
      }
    }
  }
  else
  {
    if (nocase)
    {
      for (; is < ls; is++, ps++)
      {
        for (ip = 0; ip < lp; ip++)
        {
          if (tolower(ps[ip]) != tolower(pp[ip]))
            goto __NEXT_LN;
        }
  
        pos = is + 1;
        goto __FOUND;
  
    __NEXT_LN:
      ;
      }
    }
    else
    {
      for (; is < ls; is++, ps++)
      {
        for (ip = 0; ip < lp; ip++)
        {
          if (ps[ip] != pp[ip])
            goto __NEXT_L;
        }
  
        pos = is + 1;
        goto __FOUND;
  
    __NEXT_L:
      ;
      }
    }
  }

__FOUND:

  return pos;
}


/*void *STRING_intern(const char *str, int len)
{
	return HASH_TABLE_insert(_intern, str, len);
}*/
