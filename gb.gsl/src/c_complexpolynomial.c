/***************************************************************************
	
	c_complexpolynomial.c

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

#define __C_GSL_COMPLEXPOLYNOMIAL_C

#include "c_complexpolynomial.h"


#define THIS ((CCOMPLEXPOLYNOMIAL *)_object)



/**************************************************
                 Utility Methods
**************************************************/

CCOMPLEXPOLYNOMIAL *CComplexPolynomial_create()
{
	CCOMPLEXPOLYNOMIAL *p;
	p = (CCOMPLEXPOLYNOMIAL *)GB.New(GB.FindClass("ComplexPolynomial"), NULL,  NULL);
	return p;
}


BEGIN_METHOD_VOID(CComplexPolynomial_new)

	// May change to take init array of floats
	THIS->alloc_size = 32;
	THIS->max = 32;
	GB.NewArray((void *)&THIS->c, sizeof(gsl_complex), THIS->alloc_size);
	THIS->len = 0;

END_METHOD


BEGIN_METHOD_VOID(CComplexPolynomial_call)
	// May be changed to take init array of floats
	CCOMPLEXPOLYNOMIAL *c = CComplexPolynomial_create();

	GB.ReturnObject(c);

END_METHOD


BEGIN_METHOD_VOID(CComplexPolynomial_free)

	if(THIS->c != NULL && THIS->c != 0)
		GB.FreeArray((void *)&THIS->c);

END_METHOD


BEGIN_METHOD_VOID(CComplexPolynomial_exit)

	if(THIS->c != NULL)
		GB.FreeArray((GB_FLOAT *)&THIS->c);

END_METHOD



/**************************************************
                Property Methods
**************************************************/

BEGIN_PROPERTY(CComplexPolynomial_Length)

	if (READ_PROPERTY)
		GB.ReturnInteger((THIS->len));

END_PROPERTY


BEGIN_PROPERTY(CComplexPolynomial_MaxCoef)

	if (READ_PROPERTY)
		GB.ReturnInteger((THIS->max));

END_PROPERTY


BEGIN_PROPERTY(CComplexPolynomial_AllocSize)

	if (READ_PROPERTY)
		GB.ReturnInteger((THIS->alloc_size));
	else
		THIS->alloc_size = (VPROP(GB_INTEGER));

END_PROPERTY


BEGIN_METHOD_VOID(CComplexPolynomial_ToString)

	char buffer[256];
	char *p = buffer;
	char *str;
	int len;
	double real, imag;
	int i = 0;

	for(i=0; i < THIS->len; i++)
	{
		real = THIS->c[i].dat[0];
		imag = THIS->c[i].dat[1];

		if (real == 0.0 && imag == 0.0)
		{
			GB.ReturnConstZeroString("0");
			return;
		}

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
				*p++ = ' ';
				*p++ = '-';
				*p++ = ' ';
				imag = (-imag);
			}
			else if (p != buffer)
				*p++ = ' ';
				*p++ = '+';
				*p++ = ' ';


			if (imag != 1.0)
			{
				GB.NumberToString(FALSE, imag, NULL, &str, &len);
				strncpy(p, str, len);
				p += len;
			}
			*p++ = 'i';

		}

		*p++ = '\n';
	}

	GB.ReturnNewString(buffer, p - buffer);

END_METHOD



/**************************************************
                  Data Methods
**************************************************/

BEGIN_METHOD(CComplexPolynomial_AddFloats, GB_FLOAT real; GB_FLOAT imag)

	//double *elm;

	// Add a value to coeficent array
	if(THIS->max > THIS->len)
	{
		THIS->c[THIS->len].dat[0] = (double)VARG(real);
		THIS->c[THIS->len].dat[1] = (double)VARG(imag);
		THIS->len++;
		return GB.ReturnInteger(THIS->len);
	}
	else
	{
		if(THIS->c != NULL && THIS->c != 0)
		{
			GB.Add((void *)&THIS->c);
			THIS->max++;
			// *elm = VARG(x);
			THIS->c[THIS->len].dat[0] = (double)VARG(real);
			THIS->c[THIS->len].dat[1] = (double)VARG(imag);
			THIS->len++;
		}
		return GB.ReturnInteger(THIS->len);
	}

END_METHOD



/**************************************************
             Implementation Methods
**************************************************/

BEGIN_METHOD_VOID(CComplexPolynomial_ComplexSolve)
	int i;
	gsl_complex z[THIS->len];
	GB_ARRAY cArray;
	GSLCOMPLEX *cx;


	gsl_poly_complex_workspace * w = gsl_poly_complex_workspace_alloc(THIS->len);

	gsl_poly_complex_solve((double *)THIS->c, THIS->len, w, (gsl_complex_packed_ptr)z);

	gsl_poly_complex_workspace_free(w);

	GB.Array.New(&cArray, GB.FindClass("Complex"), (long)(THIS->len-1));

	for(i = 0; i < THIS->len-1; i++)
	{
		printf ("z%d = %+.18f %+.18f\n", i, GSL_REAL(z[i]), GSL_IMAG(z[i]));
		cx = GSLComplex_create();
		if(cx)
		{
			GSLCOMPLEX *elt;
			cx->number.dat[0] = GSL_REAL(z[i]);
			cx->number.dat[1] = GSL_IMAG(z[i]);
			printf ("cx[%d] = %+.18f %+.18f\n", i, cx->number.dat[0], cx->number.dat[1]);
			elt = (GSLCOMPLEX *)GB.Array.Get(&cArray, i);
			elt = cx;
			GB.Ref(cx);
		}
		else
		{
			GB.Error("Could not create result array", NULL, NULL);
			return;
		}

    }

	return GB.ReturnObject(cArray);

END_METHOD

/*
  GB_ARRAY array;

       GB.Array.New(&array, GB.FindClass("Complex"), size);

       // Create a new Complex

       *GB.Array.Get(TheArray, index) = TheComplexPointer;

       GB.Ref(TheComplexPointer);
 */


// Function: gsl_complex gsl_complex_poly_complex_eval (const gsl_complex c[], const int len, const gsl_complex z)
// This function evaluates a polynomial with complex coefficients for the complex variable z.



GB_DESC CComplexPolynomialDesc[] =
{
	GB_DECLARE("ComplexPolynomial", sizeof(CCOMPLEXPOLYNOMIAL)),
	
	// Utility Methods
	GB_METHOD("_new", NULL, CComplexPolynomial_new, NULL),
	GB_METHOD("_call", "Polynomial", CComplexPolynomial_call, NULL),
	GB_METHOD("_free", NULL, CComplexPolynomial_free, NULL),
   GB_METHOD("_exit", NULL, CComplexPolynomial_exit, NULL),

	// Property Methods
	GB_PROPERTY_READ("Len", "i", CComplexPolynomial_Length),
	GB_PROPERTY_READ("MaxCoef", "i", CComplexPolynomial_MaxCoef),
	GB_PROPERTY("AllocSize", "i", CComplexPolynomial_AllocSize),

	// Data Methods
	//GB_METHOD("Add", "Complex", CComplexPolynomial_Add, "(Z)Complex"),
	GB_METHOD("AddFloat", "i", CComplexPolynomial_AddFloats, "[(X)f(Y)f]"),
	GB_METHOD("ToString", "s", CComplexPolynomial_ToString, NULL),

	GB_METHOD("ComplexSolve", "Complex[];", CComplexPolynomial_ComplexSolve, NULL),

	GB_END_DECLARE
};



