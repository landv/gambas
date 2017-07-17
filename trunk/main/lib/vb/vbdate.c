/***************************************************************************

  vbdate.c

  (c) 2000-2003 Nigel Gerrard <nigel@gerrard1123.freeserve.co.uk>

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

#define __VBDATE_C

#include "vbdate.h"
#include <stdio.h>
#include <string.h>


#define DATE_YEAR_MIN  -4801
#define DATE_YEAR_MAX   9999

/* Helper functions */

const char days_in_months[2][13] =
{  /* error, jan feb mar apr may jun jul aug sep oct nov dec */
  {  0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 },
  {  0, 31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 } /* leap year */
};

const short days_in_year[2][14] =
{  /* 0, jan feb mar apr may  jun  jul  aug  sep  oct  nov  dec */
  {  0, 0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334, 365 },
  {  0, 0, 31, 60, 91, 121, 152, 182, 213, 244, 274, 305, 335, 366 }  /* leap year */
};

/* Returns 1 for a leap year, 0 else */
int date_is_leap_year(short year)
{
  if (year < 0)
    year += 8001;

  if ((((year % 4) == 0) && ((year % 100) != 0)) || (year % 400) == 0)
    return 1;
  else
    return 0;
}

int date_is_valid(GB_DATE_SERIAL *date)
{
  return ((date->month >= 1) && (date->month <= 12) &&
          (date->year >= DATE_YEAR_MIN) && (date->year <= DATE_YEAR_MAX) && (date->year != 0) &&
          (date->day >= 1) && (date->day <= days_in_months[date_is_leap_year(date->year)][(short)date->month]) &&
          (date->hour >= 0) && (date->hour <= 23) && (date->min >= 0) && (date->min <= 59) &&
          (date->sec >= 0) && (date->sec <= 59));
}

/** Modulo function that works correctly for negative divisors */
//#define modulo(x,y)  ((x)%(y)<0?(x)%(y)+(y):(x)%(y))

static int modulo(int x, int y)
{
	int mod = x % y;
	if (mod < 0)
		mod += y;
	return mod;
}

void DATE_adjust( GB_DATE *vdate, int period, int interval) /* Adjust the date by the interval period */
{

  GB_DATE_SERIAL *date;
  int year, month, day;
  date = GB.SplitDate(vdate);

  switch(period){
	  case 0: /* Calendar Month */
		  year = ((date->year * 12) + (date->month - 1) + interval)/12;
		  month = modulo((date->month - 1)+interval, 12) + 1;
		  day = date->day > days_in_months[date_is_leap_year(year)][month] ? days_in_months[date_is_leap_year(year)][month] : date->day;
		  date->day = day;
		  date->month = month;
		  date->year = year;
                  GB.MakeDate(date, vdate);
		  break;
	  case 1: /* days */
		  vdate->value.date += interval;
		  break;
	  case 2: /* Time */
		  vdate->value.time += interval;
		  break;
	  case 3: /* weekdays - in this case weekdays are Mon - Fri */
		  vdate->value.date += ( 7 * ( interval / 5));
		  date->weekday += ( interval % 5 );
		  if (date->weekday > 5){
			  date->weekday -= 5;
                          vdate->value.date += 2;
		  }
		  if (date->weekday < 1){
			  date->weekday += 5;
                          vdate->value.date -= 2;
		  }
                  vdate->value.date += ( interval % 5);
		  break;
	  case 4: /* Add year */
		  while ( interval != 0){
			if ( interval < 0 ){
		             vdate->value.date -= days_in_year[date_is_leap_year(date->year)][13];
			     date->year--;
			     interval++;
			}
			else {
		             vdate->value.date += days_in_year[date_is_leap_year(date->year)][13];
			     date->year++;
			     interval--;
			}
		  }
		  break;
  }

  /* Now if time takes it into another day */
  while (vdate->value.time >= 86400000){
	  vdate->value.date++;
	  vdate->value.time -= 86400000;
  }
  /* Or time is negative so we need to take off a day */
  while (vdate->value.time < 0){
	  vdate->value.date--;
	  vdate->value.time += 86400000;
  }

  CLEAR(&date);
  date = GB.SplitDate(vdate);

  if (!date_is_valid(date)){
       /*printf("Invalid date : year [%i] month [%i] day [%i] hour [%i] min [%i] sec [%i] weekday [%i] msec [%i]\n",
		    date->year, date->month, date->day, date->hour, date->min, date->sec, date->weekday, date->msec);*/
       GB.Error("Invalid Date Returned");
  }
  CLEAR(&date);

}

int DATE_diff( GB_DATE *vdate1, GB_DATE *vdate2, int period)
{
  GB_DATE_SERIAL *date1, *date2;
  int diff = 0;
  int year1, year2, month1, month2, weekday1, weekday2;

  date1 = GB.SplitDate(vdate1);
  weekday1 = date1->weekday;
  year1 = date1->year;
  month1 = date1->month;

  date2 = GB.SplitDate(vdate2);
  weekday2 = date2->weekday;
  year2 = date2->year;
  month2 = date2->month;
  
  switch(period){
	  case 0: /* Calendar Month */
		  diff = ((year1 * 12) + month1) - ((year2 * 12) + month2);
		  break;
	  case 1: /* days */
		  diff = vdate1->value.date - vdate2->value.date;
		  break;
	  case 2: /* Time */
		  diff = ((vdate1->value.date - vdate2->value.date) * 86400000) + (vdate1->value.time - vdate2->value.time);
		  break;
	  case 3: /* weekdays */
		  /* If invalid weekday then find closest valid day:
		   * eg. 7 -1, 6 -5*/
		  weekday1 == 7 ? weekday1 = 1 : weekday1 == 6 ? weekday1 = 5 : weekday1; 
		  weekday2 == 7 ? weekday2 = 1 : weekday2 == 6 ? weekday2 = 5 : weekday2; 
		       
		  diff = (((vdate1->value.date - vdate2->value.date) / 7) * 5) 
			  + (weekday1 - weekday2);
		  /* Needs validating */
		  break;
	  case 4: /* year */
		  diff = year1 - year2;
		  break;
	  case 5: /* Weeks */
		  diff = (vdate1->value.date - vdate2->value.date)/7;
		  break;
  }
  CLEAR(&date1);
  CLEAR(&date2);
  return diff;
}

/* End Helper functions */

/*
GB_DESC CVbDateDesc[] =
{
  GB_DECLARE("Vb", 0), GB_NOT_CREATABLE(),

  GB_STATIC_METHOD("DateAdd", "d", CVB_DateAdd, "(period)s(interval)i(Date)v"),
  GB_STATIC_METHOD("DateDiff", "i", CVB_DateDiff, "(period)s(Date1)v(Date2)v"),
  
  GB_END_DECLARE
};*/
 
