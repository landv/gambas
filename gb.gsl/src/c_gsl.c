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

#ifndef __C_GSL_C
#define __C_GSL_C

#include "gambas.h"
#include "gb_common.h"
#include "c_gsl.h"
//#include "/usr/local/include/gsl/gsl_math.h"
#include <gsl/gsl_math.h>
#include <gsl/gsl_sf.h>

#endif

/*-----------------------------------------------
 Small Integer Power Functions
-----------------------------------------------*/

BEGIN_METHOD(Gsl_IntPow2, GB_FLOAT x)

   // Return x^2 using a small int safe method
   // call gsl native function double gsl_pow_2(int x)

   GB.ReturnFloat(gsl_pow_2(VARG(x)));
	 
END_METHOD


/**************************************************
  Describe Class properties and methods to Gambas
**************************************************/

GB_DESC CGslDesc[] =
{
    GB_DECLARE("Gsl",0), GB_NOT_CREATABLE(),

    GB_STATIC_METHOD("IntPow2", "f", Gsl_IntPow2, "(X)f"),

    GB_END_DECLARE
};


