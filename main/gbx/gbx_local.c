/***************************************************************************

  local.c

  The intenationalization management routines

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

#define __GBX_LOCAL_C

#include "gb_common.h"
#include "gb_common_buffer.h"
#include "gb_common_case.h"

#include "gb_error.h"
#include "gbx_value.h"
#include "gb_limit.h"

#include <locale.h>
#include <langinfo.h>

#include <time.h>
#include <ctype.h>
#include <float.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <libintl.h>

#include "gbx_math.h"
#include "gbx_date.h"
#include "gbx_string.h"
#include "gbx_api.h"
#include "gb_file.h"
#include "gbx_component.h"
#include "gbx_exec.h"
#include "gbx_archive.h"

#include "gbx_local.h"

//#define DEBUG_LANG

#define buffer_init COMMON_buffer_init
#define get_char COMMON_get_char
#define last_char COMMON_last_char
#define look_char COMMON_look_char
#define put_char COMMON_put_char
#define jump_space COMMON_jump_space
#define get_current COMMON_get_current
#define buffer_pos COMMON_pos
#define get_size_left COMMON_get_size_left

/* System encoding*/
PUBLIC char *LOCAL_encoding = NULL;

/* If system encoding is UTF-8 */
PUBLIC bool LOCAL_is_UTF8;

/* Default 'C' localization */
PUBLIC LOCAL_INFO LOCAL_default = {
  '.', '.',
  0, 0,
  3, 3,
  0,
  '/', ':',
  { 0 },
  "",
  "",
  { LO_MONTH, LO_DAY, LO_YEAR },
  { LO_HOUR, LO_MINUTE, LO_SECOND },
  "dddd mmmm d yyyy",
  "mmm d yy",
  "mm/dd/yy",
  "hh:nn:ss",
  "hh:nn AM/PM",
  "hh:nn",
  "(#,##0.##)",
  "(#,##0.##)"
  };

/* User language localization */
PUBLIC LOCAL_INFO LOCAL_local;

static char *_rtl_lang[] = { "ar", "fa", NULL };

static bool _translation_loaded = FALSE;

static LOCAL_INFO *local_current;

static char *env_LC_ALL = NULL;
static char *env_LANGUAGE = NULL;
static char *env_LANG = NULL;

static bool _currency;

static int my_setenv(const char *name, const char *value, char **ptr)
{
  char *str = NULL;

  STRING_add(&str, name, 0);
  STRING_add(&str, "=", 1);
  STRING_add(&str, value, 0);

  if (putenv(str))
  {
    STRING_free(&str);
    return 1;
  }
  else
  {
    STRING_free(ptr);
    *ptr = str;
    return 0;
  }
}

static void begin(void)
{
  buffer_init(COMMON_buffer, COMMON_BUF_MAX - 4);
}

static void end(char **str, long *len)
{
  *(get_current()) = 0;
  *str = COMMON_buffer;
  *len = buffer_pos;
}


static void stradd_sep(char *dst, const char *src, const char *sep)
{
  if (*dst)
    strcat(dst, sep);
  strcat(dst, src);
}


static void add_thousand_sep(int *before)
{
  int group;

  if (before == NULL)
    return;

  if (local_current->thousand_sep != 0)
  {
    group = _currency ? local_current->currency_group_size : local_current->group_size;

    if (group > 0 && (*before > 1) && ((*before - 1) == (((*before - 1) / group) * group)))
      put_char(local_current->thousand_sep);
  }

  (*before)--;
}


/*static char *strnadd(char *dst, char *src, int len, int *before)*/
static void add_string(const char *src, long len, int *before)
{
  if (len == 0)
    len = strlen(src);

  while (len > 0)
  {
    put_char(*src++);
    len--;

    add_thousand_sep(before);
  }
}

static void add_currency(const char *sym)
{
  const char *p = sym;
  char c;

  for(;;)
  {
    c = *p++;
    if (c == 0)
      return;
    if (c != ' ')
      put_char(c);
  }
}


static void add_zero(int zero, int *before)
{
  while (zero > 0)
  {
    put_char('0');
    zero--;

    add_thousand_sep(before);
  }
}


static long search(const char *src, long len, const char *list, long start, boolean not)
{
  long i;
  char c;

  for (i = start; i < len; i++)
  {
    c = src[i];

    if (c == '\\')
    {
      i++;
      if (i >= len)
      	break;
			c = src[i];
    }

    if ((index(list, c) != NULL) ^ not)
      return i;
  }

  return len;
}


static void add_sign(char mode, int sign, bool after)
{
  if (after && mode != '(')
    return;

  if (sign < 0)
  {
    if (mode == '(')
      put_char(after ? ')' : '(');
    else
      put_char('-');
  }
  else if (mode != 0 && mode != '(')
  {
    if (sign > 0)
      put_char(mode);
    else
      put_char(' ');
  }
}


static char *get_languages(void)
{
  char *lang;
  char *lang_list = NULL;
  char *src;

  STRING_new_temp(&lang, LOCAL_get_lang(), 0);

  STRING_add(&lang_list, lang, 0);
  STRING_add(&lang_list, ":", 1);

  src = index(lang, '_');
  if (src)
  {
    *src = 0;
    STRING_add(&lang_list, lang, 0);
    STRING_add(&lang_list, ":", 1);
  }

  #ifdef DEBUG_LANG
  fprintf(stderr, "Languages = %s\n", lang_list);
  #endif

  return lang_list;
}


static void free_local_info(void)
{
  CLEAR(&LOCAL_local);
  //STRING_unref(&LOCAL_local.currency_symbol);
  //STRING_unref(&LOCAL_local.intl_currency_symbol);
}

static void fill_local_info(void)
{
  static struct tm tm = { 0 };
  struct lconv *info;
  char buf[64];
  char *p;
  char c;
  char *dp;
  char *tp;
  char *codeset;

  /* Localisation courante */

  free_local_info();

  /* local encoding */

  STRING_free(&LOCAL_encoding);

  codeset = nl_langinfo(CODESET);
  if (!codeset || !*codeset)
  	codeset = "US-ASCII";

  STRING_new(&LOCAL_encoding, codeset, 0);

  LOCAL_is_UTF8 = (strcasecmp(LOCAL_encoding, "UTF-8") == 0);

  #ifdef DEBUG_LANG
  fprintf(stderr, "LOCAL_encoding = %s\n", LOCAL_encoding);
  #endif

  /* Numeric information */

  info = localeconv();

  LOCAL_local.decimal_point = *(info->decimal_point);
  LOCAL_local.thousand_sep = *(info->thousands_sep);
  if (LOCAL_local.thousand_sep == 0)
    LOCAL_local.thousand_sep = ' ';

  LOCAL_local.group_size = *(info->grouping);

  /*LOCAL_local.currency_symbol = STRING_conv_to_UTF8(info->currency_symbol, 0);
  STRING_ref(LOCAL_local.currency_symbol);
  LOCAL_local.intl_currency_symbol = STRING_conv_to_UTF8(info->int_curr_symbol, 0);
  STRING_ref(LOCAL_local.intl_currency_symbol);*/

  /* date information */

  tm.tm_year = 4; /* 02/03/1904 05:06:07 */
  tm.tm_mday = 2;
  tm.tm_mon = 2;
  tm.tm_hour = 5;
  tm.tm_min = 6;
  tm.tm_sec = 7;

  strftime(buf, sizeof(buf), "%x %X", &tm);

  dp = LOCAL_local.date_order;
  tp = LOCAL_local.time_order;

  for (p = buf;;)
  {
    c = *p++;
    if (!c)
      break;

    switch(c)
    {
      case '4':
        *dp++ = LO_YEAR;
        stradd_sep(LOCAL_local.long_date, "yyyy", " ");
        stradd_sep(LOCAL_local.medium_date, "yy", " ");
        stradd_sep(LOCAL_local.short_date, "yy", "/");
        stradd_sep(LOCAL_local.general_date, "yyyy", "/");
        break;

      case '3':
        *dp++ = LO_MONTH;
        stradd_sep(LOCAL_local.long_date, "mmmm", " ");
        stradd_sep(LOCAL_local.medium_date, "mmm", " ");
        stradd_sep(LOCAL_local.short_date, "mm", "/");
        stradd_sep(LOCAL_local.general_date, "mm", "/");
        break;

      case '2':
        *dp++ = LO_DAY;
        stradd_sep(LOCAL_local.long_date, "dddd d", " ");
        stradd_sep(LOCAL_local.medium_date, "dd", " ");
        stradd_sep(LOCAL_local.short_date, "dd", "/");
        stradd_sep(LOCAL_local.general_date, "dd", "/");
        break;

      case '5':
        *tp++ = LO_HOUR;
        stradd_sep(LOCAL_local.long_time, "hh", ":");
        stradd_sep(LOCAL_local.medium_time, "hh", ":");
        stradd_sep(LOCAL_local.short_time, "hh", ":");
        break;

      case '6':
        *tp++ = LO_MINUTE;
        stradd_sep(LOCAL_local.long_time, "nn", ":");
        stradd_sep(LOCAL_local.medium_time, "nn", ":");
        stradd_sep(LOCAL_local.short_time, "nn", ":");
        break;

      case '7':
        *tp++ = LO_SECOND;
        stradd_sep(LOCAL_local.long_time, "ss", ":");
        break;

      default:
        if (!isdigit(c))
        {
          if (tp != LOCAL_local.time_order)
          {
            if (LOCAL_local.time_sep == 0)
              LOCAL_local.time_sep = c;
          }
          else
          {
            if (LOCAL_local.date_sep == 0)
              LOCAL_local.date_sep = c;
          }
        }

    }
  }

  stradd_sep(LOCAL_local.general_date, LOCAL_local.long_time, " ");
  stradd_sep(LOCAL_local.medium_time, "AM/PM", " ");

  /* currency information */

  LOCAL_local.currency_thousand_sep = *(info->mon_thousands_sep);
  if (LOCAL_local.currency_thousand_sep == 0)
    LOCAL_local.currency_thousand_sep = ' ';

  LOCAL_local.currency_decimal_point = *(info->mon_decimal_point);
  LOCAL_local.currency_group_size = *(info->mon_grouping);
  LOCAL_local.currency_symbol = info->currency_symbol;
  LOCAL_local.intl_currency_symbol = info->int_curr_symbol;

  sprintf(LOCAL_local.general_currency, "($#,##0.%.*s)", Min(8, info->frac_digits), "########");
  sprintf(LOCAL_local.intl_currency, "($$#,##0.%.*s)", Min(8, info->int_frac_digits), "########");

  #define _add_flag(_flag) (LOCAL_local.currency_flag <<= 1, LOCAL_local.currency_flag |= (_flag))

  _add_flag(info->int_n_sep_by_space);
  _add_flag(info->int_p_sep_by_space);
  _add_flag(info->n_sep_by_space);
  _add_flag(info->p_sep_by_space);
  _add_flag(info->int_n_cs_precedes);
  _add_flag(info->int_p_cs_precedes);
  _add_flag(info->n_cs_precedes);
  _add_flag(info->p_cs_precedes);

	/* Right to left languages */



#if 0
  {
    char *str;
    long len;
    VALUE value;

    DATE_now(&value);

    LOCAL_format_date(DATE_split(&value), LF_USER, "ddd dd mmm yyyy hh:mm:ss AM/PM", 0, &str, &len);

    printf("FORMAT->%s\n", str);
  }
#endif
}


PUBLIC void LOCAL_init(void)
{
  LOCAL_set_lang(NULL);
}

PUBLIC void LOCAL_exit(void)
{
  STRING_free(&env_LANG);
  STRING_free(&env_LC_ALL);
  STRING_free(&env_LANGUAGE);
  STRING_free(&LOCAL_encoding);
  free_local_info();
}


PUBLIC const char *LOCAL_get_lang(void)
{
  char *lang;

  lang = getenv("LC_ALL");
  if (!lang)
    lang = getenv("LANG");
  if (!lang || !*lang)
    lang = "en_US";

  return lang;
}

PUBLIC void LOCAL_set_lang(const char *lang)
{
	char **l;
	int rtl;

  if (lang && *lang)
  {
    my_setenv("LANG", lang, &env_LANG);
    my_setenv("LC_ALL", lang, &env_LC_ALL);

    if (getenv("LANGUAGE"))
      my_setenv("LANGUAGE", lang, &env_LANGUAGE);
  }

  if (setlocale(LC_ALL, ""))
  {
    _translation_loaded = FALSE;
    COMPONENT_translation_must_be_reloaded();
  }
  else
    printf("WARNING: cannot switch to language '%s'. Did you install the corresponding locale?\n", LOCAL_get_lang());

  fill_local_info();

	/* If language is right to left written */

	lang = LOCAL_get_lang();
	rtl = FALSE;
  for (l = _rtl_lang; *l; l++)
  {
  	if (strncmp(*l, lang, 2) == 0)
  	{
   		rtl = TRUE;
   		break;
		}
  }

  HOOK(lang)(lang, rtl);
  LOCAL_local.rtl = rtl;
}

PUBLIC bool LOCAL_format_number(double number, int fmt_type, const char *fmt, long len_fmt, char **str, long *len_str, bool local)
{
  char c;
  long n;

  char buf[32];
  char *buf_start;

  long pos;
  long pos2;
  int thousand;
  int *thousand_ptr;

  char sign;
  bool comma;
  bool point;
  int before, before_zero;
  int after, after_zero;
  char exposant;
  char exp_sign;
  int exp_digit;
  int exp_zero;

  int number_sign;
  double number_mant;
  int number_exp;
  int number_real_exp;
  int ndigit;
  int pos_first_digit;

  bool intl_currency;

  if (local)
    local_current = &LOCAL_local;
  else
    local_current = &LOCAL_default;

  switch(fmt_type)
  {
    case LF_USER:
      break;

    case LF_STANDARD:
    case LF_GENERAL_NUMBER:
      if ((number != 0.0) && ((fabs(number) < 1E-4) || (fabs(number) >= 1E7)))
        fmt = "0.############E+#";
      else
        fmt = "0.############";
      break;

    case LF_FIXED:
      fmt = "0.00";
      break;

    case LF_PERCENT:
      fmt = "###%";
      break;

    case LF_SCIENTIFIC:
      fmt = "0.##################E+#";
      break;

    case LF_CURRENCY:
      fmt = local_current->general_currency;
      break;

    case LF_INTERNATIONAL:
      fmt = local_current->intl_currency;
      break;

    default:
      return TRUE;
  }

  if (len_fmt == 0)
    len_fmt = strlen(fmt);

  if (len_fmt >= COMMON_BUF_MAX)
    return TRUE;

  /* on recherche les formats de nombre négatif et nul */

  pos = search(fmt, len_fmt, ";", 0, FALSE);

  if (number <= 0.0)
  {
    if (pos < len_fmt)
    {
      if (number < 0.0)
      {
        if ((pos < (len_fmt - 1)) && fmt[pos + 1] != ';')
        {
          fmt = &fmt[pos + 1];
          len_fmt -= pos + 1;
        }
        else
          len_fmt = pos;
      }
      else /* nombre �al �0 */
      {
        pos2 = search(fmt, len_fmt, ";", pos + 1, FALSE);
        if (pos2 < len_fmt)
        {
          if ((pos2 < (len_fmt - 1)) && fmt[pos2 + 1] != ';')
          {
            fmt = &fmt[pos2 + 1];
            len_fmt -= pos2 + 1;
          }
          else
            len_fmt = pos;
        }
      }
    }
  }
  else if (pos < len_fmt)
    len_fmt = pos;


  /* on transcrit le format pour sprintf */

  sign = 0;
  comma = FALSE;
  before = 0;
  before_zero = 0;
  point = FALSE;
  after = 0;
  after_zero = 0;
  exposant = 0;
  exp_sign = 0;
  exp_zero = 0;
  _currency = FALSE;
  intl_currency = FALSE;

  begin();

  /* Recherche du '%' */

  pos = search(fmt, len_fmt, "%", 0, FALSE);
  if (pos < len_fmt)
    number *= 100;

  /* préfixe de formatage */

  pos = search(fmt, len_fmt, "-+#0.,($", 0, FALSE);

  if (pos >= len_fmt)
    return TRUE;

  if (pos > 0)
    add_string(fmt, pos, NULL);

  /* on détermine le signe */

  if (fmt[pos] == '-')
  {
    sign = ' ';
    pos++;
  }
  else if (fmt[pos] == '+')
  {
    sign = '+';
    pos++;
  }
  else if (fmt[pos] == '(')
  {
    sign = '(';
    pos++;
  }

  if (pos >= len_fmt)
    return TRUE;

  /* monétaire */

  if (fmt[pos] == '$')
  {
    _currency = TRUE;
    pos++;

    if (fmt[pos] == '$')
    {
      intl_currency = TRUE;
      pos++;
    }
  }

  /* Les chiffres avant la virgule */

  for(; pos < len_fmt; pos++)
  {
    c = fmt[pos];

    if (c == ',')
    {
      comma = TRUE;
      continue;
    }

    if (c == '#' || c == '0')
    {
      before++;
      if (c == '0' || before_zero > 0)
        before_zero++;
      continue;
    }

    break;
  }

  if (pos >= len_fmt)
    goto _FORMAT;

  /* La virgule */

  if (fmt[pos] != '.')
    goto _FORMAT;

  pos++;
  point = TRUE;

  if (pos >= len_fmt)
    goto _FORMAT;

  /* Les chiffres après la virgule */

  for(; pos < len_fmt; pos++)
  {
    c = fmt[pos];

    if (c == '#' || c == '0')
    {
      after++;
      if (c == '0')
        after_zero = after;
      continue;
    }

    break;
  }

  if (pos >= len_fmt)
    goto _FORMAT;

  /* L'exposant */

  if (fmt[pos] == 'e' || fmt[pos] == 'E')
  {
    exposant = fmt[pos];
    exp_sign = ' ';

    pos++;
    if (pos >= len_fmt)
      return TRUE;

    if (fmt[pos] == '-')
    {
      pos++;
    }
    else if (fmt[pos] == '+')
    {
      exp_sign = '+';
      pos++;
    }

    if (pos >= len_fmt)
      return TRUE;

    for(; pos < len_fmt; pos++)
    {
      c = fmt[pos];

      if (c == '#' || c == '0')
      {
        exp_digit++;
        if (c == '0' || exp_zero > 0)
          exp_zero++;
        continue;
      }

      break;
    }
  }

_FORMAT:

  if (before == 0 && after == 0)
    return TRUE;

  /* le signe */

  number_sign = fsgn(number);

  add_sign(sign, number_sign, FALSE);

  /* la monnaie (avant) */

  if (_currency)
  {
    int test = (number_sign < 0) + (intl_currency << 1);

    if (local_current->currency_flag & (1 << test))
    {
      add_currency(intl_currency ? local_current->intl_currency_symbol : local_current->currency_symbol);
      if (local_current->currency_flag & (1 << (test + 4)))
        put_char(' ');
    }
  }

  /* le nombre */

  number_mant = frexp10(fabs(number), &number_exp);

  /* 0.0 <= number_mant < 1.0 */

  //number_exp++; /* simplifie les choses */

  number_real_exp = number_exp;
  if (exposant)
    number_exp = number != 0.0;

  ndigit = sprintf(buf, "%.*f", MinMax(after + number_exp, 0, DBL_DIG), number_mant);

  // should return "0[.]...", or "1[.]..." if the number is rounded up.

  buf_start = buf;

  if (buf_start[0] == '1') // the number has been rounded up.
  {
		if (exposant)
			number_real_exp++;
		else
			number_exp++;
	}

  if (ndigit > 1) // so there is a point
  {
    if (buf_start[0] == '0')
    {
      buf_start += 2;
      ndigit -= 2;
    }
    else
    {
      buf_start[1] = buf_start[0];
      ndigit--;
      buf_start++;
    }

    while (ndigit > 0 && buf_start[ndigit - 1] == '0')
      ndigit--;
  }

  /* We note where the first digit will be printed */

  pos_first_digit = buffer_pos;

  /* les chiffres avant la virgule */

  thousand = Max(before_zero, number_exp);
  thousand_ptr = comma ? &thousand : NULL;

  if (number_exp > 0)
  {
    if (before_zero > number_exp)
      add_zero(before_zero - number_exp, thousand_ptr);

    add_string(buf_start, Min(number_exp, ndigit), thousand_ptr);

    if (number_exp > ndigit)
      add_zero(number_exp - ndigit, thousand_ptr);
  }
  else
  {
    if (before_zero > 0)
      add_zero(before_zero, thousand_ptr);
  }

  /* la virgule */

  if (point)
    put_char(local_current->decimal_point);

  /* les chiffres apr� la virgule */

  if ((ndigit - number_exp) > 0)
  {
    if (number_exp < 0)
    {
      n = Min(after, (- number_exp));
      if (n == after)
      {
        add_zero(after_zero, NULL);
        goto _EXPOSANT;
      }
      else
      {
        add_zero(n, NULL);
        after -= n;
        after_zero -= n;
      }
    }

    if (number_exp > 0)
    {
      buf_start += number_exp;
      ndigit -= number_exp;
    }

    n = Min(ndigit, after);
    if (n > 0)
    {
      add_string(buf_start, n, NULL);
      after -= n;
      after_zero -= n;
    }

    if (after_zero > 0)
      add_zero(after_zero, NULL);
  }
  else
    add_zero(after_zero, NULL);

_EXPOSANT:

  /* On enl�e la virgule si elle se trouve �la fin */

  buffer_pos--;
  if (look_char() != local_current->decimal_point)
    buffer_pos++;

  /* exposant */

  if (exposant != 0) // && number != 0.0)
  {
    put_char(exposant);
    n = sprintf(buf, "%+.*d", exp_zero, number_real_exp - 1);
    add_string(buf, n, NULL);
  }

  /* la monnaie (apr�) */

  if (_currency)
  {
    int test = (number_sign < 0) + (intl_currency << 1);

    if (!(local_current->currency_flag & (1 << test)))
    {
      if (local_current->currency_flag & (1 << (test + 4)))
        put_char(' ');
      add_currency(intl_currency ? local_current->intl_currency_symbol : local_current->currency_symbol);
    }
  }

  /* On ignore la parenth�e finale dans le format */

  if (sign == '(' && fmt[pos] == ')')
    pos++;

  /* Le signe apr� */

  add_sign(sign, number_sign, TRUE);

  /* print at least a zero */

  if (buffer_pos == pos_first_digit)
    put_char('0');

  /* suffixe de formatage */

  if (pos < len_fmt)
    add_string(&fmt[pos], len_fmt - pos, NULL);

  /* on retourne le r�ultat */

  end(str, len_str);
  return FALSE;
}

static void add_strftime(const char *format, struct tm *tm)
{
  int n;

  n = strftime(get_current(), get_size_left(), format, tm);
  buffer_pos += n;
}


static void add_date_token(DATE_SERIAL *date, char *token, int count)
{
  struct tm tm;
  char buf[8];
  int n;

  if (*token == 0)
    return;

  switch (*token)
  {
    case 'd':

      if (count <= 2)
      {
        n = sprintf(buf, (count == 1 ? "%d" : "%02d"), date->day);
        add_string(buf, n, NULL);
      }
      else if (count >= 3)
      {
        tm.tm_wday = date->weekday;
        add_strftime(count == 3 ? "%a" : "%A", &tm);
      }

      break;

    case 'm':

      if (count <= 2)
      {
        n = sprintf(buf, (count == 1 ? "%d" : "%02d"), date->month);
        add_string(buf, n, NULL);
      }
      else if (count >= 3)
      {
        tm.tm_mon = date->month - 1;
        add_strftime(count == 3 ? "%b" : "%B", &tm);
      }

      break;

    case 'y':

      if (count <= 2 && date->year >= 1939 && date->year <= 2038)
        n = sprintf(buf, "%02d", date->year - (date->year >= 2000 ? 2000 : 1900));
      else
        n = sprintf(buf, "%d", date->year);

      add_string(buf, n, NULL);

      break;

    case 'h':
    case 'n':
    case 's':

      n = sprintf(buf, (count == 1) ? "%d" : "%02d",
        (*token == 'h') ? date->hour : ((*token == 'n') ? date->min : date->sec));

      add_string(buf, n, NULL);

      break;

    case 'u':

      if (date->msec)
      {
        n = sprintf(buf, ".%03d", date->msec);
        while (buf[n - 1] == '0')
          n--;
        buf[n] = 0;
        add_string(buf, n, NULL);
      }

      break;

  }

  *token = 0;
}


PUBLIC boolean LOCAL_format_date(DATE_SERIAL *date, int fmt_type, const char *fmt, long len_fmt, char **str, long *len_str)
{
  char c;
  long pos;
  long pos_ampm = -1;
  struct tm date_tm;
  char real_hour = 0;

  char token;
  int token_count;

  local_current = &LOCAL_local;

  switch(fmt_type)
  {
    case LF_USER:
      break;

    case LF_STANDARD:
    case LF_GENERAL_DATE:
      if (date->year == 0)
        fmt = local_current->long_time;
      else if (date->hour == 0 && date->min == 0 && date->sec == 0)
        fmt = local_current->short_date;
      else
        fmt = local_current->general_date;
      break;

    case LF_LONG_DATE:
      fmt = local_current->long_date;
      break;

    case LF_MEDIUM_DATE:
      fmt = local_current->medium_date;
      break;

    case LF_SHORT_DATE:
      fmt = local_current->short_date;
      break;

    case LF_LONG_TIME:
      fmt = local_current->long_time;
      break;

    case LF_MEDIUM_TIME:
      fmt = local_current->medium_time;
      break;

    case LF_SHORT_TIME:
      fmt = local_current->short_time;
      break;

    default:
      return TRUE;
  }

  if (len_fmt == 0)
    len_fmt = strlen(fmt);

  if (len_fmt >= COMMON_BUF_MAX)
    return TRUE;

  /* recherche de AM/PM */

  for (pos = 0; pos < len_fmt - 4; pos++)
  {
    if (fmt[pos] == '\\')
    {
      pos++;
      continue;
    }

    if (strncasecmp(&fmt[pos], "am/pm", 5) == 0)
    {
      pos_ampm = pos;
      real_hour = date->hour;
      if (date->hour >= 12)
        date->hour -= 12;
      if (date->hour == 0)
      	date->hour = 12;
      break;
    }
  }

  /* Formatage */

  begin();

  token = 0;
  token_count = 0;

  for (pos = 0; pos < len_fmt; pos++)
  {
    c = fmt[pos];
    if (c == '\\')
    {
    	pos++;
    	if (pos >= len_fmt)
    		break;
			c = fmt[pos];
    }

    if (pos == pos_ampm)
    {
      /* passage en struct tm */

      date_tm.tm_sec = date->sec;
      date_tm.tm_min = date->min;
      date_tm.tm_hour = real_hour;
      date_tm.tm_mday = 1;
      date_tm.tm_mon = 0;
      date_tm.tm_year = 0;

      add_strftime((c == 'a' ? "%P" : "%p"), &date_tm);

      pos += 4;
      continue;
    }

    if (index("dmyhnsu", c) != NULL)
    {
      if (c != token)
      {
        add_date_token(date, &token, token_count);
        if (token == 'h' && c == 'm')
          c = 'n';

        token = c;
        token_count = 0;
      }

      token_count++;
    }
    else
    {
      add_date_token(date, &token, token_count);
      if (c == '/')
        put_char(local_current->date_sep);
      else if (c == ':')
        put_char(local_current->time_sep);
      else
        put_char(c);
    }
  }

  add_date_token(date, &token, token_count);

  /* on retourne le r�ultat */

  end(str, len_str);
  return FALSE;
}


PUBLIC void LOCAL_load_translation(ARCHIVE *arch)
{
  char *domain = NULL;
  char *lang_list;
  char *lang;
  char *src;
  char *test;
  char c;
  const char *dst = NULL;
  FILE *file;
  char *addr;
  long len;
  COMPONENT *save = COMPONENT_current;

  /* We must force GB_LoadFile() to look in our archive, because all translation
     files of one language have the same path!
  */

  if (arch)
  {
    domain = arch->domain;
    COMPONENT_current = COMPONENT_find(arch->name);
  }

  if (!domain)
    domain = "gb";

  #ifdef DEBUG_LANG
  fprintf(stderr, "LOCAL_load_translation: domain: %s\n", domain);
  #endif

  lang_list = get_languages();

  lang = strtok(lang_list, ":");

  for(;;)
  {
    if (!lang)
      break;

    if (*lang)
    {
      STRING_new_temp(&test, lang, 0);
      src = test;

      for(;;)
      {
        c = *src;
        if (c == 0 || c == '_')
          break;
        *src = tolower(c);
        src++;
      }

      dst = FILE_cat(".lang", test, NULL);
      dst = FILE_set_ext(dst, "mo");

      #ifdef DEBUG_LANG
      fprintf(stderr, "trying %s\n", dst);
      #endif

      if (FILE_exist(dst))
        break;
    }

    lang = strtok(NULL, ":");
  }

  if (!lang)
  {
    #ifdef DEBUG_LANG
    fprintf(stderr, "No translation\n");
    #endif
    goto __NOTRANS;
  }

  #ifdef DEBUG_LANG
  fprintf(stderr, "Loading %s\n", dst);
  #endif

  if (GB_LoadFile(dst, 0, &addr, &len))
  {
    #ifdef DEBUG_LANG
    fprintf(stderr, "Cannot load %s\n", dst);
    #endif
    goto __ERROR;
  }

  //STREAM_load(dst, &addr, &len);

  /*dst = FILE_cat("/tmp/gambas", NULL);  <-- Fait dans FILE_init()
  mkdir(dst, S_IRWXU);*/

  /*sprintf(COMMON_buffer, FILE_TEMP_DIR, getuid());*/

  dst = FILE_cat(FILE_make_temp(NULL, NULL), "tr", NULL);
  mkdir(dst, S_IRWXU);

  dst = FILE_cat(FILE_make_temp(NULL, NULL), "tr", lang, NULL);
  mkdir(dst, S_IRWXU);

  dst = FILE_cat(dst, "LC_MESSAGES", NULL);
  mkdir(dst, S_IRWXU);

  dst = FILE_cat(dst, domain, NULL);
  strcat((char *)dst, ".mo");
  //dst = FILE_set_ext(dst, "mo");

  unlink(dst);

  /*unload_translation();
  STRING_new(&local_trans_file, dst, 0);*/

  #ifdef DEBUG_LANG
  fprintf(stderr, "Writing to %s\n", dst);
  #endif

  file = fopen(dst, "w");
  fwrite(addr, len, 1, file);
  fclose(file);

  GB_ReleaseFile(&addr, len);
  //FREE(&addr, "LOCAL_load_translation");

__ERROR:

  #ifdef DEBUG_LANG

    fprintf(stderr, "bindtextdomain: %s\n", bindtextdomain(domain, FILE_cat(FILE_make_temp(NULL, NULL), "tr", NULL)));
    fprintf(stderr, "bind_textdomain_codeset: %s\n", bind_textdomain_codeset(domain, "UTF-8"));
    if (!arch)
      fprintf(stderr, "textdomain: %s\n", textdomain(domain));

  #else

    bindtextdomain(domain, FILE_cat(FILE_make_temp(NULL, NULL), "tr", NULL));
    #ifdef OS_SOLARIS
    fprintf(stderr, "Warning: bind_textdomain_codeset() unavailable.\n");
    #else
    bind_textdomain_codeset(domain, "UTF-8");
    #endif
    if (!arch)
      textdomain(domain); /* default domain */

  #endif

__NOTRANS:

  STRING_free(&lang_list);

  if (arch)
    arch->translation_loaded = TRUE;
  else
    _translation_loaded = TRUE;

  COMPONENT_current = save;
}


PUBLIC const char *LOCAL_gettext(const char *msgid)
{
  const char *tr;
  ARCHIVE *arch = NULL;

  /*
      If LOCAL_gettext() is called, then we are in the context of
      the archive, so the translation loaded will be the good one.
  */

  //ARCHIVE_get_current(&arch);

  if (!ARCHIVE_get_current(&arch))
  {
    if (!arch->translation_loaded)
      LOCAL_load_translation(arch);
    tr = dgettext(arch->domain, msgid);
    #ifdef DEBUG_LANG
    fprintf(stderr, "dgettext(\"%s\", \"%s\") -> \"%s\"\n", arch->domain, msgid, tr);
    #endif
  }
  else
  {
    if (!_translation_loaded)
      LOCAL_load_translation(NULL);
    tr = gettext(msgid);
    #ifdef DEBUG_LANG
    fprintf(stderr, "gettext(\"%s\") -> \"%s\"\n", msgid, tr);
    #endif
  }

  /*printf("tr: %s -> %s\n", msgid, tr);*/

  if (tr[0] == '-' && (tr[1] == 0 || (tr[1] == '\n' && tr[2] == 0)))
    return msgid;
  else
    return tr;
}


