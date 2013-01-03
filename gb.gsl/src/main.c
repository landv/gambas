/***************************************************************************

  main.c

  gb.gsl component

  (c) 2012 Randall Morgan <rmorgan62@gmail.com>

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

#include "c_gsl.h"
#include "c_complex.h"
#include "c_vector.h"
#include "c_matrix.h"
#include "c_polynomial.h"

GB_INTERFACE GB EXPORT;

GB_DESC *GB_CLASSES[] EXPORT =
{
  CGslDesc, /* The Elementary math functions */
  ComplexDesc,
	VectorDesc,
  PolynomialDesc,
	MatrixDesc,
  /* Other classes go here as completed */
  NULL // Must have a null entry for the end of the structure
};

GB_CLASS CLASS_Array;
GB_CLASS CLASS_Complex;
GB_CLASS CLASS_Matrix;
GB_CLASS CLASS_Vector;
GB_CLASS CLASS_Polynomial;

static void error_handler(const char *reason, const char *file, int line, int gsl_errno)
{
	//fprintf(stderr, "gb.gsl: error: %s: %s\n", gsl_strerror(gsl_errno), reason);
	GB.Error("&1: &2", gsl_strerror(gsl_errno), reason);
}

int EXPORT GB_INIT(void)
{
	CLASS_Array = GB.FindClass("Array");
	CLASS_Complex = GB.FindClass("Complex");
	CLASS_Vector = GB.FindClass("Vector");
	CLASS_Matrix = GB.FindClass("Matrix");
	CLASS_Polynomial = GB.FindClass("Polynomial");
	
	gsl_set_error_handler(error_handler);
	
	return 0;
}

void EXPORT GB_EXIT()
{
	
}

int EXPORT GB_INFO(const char *key, void **value)
{
	if (!strcasecmp(key, "PUSH_COMPLEX"))
	{
		*value = (void *)COMPLEX_push_complex;
		return TRUE;
	}
	else
		return FALSE;
}
