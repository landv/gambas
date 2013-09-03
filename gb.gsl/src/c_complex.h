/***************************************************************************

	c_complex.h

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

#ifndef __C_COMPLEX_H
#define __C_COMPLEX_H

#include "main.h"

#ifndef _C_COMPLEX_C
extern GB_DESC ComplexDesc[];
extern gsl_complex COMPLEX_zero;
extern gsl_complex COMPLEX_one;
//extern GB_DESC ComplexArrayDesc[];
#endif

typedef
	struct
	{
		GB_BASE ob;
		gsl_complex number;
	}
	CCOMPLEX;

typedef
	union
	{
		gsl_complex z;
		double x;
	}
	COMPLEX_VALUE;
	
enum
{
	CGV_ERR,
	CGV_FLOAT,
	CGV_COMPLEX
};

CCOMPLEX *COMPLEX_create(gsl_complex number);
CCOMPLEX *COMPLEX_push_complex(double value);
char *COMPLEX_to_string(gsl_complex number, bool local);

#define COMPLEX_get(_c) ((_c) ? (_c)->number : COMPLEX_zero)

int COMPLEX_get_value(GB_VALUE *value, COMPLEX_VALUE *v);
int COMPLEX_comp(gsl_complex a, gsl_complex b);

#endif /* __C_COMPLEX_H */
