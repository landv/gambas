/***************************************************************************

  local.h

  The intenationalization management routines

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

#ifndef __GBX_LOCAL_H
#define __GBX_LOCAL_H

#ifndef GBX_INFO

#include <locale.h>
#include "gbx_date.h"
#include "gbx_archive.h"

#endif

enum {
  LF_USER,
  LF_STANDARD,
  LF_GENERAL_NUMBER,
  LF_FIXED,
  LF_PERCENT,
  LF_SCIENTIFIC,
  LF_CURRENCY,
  LF_INTERNATIONAL,
  LF_GENERAL_DATE,
  LF_LONG_DATE,
  LF_MEDIUM_DATE,
  LF_SHORT_DATE,
  LF_LONG_TIME,
  LF_MEDIUM_TIME,
  LF_SHORT_TIME,
  };

#ifndef GBX_INFO

enum {
  LO_HOUR = 0,
  LO_MINUTE = 1,
  LO_SECOND = 2,
  LO_YEAR = 0,
  LO_MONTH = 1,
  LO_DAY = 2
  };

typedef
  struct {
    char decimal_point;
    char currency_decimal_point;
    char thousand_sep;
    char currency_thousand_sep;
    char group_size;
    char currency_group_size;
    char currency_flag;
    char date_sep;
    char time_sep;
    char _reserved[3];
    char *currency_symbol;
    char *intl_currency_symbol;
    char date_order[4];
    char time_order[4];
    char long_date[20];
    char medium_date[12];
    char short_date[12];
    char long_time[12];
    char medium_time[12];
    char short_time[8];
    char general_date[20];
    char general_currency[20];
    char intl_currency[20];
    bool rtl;
    }
  PACKED
  LOCAL_INFO;

#ifndef __GBX_LOCAL_C
EXTERN LOCAL_INFO LOCAL_default, LOCAL_local;
EXTERN char *LOCAL_encoding;
EXTERN bool LOCAL_is_UTF8;
#endif


PUBLIC void LOCAL_init(void);
PUBLIC void LOCAL_exit(void);
PUBLIC boolean LOCAL_format_number(double number, int fmt_type, const char *fmt, int len_fmt, char **str, int *len_str, boolean local);
PUBLIC boolean LOCAL_format_date(DATE_SERIAL *date, int fmt_type, const char *fmt, int len_fmt, char **str, int *len_str);
PUBLIC const char *LOCAL_get_lang(void);
PUBLIC void LOCAL_set_lang(const char *lang);
PUBLIC const char *LOCAL_gettext(const char *msgid);
PUBLIC void LOCAL_load_translation(ARCHIVE *arch);

#define LOCAL_get(_local) ((_local) ? &LOCAL_local : &LOCAL_default)

#endif

#endif
