/***************************************************************************

  main.c

  gb.gmp component

  (c) 2000-2013 Benoît Minisini <gambas@users.sourceforge.net>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 1, or (at your option)
  any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
  MA 02110-1301, USA.

***************************************************************************/

#define __MAIN_C

#include "main.h"

#include "c_bigint.h"
#include "c_rational.h"

GB_INTERFACE GB EXPORT;

GB_DESC *GB_CLASSES[] EXPORT =
{
	BigIntDesc,
	RationalDesc,
  NULL // Must have a null entry for the end of the structure
};

GB_CLASS CLASS_BigInt;
GB_CLASS CLASS_Rational;

/*static void error_handler(const char *reason, const char *file, int line, int gsl_errno)
{
	//fprintf(stderr, "gb.gsl: error: %s: %s\n", gsl_strerror(gsl_errno), reason);
	GB.Error("&1: &2", gsl_strerror(gsl_errno), reason);
}*/

static void *my_malloc(size_t n)
{
	void *p;
	GB.Alloc(&p, n);
	return p;
}

static void my_free(void *p, size_t n)
{
	GB.Free(&p);
}

static void *my_realloc(void *p, size_t old, size_t n)
{
	GB.Realloc(&p, n);
	return p;
}

int EXPORT GB_INIT(void)
{
	CLASS_BigInt = GB.FindClass("BigInt");
	CLASS_Rational = GB.FindClass("Rational");

	mp_set_memory_functions(my_malloc, my_realloc, my_free);

	//gsl_set_error_handler(error_handler);

	return 0;
}

void EXPORT GB_EXIT()
{

}
