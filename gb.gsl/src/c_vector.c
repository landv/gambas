/***************************************************************************

	c_vector.c

	gb.gsl component

	(c) 2012 Benoît Minisini <gambas@users.sourceforge.net>

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

#define __C_VECTOR_C

#include "c_vector.h"

#define THIS ((GSLVECTOR *)_object)

GB_DESC VectorDesc[] =
{
	GB_DECLARE("Vector", sizeof(GSLVECTOR)),
	
	// Utility Methods 
	/*GB_METHOD("_new", NULL, Complex_new, "[(Real)f(Imag)f]"),
	GB_STATIC_METHOD("_call", "Complex", Complex_call, "[(Real)f(Imag)f]"),
	GB_METHOD("Copy", "Complex", Complex_Copy, NULL),
	GB_STATIC_METHOD("Polar", "Complex", Complex_Polar, "[(Real)f(Imag)f]"),
	
	GB_INTERFACE("_operators", &_operators),
	GB_INTERFACE("_convert", &_convert),*/
	
	GB_END_DECLARE
};
