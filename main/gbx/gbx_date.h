/***************************************************************************

  gbx_date.h

  (c) 2000-2009 Benoît Minisini <gambas@users.sourceforge.net>

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
  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

***************************************************************************/

#ifndef __DATE_H
#define __DATE_H

#ifndef GBX_INFO

#include <sys/time.h>

#include "gbx_value.h"

typedef
  struct {
    short year;
    short month;
    short day;
    short hour;
    short min;
    short sec;
    short weekday;
    short msec;
    }
  PACKED
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
  DP_SECOND = 1,
  DP_MINUTE = 2,
  DP_HOUR = 3,
  DP_DAY = 4,
  DP_WEEK = 5,
  DP_WEEKDAY = 6,
  DP_MONTH = 7,
  DP_QUARTER = 8,
  DP_YEAR = 9,
  };
  
#ifndef GBX_INFO
  
#define DATE_YEAR_MIN  -4801
#define DATE_YEAR_MAX   9999

#define DATE_NDAY_BC 1753350

void DATE_init(void);
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
