/***************************************************************************

  vb.c

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

#define __VB_C

#include "vb.h"
#include "vbdate.h"
#include <stdio.h>

#ifdef OS_CYGWIN
  #include <string.h>
#else
  #include <strings.h>
#endif

#include <math.h>

BEGIN_METHOD(CVB_val, GB_STRING str)

  GB_VALUE result;

  GB.NumberFromString(GB_NB_READ_ALL | GB_NB_READ_HEX_BIN, STRING(str), LENGTH(str), &result);

  if (result.type == GB_T_INTEGER)
    GB.ReturnInteger(((GB_INTEGER *)(void *)&result)->value);
  if (result.type == GB_T_LONG)
    GB.ReturnLong(((GB_LONG *)(void *)&result)->value);
  else if (result.type == GB_T_FLOAT)
    GB.ReturnFloat(((GB_FLOAT *)(void *)&result)->value);
  else
    GB.ReturnInteger(0);
	
	GB.ReturnConvVariant();

END_METHOD


/* val should be GB_VARIANT. This is just a trick, so that
   I can use STRING() and LENGTH() macros just after having
   converted val to a string. Yes, I know... Horrible.
*/

BEGIN_METHOD(CVB_str, GB_STRING val)

  GB.Conv((GB_VALUE *)(void *)ARG(val), GB_T_STRING);

  GB.ReturnNewString(STRING(val), LENGTH(val));

END_METHOD

BEGIN_METHOD(CVB_Left, GB_STRING val;GB_INTEGER Count;)

  if (VARG(Count)<=0)
  {
  	GB.Error("Invalid parameter");
	return;
  }
  if ( LENGTH(val)<=VARG(Count) )
  {
  	GB.ReturnNewString(STRING(val), LENGTH(val));
	return;	
  }
  
  GB.ReturnNewString(STRING(val), VARG(Count));

END_METHOD

BEGIN_METHOD(CVB_Right, GB_STRING val;GB_INTEGER Count;)

  if (VARG(Count)<=0)
  {
  	GB.Error("Invalid parameter");
	return;
  }
  if ( LENGTH(val)<=VARG(Count) )
  {
  	GB.ReturnNewString(STRING(val), LENGTH(val));
	return;	
  }
  
  GB.ReturnNewString(STRING(val)+(LENGTH(val)-VARG(Count)), VARG(Count));

END_METHOD

BEGIN_METHOD(CVB_Mid, GB_STRING val;GB_INTEGER Start;GB_INTEGER Count;)

  int count;
   
  if (VARG(Count)<=0)
  {
  	GB.Error("Invalid parameter");
	return;
  }
  
  if (!MISSING (Count) )
  {
  	if (VARG(Count)<=0)
	{
		GB.Error("Invalid parameter");
		return;
	}
	count=VARG(Count);
  }
  else
  	count=LENGTH(val);
	
  if ( VARG(Start)>LENGTH(val) )
  	GB.ReturnNewString(NULL,0);
   
  if ( ( LENGTH(val)-VARG(Start) ) <=   count ) count=LENGTH(val)-VARG(Start)+1;

  GB.ReturnNewString(STRING(val)+VARG(Start)-1, count );
  

END_METHOD

BEGIN_METHOD(CVB_DateAdd, GB_STRING period;GB_INTEGER interval; GB_DATE Date)

  char *Period=NULL;
  int Interval = 0;
  GB_DATE RETURN;
  Period=GB.ToZeroString(ARG(period));
  Interval = VARG(interval);

  RETURN.type = ARG(Date)->type;
  RETURN.value.date = ARG(Date)->value.date;
  RETURN.value.time = ARG(Date)->value.time;

  if (strncasecmp(Period,"yyyy",4) == 0){ //Add Years
      DATE_adjust(&RETURN, 4, Interval);
  }
  else {
  	if (strncasecmp(Period,"ww",2)== 0){ //Add weeks
      	    DATE_adjust(&RETURN, 1, (7 * Interval));
  	}
	else {
		switch(Period[0])
		{
			case 's': /* Seconds */
			case 'S':
      			        DATE_adjust(&RETURN, 2, (1000 * Interval));
				break;
			case 'n': /* Minutes */
			case 'N':
      			        DATE_adjust(&RETURN, 2, (60000 * Interval));
				break;
			case 'h': /* Hours */
			case 'H':
      			        DATE_adjust(&RETURN, 2, (3600000 * Interval));
				break;
			case 'd': /* Days */
			case 'D':
			case 'y': /* Day of Year */
			case 'Y':
      			        DATE_adjust(&RETURN, 1, Interval);
				break;
			case 'w': /* Weekdays : Monday - Friday */
			case 'W':
      			        DATE_adjust(&RETURN, 3, Interval);
				break;
			case 'm': /* Calendar Months */
			case 'M': 
      				DATE_adjust(&RETURN, 0, Interval);
				break;
			case 'q': /* Quarters */
			case 'Q': 
      				DATE_adjust(&RETURN, 0, (3 * Interval));
				break;
			default:
    				GB.Error("Invalid date parameter");
		}
	}
  }


  GB.ReturnDate(&RETURN);

END_METHOD

BEGIN_METHOD(CVB_DateDiff, GB_STRING period;GB_DATE Date1; GB_DATE Date2)

  GB_DATE Date1, Date2;
  char *Period=NULL;
  int Interval = 0;

  Period=GB.ToZeroString(ARG(period));
  Date1.type = ARG(Date1)->type;
  Date1.value.date = ARG(Date1)->value.date;
  Date1.value.time = ARG(Date1)->value.time;

  Date2.type = ARG(Date2)->type;
  Date2.value.date = ARG(Date2)->value.date;
  Date2.value.time = ARG(Date2)->value.time;

  if (strncasecmp(Period,"yyyy",4) == 0){ //Add Years
      Interval = DATE_diff(&Date1, &Date2, 4);
  }
  else {
  	if (strncasecmp(Period,"ww",2)== 0){ //Weeks
            Interval = DATE_diff(&Date1, &Date2, 5);
  	}
	else {
		switch(Period[0])
		{
			case 's': /* Seconds */
			case 'S':
                                Interval = DATE_diff(&Date1, &Date2, 2)/1000;
				break;
			case 'n': /* Minutes */
			case 'N':
                                Interval = DATE_diff(&Date1, &Date2, 2)/60000;
				break;
			case 'h': /* Hours */
			case 'H':
                                Interval = DATE_diff(&Date1, &Date2, 2)/3600000;
				break;
			case 'd': /* Days */
			case 'D':
			case 'y': /* Day of Year */
			case 'Y':
                                Interval = DATE_diff(&Date1, &Date2, 1);
				break;
			case 'w': /* Weekdays : Monday - Friday */
			case 'W':
                                Interval = DATE_diff(&Date1, &Date2, 3);
				break;
			case 'm': /* Calendar Months */
			case 'M': 
                                Interval = DATE_diff(&Date1, &Date2, 0);
				break;
			case 'q': /* Quarters */
			case 'Q': 
                                Interval = (DATE_diff(&Date1, &Date2, 0)/3);
				break;
			default:
    				GB.Error("Invalid date parameter");
		}
	}
  }

   GB.ReturnInteger(Interval);

END_METHOD

BEGIN_METHOD(CVB_Round,GB_FLOAT Number;GB_INTEGER Decimals;)

	double decimals=VARGOPT(Decimals, 0);
	double number=VARG(Number);

	if (decimals < 0) { GB.Error("Invalid argument"); return; }
	
        #if 0 /* test */
	decimals=exp10(decimals);
	number*=decimals;
	number=round(number);
	number/=decimals;
        #endif

  decimals = pow(10, decimals);
  number *= decimals;
  number = rint(number);
  number /= decimals;

  GB.ReturnFloat(number);

END_METHOD

GB_DESC CVbDesc[] =
{
  GB_DECLARE("Vb", 0), GB_NOT_CREATABLE(),

  GB_STATIC_METHOD("Val", "v", CVB_val, "(String)s"),
  GB_STATIC_METHOD("Str", "s", CVB_str, "(Value)v"),
  GB_STATIC_METHOD("Str$", "s", CVB_str, "(Value)v"),
  GB_STATIC_METHOD("Left", "s", CVB_Left, "(String)s(Count)i"),
  GB_STATIC_METHOD("Left$", "s", CVB_Left, "(String)s(Count)i"),
  GB_STATIC_METHOD("Right", "s", CVB_Right, "(String)s(Count)i"),
  GB_STATIC_METHOD("Right$", "s", CVB_Right, "(String)s(Count)i"),
  GB_STATIC_METHOD("Mid", "s", CVB_Mid, "(String)s(Start)i[(Count)i]"),
  GB_STATIC_METHOD("Mid$", "s", CVB_Mid, "(String)s(Start)i[(Count)i]"),
  GB_STATIC_METHOD("DateAdd", "d", CVB_DateAdd, "(period)s(interval)i(Date)d"),
  GB_STATIC_METHOD("DateDiff", "i", CVB_DateDiff, "(period)s(Date1)d(Date2)d"),
  GB_STATIC_METHOD("Round","f",CVB_Round,"(Number)f[(Decimals)i"),
  //GB_METHOD("Dir", "s", CVB_dir, "(Mask)s[(Attributes)i]"),

  GB_END_DECLARE
};





