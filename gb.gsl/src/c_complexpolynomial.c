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
/*=========================================================================
		=== NOTE THIS IS SIMPLE LEARNING CODE === 
            AND NOTHING USEFUL EXISTS HERE YET
==========================================================================*/

#define __C_GSL_COMPLEXPOLYNOMIAL_C

#include "c_complexpolynomial.h"


#define THIS ((CCOMPLEXPOLYNOMIAL *)_object)

static CCOMPLEXPOLYNOMIAL *create_plynomial()
{
	return (CCOMPLEXPOLYNOMIAL *)GB.New(GB.FindClass("ComplexPolynomial"), NULL,  NULL);
}



GB_DESC CComplexPolynomialDesc[] =
{
	GB_DECLARE("ComplexPoly", sizeof(CCOMPLEXPOLYNOMIAL)),
	
	GB_END_DECLARE
};



