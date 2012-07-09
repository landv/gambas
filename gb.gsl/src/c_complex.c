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

#define __C_COMPLEX_C

#include "c_complex.h"
#include "c_gsl.h"
#include <stdio.h>

#define THIS ((GSLCOMPLEX *)_object)

//---- Complex number creation ----------------------------------------------

GSLCOMPLEX *COMPLEX_create(gsl_complex number)
{
	GSLCOMPLEX *c;
	
	c = (GSLCOMPLEX *)GB.New(CLASS_Complex, NULL, NULL);
	c->number = number;
	
	return c;
}

GSLCOMPLEX *COMPLEX_push_complex(double value)
{
	return COMPLEX_create(gsl_complex_rect(0, value));
}

//---- Arithmetic operators -------------------------------------------------

static GSLCOMPLEX *_addf(GSLCOMPLEX *a, double f)
{
	return COMPLEX_create(gsl_complex_add_real(a->number, f));
}

static GSLCOMPLEX *_add(GSLCOMPLEX *a, GSLCOMPLEX *b)
{
	return COMPLEX_create(gsl_complex_add(a->number, b->number));
}

static GSLCOMPLEX *_subf(GSLCOMPLEX *a, double f)
{
	return COMPLEX_create(gsl_complex_sub_real(a->number, f));
}

static GSLCOMPLEX *_sub(GSLCOMPLEX *a, GSLCOMPLEX *b)
{
	return COMPLEX_create(gsl_complex_sub(a->number, b->number));
}

static GSLCOMPLEX *_mulf(GSLCOMPLEX *a, double f)
{
	return COMPLEX_create(gsl_complex_mul_real(a->number, f));
}

static GSLCOMPLEX *_mul(GSLCOMPLEX *a, GSLCOMPLEX *b)
{
	return COMPLEX_create(gsl_complex_mul(a->number, b->number));
}

static GSLCOMPLEX *_divf(GSLCOMPLEX *a, double f)
{
	gsl_complex c = gsl_complex_div_real(a->number, f);
	
	if (isfinite(c.dat[0]) && isfinite(c.dat[1]))
		return COMPLEX_create(c);
	else
		return NULL;
}

static GSLCOMPLEX *_idivf(GSLCOMPLEX *a, double f)
{
	gsl_complex c = gsl_complex_inverse(a->number);
	
	if (isfinite(c.dat[0]) && isfinite(c.dat[1]))
		return COMPLEX_create(gsl_complex_mul_real(c, f));
	else
		return NULL;
}

static GSLCOMPLEX *_div(GSLCOMPLEX *a, GSLCOMPLEX *b)
{
	gsl_complex c = gsl_complex_div(a->number, b->number);
	
	if (isfinite(c.dat[0]) && isfinite(c.dat[1]))
		return COMPLEX_create(c);
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
	return COMPLEX_create(gsl_complex_negative(a->number));
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

char *COMPLEX_to_string(gsl_complex number, bool local)
{
	char buffer[64];
	char *p;
	char *str;
	int len;
	double real, imag;
	
	real = number.dat[0];
	imag = number.dat[1];
	
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
	if (a)
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
				conv->_string.value.addr = COMPLEX_to_string(a->number, type == GB_T_CSTRING);
				conv->_string.value.start = 0;
				conv->_string.value.len = GB.StringLength(conv->_string.value.addr);
				return FALSE;
				
			default:
				return TRUE;
		}
	}
	else
	{
		switch(type)
		{
			case GB_T_FLOAT:
				conv->_object.value = COMPLEX_create(gsl_complex_rect(conv->_float.value, 0));
				return FALSE;

			case GB_T_SINGLE:
				conv->_object.value = COMPLEX_create(gsl_complex_rect(conv->_single.value, 0));
				return FALSE;

			case GB_T_INTEGER:
			case GB_T_SHORT:
			case GB_T_BYTE:
				conv->_object.value = COMPLEX_create(gsl_complex_rect(conv->_integer.value, 0));
				return FALSE;
				
			default:
				return TRUE;
		}
	}
}

//---------------------------------------------------------------------------

BEGIN_METHOD(Complex_new, GB_FLOAT real; GB_FLOAT imag)

	THIS->number.dat[0] = VARGOPT(real, 0.0);
	THIS->number.dat[1] = VARGOPT(imag, 0.0);

END_METHOD


BEGIN_METHOD(Complex_call, GB_FLOAT real; GB_FLOAT imag)

	GB.ReturnObject(COMPLEX_create(gsl_complex_rect(VARG(real), VARG(imag))));

END_METHOD


BEGIN_METHOD_VOID(Complex_Copy)

	GB.ReturnObject(COMPLEX_create(THIS->number));

END_METHOD


BEGIN_METHOD(Complex_Polar, GB_FLOAT real; GB_FLOAT imag)

	GB.ReturnObject(COMPLEX_create(gsl_complex_polar(VARG(real), VARG(imag))));

END_METHOD 


BEGIN_METHOD_VOID(Complex_Arg)

	GB.ReturnFloat(gsl_complex_arg(THIS->number));
	
END_METHOD


BEGIN_METHOD_VOID(Complex_Abs)

	GB.ReturnFloat(gsl_complex_abs(THIS->number));
	
END_METHOD


BEGIN_METHOD_VOID(Complex_Abs2)

	GB.ReturnFloat(gsl_complex_abs2(THIS->number));
	
END_METHOD
 

BEGIN_METHOD_VOID(Complex_LogAbs)

	GB.ReturnFloat(gsl_complex_logabs(THIS->number));
	
END_METHOD



/******************************
      Property Methods
******************************/

BEGIN_PROPERTY(Complex_Real)

	if (READ_PROPERTY)
		GB.ReturnFloat(THIS->number.dat[0]);
	else
		THIS->number.dat[0] = (VPROP(GB_FLOAT));

END_PROPERTY


BEGIN_PROPERTY(Complex_Imagined)

	if (READ_PROPERTY)
		GB.ReturnFloat(THIS->number.dat[1]);
	else
		THIS->number.dat[1] = (VPROP(GB_FLOAT));

END_PROPERTY


/**************************************************
			Complex arithmetic operators
**************************************************/

#define IMPLEMENT_OP(_name, _func) \
BEGIN_METHOD(Complex_##_name, GB_OBJECT x) \
	\
	GSLCOMPLEX *x = VARG(x); \
	\
	if (GB.CheckObject(x)) \
		return; \
	\
	GB.ReturnObject(COMPLEX_create(_func(THIS->number, x->number))); \
	\
END_METHOD

IMPLEMENT_OP(Add, gsl_complex_add)
IMPLEMENT_OP(Sub, gsl_complex_sub)
IMPLEMENT_OP(Mul, gsl_complex_mul)
IMPLEMENT_OP(Div, gsl_complex_div)


/*********************************************
            Operations On Real
*********************************************/

#define IMPLEMENT_OP_REAL(_name, _func) \
BEGIN_METHOD(Complex_##_name, GB_FLOAT x) \
	\
	GB.ReturnObject(COMPLEX_create(_func(THIS->number, VARG(x)))); \
	\
END_METHOD

IMPLEMENT_OP_REAL(AddReal, gsl_complex_add_real)
IMPLEMENT_OP_REAL(SubReal, gsl_complex_sub_real)
IMPLEMENT_OP_REAL(MulReal, gsl_complex_mul_real)
IMPLEMENT_OP_REAL(DivReal, gsl_complex_div_real)


/*********************************************
            Operations On Imaginary
*********************************************/

IMPLEMENT_OP_REAL(AddImag, gsl_complex_add_imag)
IMPLEMENT_OP_REAL(SubImag, gsl_complex_sub_imag)
IMPLEMENT_OP_REAL(MulImag, gsl_complex_mul_imag)
IMPLEMENT_OP_REAL(DivImag, gsl_complex_div_imag)

#define IMPLEMENT_FUNC(_name, _func) \
BEGIN_METHOD_VOID(Complex_##_name) \
	\
	GB.ReturnObject(COMPLEX_create(_func(THIS->number))); \
	\
END_METHOD

IMPLEMENT_FUNC(Conjugate, gsl_complex_conjugate)
IMPLEMENT_FUNC(Inverse, gsl_complex_inverse)
IMPLEMENT_FUNC(Negative, gsl_complex_negative)

/**************************************************
          Elementary Complex Functions
**************************************************/

IMPLEMENT_FUNC(Sqrt, gsl_complex_sqrt)

#define IMPLEMENT_FUNC_REAL(_name, _func) \
BEGIN_METHOD(Complex_##_name, GB_FLOAT x) \
	\
	GB.ReturnObject(COMPLEX_create(_func(VARG(x)))); \
	\
END_METHOD

IMPLEMENT_FUNC_REAL(SqrtReal, gsl_complex_sqrt_real)

IMPLEMENT_OP(Pow, gsl_complex_pow)
IMPLEMENT_OP_REAL(PowReal, gsl_complex_pow_real)

IMPLEMENT_FUNC(Exp, gsl_complex_exp)
IMPLEMENT_FUNC(Log, gsl_complex_log)
IMPLEMENT_FUNC(Log10, gsl_complex_log10)
IMPLEMENT_OP(LogB, gsl_complex_log_b)

/**************************************************
          Complex Trigonometric Functions
**************************************************/

IMPLEMENT_FUNC(Sin, gsl_complex_sin)
IMPLEMENT_FUNC(Cos, gsl_complex_cos)
IMPLEMENT_FUNC(Tan, gsl_complex_tan)
IMPLEMENT_FUNC(Sec, gsl_complex_sec)
IMPLEMENT_FUNC(Csc, gsl_complex_csc)
IMPLEMENT_FUNC(Cot, gsl_complex_cot)

/**************************************************
     Inverse Complex Trigonometric Functions
**************************************************/    

IMPLEMENT_FUNC(Arcsin, gsl_complex_arcsin)
IMPLEMENT_FUNC_REAL(ArcsinReal, gsl_complex_arcsin_real)
IMPLEMENT_FUNC(Arccos, gsl_complex_arccos)
IMPLEMENT_FUNC_REAL(ArccosReal, gsl_complex_arccos_real)
IMPLEMENT_FUNC(Arctan, gsl_complex_arctan)
IMPLEMENT_FUNC(Arcsec, gsl_complex_arcsec)
IMPLEMENT_FUNC_REAL(ArcsecReal, gsl_complex_arcsec_real)
IMPLEMENT_FUNC(Arccsc, gsl_complex_arccsc)
IMPLEMENT_FUNC_REAL(ArccscReal, gsl_complex_arccsc_real)
IMPLEMENT_FUNC(Arccot, gsl_complex_arccot)

/**************************************************
            Complex Hyperbolic Functions
**************************************************/    

IMPLEMENT_FUNC(Sinh, gsl_complex_sinh)
IMPLEMENT_FUNC(Cosh, gsl_complex_cosh)
IMPLEMENT_FUNC(Tanh, gsl_complex_tanh)
IMPLEMENT_FUNC(Sech, gsl_complex_sech)
IMPLEMENT_FUNC(Csch, gsl_complex_csch)
IMPLEMENT_FUNC(Coth, gsl_complex_coth)

/**************************************************
       Inverse Complex Hyperbolic Functions
**************************************************/    

IMPLEMENT_FUNC(Arcsinh, gsl_complex_arcsinh)
IMPLEMENT_FUNC(Arccosh, gsl_complex_arccosh)
IMPLEMENT_FUNC_REAL(ArccoshReal, gsl_complex_arccosh_real)
IMPLEMENT_FUNC(Arctanh, gsl_complex_arctanh)
IMPLEMENT_FUNC_REAL(ArctanhReal, gsl_complex_arctanh_real)
IMPLEMENT_FUNC(Arcsech, gsl_complex_arcsech)
IMPLEMENT_FUNC(Arccsch, gsl_complex_arccsch)
IMPLEMENT_FUNC(Arccoth, gsl_complex_arccoth)

/**************************************************
  Describe Class properties and methods to Gambas
**************************************************/

GB_DESC ComplexDesc[] =
{
	GB_DECLARE("Complex", sizeof(GSLCOMPLEX)),
	
	// Utility Methods 
	GB_METHOD("_new", NULL, Complex_new, "[(Real)f(Imag)f]"),
	GB_STATIC_METHOD("_call", "Complex", Complex_call, "[(Real)f(Imag)f]"),
	GB_METHOD("Copy", "Complex", Complex_Copy, NULL),
	GB_STATIC_METHOD("Polar", "Complex", Complex_Polar, "[(Real)f(Imag)f]"),
	//GB_METHOD("Set", NULL, Complex_Set, "[(Real)f(Imag)f]"),
	
	// Properties
	GB_PROPERTY("Real", "f", Complex_Real),
	GB_PROPERTY("Imag", "f", Complex_Imagined),

	GB_METHOD("Abs", "f", Complex_Abs, NULL),
	GB_METHOD("Abs2", "f", Complex_Abs2, NULL),
	GB_METHOD("LogAbs", "f", Complex_LogAbs, NULL),
	GB_METHOD("Arg", "f", Complex_Arg, NULL),

	/* Operations on gsl_complex */
	// Elementary Math Functions
	GB_METHOD("Add", "Complex", Complex_Add, "(X)Complex"),
	GB_METHOD("Sub", "Complex", Complex_Sub, "(X)Complex"),
	GB_METHOD("Mul", "Complex", Complex_Mul, "(X)Complex"),
	GB_METHOD("Div", "Complex", Complex_Div, "(X)Complex"),
	
	// Operations On Real
	GB_METHOD("AddReal", "Complex", Complex_AddReal, "(X)f"),
	GB_METHOD("SubReal", "Complex", Complex_SubReal, "(X)f"),
	GB_METHOD("MulReal", "Complex", Complex_MulReal, "(X)f"),
	GB_METHOD("DivReal", "Complex", Complex_DivReal, "(X)f"),

	// Operations On Imaginary
	GB_METHOD("AddImag", "Complex", Complex_AddImag, "(X)f"),
	GB_METHOD("SubImag", "Complex", Complex_SubImag, "(X)f"),
	GB_METHOD("MulImag", "Complex", Complex_MulImag, "(X)f"),
	GB_METHOD("DivImag", "Complex", Complex_DivImag, "(X)f"),

	// Elementary Complex Functions
	GB_METHOD("Sqrt", "Complex", Complex_Sqrt, NULL),
	GB_STATIC_METHOD("SqrtReal", "Complex", Complex_SqrtReal, "(X)f"),
	GB_METHOD("Pow", "Complex", Complex_Pow, "(X)Complex"),
	GB_METHOD("PowReal", "Complex", Complex_PowReal, "(X)f"),
	GB_METHOD("Exp", "Complex", Complex_Exp, NULL),
	GB_METHOD("Log", "Complex", Complex_Log, NULL),
	GB_METHOD("Log10", "Complex", Complex_Log10, NULL),
	GB_METHOD("LogB", "Complex", Complex_LogB, "(X)Complex"),

	// Complex Trigonometric Functions
	GB_METHOD("Sin", "Complex", Complex_Sin, NULL),
	GB_METHOD("Cos", "Complex", Complex_Cos, NULL),
	GB_METHOD("Tan", "Complex", Complex_Tan, NULL),
	GB_METHOD("Sec", "Complex", Complex_Sec, NULL),
	GB_METHOD("Csc", "Complex", Complex_Csc, NULL),
	GB_METHOD("Cot", "Complex", Complex_Cot, NULL),

	// Inverse Complex Trigonometric Functions
	GB_METHOD("Arcsin", "Complex", Complex_Arcsin, NULL),
	GB_STATIC_METHOD("ArcsinReal", "Complex", Complex_ArcsinReal, "(X)f"),
	GB_METHOD("Arccos", "Complex", Complex_Arccos, NULL),
	GB_STATIC_METHOD("ArccosReal", "Complex", Complex_ArccosReal, "(X)f"),
	GB_METHOD("Arctan", "Complex", Complex_Arctan, NULL),
	GB_METHOD("Arcsec", "Complex", Complex_Arcsec, NULL),
	GB_STATIC_METHOD("ArcsecReal", "Complex", Complex_ArcsecReal, "(X)f"),
	GB_METHOD("Arccsc", "Complex", Complex_Arccsc, NULL),
	GB_STATIC_METHOD("ArccscReal", "Complex", Complex_ArccscReal, "(X)f"),
	GB_METHOD("Arccot", "Complex", Complex_Arccot, NULL),

	// Complex Hyperbolic Functions
	GB_METHOD("Sinh", "Complex", Complex_Sinh, NULL),
	GB_METHOD("Cosh", "Complex", Complex_Cosh, NULL),
	GB_METHOD("Tanh", "Complex", Complex_Tanh, NULL),
	GB_METHOD("Sech", "Complex", Complex_Sech, NULL),
	GB_METHOD("Csch", "Complex", Complex_Csch, NULL),
	GB_METHOD("Coth", "Complex", Complex_Coth, NULL),

	// Inverse Complex Hyperbolic Functions
	GB_METHOD("Arcsinh", "Complex", Complex_Arcsinh, NULL),
	GB_METHOD("Arccosh", "Complex", Complex_Arccosh, NULL),
	GB_STATIC_METHOD("ArccoshReal", "Complex", Complex_ArccoshReal, "(X)f"),
	GB_METHOD("Arctanh", "Complex", Complex_Arctanh, NULL),
	GB_STATIC_METHOD("ArctanhReal", "Complex", Complex_ArctanhReal, "(X)f"),
	GB_METHOD("Arcsech", "Complex", Complex_Arcsech, NULL),
	GB_METHOD("Arccsch", "Complex", Complex_Arccsch, NULL),
	GB_METHOD("Arccoth", "Complex", Complex_Arccoth, NULL),

	GB_INTERFACE("_operators", &_operators),
	GB_INTERFACE("_convert", &_convert),
	
	GB_END_DECLARE
};
