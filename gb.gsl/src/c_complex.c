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

static GSLCOMPLEX *create_complex()
{
	return (GSLCOMPLEX *)GB.New(GB.FindClass("Complex"), NULL,  NULL);
}

BEGIN_METHOD(GslComplex_new, GB_FLOAT real; GB_FLOAT imag)

	THIS->number.dat[0] = VARGOPT(real, 0.0);
	THIS->number.dat[1] = VARGOPT(imag, 0.0);

END_METHOD

BEGIN_METHOD(GslComplex_call, GB_FLOAT real; GB_FLOAT imag)

	GSLCOMPLEX *c = create_complex();
	
	c->number.dat[0] = VARG(real);
	c->number.dat[1] = VARG(imag);
	GB.ReturnObject(c);

END_METHOD

BEGIN_METHOD_VOID(GslComplex_Copy)

	GSLCOMPLEX *c = create_complex();
	
	c->number = THIS->number;
	GB.ReturnObject(c);

END_METHOD

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

BEGIN_METHOD(GslComplex_Add, GB_OBJECT x)
	
	GSLCOMPLEX *x = VARG(x);
	GSLCOMPLEX *obj;

	if (GB.CheckObject(x))
		return;
	
	// Create new object
	obj = create_complex();

	// Add two complex numbers
	obj->number = gsl_complex_add(THIS->number, x->number);

	GB.ReturnObject(obj);

END_METHOD


BEGIN_METHOD_VOID(GslComplex_ToString)

	char buffer[64];
	char *p;
	char *str;
	int len;
	double real, imag;
	
	real = THIS->number.dat[0];
	imag = THIS->number.dat[1];
	
	if (real == 0.0 && imag == 0.0)
	{
		GB.ReturnConstZeroString("0");
		return;
	}
	
	p = buffer;
	
	if (real != 0.0)
	{
		GB.NumberToString(FALSE, real, NULL, &str, &len);
		strncpy(p, str, len);
		p += len;
	}
	
	if (imag != 0.0)
	{
		if (imag < 0.0)
		{
			*p++ = '-';
			imag = (-imag);
		}
		else if (p != buffer)
			*p++ = '+';
		
		if (imag != 1.0)
		{
			GB.NumberToString(FALSE, imag, NULL, &str, &len);
			strncpy(p, str, len);
			p += len;
		}
		*p++ = 'i';
	}
	
	GB.ReturnNewString(buffer, p - buffer);
	
END_METHOD


/**************************************************
	Describe Class properties and methods to Gambas
**************************************************/
GB_DESC CGslComplexDesc[] =
{
	GB_DECLARE("Complex", sizeof(GSLCOMPLEX)),
	
	GB_METHOD("_new", NULL, GslComplex_new, "[(Real)f(Imag)f]"),
	GB_STATIC_METHOD("_call", "Complex", GslComplex_call, "[(Real)f(Imag)f]"),
	GB_METHOD("Copy", "Complex", GslComplex_Copy, NULL),

	GB_PROPERTY("Real", "f", GslComplex_Real),
	GB_PROPERTY("Imag", "f", GslComplex_Imagined),
	GB_PROPERTY("X", "f", GslComplex_Real),
	GB_PROPERTY("Y", "f", GslComplex_Imagined),

	// Operations on gsl_complex
	GB_METHOD("Add", "Complex", GslComplex_Add, "(X)Complex"),
	GB_METHOD("ToString", "s", GslComplex_ToString, NULL),
	
	GB_END_DECLARE
};


