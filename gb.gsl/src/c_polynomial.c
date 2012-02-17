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
	
END_METHOD


BEGIN_METHOD_VOID(CPolynomial_call)
	// May be changed to take init array of floats
	CPOLYNOMIAL *c = create_polynomial();
	
	GB.ReturnObject(c);

END_METHOD


BEGIN_METHOD_VOID(CPolynomial_free)

	if(THIS->c != NULL && THIS->c != 0)
		GB.Free(POINTER(THIS->c));

END_METHOD



/**************************************************
                Property Methods
**************************************************/

BEGIN_PROPERTY(CPolynomial_Length)

	if (READ_PROPERTY)
		GB.ReturnInteger((THIS->len));

END_PROPERTY

BEGIN_METHOD_VOID(CPolynomial_ToString)
	// Currently using this method to print debugging info
	// Will emplement real functionality later...
	int i = 0;

	printf("Initial Count: %d \n", THIS->len);
	printf("Address: %x = %f \n", (int)&THIS->c, THIS->c);
	
	for(i=0; i< THIS->len; i++)
	{	
		printf("i = %d, Address: %x = %f \n", i, (int)&THIS->c[i], THIS->c[i]);	
	}
		
 
END_METHOD



/**************************************************
                  Data Methods
**************************************************/

BEGIN_METHOD(CPolynomial_Add, GB_FLOAT x;)

	// Initializes the coeficent array	
	if(THIS->len == 0)
	{
		GB.Alloc(POINTER(&THIS->c), sizeof(double));
		THIS->c[THIS->len] = VARG(x);
		THIS->len++;		
		return GB.ReturnInteger(THIS->len);
	}

	// Add a value to coeficent array
	if(THIS->len > 0)
	{
		GB.Realloc(POINTER(&THIS->c), (sizeof(double)*(THIS->len+1)));
		THIS->c[THIS->len] = VARG(x);
		THIS->len++;		
		return GB.ReturnInteger(THIS->len);
	}
	else
	{
		return GB.ReturnInteger(THIS->len);
	}

END_METHOD



/**************************************************
             Implementation Methods
**************************************************/

BEGIN_METHOD(CPolynomial_Eval, GB_FLOAT x;)
	// Function: 
	// double gsl_poly_eval (const double c[], const int len, const double x)	
	// This function evaluates a polynomial with real 
	// coefficients for the real variable x.
	double r;
	int i;
	double b = VARG(x);

	r = gsl_poly_eval(THIS->c, THIS->len, b);
	
	return GB.ReturnFloat(r);

END_METHOD



/**************************************************
  Describe Class properties and methods to Gambas
**************************************************/

GB_DESC CPolynomialDesc[] =
{
	GB_DECLARE("Polynomial", sizeof(CPOLYNOMIAL)),

	// Util;ity Methods 
	GB_METHOD("_new", NULL, CPolynomial_new, NULL),
	GB_METHOD("_call", "Polynomial", CPolynomial_call, NULL),
	GB_METHOD("_free", NULL, CPolynomial_free, NULL),

	// Property Methods
	GB_PROPERTY_READ("Len", "i", CPolynomial_Length),
	GB_METHOD("ToString", "s", CPolynomial_ToString, NULL),
	
	// Data Methods
	GB_METHOD("Add", "i", CPolynomial_Add, "(x)f"),

	// Implementation Methods
	GB_METHOD("Eval", "f", CPolynomial_Eval, "(x)f"),

	GB_END_DECLARE
};


