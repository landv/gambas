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

#ifndef __C_GSL_COMPLEX_C
#define __C_GSL_COMPLEX_C

#include "c_complex.h"
#include "gambas.h"
#include "gb_common.h"
#include "c_gsl.h"
#include <gsl/gsl_math.h>
#include <gsl/gsl_complex.h>
#include <gsl/gsl_complex_math.h>
#include <gsl/gsl_sf.h>

#endif


BEGIN_PROPERTY(GslComplex_Real)

	if (READ_PROPERTY)
		GB.ReturnFloat(THIS->number.real);
	else
		THIS->number.real = (VPROP(GB_FLOAT));

END_PROPERTY


BEGIN_PROPERTY(GslComplex_Imagined)

	if (READ_PROPERTY)
		GB.ReturnFloat(THIS->number.imagined);
	else
		THIS->number.imagined = (VPROP(GB_FLOAT));

END_PROPERTY

/**************************************************
  Elemntary math functions for complex numbers
**************************************************/

BEGIN_METHOD(GslComplex_X, GB_OBJECT x;)
    GSLCOMPLEX *p;
    p = VARG(x); // Get GSLCOMPLEX pointer from GB_OBJECT:value member
    // Now using our GSLCOMPLEX pointer we can access the GSLCOMPLEX structure members
    GB.ReturnFloat(p->number.real);
END_METHOD


BEGIN_METHOD(GslComplex_Y, GB_OBJECT y;)
    GSLCOMPLEX *p;
    p = VARG(y);
    GB.ReturnFloat(p->number.imagined);
END_METHOD


BEGIN_METHOD(GslComplex_Add, GB_OBJECT x;)
    GSLCOMPLEX *p;
    GSLCOMPLEX *obj;
    gsl_complex a, b;
    gsl_complex r;

    p = (GSLCOMPLEX *) VARG(x);

    // Save the values of the current object into a.
    GSL_SET_COMPLEX(&a, THIS->number.real, THIS->number.imagined);

    // We convert p (GSLCOMPLEXNUM) to type gsl_complex because
    // at this point we cannot garauntee that GSLCOMPLEXNUM
    // will not change. It may get an error (epsilon) value
    // added in the future to make handling error diviation
    // easier.
    GSL_SET_COMPLEX(&b, p->number.real, p->number.imagined);

    // Create new object
    obj = (GSLCOMPLEX *) GB.New(GB.FindClass("Complex"), NULL,  NULL);

    // Add two complex numbers
    //r = gsl_complex_add(a, b);

    // Access the complex number
    obj->number.real = 1.245;
    obj->number.imagined = 6.789;

    return GB.ReturnObject(obj);

END_METHOD

BEGIN_METHOD(GslComplex_Sub, GB_OBJECT x;)
    GSLCOMPLEX *p;
    GSLCOMPLEX *obj;
    gsl_complex a, b;
    gsl_complex r;

    p = (GSLCOMPLEX *) VARG(x);

    // Save the values of the current object into a.
    GSL_SET_COMPLEX(&a, THIS->number.real, THIS->number.imagined);

    // We convert p (GSLCOMPLEXNUM) to type gsl_complex because
    // at this point we cannot garauntee that GSLCOMPLEXNUM
    // will not change. It may get an error (epsilon) value
    // added in the future to make handling error diviation
    // easier.
    GSL_SET_COMPLEX(&b, p->number.real, p->number.imagined);

    // Create new object
    obj = (GSLCOMPLEX *) GB.New(GB.FindClass("Complex"), NULL,  NULL);

    // Add two complex numbers
    //r = gsl_complex_sub(a, b);

    // Access the complex number
    obj->number.real = 5.4321;
    obj->number.imagined = 9.876;

    return GB.ReturnObject(obj);

END_METHOD


BEGIN_METHOD(GslComplex_Mul, GB_OBJECT x;)
    // Function: gsl_complex gsl_complex_mul (gsl_complex a, gsl_complex b)
    // This function returns the product of the complex numbers a and b, z=ab.
    GSLCOMPLEX *p;
    GSLCOMPLEX *obj;
    gsl_complex a, b;
    gsl_complex r;

    p = (GSLCOMPLEX *) VARG(x);

    // Save the values of the current object into a.
    GSL_SET_COMPLEX(&a, THIS->number.real, THIS->number.imagined);

    // Save the passed in obect's values to gsl_complex structure
    GSL_SET_COMPLEX(&b, p->number.real, p->number.imagined);

    // Create new object for return
    obj = (GSLCOMPLEX *) GB.New(GB.FindClass("Complex"), NULL,  NULL);

    // Now run the libgsl function
    //r = gls_complex_mul(&a, &b);

    //obj->number.real = r.dat[0];
    //obj->number.imagined = r.dat[1];

    GB.ReturnObject(obj);

END_METHOD


BEGIN_METHOD(GslComplex_Div, GB_OBJECT x;)
    // Function: gsl_complex gsl_complex_div (gsl_complex a, gsl_complex b)
    // This function returns the quotient of the complex numbers a and b, z=a/b.
    GSLCOMPLEX *p;
    GSLCOMPLEX *obj;
    gsl_complex a, b;
    gsl_complex r;

    p = (GSLCOMPLEX *) VARG(x);

    // Save the values of the current object into a.
    GSL_SET_COMPLEX(&a, THIS->number.real, THIS->number.imagined);

    // Save the passed in obect's values to gsl_complex structure
    GSL_SET_COMPLEX(&b, p->number.real, p->number.imagined);

    // Create new object for return
    obj = (GSLCOMPLEX *) GB.New(GB.FindClass("Complex"), NULL,  NULL);

    // Now run the libgsl function
    //r = gls_complex_div(&a, &b);

    //obj->number.real = r.dat[0];
    //obj->number.imagined = r.dat[1];

    GB.ReturnObject(obj);

END_METHOD


/*********************************************
            Operations On Real
*********************************************/
BEGIN_METHOD(GslComplex_Add_Real, GB_FLOAT x;)
    // Function: gsl_complex gsl_complex_add_real (gsl_complex a, double x)
    // This function returns the sum of the complex number a and the real number x, z=a+x.
    GSLCOMPLEX *obj;
    gsl_complex a;
    gsl_complex r;

    // Save the values of the current object into a.
    GSL_SET_COMPLEX(&a, THIS->number.real, THIS->number.imagined);

    // Create new object for return
    obj = (GSLCOMPLEX *) GB.New(GB.FindClass("Complex"), NULL,  NULL);

    // Now run the libgsl function
    //r = gls_complex_add_real(&a, VARG(x));

    //obj->number.real = r.dat[0];
    //obj->number.imagined = r.dat[1];

    GB.ReturnObject(obj);

END_METHOD


BEGIN_METHOD(GslComplex_Sub_Real, GB_FLOAT x;)
    // Function: gsl_complex gsl_complex_sub_real (gsl_complex a, double x)
    // This function returns the difference of the complex number a and the real number x, z=a-x.
    GSLCOMPLEX *obj;
    gsl_complex a;
    gsl_complex r;

    // Save the values of the current object into a.
    GSL_SET_COMPLEX(&a, THIS->number.real, THIS->number.imagined);

    // Create new object for return
    obj = (GSLCOMPLEX *) GB.New(GB.FindClass("Complex"), NULL,  NULL);

    // Now run the libgsl function
    //r = gls_complex_sub_real(&a, VARG(x));

    //obj->number.real = r.dat[0];
    //obj->number.imagined = r.dat[1];

    GB.ReturnObject(obj);

END_METHOD


BEGIN_METHOD(GslComplex_Mul_Real, GB_FLOAT x;)
    // Function: gsl_complex gsl_complex_mul_real (gsl_complex a, double x)
    // This function returns the product of the complex number a and the real number x, z=ax.
    GSLCOMPLEX *obj;
    gsl_complex a;
    gsl_complex r;

    // Save the values of the current object into a.
    GSL_SET_COMPLEX(&a, THIS->number.real, THIS->number.imagined);

    // Create new object for return
    obj = (GSLCOMPLEX *) GB.New(GB.FindClass("Complex"), NULL,  NULL);

    // Now run the libgsl function
    //r = gls_complex_mul_real(&a, VARG(x));

    //obj->number.real = r.dat[0];
    //obj->number.imagined = r.dat[1];

    GB.ReturnObject(obj);

END_METHOD


BEGIN_METHOD(GslComplex_Div_Real, GB_FLOAT x;)
    // Function: gsl_complex gsl_complex_div_real (gsl_complex a, double x)
    // This function returns the quotient of the complex number a and the real number x, z=a/x.
    GSLCOMPLEX *obj;
    gsl_complex a;
    gsl_complex r;

    // Save the values of the current object into a.
    GSL_SET_COMPLEX(&a, THIS->number.real, THIS->number.imagined);

    // Create new object for return
    obj = (GSLCOMPLEX *) GB.New(GB.FindClass("Complex"), NULL,  NULL);

    // Now run the libgsl function
    //r = gls_complex_div_real(&a, VARG(x));

    //obj->number.real = r.dat[0];
    //obj->number.imagined = r.dat[1];

    GB.ReturnObject(obj);

END_METHOD


/*********************************************
           Operations On Imagined
*********************************************/
BEGIN_METHOD(GslComplex_Add_Imag, GB_FLOAT x;)
    // Function: gsl_complex gsl_complex_add_imag (gsl_complex a, double y)
    // This function returns the sum of the complex number a and the imaginary number iy, z=a+iy.
    GSLCOMPLEX *obj;
    gsl_complex a;
    gsl_complex r;

    // Save the values of the current object into a.
    GSL_SET_COMPLEX(&a, THIS->number.real, THIS->number.imagined);

    // Create new object for return
    obj = (GSLCOMPLEX *) GB.New(GB.FindClass("Complex"), NULL,  NULL);

    // Now run the libgsl function
    //r = gls_complex_add_imag(&a, VARG(x));

    //obj->number.real = r.dat[0];
    //obj->number.imagined = r.dat[1];

    GB.ReturnObject(obj);

END_METHOD


BEGIN_METHOD(GslComplex_Sub_Imag, GB_FLOAT x;)
    // Function: gsl_complex gsl_complex_sub_imag (gsl_complex a, double y)
    // This function returns the difference of the complex number a and the imaginary number iy, z=a-iy.
    GSLCOMPLEX *obj;
    gsl_complex a;
    gsl_complex r;

    // Save the values of the current object into a.
    GSL_SET_COMPLEX(&a, THIS->number.real, THIS->number.imagined);


    // Create new object for return
    obj = (GSLCOMPLEX *) GB.New(GB.FindClass("Complex"), NULL,  NULL);

    // Now run the libgsl function
    //r = gls_complex_sub_imag(&a, VARG(x));

    //obj->number.real = r.dat[0];
    //obj->number.imagined = r.dat[1];

    GB.ReturnObject(obj);

END_METHOD


BEGIN_METHOD(GslComplex_Mul_Imag, GB_FLOAT x;)
    // Function: gsl_complex gsl_complex_mul_imag (gsl_complex a, double y)
    // This function returns the product of the complex number a and the imaginary number iy, z=a*(iy).
    GSLCOMPLEX *obj;
    gsl_complex a;
    gsl_complex r;

    // Save the values of the current object into a.
    GSL_SET_COMPLEX(&a, THIS->number.real, THIS->number.imagined);

    // Create new object for return
    obj = (GSLCOMPLEX *) GB.New(GB.FindClass("Complex"), NULL,  NULL);

    // Now run the libgsl function
    //r = gls_complex_mul_imag(&a, VARG(x));

    //obj->number.real = r.dat[0];
    //obj->number.imagined = r.dat[1];

    GB.ReturnObject(obj);

END_METHOD



BEGIN_METHOD(GslComplex_Div_Imag, GB_FLOAT x;)
    // Function: gsl_complex gsl_complex_div_imag (gsl_complex a, double y)
    // This function returns the quotient of the complex number a and the imaginary number iy, z=a/(iy).
    GSLCOMPLEX *obj;
    gsl_complex a;
    gsl_complex r;

    // Save the values of the current object into a.
    GSL_SET_COMPLEX(&a, THIS->number.real, THIS->number.imagined);

    // Create new object for return
    obj = (GSLCOMPLEX *) GB.New(GB.FindClass("Complex"), NULL,  NULL);

    // Now run the libgsl function
    //r = gls_complex_div_imag(&a, VARG(x));

    //obj->number.real = r.dat[0];
    //obj->number.imagined = r.dat[1];

    GB.ReturnObject(obj);

END_METHOD



BEGIN_METHOD_VOID(GslComplex_Conjugate)
    // Function: gsl_complex gsl_complex_conjugate (gsl_complex z)
    // This function returns the complex conjugate of the complex number z, z^* = x - i y.
    GSLCOMPLEX *obj;
    gsl_complex a;
    gsl_complex r;

    // Save the values of the current object into a.
    GSL_SET_COMPLEX(&a, THIS->number.real, THIS->number.imagined);

    // Create new object for return
    obj = (GSLCOMPLEX *) GB.New(GB.FindClass("Complex"), NULL,  NULL);

    // Now run the libgsl function
    //r = gls_complex_conjugate(&a);

    //THIS->number.real = r.dat[0];
    //THIS->number.imagined = r.dat[1];

    GB.ReturnObject(obj);

END_METHOD



BEGIN_METHOD_VOID(GslComplex_Inverse)
    // Function: gsl_complex gsl_complex_inverse (gsl_complex z)
    // This function returns the inverse, or reciprocal, of the complex number z, 1/z = (x - i y)/(x^2 + y^2).
    GSLCOMPLEX *obj;
    gsl_complex a;
    gsl_complex r;

    // Save the values of the current object into a.
    GSL_SET_COMPLEX(&a, THIS->number.real, THIS->number.imagined);

    // Create new object for return
    obj = (GSLCOMPLEX *) GB.New(GB.FindClass("Complex"), NULL,  NULL);

    // Now run the libgsl function
    //r = gls_complex_inverse(&a);

    //obj->number.real = r.dat[0];
    //obj->number.imagined = r.dat[1];

    GB.ReturnObject(obj);

END_METHOD



BEGIN_METHOD_VOID(GslComplex_Negative)
    // Function: gsl_complex gsl_complex_negative (gsl_complex z)
    // This function returns the negative of the complex number z, -z = (-x) + i(-y).
    GSLCOMPLEX *obj;
    gsl_complex a;
    gsl_complex r;

    // Save the values of the current object into a.
    GSL_SET_COMPLEX(&a, THIS->number.real, THIS->number.imagined);

    // Create new object for return
    obj = (GSLCOMPLEX *) GB.New(GB.FindClass("Complex"), NULL,  NULL);

    // Now run the libgsl function
    //r = gls_complex_negative(&a);

    //obj->number.real = r.dat[0];
    //obj->number.imagined = r.dat[1];

    GB.ReturnObject(obj);

END_METHOD

/**************************************************
          Elementary Complex Functions
**************************************************/

BEGIN_METHOD_VOID(GslComplex_Sqrt)
    // Function: gsl_complex gsl_complex_sqrt (gsl_complex z)
    // This function returns the square root of the complex number z, \sqrt z.
    // The branch cut is the negative real axis.
    // The result always lies in the right half of the complex plane.
    GSLCOMPLEX *obj;
    gsl_complex a;
    gsl_complex r;

    // Save the values of the current object into a.
    GSL_SET_COMPLEX(&a, THIS->number.real, THIS->number.imagined);

    // Create new object for return
    obj = (GSLCOMPLEX *) GB.New(GB.FindClass("Complex"), NULL,  NULL);

    // Now run the libgsl function
    //r = gls_complex_sqrt(&a);

    //obj->number.real = r.dat[0];
    //obj->number.imagined = r.dat[1];

    GB.ReturnObject(obj);

END_METHOD


BEGIN_METHOD(GslComplex_SqrtReal, GB_FLOAT x;)
    // Function: gsl_complex gsl_complex_sqrt_real (double x)
    // This function returns the complex square root of the real
    // number x, where x may be negative.
    GSLCOMPLEX *obj;
    gsl_complex r;

    // Create new object for return
    obj = (GSLCOMPLEX *) GB.New(GB.FindClass("Complex"), NULL,  NULL);

    // Now run the libgsl function
    //r = gls_complex_sqrt_real(VARG(x));

    //obj->number.real = r.dat[0];
    //obj->number.imagined = r.dat[1];

    GB.ReturnObject(obj);

END_METHOD


BEGIN_METHOD(GslComplex_Pow, GB_OBJECT x;)
    // Function: gsl_complex gsl_complex_pow (gsl_complex z, gsl_complex a)
    // The function returns the complex number z raised to the complex power a, z^a.
    // This is computed as \exp(\log(z)*a) using complex logarithms and complex exponentials.
    GSLCOMPLEX *p;
    GSLCOMPLEX *obj;
    gsl_complex a, b;
    gsl_complex r;

    p = (GSLCOMPLEX *) VARG(x);
    // Save the values of the current object into a.
    GSL_SET_COMPLEX(&a, THIS->number.real, THIS->number.imagined);

    // Save the passed in obect's values to gsl_complex structure
    GSL_SET_COMPLEX(&b, p->number.real, p->number.imagined);

    // Create new object for return
    obj = (GSLCOMPLEX *) GB.New(GB.FindClass("Complex"), NULL,  NULL);

    // Now run the libgsl function
    //r = gls_complex_pow(&a, &b);

    //obj->number.real = r.dat[0];
    //obj->number.imagined = r.dat[1];

    GB.ReturnObject(obj);

END_METHOD


BEGIN_METHOD(GslComplex_PowReal, GB_FLOAT x;)
    // Function: gsl_complex gsl_complex_pow_real (gsl_complex z, double x)
    // This function returns the complex number z raised to the real power x, z^x.
    GSLCOMPLEX *obj;
    gsl_complex a;
    gsl_complex r;

    // Save the values of the current object into a.
    GSL_SET_COMPLEX(&a, THIS->number.real, THIS->number.imagined);

    // Create new object for return
    obj = (GSLCOMPLEX *) GB.New(GB.FindClass("Complex"), NULL,  NULL);

    // Now run the libgsl function
    //r = gls_complex_pow_real(&a, VARG(x));

    //obj->number.real = r.dat[0];
    //obj->number.imagined = r.dat[1];

    GB.ReturnObject(obj);

END_METHOD


BEGIN_METHOD_VOID(GslComplex_Exp)
    // Function: gsl_complex gsl_complex_exp (gsl_complex z)
    // This function returns the complex exponential of the complex number z, \exp(z).
    GSLCOMPLEX *obj;
    gsl_complex a;
    gsl_complex r;

    // Save the values of the current object into a.
    GSL_SET_COMPLEX(&a, THIS->number.real, THIS->number.imagined);

    // Create new object for return
    obj = (GSLCOMPLEX *) GB.New(GB.FindClass("Complex"), NULL,  NULL);

    // Now run the libgsl function
    //r = gls_complex_negative(&a);

    //obj->number.real = r.dat[0];
    //obj->number.imagined = r.dat[1];

    GB.ReturnObject(obj);

END_METHOD


BEGIN_METHOD_VOID(GslComplex_Log)
    // Function: gsl_complex gsl_complex_log (gsl_complex z)
    // This function returns the complex natural logarithm (base e)
    // of the complex number z, \log(z). The branch cut is the negative real axis.
    GSLCOMPLEX *obj;
    gsl_complex a;
    gsl_complex r;

    // Save the values of the current object into a.
    GSL_SET_COMPLEX(&a, THIS->number.real, THIS->number.imagined);

    // Create new object for return
    obj = (GSLCOMPLEX *) GB.New(GB.FindClass("Complex"), NULL,  NULL);

    // Now run the libgsl function
    //r = gls_complex_log(&a);

    //obj->number.real = r.dat[0];
    //obj->number.imagined = r.dat[1];

    GB.ReturnObject(obj);

END_METHOD


BEGIN_METHOD_VOID(GslComplex_Log10)
    // Function: gsl_complex gsl_complex_log10 (gsl_complex z)
    // This function returns the complex base-10 logarithm of
    // the complex number z, \log_10 (z).
    GSLCOMPLEX *obj;
    gsl_complex a;
    gsl_complex r;

    // Save the values of the current object into a.
    GSL_SET_COMPLEX(&a, THIS->number.real, THIS->number.imagined);

    // Create new object for return
    obj = (GSLCOMPLEX *) GB.New(GB.FindClass("Complex"), NULL,  NULL);

    // Now run the libgsl function
    //r = gls_complex_log10(&a);

    //obj->number.real = r.dat[0];
    //obj->number.imagined = r.dat[1];

    GB.ReturnObject(obj);

END_METHOD


BEGIN_METHOD(GslComplex_Log_b, GB_OBJECT x;)
    // Function: gsl_complex gsl_complex_log_b (gsl_complex z, gsl_complex b)
    // This function returns the complex base-b logarithm of the complex number z, \log_b(z).
    // This quantity is computed as the ratio \log(z)/\log(b).
    GSLCOMPLEX *p;
    GSLCOMPLEX *obj;
    gsl_complex a, b;
    gsl_complex r;

    p = (GSLCOMPLEX *) VARG(x);

    // Save the values of the current object into a.
    GSL_SET_COMPLEX(&a, THIS->number.real, THIS->number.imagined);

    // Save the passed in obect's values to gsl_complex structure
    GSL_SET_COMPLEX(&b, p->number.real, p->number.imagined);

    // Create new object for return
    obj = (GSLCOMPLEX *) GB.New(GB.FindClass("Complex"), NULL,  NULL);

    // Now run the libgsl function
    //r = gls_complex_log_b(&a, &b);

    //obj->number.real = r.dat[0];
    //obj->number.imagined = r.dat[1];

    GB.ReturnObject(obj);

END_METHOD


/**************************************************
        Complex Trigonometric Functions
**************************************************/

BEGIN_METHOD_VOID(GslComplex_Sin)
    // Function: gsl_complex gsl_complex_sin (gsl_complex z)
    // This function returns the complex sine of the complex
    // number z, \sin(z) = (\exp(iz) - \exp(-iz))/(2i).
    GSLCOMPLEX *obj;
    gsl_complex a;
    gsl_complex r;

    // Save the values of the current object into a.
    GSL_SET_COMPLEX(&a, THIS->number.real, THIS->number.imagined);

    // Create new object for return
    obj = (GSLCOMPLEX *) GB.New(GB.FindClass("Complex"), NULL,  NULL);

    // Now run the libgsl function
    //r = gls_complex_sin(&a);

    //obj->number.real = r.dat[0];
    //obj->number.imagined = r.dat[1];

    GB.ReturnObject(obj);

END_METHOD


BEGIN_METHOD_VOID(GslComplex_Cos)
    // Function: gsl_complex gsl_complex_cos (gsl_complex z)
    // This function returns the complex cosine of the complex
    // number z, \cos(z) = (\exp(iz) + \exp(-iz))/2.
    GSLCOMPLEX *obj;
    gsl_complex a;
    gsl_complex r;

    // Save the values of the current object into a.
    GSL_SET_COMPLEX(&a, THIS->number.real, THIS->number.imagined);

    // Create new object for return
    obj = (GSLCOMPLEX *) GB.New(GB.FindClass("Complex"), NULL,  NULL);

    // Now run the libgsl function
    //r = gls_complex_cos(&a);

    //obj->number.real = r.dat[0];
    //obj->number.imagined = r.dat[1];

    GB.ReturnObject(obj);

END_METHOD


BEGIN_METHOD_VOID(GslComplex_Tan)
    // Function: gsl_complex gsl_complex_tan (gsl_complex z)
    // This function returns the complex tangent of the complex
    // number z, \tan(z) = \sin(z)/\cos(z).
    GSLCOMPLEX *obj;
    gsl_complex a;
    gsl_complex r;

    // Save the values of the current object into a.
    GSL_SET_COMPLEX(&a, THIS->number.real, THIS->number.imagined);

    // Create new object for return
    obj = (GSLCOMPLEX *) GB.New(GB.FindClass("Complex"), NULL,  NULL);

    // Now run the libgsl function
    //r = gls_complex_tan(&a);

    //obj->number.real = r.dat[0];
    //obj->number.imagined = r.dat[1];

    GB.ReturnObject(obj);

END_METHOD


BEGIN_METHOD_VOID(GslComplex_Sec)
    // Function: gsl_complex gsl_complex_sec (gsl_complex z)
    // This function returns the complex secant of the complex
    // number z, \sec(z) = 1/\cos(z).
    GSLCOMPLEX *obj;
    gsl_complex a;
    gsl_complex r;

    // Save the values of the current object into a.
    GSL_SET_COMPLEX(&a, THIS->number.real, THIS->number.imagined);

    // Create new object for return
    obj = (GSLCOMPLEX *) GB.New(GB.FindClass("Complex"), NULL,  NULL);

    // Now run the libgsl function
    //r = gls_complex_sec(&a);

    //obj->number.real = r.dat[0];
    //obj->number.imagined = r.dat[1];

    GB.ReturnObject(obj);

END_METHOD


BEGIN_METHOD_VOID(GslComplex_Csc)
    // Function: gsl_complex gsl_complex_csc (gsl_complex z)
    // This function returns the complex cosecant of the complex
    // number z, \csc(z) = 1/\sin(z).
    GSLCOMPLEX *obj;
    gsl_complex a;
    gsl_complex r;

    // Save the values of the current object into a.
    GSL_SET_COMPLEX(&a, THIS->number.real, THIS->number.imagined);

    // Create new object for return
    obj = (GSLCOMPLEX *) GB.New(GB.FindClass("Complex"), NULL,  NULL);

    // Now run the libgsl function
    //r = gls_complex_csc(&a);

    //obj->number.real = r.dat[0];
    //obj->number.imagined = r.dat[1];

    GB.ReturnObject(obj);

END_METHOD


BEGIN_METHOD_VOID(GslComplex_Cot)
    // Function: gsl_complex gsl_complex_cot (gsl_complex z)
    // This function returns the complex cotangent of the complex
    // number z, \cot(z) = 1/\tan(z).
    GSLCOMPLEX *obj;
    gsl_complex a;
    gsl_complex r;

    // Save the values of the current object into a.
    GSL_SET_COMPLEX(&a, THIS->number.real, THIS->number.imagined);

    // Create new object for return
    obj = (GSLCOMPLEX *) GB.New(GB.FindClass("Complex"), NULL,  NULL);

    // Now run the libgsl function
    //r = gls_complex_cot(&a);

    //obj->number.real = r.dat[0];
    //obj->number.imagined = r.dat[1];

    GB.ReturnObject(obj);

END_METHOD


/**************************************************
      Inverse Complex Trigonometric Functions
**************************************************/

BEGIN_METHOD_VOID(GslComplex_Arcsin)
    // Function: gsl_complex gsl_complex_arcsin (gsl_complex z)
    // This function returns the complex arcsine of the complex number z, \arcsin(z).
    // The branch cuts are on the real axis, less than -1 and greater than 1.
    GSLCOMPLEX *obj;
    gsl_complex a;
    gsl_complex r;

    // Save the values of the current object into a.
    GSL_SET_COMPLEX(&a, THIS->number.real, THIS->number.imagined);

    // Create new object for return
    obj = (GSLCOMPLEX *) GB.New(GB.FindClass("Complex"), NULL,  NULL);

    // Now run the libgsl function
    //r = gls_complex_arcsin(&a);

    //obj->number.real = r.dat[0];
    //obj->number.imagined = r.dat[1];

    GB.ReturnObject(obj);

END_METHOD


BEGIN_METHOD(GslComplex_Arcsin_Real, GB_FLOAT x;)
    // Function: gsl_complex gsl_complex_arcsin_real (double z)
    // This function returns the complex arcsine of the real number z, \arcsin(z).
    // For z between -1 and 1, the function returns a real value in the range [-\pi/2,\pi/2].
    // For z less than -1 the result has a real part of -\pi/2 and a positive imaginary part.
    // For z greater than 1 the result has a real part of \pi/2 and a negative imaginary part.
    GSLCOMPLEX *obj;
    gsl_complex a;
    gsl_complex r;

    // Save the values of the current object into a.
    GSL_SET_COMPLEX(&a, THIS->number.real, THIS->number.imagined);

    // Create new object for return
    obj = (GSLCOMPLEX *) GB.New(GB.FindClass("Complex"), NULL,  NULL);

    // Now run the libgsl function
    //r = gls_complex_arcsin_real(VARG(x));

    //obj->number.real = r.dat[0];
    //obj->number.imagined = r.dat[1];

    GB.ReturnObject(obj);

END_METHOD


BEGIN_METHOD_VOID(GslComplex_Arccos)
    // Function: gsl_complex gsl_complex_arccos (gsl_complex z)
    // This function returns the complex arccosine of the complex number z, \arccos(z).
    // The branch cuts are on the real axis, less than -1 and greater than 1.
    GSLCOMPLEX *obj;
    gsl_complex a;
    gsl_complex r;

    // Save the values of the current object into a.
    GSL_SET_COMPLEX(&a, THIS->number.real, THIS->number.imagined);

    // Create new object for return
    obj = (GSLCOMPLEX *) GB.New(GB.FindClass("Complex"), NULL,  NULL);

    // Now run the libgsl function
    //r = gls_complex_arcos_real(VARG(x));

    //obj->number.real = r.dat[0];
    //obj->number.imagined = r.dat[1];

    GB.ReturnObject(obj);

END_METHOD


BEGIN_METHOD(GslComplex_Arccos_Real, GB_FLOAT x;)
    // Function: gsl_complex gsl_complex_arccos_real (double z)
    // This function returns the complex arccosine of the real number z, \arccos(z).
    // For z between -1 and 1, the function returns a real value in the range [0,\pi].
    // For z less than -1 the result has a real part of \pi and a negative imaginary part.
    // For z greater than 1 the result is purely imaginary and positive.
    GSLCOMPLEX *obj;
    gsl_complex a;
    gsl_complex r;

    // Save the values of the current object into a.
    GSL_SET_COMPLEX(&a, THIS->number.real, THIS->number.imagined);

    // Create new object for return
    obj = (GSLCOMPLEX *) GB.New(GB.FindClass("Complex"), NULL,  NULL);

    // Now run the libgsl function
    //r = gls_complex_arcos_real(VARG(x));

    //obj->number.real = r.dat[0];
    //obj->number.imagined = r.dat[1];

    GB.ReturnObject(obj);

END_METHOD


BEGIN_METHOD_VOID(GslComplex_Arctan)
    // Function: gsl_complex gsl_complex_arctan (gsl_complex z)
    // This function returns the complex arctangent of the complex number z, \arctan(z).
    // The branch cuts are on the imaginary axis, below -i and above i.
    GSLCOMPLEX *obj;
    gsl_complex a;
    gsl_complex r;

    // Save the values of the current object into a.
    GSL_SET_COMPLEX(&a, THIS->number.real, THIS->number.imagined);

    // Create new object for return
    obj = (GSLCOMPLEX *) GB.New(GB.FindClass("Complex"), NULL,  NULL);

    // Now run the libgsl function
    //r = gls_complex_arctan(&a);

    //obj->number.real = r.dat[0];
    //obj->number.imagined = r.dat[1];

    GB.ReturnObject(obj);

END_METHOD


BEGIN_METHOD_VOID(GslComplex_Arcsec)
    // Function: gsl_complex gsl_complex_arcsec (gsl_complex z)
    // This function returns the complex arcsecant of the complex number z, \arcsec(z) = \arccos(1/z).
    GSLCOMPLEX *obj;
    gsl_complex a;
    gsl_complex r;

    // Save the values of the current object into a.
    GSL_SET_COMPLEX(&a, THIS->number.real, THIS->number.imagined);

    // Create new object for return
    obj = (GSLCOMPLEX *) GB.New(GB.FindClass("Complex"), NULL,  NULL);

    // Now run the libgsl function
    //r = gls_complex_arcsec(&a);

    //obj->number.real = r.dat[0];
    //obj->number.imagined = r.dat[1];

    GB.ReturnObject(obj);

END_METHOD


BEGIN_METHOD_VOID(GslComplex_Arcsec_Real)
    // Function: gsl_complex gsl_complex_arcsec_real (double z)
    // This function returns the complex arcsecant of the real number z, \arcsec(z) = \arccos(1/z).
    GSLCOMPLEX *obj;
    gsl_complex a;
    gsl_complex r;

    // Save the values of the current object into a.
    GSL_SET_COMPLEX(&a, THIS->number.real, THIS->number.imagined);

    // Create new object for return
    obj = (GSLCOMPLEX *) GB.New(GB.FindClass("Complex"), NULL,  NULL);

    // Now run the libgsl function
    //r = gls_complex_arcsec_real(VARG(x));

    //obj->number.real = r.dat[0];
    //obj->number.imagined = r.dat[1];

    GB.ReturnObject(obj);

END_METHOD


BEGIN_METHOD_VOID(GslComplex_Arccsc)
    // Function: gsl_complex gsl_complex_arccsc (gsl_complex z)
    // This function returns the complex arccosecant of the complex number z, \arccsc(z) = \arcsin(1/z).
    GSLCOMPLEX *obj;
    gsl_complex a;
    gsl_complex r;

    // Save the values of the current object into a.
    GSL_SET_COMPLEX(&a, THIS->number.real, THIS->number.imagined);

    // Create new object for return
    obj = (GSLCOMPLEX *) GB.New(GB.FindClass("Complex"), NULL,  NULL);

    // Now run the libgsl function
    //r = gls_complex_arccsc(&a));

    //obj->number.real = r.dat[0];
    //obj->number.imagined = r.dat[1];

    GB.ReturnObject(obj);

END_METHOD


BEGIN_METHOD(GslComplex_Arccsc_Real, GB_FLOAT x;)
    // Function: gsl_complex gsl_complex_arccsc_real (double z)
    // This function returns the complex arccosecant of the real number z, \arccsc(z) = \arcsin(1/z).
    GSLCOMPLEX *obj;
    gsl_complex a;
    gsl_complex r;

    // Save the values of the current object into a.
    GSL_SET_COMPLEX(&a, THIS->number.real, THIS->number.imagined);

    // Create new object for return
    obj = (GSLCOMPLEX *) GB.New(GB.FindClass("Complex"), NULL,  NULL);

    // Now run the libgsl function
    //r = gls_complex_arccsc(&a);

    //obj->number.real = r.dat[0];
    //obj->number.imagined = r.dat[1];

    GB.ReturnObject(obj);

END_METHOD


BEGIN_METHOD_VOID(GslComplex_Arccot)
    // Function: gsl_complex gsl_complex_arccot (gsl_complex z)
    // This function returns the complex arccotangent of the complex number z, \arccot(z) = \arctan(1/z).
    GSLCOMPLEX *obj;
    gsl_complex a;
    gsl_complex r;

    // Save the values of the current object into a.
    GSL_SET_COMPLEX(&a, THIS->number.real, THIS->number.imagined);

    // Create new object for return
    obj = (GSLCOMPLEX *) GB.New(GB.FindClass("Complex"), NULL,  NULL);

    // Now run the libgsl function
    //r = gls_complex_arccot(&a);

    //obj->number.real = r.dat[0];
    //obj->number.imagined = r.dat[1];

    GB.ReturnObject(obj);

END_METHOD


/**************************************************
           Complex Hyperbolic Functions
**************************************************/

BEGIN_METHOD_VOID(GslComplex_Sinh)
    // Function: gsl_complex gsl_complex_sinh (gsl_complex z)
    // This function returns the complex hyperbolic sine of the complex
    // number z, \sinh(z) = (\exp(z) - \exp(-z))/2.
    GSLCOMPLEX *obj;
    gsl_complex a;
    gsl_complex r;

    // Save the values of the current object into a.
    GSL_SET_COMPLEX(&a, THIS->number.real, THIS->number.imagined);

    // Create new object for return
    obj = (GSLCOMPLEX *) GB.New(GB.FindClass("Complex"), NULL,  NULL);

    // Now run the libgsl function
    //r = gls_complex_sinh(&a);

    //obj->number.real = r.dat[0];
    //obj->number.imagined = r.dat[1];

    GB.ReturnObject(obj);

END_METHOD


BEGIN_METHOD_VOID(GslComplex_Cosh)
    // Function: gsl_complex gsl_complex_cosh (gsl_complex z)
    // This function returns the complex hyperbolic cosine of the complex number z, \cosh(z) = (\exp(z) + \exp(-z))/2.
    GSLCOMPLEX *obj;
    gsl_complex a;
    gsl_complex r;

    // Save the values of the current object into a.
    GSL_SET_COMPLEX(&a, THIS->number.real, THIS->number.imagined);

    // Create new object for return
    obj = (GSLCOMPLEX *) GB.New(GB.FindClass("Complex"), NULL,  NULL);

    // Now run the libgsl function
    //r = gls_complex_cosh(&a);

    //obj->number.real = r.dat[0];
    //obj->number.imagined = r.dat[1];

    GB.ReturnObject(obj);

END_METHOD


BEGIN_METHOD_VOID(GslComplex_Tanh)
    // Function: gsl_complex gsl_complex_tanh (gsl_complex z)
    // This function returns the complex hyperbolic tangent of the complex number z, \tanh(z) = \sinh(z)/\cosh(z).
    GSLCOMPLEX *obj;
    gsl_complex a;
    gsl_complex r;

    // Save the values of the current object into a.
    GSL_SET_COMPLEX(&a, THIS->number.real, THIS->number.imagined);

    // Create new object for return
    obj = (GSLCOMPLEX *) GB.New(GB.FindClass("Complex"), NULL,  NULL);

    // Now run the libgsl function
    //r = gls_complex_tanh(&a);

    //obj->number.real = r.dat[0];
    //obj->number.imagined = r.dat[1];

    GB.ReturnObject(obj);

END_METHOD


BEGIN_METHOD_VOID(GslComplex_Sech)
    // Function: gsl_complex gsl_complex_sech (gsl_complex z)
    // This function returns the complex hyperbolic secant of the complex number z, \sech(z) = 1/\cosh(z).
    GSLCOMPLEX *obj;
    gsl_complex a;
    gsl_complex r;

    // Save the values of the current object into a.
    GSL_SET_COMPLEX(&a, THIS->number.real, THIS->number.imagined);

    // Create new object for return
    obj = (GSLCOMPLEX *) GB.New(GB.FindClass("Complex"), NULL,  NULL);

    // Now run the libgsl function
    //r = gls_complex_sech(&a);

    //obj->number.real = r.dat[0];
    //obj->number.imagined = r.dat[1];

    GB.ReturnObject(obj);

END_METHOD


BEGIN_METHOD_VOID(GslComplex_Csch)
    // Function: gsl_complex gsl_complex_csch (gsl_complex z)
    // This function returns the complex hyperbolic cosecant of the complex number z, \csch(z) = 1/\sinh(z).
    GSLCOMPLEX *obj;
    gsl_complex a;
    gsl_complex r;

    // Save the values of the current object into a.
    GSL_SET_COMPLEX(&a, THIS->number.real, THIS->number.imagined);

    // Create new object for return
    obj = (GSLCOMPLEX *) GB.New(GB.FindClass("Complex"), NULL,  NULL);

    // Now run the libgsl function
    //r = gls_complex_csch(&a);

    //obj->number.real = r.dat[0];
    //obj->number.imagined = r.dat[1];

    GB.ReturnObject(obj);

END_METHOD


BEGIN_METHOD_VOID(GslComplex_Coth)
    //Function: gsl_complex gsl_complex_coth (gsl_complex z)
    // This function returns the complex hyperbolic cotangent of the complex number z, \coth(z) = 1/\tanh(z).
    GSLCOMPLEX *obj;
    gsl_complex a;
    gsl_complex r;

    // Save the values of the current object into a.
    GSL_SET_COMPLEX(&a, THIS->number.real, THIS->number.imagined);

    // Create new object for return
    obj = (GSLCOMPLEX *) GB.New(GB.FindClass("Complex"), NULL,  NULL);

    // Now run the libgsl function
    //r = gls_complex_coth(&a);

    //obj->number.real = r.dat[0];
    //obj->number.imagined = r.dat[1];

    GB.ReturnObject(obj);

END_METHOD



/**************************************************
      Inverse Complex Hyperbolic Functions
**************************************************/

BEGIN_METHOD_VOID(GslComplex_Arcsinh)
    // Function: gsl_complex gsl_complex_arcsinh (gsl_complex z)
    // This function returns the complex hyperbolic arcsine of the complex
    // number z, \arcsinh(z). The branch cuts are on the imaginary axis,
    // below -i and above i.
    GSLCOMPLEX *obj;
    gsl_complex a;
    gsl_complex r;

    // Save the values of the current object into a.
    GSL_SET_COMPLEX(&a, THIS->number.real, THIS->number.imagined);

    // Create new object for return
    obj = (GSLCOMPLEX *) GB.New(GB.FindClass("Complex"), NULL,  NULL);

    // Now run the libgsl function
    //r = gls_complex_arcsinh(&a);

    //obj->number.real = r.dat[0];
    //obj->number.imagined = r.dat[1];

    GB.ReturnObject(obj);

END_METHOD


BEGIN_METHOD_VOID(GslComplex_Arccosh)
    // Function: gsl_complex gsl_complex_arccosh (gsl_complex z)
    // This function returns the complex hyperbolic arccosine of the complex
    // number z, \arccosh(z). The branch cut is on the real axis, less than 1.
    // Note that in this case we use the negative square root in formula 4.6.21
    // of Abramowitz & Stegun giving \arccosh(z)=\log(z-\sqrt{z^2-1}).
    GSLCOMPLEX *obj;
    gsl_complex a;
    gsl_complex r;

    // Save the values of the current object into a.
    GSL_SET_COMPLEX(&a, THIS->number.real, THIS->number.imagined);

    // Create new object for return
    obj = (GSLCOMPLEX *) GB.New(GB.FindClass("Complex"), NULL,  NULL);

    // Now run the libgsl function
    //r = gls_complex_arccosh(&a);

    //obj->number.real = r.dat[0];
    //obj->number.imagined = r.dat[1];

    GB.ReturnObject(obj);

END_METHOD


BEGIN_METHOD(GslComplex_Arccosh_Real, GB_FLOAT x;)
    //Function: gsl_complex gsl_complex_arccosh_real (double z)
    // This function returns the complex hyperbolic arccosine of the real number z, \arccosh(z).
    GSLCOMPLEX *obj;
    gsl_complex a;
    gsl_complex r;

    // Save the values of the current object into a.
    GSL_SET_COMPLEX(&a, THIS->number.real, THIS->number.imagined);

    // Create new object for return
    obj = (GSLCOMPLEX *) GB.New(GB.FindClass("Complex"), NULL,  NULL);

    // Now run the libgsl function
    //r = gls_complex_arccosh_real(&a, VARG(x));

    //obj->number.real = r.dat[0];
    //obj->number.imagined = r.dat[1];

    GB.ReturnObject(obj);

END_METHOD


BEGIN_METHOD_VOID(GslComplex_Arctanh)
    // Function: gsl_complex gsl_complex_arctanh (gsl_complex z)
    // This function returns the complex hyperbolic arctangent of the complex
    // number z, \arctanh(z). The branch cuts are on the real axis, less than
    // -1 and greater than 1.
    GSLCOMPLEX *obj;
    gsl_complex a;
    gsl_complex r;

    // Save the values of the current object into a.
    GSL_SET_COMPLEX(&a, THIS->number.real, THIS->number.imagined);

    // Create new object for return
    obj = (GSLCOMPLEX *) GB.New(GB.FindClass("Complex"), NULL,  NULL);

    // Now run the libgsl function
    //r = gls_complex_arctanh(&a);

    //obj->number.real = r.dat[0];
    //obj->number.imagined = r.dat[1];

    GB.ReturnObject(obj);

END_METHOD


BEGIN_METHOD(GslComplex_Arctanh_Real, GB_FLOAT x;)
    // Function: gsl_complex gsl_complex_arctanh_real (double z)
    // This function returns the complex hyperbolic arctangent of the real number z, \arctanh(z).
    GSLCOMPLEX *obj;
    gsl_complex a;
    gsl_complex r;

    // Save the values of the current object into a.
    GSL_SET_COMPLEX(&a, THIS->number.real, THIS->number.imagined);

    // Create new object for return
    obj = (GSLCOMPLEX *) GB.New(GB.FindClass("Complex"), NULL,  NULL);

    // Now run the libgsl function
    //r = gls_complex_arctanh_real(a, VARG(x));

    //obj->number.real = r.dat[0];
    //obj->number.imagined = r.dat[1];

    GB.ReturnObject(obj);

END_METHOD


BEGIN_METHOD_VOID(GslComplex_Arcsech)
    //Function: gsl_complex gsl_complex_arcsech (gsl_complex z)
    // This function returns the complex hyperbolic arcsecant of the complex
    // number z, \arcsech(z) = \arccosh(1/z).
    GSLCOMPLEX *obj;
    gsl_complex a;
    gsl_complex r;

    // Save the values of the current object into a.
    GSL_SET_COMPLEX(&a, THIS->number.real, THIS->number.imagined);

    // Create new object for return
    obj = (GSLCOMPLEX *) GB.New(GB.FindClass("Complex"), NULL,  NULL);

    // Now run the libgsl function

    //r = gls_complex_arcsech(&a);

    //obj->number.real = r.dat[0];
    //obj->number.imagined = r.dat[1];

    GB.ReturnObject(obj);

END_METHOD


BEGIN_METHOD_VOID(GslComplex_Arccsch)
    // Function: gsl_complex gsl_complex_arccsch (gsl_complex z)
    // This function returns the complex hyperbolic arccosecant of the complex
    // number z, \arccsch(z) = \arcsin(1/z).
    GSLCOMPLEX *obj;
    gsl_complex a;
    gsl_complex r;

    // Save the values of the current object into a.
    GSL_SET_COMPLEX(&a, THIS->number.real, THIS->number.imagined);

    // Create new object for return
    obj = (GSLCOMPLEX *) GB.New(GB.FindClass("Complex"), NULL,  NULL);

    // Now run the libgsl function
    //r = gls_complex_arccsch(&a);

    //obj->number.real = r.dat[0];
    //obj->number.imagined = r.dat[1];

    GB.ReturnObject(obj);

END_METHOD


BEGIN_METHOD_VOID(GslComplex_Arccoth)
    // Function: gsl_complex gsl_complex_arccoth (gsl_complex z)
    // This function returns the complex hyperbolic arccotangent of the complex
    // number z, \arccoth(z) = \arctanh(1/z).
    GSLCOMPLEX *obj;
    gsl_complex a;
    gsl_complex r;

    // Save the values of the current object into a.
    GSL_SET_COMPLEX(&a, THIS->number.real, THIS->number.imagined);

    // Create new object for return
    obj = (GSLCOMPLEX *) GB.New(GB.FindClass("Complex"), NULL,  NULL);

    // Now run the libgsl function
    //r = gls_complex_arccoth(a);

    //obj->number.real = r.dat[0];
    //obj->number.imagined = r.dat[1];

    GB.ReturnObject(obj);

END_METHOD




/**************************************************
  Describe Class properties and methods to Gambas
**************************************************/
GB_DESC CGslComplexDesc[] =
{
    GB_DECLARE("Complex", sizeof(GSLCOMPLEX)),

    GB_PROPERTY("REAL", "f", GslComplex_Real),
    GB_PROPERTY("IMAG", "f", GslComplex_Imagined),

    //GB_STATIC_METHOD("X", "f", GslComplex_X, "(x)o"),
    //GB_STATIC_METHOD("Y", "f", GslComplex_Y, "(y)o"),

    // Operations on gsl_complex
    GB_STATIC_METHOD("Add", "o", GslComplex_Add, "(x)o"),
    GB_STATIC_METHOD("Sub", "o", GslComplex_Sub, "(x)o"),
    GB_STATIC_METHOD("Mul", "o", GslComplex_Mul, "(x)o"),
    GB_STATIC_METHOD("Div", "o", GslComplex_Div, "(x)o"),

    // Operations on Real
    GB_STATIC_METHOD("AddReal", "", GslComplex_Add_Real, "(x)f"),
    GB_STATIC_METHOD("SubReal", "", GslComplex_Sub_Real, "(x)f"),
    GB_STATIC_METHOD("MulReal", "", GslComplex_Mul_Real, "(x)f"),
    GB_STATIC_METHOD("DivReal", "", GslComplex_Div_Real, "(x)f"),

    // Operations on Imag
    GB_STATIC_METHOD("AddImag", "", GslComplex_Add_Imag, "(x)f"),
    GB_STATIC_METHOD("SubImag", "", GslComplex_Sub_Imag, "(x)f"),
    GB_STATIC_METHOD("MulImag", "", GslComplex_Mul_Imag, "(x)f"),
    GB_STATIC_METHOD("DivImag", "", GslComplex_Div_Imag, "(x)f"),

    // Misc
    GB_STATIC_METHOD("Conjugate", "o", GslComplex_Conjugate, ""),
    GB_STATIC_METHOD("Inverse", "o", GslComplex_Inverse, ""),
    GB_STATIC_METHOD("Negative", "o", GslComplex_Negative, ""),

    // Elementary Complex Functions
    GB_STATIC_METHOD("Sqrt", "o", GslComplex_Sqrt, ""),
    GB_STATIC_METHOD("SqrtReal", "0", GslComplex_SqrtReal, "(x)f"),
    GB_STATIC_METHOD("Exp", "o", GslComplex_Exp, ""),
    GB_STATIC_METHOD("Log", "o", GslComplex_Log, ""),
    GB_STATIC_METHOD("Log10", "o", GslComplex_Log10, ""),
    GB_STATIC_METHOD("Logb", "o", GslComplex_Log_b, "(x)o"),

    // Complex Trigonometric Functions
    GB_STATIC_METHOD("Sin", "o", GslComplex_Sin, ""),
    GB_STATIC_METHOD("Cos", "o", GslComplex_Cos, ""),
    GB_STATIC_METHOD("Tan", "o", GslComplex_Tan, ""),
    GB_STATIC_METHOD("Sec", "o", GslComplex_Sec, ""),
    GB_STATIC_METHOD("Csc", "o", GslComplex_Csc, ""),
    GB_STATIC_METHOD("Cot", "o", GslComplex_Cot, ""),

    // Inverse Complex Trigonometric Functions
    GB_STATIC_METHOD("Arcsin", "o", GslComplex_Arcsin, ""),
    GB_STATIC_METHOD("ArcsinReal", "o", GslComplex_Arcsin_Real, "(x)f"),
    GB_STATIC_METHOD("Arccos", "o", GslComplex_Arccos, ""),
    GB_STATIC_METHOD("ArccosReal", "o", GslComplex_Arccos_Real, "(x)f"),
    GB_STATIC_METHOD("Arctanc", "o", GslComplex_Arctan, ""),
    GB_STATIC_METHOD("Arcsec", "o", GslComplex_Arcsec, ""),
    GB_STATIC_METHOD("ArcsecReal", "o", GslComplex_Arcsec_Real, "(x)f"),
    GB_STATIC_METHOD("Arccsc", "o", GslComplex_Arccsc, ""),
    GB_STATIC_METHOD("ArccscReal", "o", GslComplex_Arccsc_Real, "(x)f"),
    GB_STATIC_METHOD("Arccot", "o", GslComplex_Arccot, ""),

    // Complex Hyperbolic Functions
    GB_STATIC_METHOD("Sinh", "o", GslComplex_Sinh, ""),
    GB_STATIC_METHOD("Cosh", "o", GslComplex_Cosh, ""),
    GB_STATIC_METHOD("Tanh", "o", GslComplex_Tanh, ""),
    GB_STATIC_METHOD("Sech", "o", GslComplex_Sech, ""),
    GB_STATIC_METHOD("Csch", "o", GslComplex_Csch, ""),
    GB_STATIC_METHOD("Coth", "o", GslComplex_Coth, ""),

    // Inverse Complex Hyperbolic Functions
    GB_STATIC_METHOD("Arcsinh", "o", GslComplex_Arcsinh, ""),
    GB_STATIC_METHOD("Arccosh", "o", GslComplex_Arccosh, ""),
    GB_STATIC_METHOD("ArccoshReal", "o", GslComplex_Arccosh_Real, "(x)f"),
    GB_STATIC_METHOD("Arctanh", "o", GslComplex_Arctanh, ""),
    GB_STATIC_METHOD("ArctahReal", "o", GslComplex_Arctanh_Real, "(x)f"),
    GB_STATIC_METHOD("Arcsech", "o", GslComplex_Arcsech, ""),
    GB_STATIC_METHOD("Arccsch", "o", GslComplex_Arccsch, ""),
    GB_STATIC_METHOD("Arccoth", "o", GslComplex_Arccoth, ""),

    //GB_STATIC_METHOD("", "", GslComplex_, ""),

    GB_END_DECLARE
};


