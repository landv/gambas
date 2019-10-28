/***************************************************************************

  gbx_number.c

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

#define __GBX_NUMBER_C

#include "gb_common.h"
#include "gb_error.h"
#include "gb_limit.h"

#include <ctype.h>
#include <float.h>
#include <math.h>

#include "gbx_type.h"
#include "gb_common_buffer.h"
#include "gbx_local.h"
#include "gbx_math.h"
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
#define has_string COMMON_has_string

#define IS_PURE_INTEGER(_int64_val) ((_int64_val) == ((int)(_int64_val)))

static uint64_t _pow_10[18] = { 
	10, 
	100, 
	1000, 
	10000, 
	100000, 
	1000000, 
	10000000, 
	100000000, 
	1000000000, 
	10000000000, 
	100000000000, 
	1000000000000, 
	10000000000000, 
	100000000000000, 
	1000000000000000, 
	10000000000000000,
	100000000000000000, 
	1000000000000000000
};


static bool read_integer(int base, bool minus, int64_t *result, bool local)
{
	uint64_t nbr2, nbr;
	int d, n, c, nmax;
	const char *thsep;
	int lthsep;
	int ndigit_thsep;
	bool first_thsep;

	thsep = LOCAL_get(local)->thousand_sep;
	lthsep = LOCAL_get(local)->len_thousand_sep;
	ndigit_thsep = 0;
	first_thsep = FALSE;

	n = 0;
	nbr = 0;
	
	switch (base)
	{
		case 2: nmax = 64; break;
		case 8: nmax = 21; break;
		case 16: nmax = 16; break;
		case 10: default: nmax = 19; break;
	}

	c = last_char();

	if (base == 10)
	{
		for(;;)
		{
			if (local)
			{
				COMMON_pos--;
				
				if (has_string(thsep, lthsep) && (ndigit_thsep == 3 || (!first_thsep && ndigit_thsep >= 1 && ndigit_thsep <= 3)))
				{
					COMMON_pos += lthsep;
					c = get_char();
					first_thsep = TRUE;
					ndigit_thsep = 0;
				}
				else
					COMMON_pos++;
			}

			if (c >= '0' && c <= '9')
			{
				d = c - '0';
				if (local)
					ndigit_thsep++;
			}
			else
				break;

			n++;
			if (n < nmax)
			{
				nbr = nbr * 10 + d;
			}
			else
			{
				nbr2 = nbr * 10 + d;
			
				if ((nbr2 / base) != nbr || nbr2 > ((uint64_t)LLONG_MAX + minus))
					return TRUE;
			
				nbr = nbr2;
			}

			c = get_char();
			if (c < 0)
				break;
		}

		c = last_char();

		if (local && first_thsep && ndigit_thsep != 3)
			return TRUE;
	}
	else
	{
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
			if (n > nmax)
				return TRUE;
			
			nbr = nbr * base + d;
			
			c = get_char();
			if (c < 0)
				break;
		}

		c = last_char();

		if ((c == '&' || c == 'u' || c == 'U') && base != 10)
			c = get_char();
		else
		{
			if ((base == 16 && n == 4) || (base == 2 && n == 16))
			{
				if (nbr >= 0x8000L && nbr <= 0xFFFFL)
					nbr |= INT64_C(0xFFFFFFFFFFFF0000);
			}
			else if ((base == 16 && n == 8) || (base == 2 && n == 32))
			{
				if (nbr >= 0x80000000L && nbr <= 0xFFFFFFFFL)
					nbr |= INT64_C(0xFFFFFFFF00000000);
			}
		}
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
	const char *thsep;
	int lthsep;
	int ndigit_thsep;
	bool first_thsep;
	int c, n;

	uint64_t mantisse, mantisse_int;
	int ndigit_frac, ndigit_frac_zero;
	bool frac;
	bool frac_null;
	bool nozero;

	int nexp;
	bool nexp_minus;

	local_info = LOCAL_get(local);
	point = local_info->decimal_point;
	thsep = local_info->thousand_sep;
	lthsep = local_info->len_thousand_sep;
	ndigit_thsep = 0;
	first_thsep = FALSE;

	c = last_char();
	
	n = 0;
	mantisse = 0;
	mantisse_int = 0;
	frac = FALSE;
	frac_null = TRUE;
	ndigit_frac = 0;
	ndigit_frac_zero = 0;
	nexp = 0;
	nexp_minus = FALSE;
	nozero = FALSE;
	
	// Integer part
	
	for(;;)
	{
		if (c == point)
		{
			c = get_char();
			frac = TRUE;
			mantisse_int = mantisse;
			break;
		}

		if (local)
		{
			COMMON_pos--;
			
			if (has_string(thsep, lthsep) && (ndigit_thsep == 3 || (!first_thsep && ndigit_thsep >= 1 && ndigit_thsep <= 3)))
			{
				COMMON_pos += lthsep;
				first_thsep = TRUE;
				ndigit_thsep = 0;
				c = get_char();
			}
			else
				COMMON_pos++;
		}
		
		if (!isdigit(c) || (c < 0))
			break;
		
		if (c != '0')
			nozero = TRUE;
		
		if (nozero)
			n++;
		
		if (n > MAX_FLOAT_DIGIT)
		{
			if (n == (MAX_FLOAT_DIGIT + 1) && (c >= '5'))
				mantisse++;
			ndigit_frac--; // ???
			c = get_char();
			continue;
		}

		if (c == '0')
			mantisse *= 10;
		else
			mantisse = mantisse * 10 + (c - '0');
		
		if (local)
			ndigit_thsep++;

		c = get_char();

		if (c == 'e' || c == 'E')
			break;

		if (c < 0)
			goto __END;
	}

	// Decimal part
	
	for(;;)
	{
		if (c == point)
			break;

		if (!isdigit(c) || (c < 0))
			break;
		
		if (c != '0')
			nozero = TRUE;
		
		if (nozero)
			n++;
		
		if (n > MAX_FLOAT_DIGIT)
		{
			if (n == (MAX_FLOAT_DIGIT + 1) && (c >= '5'))
				mantisse++;
			if (!frac)
				ndigit_frac--;
			c = get_char();
			continue;
		}

		if (c == '0')
			ndigit_frac_zero++;
		else
		{
			frac_null = FALSE;
			ndigit_frac += ndigit_frac_zero + 1;
			mantisse = mantisse * _pow_10[ndigit_frac_zero] + (c - '0');
			ndigit_frac_zero = 0;
		}
		
		c = get_char();

		if (c == 'e' || c == 'E')
			break;

		if (c < 0)
			goto __END;
	}

	// Exponant

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
		
		if (nexp_minus)
			nexp = (-nexp);
	}

	if (c >= 0 && !isspace(c))
		return TRUE;

__END:
	
	if (local && first_thsep && ndigit_thsep != 3)
		return TRUE;

	if (frac && frac_null)
		mantisse = mantisse_int;
	else
		nexp -= ndigit_frac;

	//fprintf(stderr, "%.24g %d\n", (double)mantisse, nexp);
	//*result = mulpow10((double)mantisse, nexp);
	*result = (double)mantisse * pow10(nexp);
	
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
				else if (c == 'O' || c == 'o')
				{
					base = 8;
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

	if (!read_integer(base, minus, &val, (option & NB_LOCAL) != 0))
	{
		if (minus) val = (-val);
		
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
			if (minus) dval = (-dval);
			type = T_FLOAT;
			goto __END;
		}
	}

	return TRUE;

__END:

	if (last_char() >= 0) //(c >= 0 && !isspace(c))
		return TRUE;

	value->type = type;

	if (type == T_INTEGER)
		value->_integer.value = val;
	else if (type == T_LONG)
		value->_long.value = val;
	else
		value->_float.value = dval;

	//fprintf(stderr, "return FALSE\n");
	return FALSE;
}


void NUMBER_int_to_string(uint64_t nbr, int prec, int base, VALUE *value)
{
	char *ptr;
	char *src;
	int digit, len;
	bool neg;

	len = 0;
	ptr = &COMMON_buffer[COMMON_BUF_MAX];

	if (nbr == 0 && prec == 0)
	{
		STRING_char_value(value, '0');
		return;
	}
	
	neg = (nbr & (1LL << 63)) != 0;
	
	if (base == 10 && neg)
		nbr = 1 + ~nbr;
	
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
		
		if (base == 10)
		{
			len++;
			ptr--;
			*ptr = '-';
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

