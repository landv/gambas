/***************************************************************************

  gbx_number.c

  Numbers management routines

  Datatype management routines. Conversions between each Gambas datatype,
  and conversions between Gambas datatypes and native datatypes.

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

#define __GBX_NUMBER_C

#include "gb_common.h"
#include "gb_error.h"

#include <ctype.h>
#include <float.h>
#include <math.h>

#include "gbx_type.h"
#include "gb_common_buffer.h"
#include "gbx_local.h"

#include "gbx_string.h"
#include "gbx_number.h"

#define buffer_init COMMON_buffer_init
#define get_char COMMON_get_char
#define last_char COMMON_last_char
#define look_char COMMON_look_char
#define put_char COMMON_put_char
#define jump_space COMMON_jump_space
#define get_current COMMON_get_current
#define buffer_pos COMMON_pos
#define get_size_left COMMON_get_size_left

#define IS_PURE_INTEGER(_int64_val) ((_int64_val) == ((int)(_int64_val)))


static bool read_integer(int base, int64_t *result)
{
  uint64_t nbr2, nbr;
  int d, n, c, nmax;

  n = 0;
  nbr = 0;
	
	switch (base)
	{
		case 2: nmax = 64; break;
		case 10: nmax = 19; break;
		case 16: nmax = 16; break;
	}

  c = last_char();

  for(;;)
  {
    if (c >= '0' && c <= '9')
      d = c - '0';
    else if (c >= 'A' && c <='Z')
      d = c - 'A' + 10;
    else if (c >= 'a' && c <='z')
      d = c - 'a' + 10;
    else
      break;

    if (d >= base)
      break;

    n++;
    
    nbr2 = nbr * base;
    
		//if (n >= nmax)
		//{
			if (((int64_t)nbr2 / base) != (int64_t)nbr)
				return TRUE;
		//}
		
    nbr = nbr2 + d;

    c = get_char();
    if (c < 0)
      break;
  }

 	c = last_char();
	
	if ((c == '&' || c == 'u' || c == 'U') && base != 10)
		c = get_char();
	else
	{
		if (nbr >= 0x8000L && nbr <= 0xFFFFL)
			nbr |= INT64_C(0xFFFFFFFFFFFF0000);
	}
	
	if (c > 0 && !isspace(c))
		return TRUE;
	
  if (n == 0)
    return TRUE;

  *((int64_t *)result) = nbr;  
  return FALSE;
}


static bool read_float(double *result, bool local)
{
  LOCAL_INFO *local_info;
  char point;
  char thsep;
	int c;

  double nint;
  double nfrac, n;
  int nexp;
  bool nexp_minus;

  local_info = LOCAL_get(local);
  point = local_info->decimal_point;
  thsep = local_info->thousand_sep;

  nint = 0.0;
  nfrac = 0.0;
  nexp = 0;
  nexp_minus = FALSE;

	c = last_char();
	
  /* Integer part */

  for(;;)
  {
    if (c == point)
    {
      c = get_char();
      break;
    }

    if (!isdigit(c) || (c < 0))
      return TRUE;

    nint = nint * 10 + (c - '0');

    c = get_char();

    if (c == 'e' || c == 'E')
      break;

    if (c == thsep)
      c = get_char();
    
    if ((c < 0) || isspace(c))
      goto __END;
  }

  /* Decimal part */

	n = 0.1;
	for(;;)
	{
		if (!isdigit(c) || (c < 0))
			break;

		nfrac += n * (c - '0');
		n /= 10;

		c = get_char();
	}

  /* Exponent */

  if (c == 'e' || c == 'E')
  {
    c = get_char();

    if (c == '+' || c == '-')
    {
      if (c == '-')
        nexp_minus = TRUE;

      c = get_char();
    }

    if (!isdigit(c) || (c < 0))
      return TRUE;

    for(;;)
    {
      nexp = nexp * 10 + (c - '0');
      if (nexp > DBL_MAX_10_EXP)
        return TRUE;

      c = get_char();
      if (!isdigit(c) || (c < 0))
        break;
    }
  }

  if (c >= 0 && !isspace(c))
    return TRUE;

__END:
	
  *result = (nint + nfrac) * pow(10, nexp_minus ? (-nexp) : nexp);

  return FALSE;
}


bool NUMBER_from_string(int option, const char *str, int len, VALUE *value)
{
  int c;
  int64_t val = 0;
  double dval = 0.0;
  TYPE type;
  int base = 10;
  bool minus = FALSE;
	int pos;

  buffer_init(str, len);
  jump_space();

  c = get_char();

  if (c == '+' || c == '-')
  {
    minus = (c == '-');
    c = get_char();
  }

  if (option & NB_READ_INT_LONG)
  {
    if (option & NB_READ_HEX_BIN)
    {
      if (c == '&')
      {
        c = get_char();

        if (c == 'H' || c == 'h')
        {
          base = 16;
          c = get_char();
        }
        else if (c == 'X' || c == 'x')
        {
          base = 2;
          c = get_char();
        }
        else
          base = 16;
      }
      else if (c == '%')
      {
        base = 2;
        c = get_char();
      }
    }
  }

  if (c < 0)
    return TRUE;

  if (c == '-' || c == '+')
    return TRUE;

  errno = 0;
	pos = COMMON_pos - 1;

	if (!read_integer(base, &val))
	{
		if ((option & NB_READ_INTEGER) && IS_PURE_INTEGER(val))
		{
			type = T_INTEGER;
			goto __END;
		}
		else if ((option & NB_READ_LONG))
		{
      type = T_LONG;
      goto __END;
		}
		else if ((option & NB_READ_FLOAT) && base == 10)
		{
			type = T_FLOAT;
			dval = (double)val;
			goto __END;
		}
	}

  if ((option & NB_READ_FLOAT) && base == 10)
  {
		COMMON_pos = pos;
		get_char();
    if (!read_float(&dval, (option & NB_LOCAL) != 0))
    {
      type = T_FLOAT;
      goto __END;
    }
  }

  return TRUE;

__END:

  c = last_char();
  if (c >= 0 && !isspace(c))
    return TRUE;

  value->type = type;

  if (type == T_INTEGER)
    value->_integer.value = minus ? (-val) : val;
  else if (type == T_LONG)
    value->_long.value = minus ? (-val) : val;
  else
    value->_float.value = minus ? (-dval) : dval;

  return FALSE;
}


void NUMBER_int_to_string(uint64_t nbr, int prec, int base, VALUE *value)
{
  char *ptr;
  char *src;
  int digit, len;
  bool neg;

  //if (prec < 0)
  //  ERROR_panic("NUMBER_int_to_string: prec < 0");

  len = 0;
  ptr = &COMMON_buffer[COMMON_BUF_MAX];

  if (nbr == 0 && prec == 0)
  {
    STRING_char_value(value, '0');
    return;
  }
  
  neg = (nbr & (1LL << 63)) != 0;
  
  while (nbr > 0)
  {
    digit = nbr % base;
    nbr /= base;

    ptr--;
    len++;

    if (digit < 10)
      *ptr = '0' + digit;
    else
      *ptr = 'A' + digit - 10;
  }

  if (neg)
  {
    if (prec)
    {
      ptr += len - prec;
      len = prec;
    }
    
    STRING_new_temp_value(value, NULL, len);
    src = value->_string.addr;
    
    memcpy(src, ptr, len);
  }
  else
  {
    STRING_new_temp_value(value, NULL, Max(len, prec));
    src = value->_string.addr;
  
    while (prec > len)
    {
      *src++ = '0';
      prec--;
    }
    
    memcpy(src, ptr, len);
  }
}
