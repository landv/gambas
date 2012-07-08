/***************************************************************************

	c_complex.c

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
#endif

#include "c_complex.h"
#include "../gambas.h"
#include "gb_common.h"
#include "c_gsl.h"
#include <gsl/gsl_math.h>
#include <gsl/gsl_cblas.h>
#include <stdio.h>



#define THIS ((GSLCOMPLEX *)_object)

//---- Complex number creation ----------------------------------------------

GSLCOMPLEX *COMPLEX_create()
{
	static GB_CLASS _klass = (GB_CLASS)NULL;

	GSLCOMPLEX *c;
	
	if (!_klass)
		_klass = GB.FindClass("Complex");
	
	c = (GSLCOMPLEX *)GB.New(_klass, NULL, NULL);
	c->number.dat[0] = 0.0;
	c->number.dat[1] = 0.0;

	return c;
}

//---- Arithmetic operators -------------------------------------------------

static GSLCOMPLEX *_addf(GSLCOMPLEX *a, double f)
{
	GSLCOMPLEX *r = COMPLEX_create();
	r->number = gsl_complex_add_real(a->number, f);
	return r;
}

static GSLCOMPLEX *_add(GSLCOMPLEX *a, GSLCOMPLEX *b)
{
	GSLCOMPLEX *r = COMPLEX_create();
	r->number = gsl_complex_add(a->number, b->number);
	return r;
}

static GSLCOMPLEX *_subf(GSLCOMPLEX *a, double f)
{
	GSLCOMPLEX *r = COMPLEX_create();
	r->number = gsl_complex_sub_real(a->number, f);
	return r;
}

static GSLCOMPLEX *_sub(GSLCOMPLEX *a, GSLCOMPLEX *b)
{
	GSLCOMPLEX *r = COMPLEX_create();
	r->number = gsl_complex_sub(a->number, b->number);
	return r;
}

static GSLCOMPLEX *_mulf(GSLCOMPLEX *a, double f)
{
	GSLCOMPLEX *r = COMPLEX_create();
	r->number = gsl_complex_mul_real(a->number, f);
	return r;
}

static GSLCOMPLEX *_mul(GSLCOMPLEX *a, GSLCOMPLEX *b)
{
	GSLCOMPLEX *r = COMPLEX_create();
	r->number = gsl_complex_mul(a->number, b->number);
	return r;
}

static GSLCOMPLEX *_divf(GSLCOMPLEX *a, double f)
{
	gsl_complex c = gsl_complex_div_real(a->number, f);
	
	if (isfinite(c.dat[0]) && isfinite(c.dat[1]))
	{
		GSLCOMPLEX *r = COMPLEX_create();
		r->number = c;
		return r;
	}
	else
		return NULL;
}

static GSLCOMPLEX *_idivf(GSLCOMPLEX *a, double f)
{
	GSLCOMPLEX *r = COMPLEX_create();
	r->number = gsl_complex_mul_real(gsl_complex_inverse(a->number), f);
	return r;
}

static GSLCOMPLEX *_div(GSLCOMPLEX *a, GSLCOMPLEX *b)
{
	gsl_complex c = gsl_complex_div(a->number, b->number);
	
	if (isfinite(c.dat[0]) && isfinite(c.dat[1]))
	{
		GSLCOMPLEX *r = COMPLEX_create();
		r->number = c;
		return r;
	}
	else
		return NULL;
}

static int _equal(GSLCOMPLEX *a, GSLCOMPLEX *b)
{
	return a->number.dat[0] == b->number.dat[0] && a->number.dat[1] == b->number.dat[1];
}

static int _equalf(GSLCOMPLEX *a, double f)
{
	return a->number.dat[0] == f && a->number.dat[1] == 0.0;
}

static GSLCOMPLEX *_neg(GSLCOMPLEX *a)
{
	GSLCOMPLEX *r = COMPLEX_create();
	r->number = gsl_complex_negative(a->number);
	return r;
}

static double _abs(GSLCOMPLEX *a)
{
	return gsl_complex_abs(a->number);
}

static GB_OPERATOR_DESC _operators =
{
	add: (void *)_add,
	addf: (void *)_addf,
	sub: (void *)_sub,
	subf: (void *)_subf,
	mul: (void *)_mul,
	mulf: (void *)_mulf,
	div: (void *)_div,
	divf: (void *)_divf,
	idivf: (void *)_idivf,
	equal: (void *)_equal,
	equalf: (void *)_equalf,
	abs: (void *)_abs,
	neg: (void *)_neg
};

//---- Conversions ----------------------------------------------------------

static char *_to_string(GSLCOMPLEX *_object, bool local)
{
	char buffer[64];
	char *p;
	char *str;
	int len;
	double real, imag;
	
	if (!THIS)
		return NULL;
	
	real = THIS->number.dat[0];
	imag = THIS->number.dat[1];
	
	if (real == 0.0 && imag == 0.0)
		return GB.NewString("0", 1);
	
	p = buffer;
	
	if (real != 0.0)
	{
		GB.NumberToString(local, real, NULL, &str, &len);
		strncpy(p, str, len);
		p += len;
	}
	
	if (imag != 0.0)
	{
		if (imag < 0.0)
		{
			*p++ = '-';
			imag = (-imag);
		}
		else if (p != buffer)
			*p++ = '+';
		
		if (imag != 1.0)
		{
			GB.NumberToString(local, imag, NULL, &str, &len);
			strncpy(p, str, len);
			p += len;
		}
		*p++ = 'i';
	}
	
	return GB.NewString(buffer, p - buffer);
}

static bool _convert(GSLCOMPLEX *a, GB_TYPE type, GB_VALUE *conv)
{
	switch (type)
	{
		case GB_T_FLOAT:
			conv->_float.value = gsl_complex_abs(a->number);
			return FALSE;
			
		case GB_T_SINGLE:
			conv->_single.value = gsl_complex_abs(a->number);
			return FALSE;
			
		case GB_T_INTEGER:
		case GB_T_SHORT:
		case GB_T_BYTE:
			conv->_integer.value = gsl_complex_abs(a->number);
			return FALSE;
			
		case GB_T_LONG:
			conv->_long.value = gsl_complex_abs(a->number);
			return FALSE;
			
		case GB_T_STRING:
		case GB_T_CSTRING:
			conv->_string.value.addr = _to_string(a, type == GB_T_CSTRING);
			conv->_string.value.start = 0;
			conv->_string.value.len = GB.StringLength(conv->_string.value.addr);
			return FALSE;
			
		default:
			return TRUE;
	}
}

//---------------------------------------------------------------------------

BEGIN_METHOD(GslComplex_new, GB_FLOAT real; GB_FLOAT imag)

	THIS->number.dat[0] = VARGOPT(real, 0.0);
	THIS->number.dat[1] = VARGOPT(imag, 0.0);

END_METHOD


BEGIN_METHOD(GslComplex_call, GB_FLOAT real; GB_FLOAT imag)

	GSLCOMPLEX *c = COMPLEX_create();
	
	c->number.dat[0] = VARG(real);
	c->number.dat[1] = VARG(imag);
	GB.ReturnObject(c);

END_METHOD


BEGIN_METHOD_VOID(GslComplex_Copy)

	GSLCOMPLEX *c = COMPLEX_create();
	
	c->number = THIS->number;
	GB.ReturnObject(c);

END_METHOD

BEGIN_METHOD_VOID(GslComplex_ToString)

	char buffer[64];
	char *p;
	char *str;
	int len;
	double real, imag;
	
	real = THIS->number.dat[0];
	imag = THIS->number.dat[1];
	
	if (real == 0.0 && imag == 0.0)
	{
		GB.ReturnConstZeroString("0");
		return;
	}
	
	p = buffer;
	
	if (real != 0.0)
	{
		GB.NumberToString(FALSE, real, NULL, &str, &len);
		strncpy(p, str, len);
		p += len;
	}
	
	if (imag != 0.0)
	{
		if (imag < 0.0)
		{
			*p++ = '-';
			imag = (-imag);
		}
		else if (p != buffer)
			*p++ = '+';
		
		if (imag != 1.0)
		{
			GB.NumberToString(FALSE, imag, NULL, &str, &len);
			strncpy(p, str, len);
			p += len;
		}
		*p++ = 'i';
	}
	
	GB.ReturnNewString(buffer, p - buffer);
	
END_METHOD


BEGIN_METHOD(GslComplex_Set, GB_FLOAT real; GB_FLOAT imag)
	
	THIS->number.dat[0] = VARG(real);
	THIS->number.dat[1] = VARG(imag);

END_METHOD


BEGIN_METHOD(GslComplex_Rect, GB_FLOAT real; GB_FLOAT imag)

	THIS->number = gsl_complex_rect(VARG(real), VARG(imag));

END_METHOD 


BEGIN_METHOD(GslComplex_Polar, GB_FLOAT real; GB_FLOAT imag)

	THIS->number = gsl_complex_polar(VARG(real), VARG(imag));

END_METHOD 


BEGIN_METHOD_VOID(GslComplex_Arg)

	double r;
	
	r = gsl_complex_arg(THIS->number);

	GB.ReturnFloat(r);
END_METHOD


BEGIN_METHOD_VOID(GslComplex_Abs)

	double r;
	
	r = gsl_complex_abs(THIS->number);

	GB.ReturnFloat(r);
END_METHOD


BEGIN_METHOD_VOID(GslComplex_Abs2)

	double r;
	
	r = gsl_complex_abs2(THIS->number);

	GB.ReturnFloat(r);
END_METHOD
 

BEGIN_METHOD_VOID(GslComplex_LogAbs)

	double r;
	
	r = gsl_complex_logabs(THIS->number);

	GB.ReturnFloat(r);
END_METHOD



/******************************
      Property Methods
******************************/
BEGIN_PROPERTY(GslComplex_Real)

	if (READ_PROPERTY)
		GB.ReturnFloat(THIS->number.dat[0]);
	else
		THIS->number.dat[0] = (VPROP(GB_FLOAT));

END_PROPERTY


BEGIN_PROPERTY(GslComplex_Imagined)

	if (READ_PROPERTY)
		GB.ReturnFloat(THIS->number.dat[1]);
	else
		THIS->number.dat[1] = (VPROP(GB_FLOAT));

END_PROPERTY


/**************************************************
			Complex arithmetic operators
**************************************************/

BEGIN_METHOD(GslComplex_Add, GB_OBJECT x)
	
	GSLCOMPLEX *x = VARG(x);
	GSLCOMPLEX *obj;

	if (GB.CheckObject(x))
		return;
	
	// Create new object
	obj = COMPLEX_create();

	// Add two complex numbers
	obj->number = gsl_complex_add(THIS->number, x->number);

	GB.ReturnObject(obj);

END_METHOD


BEGIN_METHOD(GslComplex_Sub, GB_OBJECT x)
	
	GSLCOMPLEX *x = VARG(x);
	GSLCOMPLEX *obj;

	if (GB.CheckObject(x))
		return;
	
	obj = COMPLEX_create();

	obj->number = gsl_complex_sub(THIS->number, x->number);

	GB.ReturnObject(obj);

END_METHOD


BEGIN_METHOD(GslComplex_Mul, GB_OBJECT x)
	
	GSLCOMPLEX *x = VARG(x);
	GSLCOMPLEX *obj;

	if (GB.CheckObject(x))
		return;
	
	obj = COMPLEX_create();

	obj->number = gsl_complex_mul(THIS->number, x->number);

	GB.ReturnObject(obj);

END_METHOD


BEGIN_METHOD(GslComplex_Div, GB_OBJECT x)
	
	GSLCOMPLEX *x = VARG(x);
	GSLCOMPLEX *obj;

	if (GB.CheckObject(x))
		return;
	
	obj = COMPLEX_create();

	obj->number = gsl_complex_div(THIS->number, x->number);

	GB.ReturnObject(obj);

END_METHOD


/*********************************************
            Operations On Real
*********************************************/
BEGIN_METHOD(GslComplex_Add_Real, GB_FLOAT x)
	
	GSLCOMPLEX *obj;
	
	obj = COMPLEX_create();

	obj->number = gsl_complex_add_real(THIS->number, VARG(x));

	GB.ReturnObject(obj);

END_METHOD


BEGIN_METHOD(GslComplex_Sub_Real, GB_FLOAT x)
	
	GSLCOMPLEX *obj;
	
	obj = COMPLEX_create();

	obj->number = gsl_complex_sub_real(THIS->number, VARG(x));

	GB.ReturnObject(obj);

END_METHOD


BEGIN_METHOD(GslComplex_Mul_Real, GB_FLOAT x)
	
	GSLCOMPLEX *obj;
	
	obj = COMPLEX_create();

	obj->number = gsl_complex_mul_real(THIS->number, VARG(x));

	GB.ReturnObject(obj);

END_METHOD


BEGIN_METHOD(GslComplex_Div_Real, GB_FLOAT x)
	
	GSLCOMPLEX *obj;
	
	obj = COMPLEX_create();

	obj->number = gsl_complex_div_real(THIS->number, VARG(x));

	GB.ReturnObject(obj);

END_METHOD


/*********************************************
            Operations On Imaginary
*********************************************/
BEGIN_METHOD(GslComplex_Add_Imag, GB_FLOAT x)
	
	GSLCOMPLEX *obj;
	
	obj = COMPLEX_create();

	obj->number = gsl_complex_add_imag(THIS->number, VARG(x));

	GB.ReturnObject(obj);

END_METHOD


BEGIN_METHOD(GslComplex_Sub_Imag, GB_FLOAT x)
	
	GSLCOMPLEX *obj;
	
	obj = COMPLEX_create();

	obj->number = gsl_complex_sub_imag(THIS->number, VARG(x));

	GB.ReturnObject(obj);

END_METHOD


BEGIN_METHOD(GslComplex_Mul_Imag, GB_FLOAT x)
	
	GSLCOMPLEX *obj;
	
	obj = COMPLEX_create();

	obj->number = gsl_complex_mul_imag(THIS->number, VARG(x));

	GB.ReturnObject(obj);

END_METHOD


BEGIN_METHOD(GslComplex_Div_Imag, GB_FLOAT x)
	
	GSLCOMPLEX *obj;
	
	obj = COMPLEX_create();

	obj->number = gsl_complex_div_imag(THIS->number, VARG(x));

	GB.ReturnObject(obj);

END_METHOD


BEGIN_METHOD_VOID(GslComplex_Conjugate)
	
	GSLCOMPLEX *obj;
	
	obj = COMPLEX_create();

	obj->number = gsl_complex_conjugate(THIS->number);

	GB.ReturnObject(obj);

END_METHOD

BEGIN_METHOD_VOID(GslComplex_Inverse)
	
	GSLCOMPLEX *obj;
	
	obj = COMPLEX_create();

	obj->number = gsl_complex_inverse(THIS->number);

	GB.ReturnObject(obj);

END_METHOD


BEGIN_METHOD_VOID(GslComplex_Negative)
	
	GSLCOMPLEX *obj;
	
	obj = COMPLEX_create();

	obj->number = gsl_complex_negative(THIS->number);

	GB.ReturnObject(obj);

END_METHOD


/**************************************************
          Elementary Complex Functions
**************************************************/
BEGIN_METHOD_VOID(GslComplex_Sqrt)
	
	GSLCOMPLEX *obj;
	
	obj = COMPLEX_create();

	obj->number = gsl_complex_sqrt(THIS->number);

	GB.ReturnObject(obj);

END_METHOD


BEGIN_METHOD(GslComplex_SqrtReal, GB_FLOAT x;)
	
	GSLCOMPLEX *obj;
	
	obj = COMPLEX_create();

	obj->number = gsl_complex_sqrt_real(VARG(x));

	GB.ReturnObject(obj);

END_METHOD


BEGIN_METHOD(GslComplex_Pow, GB_OBJECT x;)
	
	GSLCOMPLEX *x = VARG(x);
	GSLCOMPLEX *obj;

	if (GB.CheckObject(x))
		return;
	
	obj = COMPLEX_create();

	obj->number = gsl_complex_pow(THIS->number, x->number);

	GB.ReturnObject(obj);

END_METHOD


BEGIN_METHOD(GslComplex_PowReal, GB_FLOAT x;)
	
	GSLCOMPLEX *obj;
	
	obj = COMPLEX_create();

	obj->number = gsl_complex_pow_real(THIS->number, VARG(x));

	GB.ReturnObject(obj);

END_METHOD


BEGIN_METHOD_VOID(GslComplex_Exp)
	
	GSLCOMPLEX *obj;
	
	obj = COMPLEX_create();

	obj->number = gsl_complex_exp(THIS->number);

	GB.ReturnObject(obj);

END_METHOD


BEGIN_METHOD_VOID(GslComplex_Log)
	
	GSLCOMPLEX *obj;
	
	obj = COMPLEX_create();

	obj->number = gsl_complex_log(THIS->number);

	GB.ReturnObject(obj);

END_METHOD


BEGIN_METHOD_VOID(GslComplex_Log10)
	
	GSLCOMPLEX *obj;
	
	obj = COMPLEX_create();

	obj->number = gsl_complex_log10(THIS->number);

	GB.ReturnObject(obj);

END_METHOD


BEGIN_METHOD(GslComplex_Log_b, GB_OBJECT x;)
	
	GSLCOMPLEX *x = VARG(x);
	GSLCOMPLEX *obj;

	if (GB.CheckObject(x))
		return;
	
	obj = COMPLEX_create();

	obj->number = gsl_complex_log_b(THIS->number, x->number);

	GB.ReturnObject(obj);

END_METHOD



/**************************************************
          Complex Trigonometric Functions
**************************************************/

BEGIN_METHOD_VOID(GslComplex_Sin)
	
	GSLCOMPLEX *obj;
	
	obj = COMPLEX_create();

	obj->number = gsl_complex_sin(THIS->number);

	GB.ReturnObject(obj);

END_METHOD


BEGIN_METHOD_VOID(GslComplex_Cos)
	
	GSLCOMPLEX *obj;
	
	obj = COMPLEX_create();

	obj->number = gsl_complex_cos(THIS->number);

	GB.ReturnObject(obj);

END_METHOD


BEGIN_METHOD_VOID(GslComplex_Tan)
	
	GSLCOMPLEX *obj;
	
	obj = COMPLEX_create();

	obj->number = gsl_complex_tan(THIS->number);

	GB.ReturnObject(obj);

END_METHOD


BEGIN_METHOD_VOID(GslComplex_Sec)
	
	GSLCOMPLEX *obj;
	
	obj = COMPLEX_create();

	obj->number = gsl_complex_sec(THIS->number);

	GB.ReturnObject(obj);

END_METHOD


BEGIN_METHOD_VOID(GslComplex_Csc)
	
	GSLCOMPLEX *obj;
	
	obj = COMPLEX_create();

	obj->number = gsl_complex_csc(THIS->number);

	GB.ReturnObject(obj);

END_METHOD


BEGIN_METHOD_VOID(GslComplex_Cot)
	
	GSLCOMPLEX *obj;
	
	obj = COMPLEX_create();

	obj->number = gsl_complex_cot(THIS->number);

	GB.ReturnObject(obj);

END_METHOD



/**************************************************
     Inverse Complex Trigonometric Functions
**************************************************/    

BEGIN_METHOD_VOID(GslComplex_Arcsin)
	GSLCOMPLEX *obj;
	
	obj = COMPLEX_create();

	obj->number = gsl_complex_arcsin(THIS->number);

	GB.ReturnObject(obj);

END_METHOD


BEGIN_METHOD(GslComplex_Arcsin_Real, GB_FLOAT x;)
	
	GSLCOMPLEX *obj;
	
	obj = COMPLEX_create();

	obj->number = gsl_complex_arcsin_real(VARG(x));

	GB.ReturnObject(obj);

END_METHOD


BEGIN_METHOD_VOID(GslComplex_Arccos)
	GSLCOMPLEX *obj;
	
	obj = COMPLEX_create();

	obj->number = gsl_complex_arccos(THIS->number);

	GB.ReturnObject(obj);

END_METHOD


BEGIN_METHOD(GslComplex_Arccos_Real, GB_FLOAT x;)
	
	GSLCOMPLEX *obj;
	
	obj = COMPLEX_create();

	obj->number = gsl_complex_arccos_real(VARG(x));

	GB.ReturnObject(obj);

END_METHOD


BEGIN_METHOD_VOID(GslComplex_Arctan)
	GSLCOMPLEX *obj;
	
	obj = COMPLEX_create();

	obj->number = gsl_complex_arctan(THIS->number);

	GB.ReturnObject(obj);

END_METHOD


BEGIN_METHOD_VOID(GslComplex_Arcsec)
	GSLCOMPLEX *obj;
	
	obj = COMPLEX_create();

	obj->number = gsl_complex_arcsec(THIS->number);

	GB.ReturnObject(obj);

END_METHOD


BEGIN_METHOD(GslComplex_Arcsec_Real, GB_FLOAT x;)
	
	GSLCOMPLEX *obj;
	
	obj = COMPLEX_create();

	obj->number = gsl_complex_arcsec_real(VARG(x));

	GB.ReturnObject(obj);

END_METHOD


BEGIN_METHOD_VOID(GslComplex_Arccsc)
	GSLCOMPLEX *obj;
	
	obj = COMPLEX_create();

	obj->number = gsl_complex_arccsc(THIS->number);

	GB.ReturnObject(obj);

END_METHOD    


BEGIN_METHOD(GslComplex_Arccsc_Real, GB_FLOAT x;)
	
	GSLCOMPLEX *obj;
	
	obj = COMPLEX_create();

	obj->number = gsl_complex_arccsc_real(VARG(x));

	GB.ReturnObject(obj);

END_METHOD


BEGIN_METHOD_VOID(GslComplex_Arccot)
	GSLCOMPLEX *obj;
	
	obj = COMPLEX_create();

	obj->number = gsl_complex_arccot(THIS->number);

	GB.ReturnObject(obj);

END_METHOD



/**************************************************
            Complex Hyperbolic Functions
**************************************************/    

BEGIN_METHOD_VOID(GslComplex_Sinh)
	GSLCOMPLEX *obj;
	
	obj = COMPLEX_create();

	obj->number = gsl_complex_sinh(THIS->number);

	GB.ReturnObject(obj);

END_METHOD


BEGIN_METHOD_VOID(GslComplex_Cosh)
	GSLCOMPLEX *obj;
	
	obj = COMPLEX_create();

	obj->number = gsl_complex_cosh(THIS->number);

	GB.ReturnObject(obj);

END_METHOD


BEGIN_METHOD_VOID(GslComplex_Tanh)
	GSLCOMPLEX *obj;
	
	obj = COMPLEX_create();

	obj->number = gsl_complex_tanh(THIS->number);

	GB.ReturnObject(obj);

END_METHOD


BEGIN_METHOD_VOID(GslComplex_Sech)
	GSLCOMPLEX *obj;
	
	obj = COMPLEX_create();

	obj->number = gsl_complex_sech(THIS->number);

	GB.ReturnObject(obj);

END_METHOD


BEGIN_METHOD_VOID(GslComplex_Csch)
	GSLCOMPLEX *obj;
	
	obj = COMPLEX_create();

	obj->number = gsl_complex_csch(THIS->number);

	GB.ReturnObject(obj);

END_METHOD


BEGIN_METHOD_VOID(GslComplex_Coth)
	GSLCOMPLEX *obj;
	
	obj = COMPLEX_create();

	obj->number = gsl_complex_coth(THIS->number);

	GB.ReturnObject(obj);

END_METHOD



/**************************************************
       Inverse Complex Hyperbolic Functions
**************************************************/    

BEGIN_METHOD_VOID(GslComplex_Arcsinh)
	GSLCOMPLEX *obj;
	
	obj = COMPLEX_create();

	obj->number = gsl_complex_arcsinh(THIS->number);

	GB.ReturnObject(obj);

END_METHOD


BEGIN_METHOD_VOID(GslComplex_Arccosh)
	GSLCOMPLEX *obj;
	
	obj = COMPLEX_create();

	obj->number = gsl_complex_arccosh(THIS->number);

	GB.ReturnObject(obj);

END_METHOD


BEGIN_METHOD(GslComplex_Arccosh_Real, GB_FLOAT x;)
	
	GSLCOMPLEX *obj;
	
	obj = COMPLEX_create();

	obj->number = gsl_complex_arccosh_real(VARG(x));

	GB.ReturnObject(obj);

END_METHOD


BEGIN_METHOD_VOID(GslComplex_Arctanh)
	GSLCOMPLEX *obj;
	
	obj = COMPLEX_create();

	obj->number = gsl_complex_arctanh(THIS->number);

	GB.ReturnObject(obj);

END_METHOD


BEGIN_METHOD(GslComplex_Arctanh_Real, GB_FLOAT x;)
	
	GSLCOMPLEX *obj;
	
	obj = COMPLEX_create();

	obj->number = gsl_complex_arctanh_real(VARG(x));

	GB.ReturnObject(obj);

END_METHOD



BEGIN_METHOD_VOID(GslComplex_Arcsech)
	GSLCOMPLEX *obj;
	
	obj = COMPLEX_create();

	obj->number = gsl_complex_arcsech(THIS->number);

	GB.ReturnObject(obj);

END_METHOD


BEGIN_METHOD_VOID(GslComplex_Arccsch)
	GSLCOMPLEX *obj;
	
	obj = COMPLEX_create();

	obj->number = gsl_complex_arccsch(THIS->number);

	GB.ReturnObject(obj);

END_METHOD


BEGIN_METHOD_VOID(GslComplex_Arccoth)
	GSLCOMPLEX *obj;
	
	obj = COMPLEX_create();

	obj->number = gsl_complex_arccoth(THIS->number);

	GB.ReturnObject(obj);

END_METHOD



/**************************************************
  Describe Class properties and methods to Gambas
**************************************************/
GB_DESC CComplexDesc[] =
{
	GB_DECLARE("Complex", sizeof(GSLCOMPLEX)),
	
	// Utility Methods 
	GB_METHOD("_new", NULL, GslComplex_new, "[(Real)f(Imag)f]"),
	GB_STATIC_METHOD("_call", "Complex", GslComplex_call, "[(Real)f(Imag)f]"),
	GB_METHOD("ToString", "s", GslComplex_ToString, NULL),
	GB_METHOD("Copy", "Complex", GslComplex_Copy, NULL),
	GB_METHOD("Set", NULL, GslComplex_Set, "[(Real)f(Imag)f]"),
	GB_METHOD("Rect", NULL, GslComplex_Rect, "[(Real)f(Imag)f]"),
	GB_METHOD("Polar", NULL, GslComplex_Polar, "[(Real)f(Imag)f]"),
	GB_METHOD("Arg", "f", GslComplex_Arg, NULL),
	
	GB_INTERFACE("_operators", &_operators),
	GB_INTERFACE("_convert", &_convert),
	
	GB_METHOD("Abs", "f", GslComplex_Abs, NULL),
	GB_METHOD("Abs2", "f", GslComplex_Abs2, NULL),
	GB_METHOD("LogAbs", "f", GslComplex_LogAbs, NULL),

	// Properties
	GB_PROPERTY("Real", "f", GslComplex_Real),
	GB_PROPERTY("Imag", "f", GslComplex_Imagined),

	/* Operations on gsl_complex */
	// Elementary Math Functions
	GB_METHOD("Add", "Complex", GslComplex_Add, "(X)Complex"),
	GB_METHOD("Sub", "Complex", GslComplex_Sub, "(X)Complex"),
	GB_METHOD("Mul", "Complex", GslComplex_Mul, "(X)Complex"),
	GB_METHOD("Div", "Complex", GslComplex_Div, "(X)Complex"),
	
	// Operations On Real
	GB_METHOD("AddReal", "Complex", GslComplex_Add_Real, "(X)f"),
	GB_METHOD("SubReal", "Complex", GslComplex_Sub_Real, "(X)f"),
	GB_METHOD("MulReal", "Complex", GslComplex_Mul_Real, "(X)f"),
	GB_METHOD("DivReal", "Complex", GslComplex_Div_Real, "(X)f"),

	// Operations On Imaginary
	GB_METHOD("AddImag", "Complex", GslComplex_Add_Imag, "(X)f"),
	GB_METHOD("SubImag", "Complex", GslComplex_Sub_Imag, "(X)f"),
	GB_METHOD("MulImag", "Complex", GslComplex_Mul_Imag, "(X)f"),
	GB_METHOD("DivImag", "Complex", GslComplex_Div_Imag, "(X)f"),

	// Elementary Complex Functions
    GB_METHOD("Sqrt", "Complex", GslComplex_Sqrt, NULL),
    GB_METHOD("SqrtReal", "Complex", GslComplex_SqrtReal, "(X)f"),
    GB_METHOD("Pow", "Complex", GslComplex_Pow, "(X)Complex"),
    GB_METHOD("PowReal", "Complex", GslComplex_PowReal, "(X)f"),
    GB_METHOD("Exp", "Complex", GslComplex_Exp, NULL),
    GB_METHOD("Log", "Complex", GslComplex_Log, NULL),
    GB_METHOD("Log10", "Complex", GslComplex_Log10, NULL),
    GB_METHOD("Logb", "Complex", GslComplex_Log_b, "(X)Complex"),

	// Complex Trigonometric Functions
    GB_METHOD("Sin", "Complex", GslComplex_Sin, NULL),
    GB_METHOD("Cos", "Complex", GslComplex_Cos, NULL),
    GB_METHOD("Tan", "Complex", GslComplex_Tan, NULL),
    GB_METHOD("Sec", "Complex", GslComplex_Sec, NULL),
    GB_METHOD("Csc", "Complex", GslComplex_Csc, NULL),
    GB_METHOD("Cot", "Complex", GslComplex_Cot, NULL),

    // Inverse Complex Trigonometric Functions
    GB_METHOD("Arcsin", "Complex", GslComplex_Arcsin, NULL),
    GB_STATIC_METHOD("ArcsinReal", "Complex", GslComplex_Arcsin_Real, "(X)f"),
    GB_METHOD("Arccos", "Complex", GslComplex_Arccos, NULL),
    GB_STATIC_METHOD("ArccosReal", "Complex", GslComplex_Arccos_Real, "(X)f"),
    GB_METHOD("Arctan", "Complex", GslComplex_Arctan, NULL),
    GB_METHOD("Arcsec", "Complex", GslComplex_Arcsec, NULL),
    GB_STATIC_METHOD("ArcsecReal", "Complex", GslComplex_Arcsec_Real, "(X)f"),
    GB_METHOD("Arccsc", "Complex", GslComplex_Arccsc, NULL),
    GB_STATIC_METHOD("ArccscReal", "Complex", GslComplex_Arccsc_Real, "(X)f"),
    GB_METHOD("Arccot", "Complex", GslComplex_Arccot, NULL),

    // Complex Hyperbolic Functions
    GB_METHOD("Sinh", "Complex", GslComplex_Sinh, NULL),
    GB_METHOD("Cosh", "Complex", GslComplex_Cosh, NULL),
    GB_METHOD("Tanh", "Complex", GslComplex_Tanh, NULL),
    GB_METHOD("Sech", "Complex", GslComplex_Sech, NULL),
    GB_METHOD("Csch", "Complex", GslComplex_Csch, NULL),
    GB_METHOD("Coth", "Complex", GslComplex_Coth, NULL),

    // Inverse Complex Hyperbolic Functions
    GB_METHOD("Arcsinh", "Complex", GslComplex_Arcsinh, NULL),
    GB_METHOD("Arccosh", "Complex", GslComplex_Arccosh, NULL),
    GB_METHOD("ArccoshReal", "Complex", GslComplex_Arccosh_Real, "(X)f"),
    GB_METHOD("Arctanh", "Complex", GslComplex_Arctanh, NULL),
    GB_METHOD("ArctanhReal", "Complex", GslComplex_Arctanh_Real, "(X)f"),
    GB_METHOD("Arcsech", "Complex", GslComplex_Arcsech, NULL),
    GB_METHOD("Arccsch", "Complex", GslComplex_Arccsch, NULL),
    GB_METHOD("Arccoth", "Complex", GslComplex_Arccoth, NULL),

	GB_END_DECLARE
};


