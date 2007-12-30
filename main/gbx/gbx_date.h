/***************************************************************************

  Date.h

  The Date management routines

  (c) 2000-2005 Benoît Minisini <gambas@users.sourceforge.net>

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
  DATE_SERIAL;

typedef
  struct {
    long date;
    long time;
    }
  DATE;

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

PUBLIC void DATE_init(void);
PUBLIC DATE_SERIAL *DATE_split(VALUE *value);
PUBLIC bool DATE_make(DATE_SERIAL *date, VALUE *val);
PUBLIC void DATE_from_time(time_t time, long usec, VALUE *val);
PUBLIC void DATE_now(VALUE *val);

PUBLIC int DATE_to_string(char *buffer, VALUE *value);
PUBLIC boolean DATE_from_string(const char *str, long len, VALUE *val, boolean local);
PUBLIC int DATE_comp_value(VALUE *date1, VALUE *date2);
PUBLIC int DATE_comp(DATE *date1, DATE *date2);

PUBLIC bool DATE_timer(double *time, int from_start);

PUBLIC void DATE_void_value(VALUE *value);

PUBLIC void DATE_add(VALUE *date, int period, long val);
PUBLIC long DATE_diff(VALUE *date1, VALUE *date2, int period);

#endif

#endif
