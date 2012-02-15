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
/*=========================================================================
		=== NOTE THIS IS SIMPLE LEARNING CODE === 
            AND NOTHING USEFUL EXISTS HERE YET
==========================================================================*/

#define __C_GSL_POLYNOMIAL_C

#include "c_polynomial.h"


#define THIS ((GSLPOLY *)_object)


BEGIN_METHOD(GSLPOLY_Eval, GB_OBJECT array; GB_FLOAT x;)
	// Function: 
	// double gsl_poly_eval (const double c[], const int len, const double x)	
	// This function evaluates a polynomial with real 
	// coefficients for the real variable x.
	GB_ARRAY arr = (GB_ARRAY) VARG(array);	
	double r;
	int i;
	int count = GB.Array.Count(arr);
	double a[count];
	double b = VARG(x);

	
	for (i=0; i<count; i++)
	{
		a[i] = *((double *)GB.Array.Get(arr,i));
		//printf("a[%i] = %f; ", i, *((double *)GB.Array.Get(arr,i)));
	}
	
	//printf("\n");
	

	r = gsl_poly_eval(a, count, b);
	
	GB.ReturnFloat(r);

END_METHOD


/**************************************************
  Describe Class properties and methods to Gambas
**************************************************/
GB_DESC CGslPolynomialDesc[] =
{
	GB_DECLARE("Polynomial", sizeof(GSLPOLY)),

	GB_METHOD("Eval", "f", GSLPOLY_Eval, "(a)Float[];(x)f"),

	GB_END_DECLARE
};

