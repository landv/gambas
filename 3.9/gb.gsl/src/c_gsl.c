/***************************************************************************

	c_gsl.c

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

#define __C_GSL_C

#include "c_gsl.h"
#include <gsl/gsl_math.h>
#include <gsl/gsl_sf.h>


/*--------------------------------
	Number testing functions
--------------------------------*/

BEGIN_METHOD(GSL_ISNAN, GB_FLOAT x;)
		// This function returns 1 if x is not-a-number.
		// Call GSL Function int gsl_isnan(const double x)
		int c;

		c = gsl_isnan(VARG(x));

		GB.ReturnBoolean(c);

END_METHOD


BEGIN_METHOD(GSL_ISINF, GB_FLOAT x;)
		// This function returns +1 if x is positive infinity,
		// -1 if x is negative infinity and 0 otherwise.
		// Call GSL Function int gsl_isinf(const double x)
		int c;

		c= gsl_isinf(VARG(x));

		GB.ReturnInteger(c);

END_METHOD


BEGIN_METHOD(GSL_ISFINITE, GB_FLOAT x;)
		// This function returns 1 if x is a real number,
		// and -1 if it is infinite or not-a-number.
		// Call GSL Function int gsl_isfinite(const double x)
		int c;

		c = gsl_finite(VARG(x));

		GB.ReturnBoolean(c);

END_METHOD


BEGIN_METHOD(GSL_FCMP, GB_FLOAT x; GB_FLOAT y; GB_FLOAT e;)
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


BEGIN_METHOD(GSL_FREXP, GB_FLOAT x;)
	// Function: double gsl_frexp (double x, int * e)
	// This function splits the number x into its normalized 
	// fraction f and exponent e, such that x = f * 2^e and
	// 0.5 <= f < 1. The function returns f and stores the 
	// exponent in e. If x is zero, both f and e are set to
	// zero. This function provides an alternative to the
	// standard math function frexp(x, e).
	int b;
	double r;
	GB_ARRAY arr;

	b = 0.0;
	r = gsl_frexp(VARG(x), &b);
	//printf("r: %f \te: %i\n", r, b);

	GB.Array.New(&arr, GB_T_FLOAT, 2);
	*((double *)GB.Array.Get(arr, 0)) = r;
	*((double *)GB.Array.Get(arr, 1)) = b;
	GB.ReturnObject(arr);

END_METHOD

// BM they are useless, as they are already implemented that way in the interpreter.

#if 0
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
#endif

/**************************************************
	Describe Class properties and methods to Gambas
**************************************************/
GB_DESC CGslDesc[] =
{
		GB_DECLARE("GSL",0), GB_NOT_CREATABLE(),

		// Number testing functions
		GB_STATIC_METHOD("IsNan", "b", GSL_ISNAN, "(X)f"),
		GB_STATIC_METHOD("IsInf", "i", GSL_ISINF, "(X)f"),
		GB_STATIC_METHOD("IsFinite", "b", GSL_ISFINITE, "(X)f"),
		GB_STATIC_METHOD("Fcmp", "i", GSL_FCMP, "(X)f(Y)f(E)f"),

		// Elementary Functions
		GB_STATIC_METHOD("Log1p", "f", GSL_LOG1P, "(X)f"),
		GB_STATIC_METHOD("Expm1", "f", GSL_EXPM1, "(X)f"),
		GB_STATIC_METHOD("Hypot", "f", GSL_HYPOT, "(X)f(Y)f"),
		GB_STATIC_METHOD("Hypot3", "f", GSL_HYPOT3, "(X)f(Y)f(Z)f"),
		GB_STATIC_METHOD("Acosh", "f", GSL_ACOSH, "(X)f"),
		GB_STATIC_METHOD("Asinh", "f", GSL_ASINH, "(X)f"),
		GB_STATIC_METHOD("Atanh", "f", GSL_ATANH, "(X)f"),
		GB_STATIC_METHOD("Ldexp", "f", GSL_LDEXP, "(X)f(E)i"),
		GB_STATIC_METHOD("Frexp", "Float[]", GSL_FREXP, "(X)f"),

		// Return x^y using a small int safe method
		/*GB_STATIC_METHOD("IntPow", "f", GSL_INTPOW, "(X)f(I)i"),
		GB_STATIC_METHOD("IntPow2", "f", GSL_INTPOW2, "(X)f"),
		GB_STATIC_METHOD("IntPow3", "f", GSL_INTPOW3, "(X)f"),
		GB_STATIC_METHOD("IntPow4", "f", GSL_INTPOW4, "(X)f"),
		GB_STATIC_METHOD("IntPow5", "f", GSL_INTPOW5, "(X)f"),
		GB_STATIC_METHOD("IntPow6", "f", GSL_INTPOW6, "(X)f"),
		GB_STATIC_METHOD("IntPow7", "f", GSL_INTPOW7, "(X)f"),
		GB_STATIC_METHOD("IntPow8", "f", GSL_INTPOW8, "(X)f"),
		GB_STATIC_METHOD("IntPow9", "f", GSL_INTPOW9, "(X)f"),*/

		// Class Constants
		GB_FLOAT_CONSTANT("E", M_E),
		GB_FLOAT_CONSTANT("LOG2E", M_LOG2E),
		GB_FLOAT_CONSTANT("LOG10E", M_LOG10E),
		GB_FLOAT_CONSTANT("SQRT2", M_SQRT2),
		GB_FLOAT_CONSTANT("SQRT1_2", M_SQRT1_2),
		GB_FLOAT_CONSTANT("SQRT3", M_SQRT3),
		GB_FLOAT_CONSTANT("PI", M_PI),
		GB_FLOAT_CONSTANT("PI_2", M_PI_2),
		GB_FLOAT_CONSTANT("PI_4", M_PI_4),
		GB_FLOAT_CONSTANT("SQRTPI", M_SQRTPI),
		GB_FLOAT_CONSTANT("INV2_SQRTPI", M_2_SQRTPI),
		GB_FLOAT_CONSTANT("INV_PI", M_1_PI),
		GB_FLOAT_CONSTANT("INV2_PI", M_2_PI),
		GB_FLOAT_CONSTANT("LN10", M_LN10),
		GB_FLOAT_CONSTANT("LN2", M_LN2),
		GB_FLOAT_CONSTANT("LNPI", M_LNPI),
		GB_FLOAT_CONSTANT("EULER", M_EULER),

		GB_END_DECLARE
};


