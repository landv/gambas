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
#include <gsl/gsl_math.h>
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

    GB.ReturnBoolean((c==1?-1:0));

END_METHOD


BEGIN_METHOD(GSL_ISINF, GB_FLOAT x;)
    // This function returns +1 if x is positive infinity,
    // -1 if x is negative infinity and 0 otherwise.
    // Call GSL Function int gsl_isinf(const double x)
    int c;

     c= gsl_isinf(VARG(x));

     GB.ReturnBoolean((c==1?-1:0));

END_METHOD


BEGIN_METHOD(GSL_ISFINITE, GB_FLOAT x;)
    // This function returns 1 if x is a real number,
    // and -1 if it is infinite or not-a-number.
    // Call GSL Function int gsl_isfinite(const double x)
    int c;

    c = gsl_finite(VARG(x));

   GB.ReturnBoolean((c == 1?-1:0));

END_METHOD


BEGIN_METHOD(GSL_ISPOSITIVE, GB_FLOAT x;)
   // This method returns the sign of x.
   // It is defined as ((x) >= 0 ? 1 : -1).
   // Note that with this definition the sign of
   // zero is positive (regardless of its ieee sign bit).
   GB.ReturnBoolean((VARG(x) >= 0 ? -1 : 0));

END_METHOD


BEGIN_METHOD(GSL_SIGNF, GB_FLOAT x;)
   // This method returns the sign of x.
   // It is defined as ((x) >= 0 ? 1 : -1).
   // Note that with this definition the sign of
   // zero is positive (regardless of its ieee sign bit).
   GB.ReturnInteger((VARG(x) >= 0 ? 1 : 0));

END_METHOD


BEGIN_METHOD(GSL_ISODD, GB_INTEGER x;)
   // This method evaluates to -1 if n is odd and 0
   // if n is even. The argument n must be of integer type.
   GB.ReturnBoolean((VARG(x)%2 ? -1:0));

END_METHOD


BEGIN_METHOD(GSL_ISEVEN, GB_INTEGER x;)
   // This method evaluates to -1 if n is even and 0
   // if n is odd. The argument n must be of integer type.
   GB.ReturnBoolean((VARG(x)%2 ? 0:-1));

END_METHOD


BEGIN_METHOD(GSL_MAXFLOAT, GB_FLOAT x; GB_FLOAT y;)

   GB.ReturnFloat((VARG(x) > VARG(y) ? VARG(x): VARG(y)));

END_METHOD


BEGIN_METHOD(GSL_MINFLOAT, GB_FLOAT x; GB_FLOAT y;)

   GB.ReturnFloat((VARG(x) < VARG(y) ? VARG(x): VARG(y)));

END_METHOD


BEGIN_METHOD(GSL_MAXINT, GB_FLOAT x; GB_FLOAT y;)

    GB.ReturnInteger((VARG(x) > VARG(y) ? VARG(x) : VARG(y)));

END_METHOD


BEGIN_METHOD(GSL_MININT, GB_FLOAT x; GB_FLOAT y;)

    GB.ReturnInteger((VARG(x) < VARG(y) ? VARG(x) : VARG(y)));

END_METHOD

BEGIN_METHOD(GSL_FCMPB, GB_FLOAT x; GB_FLOAT y; GB_FLOAT e;)
    // Function: int gsl_fcmp (double x, double y, double epsilon)
    // This function determines whether x and y are approximately
    // equal to a relative accuracy epsilon.
    // The relative accuracy is measured using an interval of size 2 \delta,
    // where \delta = 2^k \epsilon and k is the maximum base-2 exponent of x
    // and y as computed by the function frexp.
    // If x and y lie within this interval, they are considered approximately
    // equal and the function returns 0. Otherwise if x < y, the function returns
    // -1, or if x > y, the function returns +1.
    // Note that x and y are compared to relative accuracy, so this function is
    // not suitable for testing whether a value is approximately zero.

    int c;

    c = gsl_fcmp (VARG(x), VARG(y), VARG(e));

    GB.ReturnBoolean((c == 0 ? -1: 0));

END_METHOD


BEGIN_METHOD(GSL_FCMPI, GB_FLOAT x; GB_FLOAT y; GB_FLOAT e;)
    // Function: int gsl_fcmp (double x, double y, double epsilon)
    // This function determines whether x and y are approximately
    // equal to a relative accuracy epsilon.
    // The relative accuracy is measured using an interval of size 2 \delta,
    // where \delta = 2^k \epsilon and k is the maximum base-2 exponent of x
    // and y as computed by the function frexp.
    // If x and y lie within this interval, they are considered approximately
    // equal and the function returns 0. Otherwise if x < y, the function returns
    // -1, or if x > y, the function returns +1.
    // Note that x and y are compared to relative accuracy, so this function is
    // not suitable for testing whether a value is approximately zero.

    GB.ReturnInteger(gsl_fcmp (VARG(x), VARG(y), VARG(e)));

END_METHOD



/*-----------------------------------------------
 Elementary Functions
-----------------------------------------------*/

BEGIN_METHOD(GSL_LOG1P, GB_FLOAT x;)
    // This function computes the value of \log(1+x)
    // in a way that is accurate for small x.
    // Call GSL Function int gsl_isnan(const double x)
    GB.ReturnFloat(gsl_log1p (VARG(x)));

END_METHOD


BEGIN_METHOD(GSL_EXPM1, GB_FLOAT x;)
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

/*
BEGIN_METHOD(GSL_HYPOT3, GB_FLOAT x; GB_FLOAT y; GB_FLOAT z;)
    // This function computes the value of \sqrt{x^2 + y^2 + z^2}
    // in a way that avoids overflow.
    // Call GSL function double gsl_hypot3 (const double x, const double y, const double z)
    GB.ReturnFloat(gsl_hypot3(VARG(x), VARG(y), ARG(z)));

END_METHOD
*/

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


BEGIN_METHOD(GSL_Frexp, GB_FLOAT x; GB_POINTER e)
	// Function: double gsl_frexp (double x, int * e)
	// This function splits the number x into its normalized 
	// fraction f and exponent e, such that x = f * 2^e and
	// 0.5 <= f < 1. The function returns f and stores the 
	// exponent in e. If x is zero, both f and e are set to
	// zero. This function provides an alternative to the
	// standard math function frexp(x, e).
	int *b;
	double r;

	b = VARG(e);

	r = gsl_frexp(VARG(x), (int *)VARG(e));

	GB.ReturnFloat(r);
	
END_METHOD



/*-----------------------------------------------
 Small Integer Power Functions
-----------------------------------------------*/
BEGIN_METHOD(GSL_INTPOW, GB_FLOAT x; GB_INTEGER i;)
    // A common complaint about the standard C library is its lack
    // of a function for calculating (small) integer powers. GSL
    // provides some simple functions to fill this gap. For reasons
    // of efficiency, these functions do not check for overflow or
    // underflow conditions.
    GB.ReturnFloat(gsl_pow_int(VARG(x), VARG(i)));

END_METHOD


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
    GB_STATIC_METHOD("IsNan", "b", GSL_ISNAN, "(X)f"),
    GB_STATIC_METHOD("IsInf", "b", GSL_ISINF, "(X)f"),
    GB_STATIC_METHOD("IsFinite", "b", GSL_ISFINITE, "(X)f"),
    GB_STATIC_METHOD("IsPos", "b", GSL_ISPOSITIVE, "(X)f"),
    GB_STATIC_METHOD("Sign", "i", GSL_SIGNF, "(X)i"),
    GB_STATIC_METHOD("IsOdd", "b", GSL_ISODD, "(X)i"),
    GB_STATIC_METHOD("IsEven", "b", GSL_ISEVEN, "(X)i"),
    GB_STATIC_METHOD("MaxFloat", "f", GSL_MAXFLOAT, "[(X)f(Y)f"),
    GB_STATIC_METHOD("MinFLoat", "f", GSL_MINFLOAT, "[(X)f(Y)f]"),
    GB_STATIC_METHOD("MaxInt", "i", GSL_MAXINT, "[(X)i(Y)i]"),
    GB_STATIC_METHOD("MinInt", "i", GSL_MININT, "[(X)i(Y)i"),
    GB_STATIC_METHOD("Fcmpb", "b", GSL_FCMPB, "[(X)f(Y)f(E)f]"),
    GB_STATIC_METHOD("Fcmpi", "i", GSL_FCMPI, "[(X)f(Y)f(E)f]"),

    // Elementary Functions
    GB_STATIC_METHOD("Log1p", "f", GSL_LOG1P, "(X)f"),
    GB_STATIC_METHOD("Expm1", "f", GSL_EXPM1, "(X)f"),
    GB_STATIC_METHOD("Hypot", "f", GSL_HYPOT, "[(X)f(Y)f]"),
    //GB_STATIC_METHOD("Hypot3", "f", GSL_HYPOT3, "[(x)f(y)f(z)f]"),
    GB_STATIC_METHOD("Acosh", "f", GSL_ACOSH, "(X)f"),
    GB_STATIC_METHOD("Asinh", "f", GSL_ASINH, "(X)f"),
    GB_STATIC_METHOD("Atanh", "f", GSL_ATANH, "(X)f"),
    GB_STATIC_METHOD("Ldexp", "f", GSL_LDEXP, "[(X)f(E)i]"),
    GB_STATIC_METHOD("Frexp", "f", GSL_FREXP, "[(X)f(E)p]"),

    // Return x^y using a small int safe method
    GB_STATIC_METHOD("IntPow", "f", GSL_INTPOW, "[(X)f(I)i]"),
    GB_STATIC_METHOD("IntPow2", "f", GSL_INTPOW2, "(X)f"),
    GB_STATIC_METHOD("IntPow3", "f", GSL_INTPOW3, "(X)f"),
    GB_STATIC_METHOD("IntPow4", "f", GSL_INTPOW4, "(X)f"),
    GB_STATIC_METHOD("IntPow5", "f", GSL_INTPOW5, "(X)f"),
    GB_STATIC_METHOD("IntPow6", "f", GSL_INTPOW6, "(X)f"),
    GB_STATIC_METHOD("IntPow7", "f", GSL_INTPOW7, "(X)f"),
    GB_STATIC_METHOD("IntPow8", "f", GSL_INTPOW8, "(X)f"),
    GB_STATIC_METHOD("IntPow9", "f", GSL_INTPOW9, "(X)f"),

    // Class Constants
    // Appears that GSL Macros cannot be used in GB_CONSTANT
    // So we must define them here ourselves.
    GB_CONSTANT("M_E", "f", "2.71828182845904523536028747135"),             /* e */
    GB_CONSTANT("M_LOGE", "f", "1.44269504088896340735992468100"),	        /* LOG_E */
    GB_CONSTANT("M_LOG10E", "f", "0.43429448190325182765112891892"), 	    /* log_10 (e) */
    GB_CONSTANT("M_SQRT2", "f", "1.41421356237309504880168872421"),  	    /* sqrt(2) */
    GB_CONSTANT("M_SQRT1_2", "f", "0.70710678118654752440084436210"),       /* sqrt(1/2) */
    GB_CONSTANT("M_SQRT3", "f", "1.73205080756887729352744634151"),         /* sqrt(3) */
    GB_CONSTANT("M_PI", "f", "3.14159265358979323846264338328"),            /* pi */
    GB_CONSTANT("M_PI_2", "f", "1.57079632679489661923132169164"),          /* pi/2 */
    GB_CONSTANT("M_PI_4", "f", "0.78539816339744830961566084582"),          /* pi/4 */
    GB_CONSTANT("M_SQRTPI", "f", "1.77245385090551602729816748334"),        /* sqrt(pi) */
    GB_CONSTANT("M_2_SQRTPI", "f", "1.12837916709551257389615890312"),      /* 2/sqrt(pi) */
    GB_CONSTANT("M_1_PI", "f", "0.31830988618379067153776752675"),          /* 1/pi */
    GB_CONSTANT("M_2_PI", "f", "0.63661977236758134307553505349"),          /* 2/pi */
    GB_CONSTANT("M_LN10", "f", "2.30258509299404568401799145468"),          /* ln(10) */
    GB_CONSTANT("M_LN2", "f", "0.69314718055994530941723212146"),           /* ln(2) */
    GB_CONSTANT("M_LNPI", "f", "1.14472988584940017414342735135"),          /* ln(pi) */
    GB_CONSTANT("M_EULER", "f", "0.57721566490153286060651209008"),         /* Euler constant */

    GB_END_DECLARE
};


