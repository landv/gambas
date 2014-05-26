/***************************************************************************

  gbx_date.c

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

#define __DATE_C

//#define _BSD_SOURCE

#include <unistd.h>
#include <ctype.h>
#include <time.h>
#include <stdlib.h>

#include "gb_common.h"
#include "gb_common_buffer.h"
#include "gb_error.h"
#include "gbx_value.h"
#include "gbx_local.h"
#include "gbx_number.h"

#include "gbx_date.h"

//#define DEBUG_DATE

#define buffer_init COMMON_buffer_init
#define get_char COMMON_get_char
#define last_char COMMON_last_char
#define look_char COMMON_look_char
#define put_char COMMON_put_char
#define jump_space COMMON_jump_space
#define get_current COMMON_get_current
#define buffer_pos COMMON_pos
#define get_size_left COMMON_get_size_left

static const char days_in_months[2][13] =
{  /* error, jan feb mar apr may jun jul aug sep oct nov dec */
	{  0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 },
	{  0, 31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 } /* leap year */
};

static const short days_in_year[2][14] =
{  /* 0, jan feb mar apr may  jun  jul  aug  sep  oct  nov  dec */
	{  0, 0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334, 365 },
	{  0, 0, 31, 60, 91, 121, 152, 182, 213, 244, 274, 305, 335, 366 }  /* leap year */
};

static double _start_time;

// Returns 1 for a leap year, 0 else

static int date_is_leap_year(short year)
{
	if (year < 0)
		year += 8001;

	if ((((year % 4) == 0) && ((year % 100) != 0)) || (year % 400) == 0)
		return 1;
	else
		return 0;
}

static bool date_is_valid(DATE_SERIAL *date)
{
	return ((date->year == 0
	         || ((date->month >= 1) && (date->month <= 12) &
	             (date->year >= DATE_YEAR_MIN) && (date->year <= DATE_YEAR_MAX) && (date->year != 0) &&
	             (date->day >= 1) && (date->day <= days_in_months[date_is_leap_year(date->year)][(short)date->month])))
	        && (date->hour >= 0) && (date->hour <= 23) && (date->min >= 0) && (date->min <= 59)
	        && (date->sec >= 0) && (date->sec <= 59));
}

static short date_to_julian_year(short year)
{
	if (year < 0)
		return year - DATE_YEAR_MIN;
	else
		return year - DATE_YEAR_MIN - 1;
}

static short date_from_julian_year(short year)
{
	if (year < (-DATE_YEAR_MIN))
		return year + DATE_YEAR_MIN;
	else
		return year + DATE_YEAR_MIN + 1;
}

void DATE_init(void)
{
	struct timeval tv;

	if (gettimeofday(&tv, NULL) == 0)
		_start_time = (double)tv.tv_sec + (double)tv.tv_usec / 1E6;
	else
		_start_time = 0.0;

	//DATE_init_local();
}

void DATE_init_local(void)
{
	// Prevent glibc for calling stat("/etc/localtime") again and again...
	if (!getenv("TZ"))
		putenv("TZ=:/etc/localtime");

	tzset();
}

DATE_SERIAL *DATE_split(VALUE *value)
{
	static int last_nday, last_nmsec;
	static DATE_SERIAL last_date = { 0 };

	int nday, nmsec;
	int A, B, C, D, E, M;

	nday = value->_date.date;
	nmsec = value->_date.time;

	if (nday > 0)
		nmsec += DATE_get_timezone() * 1000;

	if (nmsec < 0)
	{
		nday--;
		nmsec += 86400000;
	}
	else if (nmsec >= 86400000)
	{
		nday++;
		nmsec -= 86400000;
	}

	if (last_nmsec != nmsec)
	{
		last_nmsec = nmsec;

		last_date.msec = nmsec % 1000;
		nmsec /= 1000;
		last_date.sec = nmsec % 60;
		nmsec /= 60;
		last_date.min = nmsec % 60;
		nmsec /= 60;
		last_date.hour = nmsec;
	}

	if (last_nday != nday)
	{
		last_nday = nday;

		/*nday += DATE_NDAY_BC;*/
		if (nday <= 0)
		{
			last_date.month = 0;
			last_date.day = 0;
			last_date.year = 0;
			last_date.weekday = 0;
		}
		else
		{
			A = nday - 58 - 1;
			B = (4 * (A + 36524))/ 146097 - 1;
			C = A - (146097 * B)/4;
			D = (4 * (C + 365)) / 1461 - 1;
			E = C - ((1461 * D) / 4);
			M = (5 * (E - 1) + 2) / 153;

			last_date.month = M + 3 - (12 * (M / 10));
			last_date.day = E - (153 * M + 2)/5;
			last_date.year = 100 * B + D + (M / 10) /*- 4800*/ ;
			last_date.year = date_from_julian_year(last_date.year);
			last_date.weekday = (nday - 1) % 7;
		}
	}

	//fprintf(stderr, "DATE_split: %d %d %d / %d %d %d . %d\n", last_date.year, last_date.month, last_date.day, last_date.hour, last_date.min, last_date.sec, last_date.msec);

	return &last_date;
}


bool DATE_make(DATE_SERIAL *date, VALUE *val)
{
	short year;
	int nday;
	bool timezone;

	if (!date_is_valid(date))
		return TRUE;

	if (date->year == 0)
	{
		nday = 0; /*(-DATE_NDAY_BC - 1);*/
		timezone = FALSE;
	}
	else
	{
		year = date_to_julian_year(date->year);

		nday = year * 365;
		year--;
		nday += (year >>= 2);
		nday -= (year /= 25);
		nday += year >> 2;

		nday += days_in_year[date_is_leap_year(date->year)][(short)date->month] + date->day;

		/*nday -= DATE_NDAY_BC;*/

		timezone = TRUE;
	}

	val->_date.date = nday;
	val->_date.time = ((date->hour * 60) + date->min) * 60 + date->sec;
	if (timezone)
		val->_date.time -= DATE_get_timezone();

	if (val->_date.time < 0)
	{
		val->_date.date--;
		val->_date.time += 86400;
	}
	else if (val->_date.time >= 86400)
	{
		val->_date.date++;
		val->_date.time -= 86400;
	}

	val->_date.time = val->_date.time * 1000 + date->msec;
	val->type = T_DATE;

	return FALSE;
}


static int get_current_year(void)
{
	struct tm *tm;
	struct timeval tv;

	if (gettimeofday(&tv, NULL) != 0)
		THROW(E_DATE);

	tm = localtime((time_t *)&tv.tv_sec);
	return tm->tm_year + 1900;
}


void DATE_from_time(time_t time, int usec, VALUE *val)
{
	static struct tm tm;
	static time_t last_time = (time_t)-1;
	
	DATE_SERIAL date;

	if (time != last_time)
	{
		localtime_r(&time, &tm);
		last_time = time;
	}

	date.year = tm.tm_year + 1900;
	date.month = tm.tm_mon + 1;
	date.day = tm.tm_mday;
	date.hour = tm.tm_hour;
	date.min = tm.tm_min;
	date.sec = tm.tm_sec;
	date.msec = usec / 1000;

	if (DATE_make(&date, val))
		val->type = T_NULL;
}


void DATE_now(VALUE *val)
{
	struct timeval tv;

	if (gettimeofday(&tv, NULL))
		val->type = T_NULL;
	else
	{
		//fprintf(stderr, "DATE_now: %d %d\n", tv.tv_sec, tv.tv_usec);
		DATE_from_time((time_t)tv.tv_sec, tv.tv_usec, val);
	}
}

int DATE_to_string(char *buffer, VALUE *value)
{
	DATE_SERIAL *date;
	int len;

	if (value->_date.date == 0 && value->_date.time == 0)
		return 0;
	
	date = DATE_split(value);

	if (value->_date.date == 0)
		len = sprintf(buffer,"%02d:%02d:%02d", date->hour, date->min, date->sec);
	else if ((date->hour | date->min | date->sec | date->msec) == 0)
		len = sprintf(buffer,"%02d/%02d/%04d", date->month, date->day, date->year);
	else
		len = sprintf(buffer,"%02d/%02d/%04d %02d:%02d:%02d", date->month, date->day, date->year, date->hour, date->min, date->sec);

	if (date->msec)
	{
		len += sprintf(&buffer[len], ".%03d", date->msec);
		while (buffer[len - 1] == '0')
			len--;
		buffer[len] = 0;
	}

	return len;
}


static bool read_integer(int *number)
{
	int nbr = 0;
	int nbr2;
	int c;
	bool minus = FALSE;

	c = get_char();

	if (c == '-')
	{
		minus = TRUE;
		c = get_char();
	}
	else if (c == '+')
		c = get_char();

	if ((c < 0) || !isdigit(c))
		return TRUE;

	for(;;)
	{
		nbr2 = nbr * 10 + (c - '0');
		if (nbr2 < nbr)
			return TRUE;
		nbr = nbr2;

		c = look_char();
		if ((c < 0) || !isdigit(c))
			break;

		buffer_pos++;
	}
	
	if (minus)
		nbr = (-nbr);

	*number = nbr;
	return FALSE;
}


static bool read_msec(int *number)
{
	int nbr = 0;
	int nbr2;
	int c;
	int i;

	c = get_char();

	if ((c < 0) || !isdigit(c))
		return TRUE;

	i = 0;
	for(;;)
	{
		i++;
		nbr2 = nbr * 10 + (c - '0');
		if (nbr2 < nbr)
			return TRUE;
		nbr = nbr2;

		if (i == 3)
			break;

		c = look_char();
		if ((c < 0) || !isdigit(c))
			break;

		buffer_pos++;
	}

	for (; i < 3; i++)
		nbr *= 10;

	*number = nbr;
	return FALSE;
}


static void set_date(DATE_SERIAL *date, int which, int value)
{
	if (which == LO_YEAR)
	{
		if (value >= 0 && value <= 99)
		{
			if (value > 30)
				value += 1900;
			else
				value += 2000;
		}
		date->year = value;
	}
	else if (which == LO_MONTH)
		date->month = value;
	else if (which == LO_DAY)
		date->day = value;
}


static void set_time(DATE_SERIAL *date, int which, int value)
{
	if (which == LO_HOUR)
		date->hour = value;
	else if (which == LO_MINUTE)
		date->min = value;
	else if (which == LO_SECOND)
		date->sec = value;
}


bool DATE_from_string(const char *str, int len, VALUE *val, bool local)
{
	DATE_SERIAL date;
	LOCAL_INFO *info = LOCAL_get(local);
	int nbr, nbr2;
	int c, i;
	bool has_date = FALSE;
	//bool has_time = FALSE;

	if (!len)
	{
		DATE_void_value(val);
		return FALSE;
	}

	CLEAR(&date);

	buffer_init(str, len);
	jump_space();

	if (read_integer(&nbr))
		return TRUE;

	c = get_char();

	if (c == info->date_sep)
	{
		has_date = TRUE;

		if (read_integer(&nbr2))
			return TRUE;

		c = get_char();

		if ((c < 0) || isspace(c))
		{
			i = 0;

			set_date(&date, LO_YEAR, get_current_year());

			if (info->date_order[i] == LO_YEAR) i++;
			set_date(&date, info->date_order[i], nbr); i++;

			if (info->date_order[i] == LO_YEAR) i++;
			set_date(&date, info->date_order[i], nbr2);
		}
		else if (c == info->date_sep)
		{
			set_date(&date, info->date_order[0], nbr);
			set_date(&date, info->date_order[1], nbr2);

			if (read_integer(&nbr))
				return TRUE;

			set_date(&date, info->date_order[2], nbr);
		}

		jump_space();

		c = look_char();
		if (c < 0)
			goto _OK;

		if (read_integer(&nbr))
			return TRUE;

		c = get_char();
	}

	if (c == info->time_sep)
	{
		//has_time = TRUE;

		if (read_integer(&nbr2))
			return TRUE;

		c = get_char();

		if ((c < 0) || isspace(c))
		{
			i = 0;

			if (info->time_order[i] == LO_SECOND) i++;
			set_time(&date, info->time_order[i], nbr); i++;

			if (info->time_order[i] == LO_SECOND) i++;
			set_time(&date, info->time_order[i], nbr2);
		}
		else if (c == info->time_sep)
		{
			set_time(&date, info->time_order[0], nbr);
			set_time(&date, info->time_order[1], nbr2);

			if (read_integer(&nbr))
				return TRUE;

			set_time(&date, info->time_order[2], nbr);

			c = get_char();
			if (c == '.') // msec separator
			{
				if (read_msec(&nbr))
					return TRUE;
				date.msec = nbr;
			}
		}

		c = get_char();
		if ((c < 0) || isspace(c))
			goto _OK;
	}

	return TRUE;

_OK:

	if (DATE_make(&date, val))
		return TRUE;

	if (!has_date)
		val->_date.date = 0;

	return FALSE;
}


int DATE_comp(DATE *date1, DATE *date2)
{
	if (date1->date < date2->date)
		return (-1);

	if (date1->date > date2->date)
		return 1;

	if (date1->time < date2->time)
		return (-1);

	if (date1->time > date2->time)
		return 1;

	return 0;
}


int DATE_comp_value(VALUE *date1, VALUE *date2)
{
	return DATE_comp((DATE *)&date1->_date.date, (DATE *)&date2->_date.date);
}


double DATE_to_double(struct timeval *time, int from_start)
{
	double result = (double)time->tv_sec + (double)time->tv_usec / 1E6;
	
	if (from_start)
		result -= _start_time;
	
	return result;
}

bool DATE_timer(double *result, int from_start)
{
	struct timeval tv;

	if (gettimeofday(&tv, NULL))
	{
		*result = 0;
		return TRUE;
	}
	
	*result = DATE_to_double(&tv, from_start);
	return FALSE;
}


void DATE_void_value(VALUE *value)
{
	value->type = T_DATE;
	value->_date.time = 0;
	value->_date.date = 0;
}


void DATE_add(VALUE *date, int period, int val)
{
	int64_t add_time = 0;
	int64_t add_date = 0;
	int64_t fix_date;
	int64_t new_time;
	int64_t new_date;
	DATE_SERIAL ds;
	int y, m, d;

	switch(period)
	{
		case DP_MILLISECOND:
			add_date = val / 86400000;
			add_time = val % 86400000 ;
			goto __ADD_DATE_TIME;

		case DP_SECOND:
			add_date = val / 86400;
			add_time = (val % 86400) * 1000;
			goto __ADD_DATE_TIME;

		case DP_MINUTE:
			add_date = val / 1440;
			add_time = (val % 1440) * 60000;
			goto __ADD_DATE_TIME;

		case DP_HOUR:
			add_date = val / 24;
			add_time = (val % 24) * 3600000;
			goto __ADD_DATE_TIME;

		case DP_DAY:
			add_date = val;
			goto __ADD_DATE_TIME;

		case DP_WEEK:
			add_date = val * 7;
			goto __ADD_DATE_TIME;

		default:
			break;
	}

	ds = *DATE_split(date);

	switch(period)
	{
		case DP_WEEKDAY:
			add_date = (val / 5) * 7;
			ds.weekday += val % 5;
			if (ds.weekday > 5)
				add_date += 2;
			else if (ds.weekday < 1)
				add_date -= 2;
			add_date += val % 5;
			goto __ADD_DATE_TIME;

		case DP_QUARTER:
			val *= 3;
			/* continue; */

		case DP_MONTH:
			y = ((ds.year * 12) + (ds.month - 1) + val) / 12;
			m = ((ds.month - 1) + val) % 12;
			if (m < 0) m += 12;
			m++;
			d = days_in_months[date_is_leap_year(y)][m];
			d = ds.day > d ? d : ds.day;
			ds.day = d;
			ds.month = m;
			ds.year = y;
			goto __MAKE_DATE;

		case DP_YEAR:
			ds.year += val;
			if (ds.month == 2 && ds.day == 29)
			{
				d = days_in_months[date_is_leap_year(ds.year)][ds.month];
				if (ds.day > d)
					ds.day = d;
			}
			goto __MAKE_DATE;

		default:
			THROW(E_ARG);
	}

__ADD_DATE_TIME:

	new_time = date->_date.time + add_time;
	new_date = date->_date.date + add_date;

	if (new_time >= 86400000)
	{
		fix_date = new_time / 86400000;
		new_time -= fix_date * 86400000;
		new_date += fix_date;
	}
	else if (new_time < 0)
	{
		fix_date = (-new_time + 86400000 - 1) / 86400000;
		new_time += fix_date * 86400000;
		new_date -= fix_date;
	}

	if (new_time < INT32_MIN || new_time > INT32_MAX || new_date < 0 || new_date > INT32_MAX)
		THROW(E_OVERFLOW);

	date->_date.date = (int)new_date;
	date->_date.time = (int)new_time;
	return;

__MAKE_DATE:

	if (DATE_make(&ds, date))
		THROW(E_DATE);
	return;
}


int DATE_diff(VALUE *date1, VALUE *date2, int period)
{
	int64_t diff = 0;
	DATE_SERIAL ds1 = {0};
	DATE_SERIAL ds2 = {0};
	bool neg;

	switch (period)
	{
		case DP_DAY:
		case DP_WEEK:
			diff = date1->_date.date - date2->_date.date;
			break;

		case DP_MILLISECOND:
		case DP_SECOND:
		case DP_MINUTE:
		case DP_HOUR:
			diff = date1->_date.date - date2->_date.date;
			diff = diff * 86400000 + (date1->_date.time - date2->_date.time);
			break;

		case DP_MONTH:
		case DP_QUARTER:
		case DP_YEAR:
			ds1 = *DATE_split(date1);
			ds2 = *DATE_split(date2);
			break;

		case DP_WEEKDAY:
			diff = date1->_date.date - date2->_date.date;
			ds1 = *DATE_split(date1);
			ds2 = *DATE_split(date2);
			break;

		default:
			THROW(E_ARG);
	}

	switch (period)
	{
		case DP_DAY:
			break;

		case DP_WEEK:
			diff /= 7;
			break;

		case DP_SECOND:
			diff /= 1000;
			break;

		case DP_MINUTE:
			diff /= 60000;
			break;

		case DP_HOUR:
			diff /= 3600000;
			break;

		case DP_WEEKDAY:

			neg = (diff < 0);
			if (neg)
			{
				int swap;
				diff = (-diff);
				swap = ds1.weekday;
				ds1.weekday = ds2.weekday;
				ds2.weekday = swap;
			}

			diff = diff / 7 * 5;

			/* last day is not included ! */
			while (ds2.weekday != ds1.weekday)
			{
				if (ds2.weekday > 0 && ds2.weekday < 6)
					diff++;
				ds2.weekday++;
				if (ds2.weekday == 7)
					ds2.weekday = 0;
			}

			if (neg)
				diff = (-diff);

			break;

		case DP_MONTH:
			diff = (ds1.year - ds2.year) * 12 + ds1.month - ds2.month;
			break;

		case DP_QUARTER:
			diff = (ds1.year - ds2.year) * 4 + (ds1.month - ds2.month) / 3;
			break;

		case DP_YEAR:
			diff = ds1.year - ds2.year;
			break;

		case DP_MILLISECOND:
			break;
	}

	if (diff < INT32_MIN || diff > INT32_MAX)
		THROW(E_OVERFLOW);

	return diff;
}

// Beware: System.TimeZone is what must be added to go to UTC
// So if the shell command `date` displays "UTC+2", then that
// function must return -7200!

int DATE_get_timezone(void)
{
	static time_t last = (time_t)0;
	static int tz = 0;

	time_t t = time(NULL);
	if ((t - last) >= 600)
	{
		struct tm *tm = localtime(&t);
		tz = -tm->tm_gmtoff;
	}

	return tz;
}
