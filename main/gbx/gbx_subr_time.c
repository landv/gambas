/***************************************************************************

  gbx_subr_time.c

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

#include "gb_common.h"

#include <unistd.h>
#include <sys/time.h>

#include "gb_error.h"
#include "gbx_value.h"
#include "gbx_subr.h"
#include "gbx_local.h"
#include "gbx_date.h"


void SUBR_timer(void)
{
  double result = 0.0;

  DATE_timer(&result, TRUE);

  SP->type = T_FLOAT;
  SP->_float.value = result;
  SP++;
}


void SUBR_now(void)
{
  DATE_now(SP);
  SP++;
}


void SUBR_year(ushort code)
{
  DATE_SERIAL *date;
  int val;

  SUBR_ENTER_PARAM(1);

  VALUE_conv(PARAM, T_DATE);

  date = DATE_split(PARAM);

  switch(code & 0xF)
  {
    case 1: val = date->year; break;
    case 2: val = date->month; break;
    case 3: val = date->day; break;
    case 4: val = date->hour; break;
    case 5: val = date->min; break;
    case 6: val = date->sec; break;
    case 7: val = date->weekday; break;
    case 8: val = date->msec; break;
    default: val = 0;
  }

  PARAM->type = T_INTEGER;
  PARAM->_integer.value = val;
  
  #if 0
  SUBR_LEAVE() /* Not necessary */
  #endif
}


void SUBR_date(ushort code)
{
  DATE_SERIAL date;

  SUBR_ENTER();

  if (NPARAM <= 1)
  {
    if (NPARAM == 0)
      DATE_now(PARAM);
		else
			VALUE_conv(PARAM, T_DATE);

    date = *DATE_split(PARAM);
    date.hour = 0;
    date.min = 0;
    date.sec = 0;
    date.msec = 0;
  }
  else if (NPARAM >= 3)
  {
    VALUE_conv_integer(PARAM);
    VALUE_conv_integer(&PARAM[1]);
    VALUE_conv_integer(&PARAM[2]);

    CLEAR(&date);
    date.year = PARAM->_integer.value;
    date.month = PARAM[1]._integer.value;
    date.day = PARAM[2]._integer.value;

    if (NPARAM >= 4)
    {
      VALUE_conv_integer(&PARAM[3]);
      date.hour = PARAM[3]._integer.value;
    }

    if (NPARAM >= 5)
    {
      VALUE_conv_integer(&PARAM[4]);
      date.min = PARAM[4]._integer.value;
    }

    if (NPARAM >= 6)
    {
      VALUE_conv_integer(&PARAM[5]);
      date.sec = PARAM[5]._integer.value;
    }

    if (NPARAM >= 7)
    {
      VALUE_conv_integer(&PARAM[6]);
      date.msec = PARAM[6]._integer.value;
    }
	}
  else
		THROW(E_NEPARAM);

  if (DATE_make(&date, RETURN))
    THROW(E_DATE);
  
  SUBR_LEAVE();
}


void SUBR_time(ushort code)
{
  DATE_SERIAL date;

  SUBR_ENTER();

  if (NPARAM <= 1)
  {
    if (NPARAM == 0)
      DATE_now(PARAM);
		else
			VALUE_conv(PARAM, T_DATE);

    date = *DATE_split(PARAM);
    date.year = 0;
  }
  else if (NPARAM >= 3)
  {
    VALUE_conv_integer(PARAM);
    VALUE_conv_integer(&PARAM[1]);
    VALUE_conv_integer(&PARAM[2]);

    CLEAR(&date);
    date.hour = PARAM->_integer.value;
    date.min = PARAM[1]._integer.value;
    date.sec = PARAM[2]._integer.value;
		if (NPARAM == 4)
		{
			VALUE_conv_integer(&PARAM[3]);
			date.msec = PARAM[3]._integer.value;
		}
  }
  else
    THROW(E_NEPARAM);

  if (DATE_make(&date, RETURN))
    THROW(E_DATE);

  SUBR_LEAVE();
}


void SUBR_date_op(ushort code)
{
  SUBR_ENTER_PARAM(3);

  switch (code & 0xF)
  {
    case 0: /* DateAdd */
    
      VALUE_conv(PARAM, T_DATE);
      *RETURN = *PARAM;
      DATE_add(RETURN, SUBR_get_integer(&PARAM[1]), SUBR_get_integer(&PARAM[2]));
      
      break;
    
    case 1: /* DateDiff */
    
      VALUE_conv(PARAM, T_DATE);
      VALUE_conv(&PARAM[1], T_DATE);
      
      /* Dates are inverted! */
      RETURN->_integer.value = DATE_diff(&PARAM[1], PARAM, SUBR_get_integer(&PARAM[2]));
      RETURN->type = T_INTEGER;
      
      break;
  }

  SUBR_LEAVE();
}


void SUBR_week(ushort code)
{
  bool plain = FALSE;
  int start = LOCAL_get_first_day_of_week();
  DATE_SERIAL ds;
  VALUE date, first;
  int day, n;
  
  SUBR_ENTER();
  
  if (NPARAM >= 1)
  {
    VALUE_conv(PARAM, T_DATE);
    date = *PARAM;
  
    if (NPARAM >= 2)
    {
      start = SUBR_get_integer(&PARAM[1]);
      if (start < 0 || start > 6)
        THROW(E_ARG);
        
      if (NPARAM == 3)
        plain = SUBR_get_boolean(&PARAM[2]);
    }
  }
  else
    DATE_now(&date);
  
  /* Split it */
  ds = *DATE_split(&date);
  /* Set to 1 Jan of the current year */
  ds.month = 1;
  ds.day = 1;
  ds.hour = 0;
  ds.min = 0;
  ds.sec = 0;
  /* Convert to date & time */
  DATE_make(&ds, &first);
  /* Get the weekday of this 1 Jan */
  day = DATE_split(&first)->weekday;
  
  /* number of beginning days to ignore */
  
  n = 0;
  while (day != start)
  {
    day++;
    if (day > 6)
      day = 0;
    n++;
  }
  
  if (!plain)
  {
    if (n >= 4)
      n -= 7;
  }
  
  RETURN->type = T_INTEGER;
  RETURN->_integer.value = (date._date.date - first._date.date - n + 7) / 7;
  
  SUBR_LEAVE();
}
