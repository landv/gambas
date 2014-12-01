/***************************************************************************

  gbx_local.c

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
char *LOCAL_encoding = NULL;

/* If system encoding is UTF-8 */
bool LOCAL_is_UTF8;

/* Default 'C' localization */
LOCAL_INFO LOCAL_default = {
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
	"mmm d yyyy",
	"mm/dd/yyyy",
	"hh:nn:ss",
	"hh:nn AM/PM",
	"hh:nn",
	"mm/dd/yyyy hh:nn:ss",
	"(#,##0.##)",
	"(#,##0.##)",
	"True", 4,
	"False", 5,
	0
	};

/* User language localization */
LOCAL_INFO LOCAL_local = { 0 };

// First day of weekday
char LOCAL_first_day_of_week = -1;

static char *_rtl_lang[] = { "ar", "fa", NULL };

static bool _translation_loaded = FALSE;

static LOCAL_INFO *local_current;

static char *env_LC_ALL = NULL;
static char *env_LANGUAGE = NULL;
static char *env_LANG = NULL;

static bool _currency;

static char *_lang = NULL;

#define add_currency_flag(_flag) (LOCAL_local.currency_flag <<= 1, LOCAL_local.currency_flag |= (!!(_flag)))
#define test_currency_flag(_negative, _space, _before, _intl) (!!(LOCAL_local.currency_flag & (1 << ((!!_negative) + ((!!_before) << 1) + ((!!_intl) << 2)))))

static void init_currency_flag(struct lconv *info)
{
#ifndef OS_OPENBSD
	add_currency_flag(info->int_n_cs_precedes); // 7
	add_currency_flag(info->int_p_cs_precedes); // 6
	//add_currency_flag(info->int_n_sep_by_space); // 5
	//add_currency_flag(info->int_p_sep_by_space); // 4
	add_currency_flag(1); // 5
	add_currency_flag(1); // 4
#else
	add_currency_flag(info->n_cs_precedes); // 7
	add_currency_flag(info->p_cs_precedes); // 6
	add_currency_flag(1); // 5
	add_currency_flag(1); // 4
#endif
	add_currency_flag(info->n_cs_precedes); // 3
	add_currency_flag(info->p_cs_precedes); // 2
	add_currency_flag(info->n_sep_by_space); // 1
	add_currency_flag(info->p_sep_by_space); // 0
}

static bool is_currency_before(bool negative, bool intl)
{
	return test_currency_flag(negative, FALSE, TRUE, intl);
}

static bool is_currency_space(bool negative, bool intl)
{
	//fprintf(stderr, "%02X & %02X\n", LOCAL_local.currency_flag, (1 << ((!!negative) + ((!!0) << 1) + ((!!intl) << 2))));
	return test_currency_flag(negative, TRUE, FALSE, intl);
}

static int my_setenv(const char *name, const char *value, char **ptr)
{
	char *str = NULL;

	str = STRING_add(str, name, 0);
	str = STRING_add_char(str, '=');
	str = STRING_add(str, value, 0);

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

static void end(char **str, int *len)
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
		{
			if (buffer_pos > 0 && (get_current()[-1] == ' '))
				put_char(' ');
			else
				put_char(_currency ? local_current->currency_thousand_sep : local_current->thousand_sep);
		}
	}

	(*before)--;
}


/*static char *strnadd(char *dst, char *src, int len, int *before)*/
static void add_string(const char *src, int len, int *before)
{
	if (len == 0)
		len = strlen(src);

	while (len > 0)
	{
		put_char(*src++);
		len--;

		if (before)
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


static void add_char(char c, int count, int *before)
{
	while (count > 0)
	{
		put_char(c);
		count--;

		add_thousand_sep(before);
	}
}

static void add_zero(int count, int *before)
{
	add_char('0', count, before);
}


static int search(const char *src, int len, const char *list, int start, bool not)
{
	int i;
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

	lang = STRING_new_temp_zero(LOCAL_get_lang());

	lang_list = STRING_add(lang_list, lang, 0);
	lang_list = STRING_add_char(lang_list, ':');

	src = index(lang, '_');
	if (src)
	{
		*src = 0;
		lang_list = STRING_add(lang_list, lang, 0);
		lang_list = STRING_add_char(lang_list, ':');
	}

	#ifdef DEBUG_LANG
	fprintf(stderr, "Languages = %s\n", lang_list);
	#endif

	return lang_list;
}


static void free_local_info(void)
{
	STRING_free(&LOCAL_local.true_str);
	STRING_free(&LOCAL_local.false_str);
	CLEAR(&LOCAL_local);
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
	const char *lang;
	char *am_pm;

	/* Localisation courante */

	free_local_info();

	/* local encoding */
	
	if (!LOCAL_is_UTF8)
		STRING_free(&LOCAL_encoding);

	codeset = nl_langinfo(CODESET);
	if (!codeset || !*codeset)
		codeset = "US-ASCII";

	LOCAL_is_UTF8 = (strcasecmp(codeset, "UTF-8") == 0);
	if (LOCAL_is_UTF8)
		LOCAL_encoding = SC_UTF8;
	else
		LOCAL_encoding = STRING_new_zero(codeset);

	#ifdef DEBUG_LANG
	fprintf(stderr, "LOCAL_encoding = %s\n", LOCAL_encoding == SC_UTF8 ? "UTF-8" : LOCAL_encoding);
	#endif

	/* Numeric information */

	info = localeconv();
	
	//fprintf(stderr, "'%s' '%s' %d %d\n", info->thousands_sep, info->mon_thousands_sep, *info->thousands_sep, *info->grouping);
	//fprintf(stderr, "'%s' '%s'\n", nl_langinfo(THOUSANDS_SEP), nl_langinfo(MON_THOUSANDS_SEP));

	LOCAL_local.decimal_point = *(info->decimal_point);
	LOCAL_local.thousand_sep = *(info->thousands_sep);
	if (LOCAL_local.thousand_sep == 0)
		LOCAL_local.thousand_sep = ' ';
	LOCAL_local.group_size = *(info->grouping);
	if (LOCAL_local.group_size == 0)
		LOCAL_local.group_size = 3;

	/*LOCAL_local.currency_symbol = STRING_conv_to_UTF8(info->currency_symbol, 0);
	STRING_ref(LOCAL_local.currency_symbol);
	LOCAL_local.intl_currency_symbol = STRING_conv_to_UTF8(info->int_curr_symbol, 0);
	STRING_ref(LOCAL_local.intl_currency_symbol);*/

	// Date/time format

	tm.tm_year = 4; /* 02/03/1904 05:06:07 */
	tm.tm_mday = 2;
	tm.tm_mon = 2;
	tm.tm_hour = 5;
	tm.tm_min = 6;
	tm.tm_sec = 7;

	// Date format
	
	strftime(buf, sizeof(buf), "%x", &tm);
	if (!isdigit(buf[0])) // The default date is not a numeric one, so we use the american format
		strcpy(buf,"03/02/1904");
	
	dp = LOCAL_local.date_order;

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
				stradd_sep(LOCAL_local.medium_date, "yyyy", " ");
				stradd_sep(LOCAL_local.short_date, "yyyy", "/");
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

			default:
				if (!isdigit(c))
				{
					if (LOCAL_local.date_sep == 0)
						LOCAL_local.date_sep = c;
				}

		}
	}

	// Time format

	strftime(buf, sizeof(buf), "%X", &tm);

	tp = LOCAL_local.time_order;

	for (p = buf;;)
	{
		c = *p++;
		if (!c)
			break;

		switch(c)
		{
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
					if (LOCAL_local.time_sep == 0)
						LOCAL_local.time_sep = c;
				}
		}
	}

	// Fix the french date separator
	
	lang = LOCAL_get_lang();
	if (strcmp(lang, "fr") == 0 || strncmp(lang, "fr_", 3) == 0)
	{
		LOCAL_local.date_sep = '/';
		//LOCAL_local.thousand_sep = '~';
	}

	stradd_sep(LOCAL_local.general_date, LOCAL_local.long_time, " ");
	am_pm = nl_langinfo(AM_STR);
	if (am_pm && *am_pm)
	{
		am_pm = nl_langinfo(PM_STR);
		if (am_pm && *am_pm)
		{
			stradd_sep(LOCAL_local.medium_time, "AM/PM", " ");
		}
	}

	// Currency format

	LOCAL_local.currency_thousand_sep = *(info->mon_thousands_sep);
	if (LOCAL_local.currency_thousand_sep == 0)
		LOCAL_local.currency_thousand_sep = ' ';
	LOCAL_local.currency_group_size = *(info->mon_grouping);
	if (LOCAL_local.currency_group_size == 0)
		LOCAL_local.currency_group_size = 3;

	LOCAL_local.currency_decimal_point = *(info->mon_decimal_point);
	LOCAL_local.currency_symbol = info->currency_symbol;
	LOCAL_local.intl_currency_symbol = info->int_curr_symbol;

	strcpy(LOCAL_local.general_currency, "($#,##0.");
	strncat(LOCAL_local.general_currency, "########", Min(8, info->frac_digits));
	strcat(LOCAL_local.general_currency, ")");
	
	strcpy(LOCAL_local.intl_currency, "($$#,##0.");
	strncat(LOCAL_local.intl_currency, "########", Min(8, info->int_frac_digits));
	strcat(LOCAL_local.intl_currency, ")");

	init_currency_flag(info);
	
	LOCAL_local.true_str = STRING_new_zero(LOCAL_gettext(LOCAL_default.true_str));
	LOCAL_local.len_true_str = STRING_length(LOCAL_local.true_str);
	LOCAL_local.false_str = STRING_new_zero(LOCAL_gettext(LOCAL_default.false_str));
	LOCAL_local.len_false_str = STRING_length(LOCAL_local.false_str);
}


void LOCAL_init(void)
{
	LOCAL_set_lang(NULL);
}

void LOCAL_exit(void)
{
	STRING_free(&env_LANG);
	STRING_free(&env_LC_ALL);
	STRING_free(&env_LANGUAGE);
	if (!LOCAL_is_UTF8)
		STRING_free(&LOCAL_encoding);
	STRING_free(&_lang);
	free_local_info();
}


const char *LOCAL_get_lang(void)
{
	char *lang;

	if (!_lang)
	{
		lang = getenv("LC_ALL");
		if (!lang)
			lang = getenv("LANG");
		if (!lang || !*lang)
			lang = "en_US";
		_lang = STRING_new_zero(lang);
	}
	
	return _lang;
}

void LOCAL_set_lang(const char *lang)
{
	char **l;
	int rtl;
	char *var;
	char *err;

	#ifdef DEBUG_LANG
	fprintf(stderr, "LOCAL_set_lang: %s\n", lang);
	#endif

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
	{
		err = strerror(errno);
		ERROR_warning("cannot switch to language '%s': %s. Did you install the corresponding locale packages?", lang ? lang : LOCAL_get_lang(), err);
		setlocale(LC_ALL, "C");
	}

	STRING_free(&_lang);
	_lang = STRING_new_zero(lang);

	DATE_init_local();
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
	
	var = getenv("GB_REVERSE");
	if (var && !(var[0] == '0' && var[1] == 0))
		rtl = !rtl;

	HOOK(lang)(lang, rtl);
	LOCAL_local.rtl = rtl;
}

bool LOCAL_format_number(double number, int fmt_type, const char *fmt, int len_fmt, char **str, int *len_str, bool local)
{
	char c;
	int n;

	char buf[32];
	char *buf_start;

	int pos;
	int pos2;
	int thousand;
	int *thousand_ptr;

	char sign;
	bool comma;
	bool point;
	int before, before_zero;
	int after, after_zero;
	char exposant;
	//char exp_sign;
	int exp_zero;

	int number_sign;
	uint64_t mantisse;
	uint64_t power;
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
			if ((number != 0.0) && ((fabs(number) < 1E-4) || (fabs(number) >= 1E10)))
				fmt = "0.##############E+#";
			else
				fmt = "0.##############";
			break;

		case LF_SHORT_NUMBER:
			if ((number != 0.0) && ((fabs(number) < 1E-4) || (fabs(number) >= 1E10)))
				fmt = "0.#######E+#";
			else
				fmt = "0.#######";
			break;

		case LF_FIXED:
			fmt = "0.00";
			break;

		case LF_PERCENT:
			fmt = "###%";
			break;

		case LF_SCIENTIFIC:
			fmt = "0.################E+#";
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
	//exp_sign = 0;
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
		//exp_sign = ' ';

		pos++;
		if (pos >= len_fmt)
			return TRUE;

		if (fmt[pos] == '-')
		{
			pos++;
		}
		else if (fmt[pos] == '+')
		{
			//exp_sign = '+';
			pos++;
		}

		if (pos >= len_fmt)
			return TRUE;

		for(; pos < len_fmt; pos++)
		{
			c = fmt[pos];

			if (c == '#' || c == '0')
			{
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

	/* currency (before) */

	if (_currency && is_currency_before(number_sign < 0, intl_currency))
	{
		add_currency(intl_currency ? local_current->intl_currency_symbol : local_current->currency_symbol);
		if (is_currency_space(number_sign < 0, intl_currency))
			put_char(' ');
	}

	/* We note where the first digit will be printed */

	pos_first_digit = buffer_pos;

	/* the number */

	if (isfinite(number))
	{
		number_mant = frexp10(fabs(number), &number_exp);
		ndigit = after;
		if (!exposant) ndigit += number_exp;
		ndigit = MinMax(ndigit, 0, MAX_FLOAT_DIGIT);
		//fprintf(stderr, "number_mant = %.24g  number_exp = %d  ndigit = %d\n", number_mant, number_exp, ndigit);
		
		power = pow10_uint64_p(ndigit + 1);
		
		mantisse = number_mant * power;
		if ((mantisse % 10) >= 5)
			mantisse += 10;
		
		//fprintf(stderr, "-> power = %" PRId64 " mantisse = %" PRId64 "\n", power, mantisse);
		
		if (mantisse >= power)
		{
			ndigit = sprintf(buf, ".%" PRId64, mantisse);
			buf[0] = buf[1];
			buf[1] = '.';
		}
		else
		{
			ndigit = sprintf(buf, "0.%" PRId64, mantisse);
		}
		
		ndigit--;
		buf[ndigit] = 0;

		/* 0.0 <= number_mant < 1.0 */

		//number_exp++; /* simplifie les choses */

		number_real_exp = number_exp;
		if (exposant)
			number_exp = number != 0.0;

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

		/* les chiffres avant la virgule */

		thousand = Max(before, Max(before_zero, number_exp));
		thousand_ptr = comma ? &thousand : NULL;

		if (number_exp > 0)
		{
			add_char(' ', before - Max(before_zero, number_exp), thousand_ptr);
			add_zero(before_zero - number_exp, thousand_ptr);

			add_string(buf_start, Min(number_exp, ndigit), thousand_ptr);

			if (number_exp > ndigit)
				add_zero(number_exp - ndigit, thousand_ptr);
		}
		else
		{
			add_char(' ', before - before_zero, thousand_ptr);
			add_zero(before_zero, thousand_ptr);
		}

		/* decimal point */

		if (point)
			put_char(local_current->decimal_point);

		/* digits after the decimal point */

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

		/* The decimal point is removed if it is located at the end */

		buffer_pos--;
		if (look_char() != local_current->decimal_point)
			buffer_pos++;

		/* exponant */

		if (exposant != 0) // && number != 0.0)
		{
			put_char(exposant);
			n = snprintf(buf, sizeof(buf), "%+.*d", exp_zero, number_real_exp - 1);
			add_string(buf, n, NULL);
		}
	}
	else // isfinite
	{
		if (isnan(number))
			add_string("NaN", 3, NULL);
		else if (isinf(number))
			add_string("Inf", 3, NULL);
	}

	/* currency (after) */

	if (_currency && !is_currency_before(number_sign < 0, intl_currency))
	{
		if (is_currency_space(number_sign < 0, intl_currency))
			put_char(' ');
		add_currency(intl_currency ? local_current->intl_currency_symbol : local_current->currency_symbol);
	}

	/* The last format brace is ignored */

	if (sign == '(' && fmt[pos] == ')')
		pos++;

	/* The sign after */

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


static void add_number(int value, int pad)
{
	static char temp[8] = { 0 };
	int i, n;
	bool minus = FALSE;
	
	if (value < 0)
	{
		value = (-value);
		minus = TRUE;
	}
	
	n = 0;
	for (i = 7; i >= 0; i--)
	{
		n++;
		if (value < 10)
		{
			temp[i] = value + '0';
			value = 0;
			if (n >= pad)
				break;
		}
		else
		{
			temp[i] = (value % 10) + '0';
			value /= 10;
		}
	}
	
	if (minus)
	{
		i--;
		temp[i] = '-';
		n++;
	}
	
	add_string(&temp[i], n, NULL);
}

static bool add_date_token(DATE_SERIAL *date, char *token, int count)
{
	struct tm tm = {0};
	char buf[8];
	int n;
	bool date_token;

	if (*token == 0)
		return FALSE;

	date_token = *token == 'd' || *token == 'm' || *token == 'y';
	
	if ((date_token && DATE_SERIAL_has_no_date(date))) // || (!date_token && DATE_SERIAL_has_no_time(date)))
	{
		*token = 0;
		return TRUE;
	}
	
	switch (*token)
	{
		case 'd':

			if (count <= 2)
			{
				add_number(date->day, (count == 1 ? 0 : 2));
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
				add_number(date->month, (count == 1 ? 0 : 2));
			}
			else if (count >= 3)
			{
				tm.tm_mon = date->month - 1;
				add_strftime(count == 3 ? "%b" : "%B", &tm);
			}

			break;

		case 'y':

			if (count <= 2 && date->year >= 1939 && date->year <= 2038)
				add_number(date->year - (date->year >= 2000 ? 2000 : 1900), 2);
			else
				add_number(date->year, 0);

			break;

		case 'h':
		case 'n':
		case 's':

			add_number((*token == 'h') ? date->hour : ((*token == 'n') ? date->min : date->sec), (count == 1 ? 0 : 2));
			break;

		case 'u':

			if (date->msec || count == 2)
			{
				if (count >= 2)
					add_number(date->msec, 3);
				else
				{
					n = snprintf(buf, sizeof(buf), "%03d", date->msec);
					while (buf[n - 1] == '0')
						n--;
					buf[n] = 0;
					add_string(buf, n, NULL);
				}
			}

			break;
			
		case 't':
			
			if (count <= 2)
			{
				time_t t = (time_t)0L;
				localtime_r(&t, &tm);
				add_strftime(count == 2 ? "%z" : "%Z", &tm);
			}
			break;
	}

	*token = 0;
	return FALSE;
}


bool LOCAL_format_date(const DATE_SERIAL *date, int fmt_type, const char *fmt, int len_fmt, char **str, int *len_str)
{
	DATE_SERIAL vdate;
	char c;
	int pos;
	int pos_ampm = -1;
	struct tm date_tm;

	char token;
	int token_count;

	local_current = &LOCAL_local;
	vdate = *date;

	switch(fmt_type)
	{
		case LF_USER:
			break;

		case LF_STANDARD:
		case LF_GENERAL_DATE:
			if (date->year == 0)
			{
				if (date->hour == 0 && date->min == 0 && date->sec == 0)
				{
					*str = NULL;
					*len_str = 0;
					return FALSE;
				}
				fmt = local_current->long_time;
			}
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
			if (vdate.hour > 12)
				vdate.hour -= 12;
			else if (vdate.hour == 0)
				vdate.hour = 12;
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
			add_date_token(&vdate, &token, token_count);
			put_char(fmt[pos]);
			continue;
		}

		if (pos == pos_ampm)
		{
			add_date_token(&vdate, &token, token_count);
			
			/* passage en struct tm */

			date_tm.tm_sec = date->sec;
			date_tm.tm_min = date->min;
			date_tm.tm_hour = date->hour;
			date_tm.tm_mday = 1;
			date_tm.tm_mon = 0;
			date_tm.tm_year = 0;

			add_strftime((c == 'a' ? "%P" : "%p"), &date_tm);

			pos += 4;
			continue;
		}

		if (c == 'd' || c == 'm' || c == 'y' || c == 'h' || c == 'n' || c == 's' || c == 'u' || c == 't')
		{
			if (c != token)
			{
				add_date_token(&vdate, &token, token_count);

				token = c;
				token_count = 0;
			}

			token_count++;
		}
		else
		{
			if (!add_date_token(&vdate, &token, token_count))
			{
				if (c == '/')
					put_char(local_current->date_sep);
				else if (c == ':')
					put_char(local_current->time_sep);
				else
					put_char(c);
			}
		}
	}

	add_date_token(&vdate, &token, token_count);

	/* on retourne le r�ultat */

	end(str, len_str);
	return FALSE;
}


void LOCAL_load_translation(ARCHIVE *arch)
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
	int len;
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
			test = STRING_new_temp_zero(lang);
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

	// These temporary files have predictable names because they
	// are *.mo files read by the gettext system.

	dst = FILE_cat(FILE_make_temp(NULL, NULL), "tr", NULL);
	mkdir(dst, S_IRWXU);
		
	dst = FILE_cat(FILE_make_temp(NULL, NULL), "tr", lang, NULL);
	mkdir(dst, S_IRWXU);

	dst = FILE_cat(dst, "LC_MESSAGES", NULL);
	mkdir(dst, S_IRWXU);

	dst = FILE_cat(dst, domain, NULL);
	strcat((char *)dst, ".mo");

	unlink(dst);

	#ifdef DEBUG_LANG
	fprintf(stderr, "Writing to %s\n", dst);
	#endif

	// No need to test previous system calls as the failure will be detected now

	file = fopen(dst, "w");
	if (file)
	{
		fwrite(addr, len, 1, file);
		fclose(file);
	}

	GB_ReleaseFile(addr, len);

__ERROR:

	// If the *.mo was not copied, then the following functions will failed
	
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


const char *LOCAL_gettext(const char *msgid)
{
	const char *tr = msgid;
	ARCHIVE *arch = NULL;

	/*
			If LOCAL_gettext() is called, then we are in the context of
			the archive, so the translation loaded will be the good one.
	*/

	if (!msgid)
		return "";

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
	
	if (tr == msgid)
	{
		if (!_translation_loaded)
			LOCAL_load_translation(NULL);
		tr = gettext(msgid);
		#ifdef DEBUG_LANG
		fprintf(stderr, "gettext(\"%s\") -> \"%s\"\n", msgid, tr);
		#endif
	}

	/*printf("tr: %s -> %s\n", msgid, tr);*/

	if (!tr || tr[0] == 0 || (tr[0] == '-' && (tr[1] == 0 || (tr[1] == '\n' && tr[2] == 0))))
		tr = msgid;

	return tr;
}

int LOCAL_get_first_day_of_week()
{
	const char *lang;
	
	if (LOCAL_first_day_of_week >= 0)
		return LOCAL_first_day_of_week;
	
	lang = LOCAL_get_lang();
	
	if (strcmp(lang, "en") == 0 || strncmp(lang, "en_", 3) == 0 || strcmp(lang, "C") == 0)
		return 0;
	else
		return 1;
}

void LOCAL_set_first_day_of_week(char day)
{
	if (day >= -1 && day <= 6)
		LOCAL_first_day_of_week = day;
}
