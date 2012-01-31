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
#include "/usr/local/include/gsl/gsl_math.h"
#include <gsl/gsl_sf.h>

#endif

/*--------------------------------
  Number testing functions
--------------------------------*/

BEGIN_METHOD(GSL_ISNAN, GB_FLOAT x;)
    // This function returns 1 if x is not-a-number.
    // Call GSL Function int gsl_isnan(const double x)
    int c;
    
    c = gsl_isnan(VARG(x));

    GB.ReturnBoolean((c==1?1:-1));

END_METHOD


BEGIN_METHOD(GSL_ISINF, GB_FLOAT x;)
    // This function returns +1 if x is positive infinity,
    // -1 if x is negative infinity and 0 otherwise.
    // Call GSL Function int gsl_isinf(const double x)
    int c;

     c= gsl_isinf(VARG(x));

     GB.ReturnBoolean((c==1?1:-1));

END_METHOD


BEGIN_METHOD(GSL_ISFINITE, GB_FLOAT x;)
    // This function returns 1 if x is a real number,
    // and -1 if it is infinite or not-a-number.
    // Call GSL Function int gsl_isfinite(const double x)
    int c;

    c = gsl_isfinite(VARG(x));

   GB.ReturnBoolean((c == 1?1:-1));

END_METHOD


/*-----------------------------------------------
 Elementary Functions
-----------------------------------------------*/

BEGIN_METHOD(GSL_LOGLP, GB_FLOAT x;)
    // This function computes the value of \log(1+x)
    // in a way that is accurate for small x.
    // Call GSL Function int gsl_isnan(const double x)
    GB.ReturnFloat(gsl_log1p (VARG(x)));

END_METHOD


BEGIN_METHOD(GSL_EXPML, GB_FLOAT x;)
    // This function computes the value of \exp(x)-1
    // in a way that is accurate for small x.
    GB.ReturnFloat(gsl_expm1 (VARG(x)));

END_METHOD


BEGIN_METHOD(GSL_HYPOT, GB_FLOAT x; GB_FLOAT y;)
    // This function computes the value of
    // \sqrt{x^2 + y^2} in a way that avoids overflow.
    // Call GSL function double gsl_hypot (const double x, const double y)
    GB.ReturnFloat(gsl_hypot(VARG(x), VARG(y)));

END_METHOD


BEGIN_METHOD(GSL_HYPOT3, GB_FLOAT x; GB_FLOAT y; GB_FLOAT z;)
    // This function computes the value of \sqrt{x^2 + y^2 + z^2}
    // in a way that avoids overflow.
    // Call GSL function double gsl_hypot3 (const double x, const double y, const double z)
    GB.ReturnFloat(gsl_hypot3(VARG(x), VARG(y), VARG(z)));

END_METHOD


BEGIN_METHOD(GSL_ACOSH, GB_FLOAT x;)
    // This function computes the value of \arccosh(x).
    // It provides an alternative to the standard math function acosh(x).
    GB.ReturnFloat(gsl_acosh(VARG(x)));

END_METHOD

BEGIN_METHOD(GSL_ASINH, GB_FLOAT x;)
    // Function: double gsl_asinh (const double x)
    // This function computes the value of arcsinh(x). 
    // It provides an alternative to the standard math function asinh(x).
    GB.ReturnFloat(gsl_asinh(VARG(x))); 
END_METHOD

BEGIN_METHOD(GSL_ATANH, GB_FLOAT x;)
    // Function: double gsl_atanh (const double x)
    // This function computes the value of \arctanh(x). 
    // It provides an alternative to the standard math function atanh(x). 
    GB.ReturnFloat(gsl_atanh(VARG(x)));
END_METHOD

BEGIN_METHOD(GSL_LDEXP, GB_FLOAT x; GB_INTEGER e;)
    // Function: double gsl_ldexp (double x, int e)
    // This function computes the value of x * 2^e. 
    // It provides an alternative to the standard math function ldexp(x,e).
    GB.ReturnFloat(gsl_ldexp(VARG(x), VARG(e))); 
END_METHOD

/*
BEGIN_METHOD(GSL_FREXP, GB_FLOAT x; GB_INTEGER e;)
    // Function: double gsl_frexp (double x, int * e)
    // This function splits the number x into its normalized fraction f 
    // and exponent e, such that x = f * 2^e and 0.5 <= f < 1. The function 
    // returns f and stores the exponent in e. If x is zero, both f and e 
    // are set to zero. This function provides an alternative to the
    // standard math function frexp(x, e).
    GB.ReturnFloat(gsl_frexp(VARG(x), VARG(e))); 
END_METHOD
*/




/*-----------------------------------------------
 Small Integer Power Functions
-----------------------------------------------*/
BEGIN_METHOD(GSL_INTPOW2, GB_FLOAT x;)
   // Return x^2 using a small int safe method
   // call gsl native function double gsl_pow_2(double x)
   GB.ReturnFloat(gsl_pow_2(VARG(x)));
END_METHOD

BEGIN_METHOD(GSL_INTPOW3, GB_FLOAT x;)
   // Return x^3 using a small int safe method
   // call gsl native function double gsl_pow_3(double x)
   GB.ReturnFloat(gsl_pow_3(VARG(x)));
END_METHOD

BEGIN_METHOD(GSL_INTPOW4, GB_FLOAT x;)
   // Return x^4 using a small int safe method
   // call gsl native function double gsl_pow_3(double x)
   GB.ReturnFloat(gsl_pow_4(VARG(x)));
END_METHOD

BEGIN_METHOD(GSL_INTPOW5, GB_FLOAT x;)
   // Return x^5 using a small int safe method
   // call gsl native function double gsl_pow_3(double x)
   GB.ReturnFloat(gsl_pow_5(VARG(x)));
END_METHOD

BEGIN_METHOD(GSL_INTPOW6, GB_FLOAT x;)
   // Return x^6 using a small int safe method
   // call gsl native function double gsl_pow_3(double x)
   GB.ReturnFloat(gsl_pow_6(VARG(x)));
END_METHOD

BEGIN_METHOD(GSL_INTPOW7, GB_FLOAT x;)
   // Return x^7 using a small int safe method
   // call gsl native function double gsl_pow_3(double x)
   GB.ReturnFloat(gsl_pow_7(VARG(x)));
END_METHOD

BEGIN_METHOD(GSL_INTPOW8, GB_FLOAT x;)
   // Return x^8 using a small int safe method
   // call gsl native function double gsl_pow_3(double x)
   GB.ReturnFloat(gsl_pow_8(VARG(x)));
END_METHOD

BEGIN_METHOD(GSL_INTPOW9, GB_FLOAT x;)
   // Return x^9 using a small int safe method
   // call gsl native function double gsl_pow_3(double x)
   GB.ReturnFloat(gsl_pow_9(VARG(x)));
END_METHOD


/**************************************************
  Describe Class properties and methods to Gambas
**************************************************/
GB_DESC CGslDesc[] =
{
    GB_DECLARE("GSL",0), GB_NOT_CREATABLE(),

    // Number testing functions
    GB_STATIC_METHOD("IsNAN", "f", GSL_ISNAN, "(x)f"),
    GB_STATIC_METHOD("IsINF", "f", GSL_ISINF, "(x)f"),
    GB_STATIC_METHOD("IsFinite", "f", GSL_ISFINITE, "(x)f"),

    // Elementary Functions
    GB_STATIC_METHOD("LogLP", "f", GSL_LOGLP, "(x)f"),
    GB_STATIC_METHOD("Expml", "f", GSL_EXPML, "(x)f"),
    GB_STATIC_METHOD("Hypot", "f", GSL_HYPOT, "[(x)f(y)f]"),
    GB_STATIC_METHOD("Hypot3", "f", GSL_HYPOT3, "[(x)f(y)f(z)f]"),
    GB_STATIC_METHOD("Acosh", "f", GSL_ACOSH, "(x)f"),
    GB_STATIC_METHOD("Asinh", "f", GSL_ASINH, "(x)f"),
    GB_STATIC_METHOD("Atanh", "f", GSL_ATANH, "(x)f"),
    GB_STATIC_METHOD("Ldexp", "f", GSL_LDEXP, "[(x)f(e)i]"),
    //GB_STATIC_METHOD("Frexp", "f", GSL_FREXP, "[(x)f(e)i]"),
    
    // Return x^y using a small int safe method
    GB_STATIC_METHOD("IntPow2", "f", GSL_INTPOW2, "(x)f"),
    GB_STATIC_METHOD("IntPow3", "f", GSL_INTPOW3, "(x)f"),    
    GB_STATIC_METHOD("IntPow4", "f", GSL_INTPOW4, "(x)f"),
    GB_STATIC_METHOD("IntPow5", "f", GSL_INTPOW5, "(x)f"),
    GB_STATIC_METHOD("IntPow6", "f", GSL_INTPOW6, "(x)f"),
    GB_STATIC_METHOD("IntPow7", "f", GSL_INTPOW7, "(x)f"),
    GB_STATIC_METHOD("IntPow8", "f", GSL_INTPOW8, "(x)f"),
    GB_STATIC_METHOD("IntPow9", "f", GSL_INTPOW9, "(x)f"),

    GB_END_DECLARE
};


