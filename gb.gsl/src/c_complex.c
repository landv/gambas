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
#include <stdio.h>
#endif


BEGIN_PROPERTY(GslComplex_Real)

	if (READ_PROPERTY)
		GB.ReturnFloat(THIS->number.dat[0]);
	else
		THIS->number.dat[0] = (VPROP(GB_FLOAT));

END_PROPERTY


BEGIN_PROPERTY(GslComplex_Imagined)

	if (READ_PROPERTY)
		GB.ReturnFloat(THIS->number.dat[1]);
	else
		THIS->number.dat[1] = (VPROP(GB_FLOAT));

END_PROPERTY

/**************************************************
  Elemntary math functions for complex numbers
**************************************************/

BEGIN_METHOD(GslComplex_Add, GB_OBJECT x;)
    GSLCOMPLEX *p = VARG(x);
    GSLCOMPLEX *obj;

    // Create new object
    obj = (GSLCOMPLEX *) GB.New(GB.FindClass("Complex"), NULL,  NULL);

    // Add two complex numbers
    obj->number = gsl_complex_add(THIS->number, p->number);

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

    // Operations on gsl_complex
    GB_STATIC_METHOD("Add", "Complex", GslComplex_Add, "(x)Complex"),
    
    GB_END_DECLARE
};


