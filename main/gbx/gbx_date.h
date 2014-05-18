/***************************************************************************

  gbx_date.h

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

#ifndef __GBX_DATE_H
#define __GBX_DATE_H

#ifndef GBX_INFO

#include <sys/time.h>

#include "gbx_value.h"

typedef
  struct {
    int year;
    int month;
    int day;
    int hour;
    int min;
    int sec;
    int weekday;
    int msec;
    }
  DATE_SERIAL;

#ifndef __DATE_DECLARED
#define __DATE_DECLARED
typedef
  struct {
    int date;
    int time;
    }
  DATE;
#endif

#endif
  
enum {
	DP_MILLISECOND = 1,
  DP_SECOND = 2,
  DP_MINUTE = 3,
  DP_HOUR = 4,
  DP_DAY = 5,
  DP_WEEK = 6,
  DP_WEEKDAY = 7,
  DP_MONTH = 8,
  DP_QUARTER = 9,
  DP_YEAR = 10,
  };
  
#ifndef GBX_INFO

#define DATE_YEAR_MIN  -4801
#define DATE_YEAR_MAX   9999

#define DATE_NDAY_BC 1753350

#define DATE_SERIAL_has_no_date(_date) ((_date)->year == 0)
#define DATE_SERIAL_has_no_time(_date) ((_date)->hour == 0 && (_date)->min == 0 && (_date)->sec == 0 && (_date)->msec == 0)
	
void DATE_init(void);
void DATE_init_local(void);
int DATE_get_timezone(void);
DATE_SERIAL *DATE_split(VALUE *value);
bool DATE_make(DATE_SERIAL *date, VALUE *val);
void DATE_from_time(time_t time, int usec, VALUE *val);
void DATE_now(VALUE *val);

int DATE_to_string(char *buffer, VALUE *value);
bool DATE_from_string(const char *str, int len, VALUE *val, bool local);
int DATE_comp_value(VALUE *date1, VALUE *date2);
int DATE_comp(DATE *date1, DATE *date2);

double DATE_to_double(struct timeval *time, int from_start);
bool DATE_timer(double *result, int from_start);

void DATE_void_value(VALUE *value);

void DATE_add(VALUE *date, int period, int val);
int DATE_diff(VALUE *date1, VALUE *date2, int period);

#endif

#endif
