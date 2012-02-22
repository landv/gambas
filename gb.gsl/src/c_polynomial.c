/***************************************************************************

	c_polynomial.c

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

#define __C_GSL_POLYNOMIAL_C

#include <gsl/gsl_poly.h>
#include "c_polynomial.h"
#include "c_complex.h"


#define THIS ((CPOLYNOMIAL *)_object)


/**************************************************
                 Utility Methods
**************************************************/

static CPOLYNOMIAL *create_plynomial()
{
	return (CPOLYNOMIAL *)GB.New(GB.FindClass("Polynomial"), NULL,  NULL);
}


BEGIN_METHOD_VOID(CPolynomial_new)

	// May change to take init array of floats
	THIS->alloc_size = 32;
	THIS->max = 32;
	GB.NewArray((void *)&THIS->c, sizeof(double), THIS->alloc_size);
	THIS->len = 0;

END_METHOD


BEGIN_METHOD_VOID(CPolynomial_call)
	// May be changed to take init array of floats
	CPOLYNOMIAL *c = create_polynomial();
	
	GB.ReturnObject(c);

END_METHOD


BEGIN_METHOD_VOID(CPolynomial_free)

	if(THIS->c != NULL && THIS->c != 0)
		GB.FreeArray((void *)&THIS->c);

END_METHOD


BEGIN_METHOD_VOID(CPolynomial_exit)

	if(THIS->c != NULL && THIS->c != 0)
		GB.FreeArray((void *)&THIS->c);

END_METHOD



/**************************************************
                Property Methods
**************************************************/

BEGIN_PROPERTY(CPolynomial_Length)

	if (READ_PROPERTY)
		GB.ReturnInteger((THIS->len));

END_PROPERTY


BEGIN_PROPERTY(CPolynomial_MaxCoef)

	if (READ_PROPERTY)
		GB.ReturnInteger((THIS->max));

END_PROPERTY


BEGIN_PROPERTY(CPolynomial_AllocSize)

	if (READ_PROPERTY)
		GB.ReturnInteger((THIS->alloc_size));
	else
		THIS->alloc_size = (VPROP(GB_INTEGER));
	
END_PROPERTY


BEGIN_METHOD_VOID(CPolynomial_ToString)

    // Do we have anything to print
    if(THIS->len == 0)
    {
        // P(x) = c[0] + c[1] x + c[2] x^2 + \dots + c[len-1] x^{len-1}
        GB.ReturnConstZeroString("c[0] = NULL");
		return;
    }
    else
    {
        // TODO Improve memory mangement of string
        // We have coefficiants to display.
        // P(x) = c[0] + c[1] x + c[2] x^2 + ... + c[len-1] x^{len-1}
        char buffer[(32*THIS->len)]; // A bit wasteful but it works for now....
    	char *p;
    	int i, len = 0;
        
        p = buffer;
        
        for(i = 0; i < THIS->len; i++)
        {
            if(i == 0)
            {
                len = sprintf(p , "c[%d] = %f", i, THIS->c[i]);
                p+=len;
            }
            else
            {
                len = sprintf(p , ", c[%d] = %f", i, THIS->c[i]);
                p+=len;
            }
                
        }
        
        return GB.ReturnNewZeroString(buffer);
    }

END_METHOD



/**************************************************
                  Data Methods
**************************************************/

BEGIN_METHOD(CPolynomial_Add, GB_FLOAT x;)
	
	double *elm;

	// Add a value to coeficent array
	if(THIS->max > THIS->len)
	{
		THIS->c[THIS->len] = VARG(x);
		THIS->len++;		
		return GB.ReturnInteger(THIS->len);
	}
	else
	{
		if(THIS->c != NULL && THIS->c != 0)
		{
			elm = (double *)GB.Add((void *)&THIS->c);
			THIS->max++;
			// *elm = VARG(x);
			THIS->c[THIS->len] = VARG(x);
			THIS->len++;			
		}
		return GB.ReturnInteger(THIS->len);
	}

END_METHOD



/**************************************************
             Implementation Methods
**************************************************/

BEGIN_METHOD(CPolynomial_Eval, GB_FLOAT x;)
	double r;
	double b = VARG(x);

	r = gsl_poly_eval(THIS->c, THIS->len, b);
	
	return GB.ReturnFloat(r);

END_METHOD


BEGIN_METHOD(CPolynomial_ComplexEval, GB_OBJECT z)
	GSLCOMPLEX *z = (GSLCOMPLEX *)VARG(z);
	GSLCOMPLEX *obj;

	if (GB.CheckObject(z))
		return;

	obj = GSLComplex_create();

    
    // TODO Figure out error when compiling gsl_poly_complex_eval()
    // obj = gsl_poly_complex_eval(THIS->c, THIS->len, z->number);

    GB.ReturnObject(obj);
	
END_METHOD


BEGIN_METHOD(CPolynomial_SolveQuadratic, GB_FLOAT a; GB_FLOAT b; GB_FLOAT c)
	double x0 = 0.0;
	double x1 = 0.0;
	int r = 0;
	int i = 0;
	GB_ARRAY arr;
		
	r = gsl_poly_solve_quadratic(VARG(a), VARG(b), VARG(c), &x0, &x1);

	GB.Array.New(&arr, GB_T_FLOAT, (long)r);

	for(i=0; i<r; i++)
	{
		switch(i)
		{
			case 1:
			{
				*((double *)GB.Array.Get(arr, i)) = x0;
			}break;

			case 2:
			{
				*((double *)GB.Array.Get(arr, i)) = x1;
			}break;
		}
	}

	GB.ReturnObject(arr);
		
END_METHOD


BEGIN_METHOD(CPolynomial_SolveCubic, GB_FLOAT a; GB_FLOAT b; GB_FLOAT c)
	double x0 = 0.0;
	double x1 = 0.0;
	double x2 = 0.0;
	int r = 0;
	int i = 0;
	GB_ARRAY arr;
		
	r = gsl_poly_solve_cubic(VARG(a), VARG(b), VARG(c), &x0, &x1, &x2);

	GB.Array.New(&arr, GB_T_FLOAT, (long)r);

	for(i=0; i<r; i++)
	{
		switch(i)
		{
			case 1:
			{
				*((double *)GB.Array.Get(arr, i)) = x0;
			}break;

			case 2:
			{
				*((double *)GB.Array.Get(arr, i)) = x1;
			}break;

			case 3:
			{
				*((double *)GB.Array.Get(arr, i)) = x2;
			}break;
		}
	}

	GB.ReturnObject(arr);
		
END_METHOD



/**************************************************
  Describe Class properties and methods to Gambas
**************************************************/

GB_DESC CPolynomialDesc[] =
{
	GB_DECLARE("Polynomial", sizeof(CPOLYNOMIAL)),

	// Utility Methods 
	GB_METHOD("_new", NULL, CPolynomial_new, NULL),
	GB_METHOD("_call", "Polynomial", CPolynomial_call, NULL),
	GB_METHOD("_free", NULL, CPolynomial_free, NULL),
    GB_METHOD("_exit", NULL, CPolynomial_exit, NULL),

	// Property Methods
	GB_PROPERTY_READ("Len", "i", CPolynomial_Length),
	GB_PROPERTY_READ("MaxCoef", "i", CPolynomial_MaxCoef),
	GB_PROPERTY("AllocSize", "i", CPolynomial_AllocSize),
	
	// Data Methods
	GB_METHOD("Add", "i", CPolynomial_Add, "(X)f"),
	GB_METHOD("ToString", "s", CPolynomial_ToString, NULL),

	// Implementation Methods
	GB_METHOD("Eval", "f", CPolynomial_Eval, "(X)f"),
	GB_METHOD("ComplexEval", "Complex", CPolynomial_ComplexEval, "(Z)Complex"),
	GB_METHOD("SolveQuadratic", "f[];", CPolynomial_SolveQuadratic, "[(A)f(B)f]"),
	GB_METHOD("SolveCubic", "f[];", CPolynomial_SolveCubic, "[(A)f(B)f(C)f]"),

	GB_END_DECLARE
};


