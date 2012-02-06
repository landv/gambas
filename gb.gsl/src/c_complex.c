/***************************************************************************

  gsl.c

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

#ifndef __C_GSL_COMPLEX_C
#define __C_GSL_COMPLEX_C

#include "c_complex.h"
#include "gambas.h"
#include "gb_common.h"
#include "c_gsl.h"
#include <gsl/gsl_math.h>
#include <gsl/gsl_complex.h>
#include <gsl/gsl_complex_math.h>
#include <gsl/gsl_sf.h>

#endif


BEGIN_PROPERTY(GslComplex_Real)

	if (READ_PROPERTY)
		GB.ReturnFloat(THIS->number.real);
	else
		THIS->number.real = (VPROP(GB_FLOAT));

END_PROPERTY


BEGIN_PROPERTY(GslComplex_Imagined)

	if (READ_PROPERTY)
		GB.ReturnFloat(THIS->number.imagined);
	else
		THIS->number.imagined = (VPROP(GB_FLOAT));

END_PROPERTY

/**************************************************
  Elemntary math functions for complex numbers
**************************************************/

BEGIN_METHOD(GslComplex_X, GB_OBJECT x;)
    GSLCOMPLEX *p;
    p = VARG(x); // Get GSLCOMPLEX pointer from GB_OBJECT:value member
    // Now using our GSLCOMPLEX pointer we can access the GSLCOMPLEX structure members
    GB.ReturnFloat(p->number.real);
END_METHOD


BEGIN_METHOD(GslComplex_Y, GB_OBJECT y;)
    GSLCOMPLEX *p;
    p = VARG(y);
    GB.ReturnFloat(p->number.imagined);
END_METHOD


BEGIN_METHOD(GslComplex_Add, GB_OBJECT x;)
    void **t;
    GSLCOMPLEX *p;
    GSLCOMPLEX *obj;
    gsl_complex a, b;
    gsl_complex r;

    p = (GSLCOMPLEX *) VARG(x);

    // Save the values of the current object into a.
    GSL_SET_COMPLEX(&a, THIS->number.real, THIS->number.imagined);

    // We convert p (GSLCOMPLEXNUM) to type gsl_complex because
    // at this point we cannot garauntee that GSLCOMPLEXNUM
    // will not change. It may get an error (epsilon) value
    // added in the future to make handling error diviation
    // easier.
    GSL_SET_COMPLEX(&b, p->number.real, p->number.imagined);

    // Create new object
    obj = (GSLCOMPLEX *) GB.New(GB.FindClass("Complex"), NULL,  NULL);

    // Add two complex numbers
    r = gsl_complex_add(a, b);

    // Access the complex number
    //obj->number.real = r.dat[0];
    //obj->number.imagined = r.dat[1];

    return GB.ReturnObject(obj);

END_METHOD

/**************************************************
  Describe Class properties and methods to Gambas
**************************************************/
GB_DESC CGslComplexDesc[] =
{
    GB_DECLARE("Complex", sizeof(GSLCOMPLEX)),

    GB_PROPERTY("REAL", "f", GslComplex_Real),
    GB_PROPERTY("IMAG", "f", GslComplex_Imagined),

    GB_STATIC_METHOD("X", "f", GslComplex_X, "(x)o"),
    GB_STATIC_METHOD("Y", "f", GslComplex_Y, "(y)o"),

    GB_STATIC_METHOD("Add", "o", GslComplex_Add, "(x)o"),

    GB_END_DECLARE
};


