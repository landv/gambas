/***************************************************************************

  ccomplex.c

  (c) 2000-2013 Benoît Minisini <gambas@users.sourceforge.net>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2, or (at your option)
  any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
  MA 02110-1301, USA.

***************************************************************************/

#define __CCOMPLEX_C

#include "ccomplex.h"

#define THIS ((CCOMPLEX *)_object)
#define RE(_c) ((_c)->v[0])
#define IM(_c) ((_c)->v[1])
#define ABS(_c) (hypot(RE(_c), IM(_c)))
#define ABS2(_c) (RE(_c) * RE(_c) + IM(_c) * IM(_c))
#define ZERO(_c) (RE(_c) == 0.0 && IM(_c) == 0.0)


//---- Complex number creation ----------------------------------------------

CCOMPLEX *COMPLEX_create(double re, double im)
{
	static GB_CLASS CLASS_Complex = (GB_CLASS)NULL;
	CCOMPLEX *c;
	
	if (!CLASS_Complex)
		CLASS_Complex = GB.FindClass("Complex");
	
	c = (CCOMPLEX *)GB.New(CLASS_Complex, NULL, NULL);
	c->v[0] = re;
	c->v[1] = im;
	
	return c;
}

//#define COMPLEX_make(_a, _re, _im) (((_a)->ob.ref <= 1) ? ((_a)->v[0] = (_re), (_a)->v[1] = (_im), (_a)) : COMPLEX_create((_re), (_im)))

static inline CCOMPLEX *COMPLEX_make(CCOMPLEX *a, const double re, const double im)
{
	if (a->ob.ref <= 1)
	{
		a->v[0] = re;
		a->v[1] = im;
		return a;
	}
	else
		return COMPLEX_create(re, im);
}

CCOMPLEX *COMPLEX_push_complex(double value)
{
	return COMPLEX_create(0, value);
}

//---- Arithmetic operators -------------------------------------------------

static CCOMPLEX *_addf(CCOMPLEX *a, double f, bool invert)
{
	return COMPLEX_make(a, RE(a) + f, IM(a));
}

static CCOMPLEX *_add(CCOMPLEX *a, CCOMPLEX *b, bool invert)
{
	return COMPLEX_make(a, RE(a) + RE(b), IM(a) + IM(b));
}

static CCOMPLEX *_subf(CCOMPLEX *a, double f, bool invert)
{
	if (invert)
		return COMPLEX_make(a, f - RE(a), -IM(a));
	else
		return COMPLEX_make(a, RE(a) - f, IM(a));
}

static CCOMPLEX *_sub(CCOMPLEX *a, CCOMPLEX *b, bool invert)
{
	return COMPLEX_make(a, RE(a) - RE(b), IM(a) - IM(b));
}

static CCOMPLEX *_mulf(CCOMPLEX *a, double f, bool invert)
{
	return COMPLEX_make(a, RE(a) * f, IM(a) * f);
}

static CCOMPLEX *_mul(CCOMPLEX *a, CCOMPLEX *b, bool invert)
{
	return COMPLEX_make(a, RE(a) * RE(b) - IM(a) * IM(b), RE(a) * IM(b) + IM(a) * RE(b));
}

static CCOMPLEX *_divf(CCOMPLEX *a, double f, bool invert)
{
	if (invert)
	{
		if (ZERO(a))
			return NULL;
		
		double s = ABS2(a);
		double re, im;

		re = RE(a) / s;
		im = -IM(a) / s;
		
		return COMPLEX_make(a, re * f, im * f);
	}
	else
	{
		if (f == 0.0)
			return NULL;
		
		return COMPLEX_make(a, RE(a) / f, IM(a) / f);
	}
}

static CCOMPLEX *_div(CCOMPLEX *a, CCOMPLEX *b, bool invert)
{
  double ar = RE(a), ai = IM(a);
  double br = RE(b), bi = IM(b);

	if (br == 0.0 && bi == 0.0)
		return NULL;
	
  double s = 1.0 / ABS(b);

  double sbr = s * br;
  double sbi = s * bi;

  double zr = (ar * sbr + ai * sbi) * s;
  double zi = (ai * sbr - ar * sbi) * s;

	return COMPLEX_make(a, zr, zi);
}

static int _equal(CCOMPLEX *a, CCOMPLEX *b, bool invert)
{
	return RE(a) == RE(b) && IM(a) == IM(b);
}

static int _equalf(CCOMPLEX *a, double f, bool invert)
{
	return RE(a) == f && IM(a) == 0;
}

static double _fabs(CCOMPLEX *a)
{
	return ABS(a);
}

static CCOMPLEX *_neg(CCOMPLEX *a)
{
	return COMPLEX_make(a, -RE(a), -IM(a));
}

double _logabs(CCOMPLEX *a)
{
	double xabs = fabs(RE(a));
	double yabs = fabs(IM(a));
	double max, u;

	if (xabs >= yabs)
	{
		max = xabs;
		u = yabs / xabs;
	}
	else
	{
		max = yabs;
		u = xabs / yabs;
	}

	/* Handle underflow when u is close to 0 */
	return log(max) + 0.5 * log1p (u * u);
}

static double _arg(CCOMPLEX *a)
{
	if (ZERO(a))
		return 0.0;
	else
		return atan2(IM(a), RE(a));
}

static CCOMPLEX *_powi(CCOMPLEX *a, int i)
{
	CCOMPLEX *r;
	bool inv;
	
	inv = i < 0;
	i = abs(i);
	
	if (i == 2)
		r = _mul(a, a, FALSE);
	else if (i == 3)
	{
		r = COMPLEX_create(RE(a), IM(a));
		r = _mul(r, a, FALSE);
		r = _mul(r, a, FALSE);
	}
	else if (i == 4)
	{
		a = _mul(a, a, FALSE);
		r = _mul(a, a, FALSE);
	}
	else
		r = COMPLEX_make(a, RE(a), IM(a));
	
	if (inv)
		return _divf(r, 1, TRUE);
	else
		return r;
}

static CCOMPLEX *_pow(CCOMPLEX *a, CCOMPLEX *b)
{
	if (RE(a) == 0.0 && IM(a) == 0.0)
	{
		if (RE(b) == 0.0 && IM(b) == 0.0)
			return COMPLEX_make(a, 1.0, 0.0);
		else 
			return COMPLEX_make(a, 0.0, 0.0);
	}
	else if (IM(b) == 0.0)
	{
		if (RE(b) >= 4.0 && RE(b) <= -4.0 && RE(b) == (int)RE(b))
			return _powi(a, (int)RE(b));
	}
	
	double logr = _logabs (a);
	double theta = _arg(a);

	double br = RE(b), bi = IM(b);

	double rho = exp(logr * br - bi * theta);
	double beta = theta * br + bi * logr;

	return COMPLEX_make(a, rho * cos (beta), rho * sin (beta));
}

static CCOMPLEX *_powf(CCOMPLEX *a, double b)
{
	if (RE(a) == 0.0 && IM(a) == 0.0)
	{
		if (b == 0.0)
			return COMPLEX_make(a, 1.0, 0.0);
		else
			return COMPLEX_make(a, 0.0, 0.0);
	}
	else if (b == 0.0)
		return COMPLEX_make(a, 1.0, 0.0);
	else if (b <= 4.0 && b >= -4.0 && b == (int)b)
		return _powi(a, (int)b);
	else
	{
		double logr = _logabs (a);
		double theta = _arg (a);
		double rho = exp (logr * b);
		double beta = theta * b;
		
		return COMPLEX_make(a, rho * cos(beta), rho * sin(beta));
	}
}

static GB_OPERATOR_DESC _operator =
{
	.equal   = (void *)_equal,
	.equalf  = (void *)_equalf,
	.add     = (void *)_add,
	.addf    = (void *)_addf,
	.sub     = (void *)_sub,
	.subf    = (void *)_subf,
	.mul     = (void *)_mul,
	.mulf    = (void *)_mulf,
	.div     = (void *)_div,
	.divf    = (void *)_divf,
	.pow     = (void *)_pow,
	.powf    = (void *)_powf,
	.fabs    = (void *)_fabs,
	.neg     = (void *)_neg
};

//---- Conversions ----------------------------------------------------------

char *COMPLEX_to_string(double real, double imag, bool local)
{
	char buffer[64];
	char *p;
	char *str;
	int len;
	
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

static bool _convert(CCOMPLEX *a, GB_TYPE type, GB_VALUE *conv)
{
	if (a)
	{
		switch (type)
		{
			case GB_T_FLOAT:
				if (IM(a))
					return TRUE;
				conv->_float.value = RE(a);
				return FALSE;
				
			case GB_T_SINGLE:
				if (IM(a))
					return TRUE;
				conv->_single.value = RE(a);
				return FALSE;
				
			case GB_T_INTEGER:
			case GB_T_SHORT:
			case GB_T_BYTE:
				if (IM(a))
					return TRUE;
				conv->_integer.value = RE(a);
				return FALSE;
				
			case GB_T_LONG:
				if (IM(a))
					return TRUE;
				conv->_long.value = RE(a);
				return FALSE;
				
			case GB_T_STRING:
			case GB_T_CSTRING:
				conv->_string.value.addr = COMPLEX_to_string(RE(a), IM(a), type == GB_T_CSTRING);
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
				conv->_object.value = COMPLEX_create(conv->_float.value, 0);
				return FALSE;

			case GB_T_SINGLE:
				conv->_object.value = COMPLEX_create(conv->_single.value, 0);
				return FALSE;

			case GB_T_LONG:
				conv->_object.value = COMPLEX_create((double)conv->_long.value, 0);
				return FALSE;
				
			case GB_T_INTEGER:
			case GB_T_SHORT:
			case GB_T_BYTE:
				conv->_object.value = COMPLEX_create(conv->_integer.value, 0);
				return FALSE;
				
			default:
				return TRUE;
		}
	}
}

//---------------------------------------------------------------------------

BEGIN_METHOD(Complex_new, GB_FLOAT real; GB_FLOAT imag)

	THIS->v[0] = VARGOPT(real, 0.0);
	THIS->v[1] = VARGOPT(imag, 0.0);

END_METHOD


BEGIN_METHOD(Complex_call, GB_FLOAT real; GB_FLOAT imag)

	GB.ReturnObject(COMPLEX_create(VARG(real), VARG(imag)));

END_METHOD


BEGIN_METHOD_VOID(Complex_Copy)

	GB.ReturnObject(COMPLEX_create(RE(THIS), IM(THIS)));

END_METHOD


BEGIN_METHOD(Complex_Polar, GB_FLOAT abs; GB_FLOAT arg)

	double mod = VARG(abs);
	double arg = VARG(arg);

	GB.ReturnObject(COMPLEX_create(cos(arg) * mod, sin(arg) * mod));

END_METHOD 


BEGIN_METHOD_VOID(Complex_Arg)

	GB.ReturnFloat(_arg(THIS));
	
END_METHOD


BEGIN_METHOD_VOID(Complex_Abs)

	GB.ReturnFloat(ABS(THIS));
	
END_METHOD


BEGIN_METHOD_VOID(Complex_Abs2)

	GB.ReturnFloat(ABS2(THIS));
	
END_METHOD


BEGIN_PROPERTY(Complex_Real)

	if (READ_PROPERTY)
		GB.ReturnFloat(RE(THIS));
	else
		THIS->v[0] = VPROP(GB_FLOAT);

END_PROPERTY


BEGIN_PROPERTY(Complex_Imag)

	if (READ_PROPERTY)
		GB.ReturnFloat(IM(THIS));
	else
		THIS->v[1] = VPROP(GB_FLOAT);

END_PROPERTY


BEGIN_METHOD_VOID(Complex_Inv)

	GB.ReturnObject(_divf(THIS, 1, TRUE));

END_METHOD


BEGIN_METHOD_VOID(Complex_Conj)

	GB.ReturnObject(COMPLEX_create(RE(THIS), -IM(THIS)));

END_METHOD


BEGIN_METHOD(Complex_ToString, GB_BOOLEAN local)

	GB.ReturnString(GB.FreeStringLater(COMPLEX_to_string(RE(THIS), IM(THIS), VARGOPT(local, FALSE))));

END_METHOD

//---------------------------------------------------------------------------

GB_DESC ComplexDesc[] =
{
	GB_DECLARE("Complex", sizeof(CCOMPLEX)),
	
	// Utility Methods 
	GB_METHOD("_new", NULL, Complex_new, "[(Real)f(Imag)f]"),
	GB_STATIC_METHOD("_call", "Complex", Complex_call, "[(Real)f(Imag)f]"),
	GB_STATIC_METHOD("Polar", "Complex", Complex_Polar, "[(Abs)f(Arg)f]"),

	GB_METHOD("Copy", "Complex", Complex_Copy, NULL),
	GB_METHOD("ToString", "s", Complex_ToString, "[(Local)b]"),

	GB_PROPERTY("Real", "f", Complex_Real),
	GB_PROPERTY("Imag", "f", Complex_Imag),

	GB_METHOD("Abs2", "f", Complex_Abs2, NULL),
	GB_METHOD("Arg", "f", Complex_Arg, NULL),

	GB_METHOD("Conj", "Complex", Complex_Conj, NULL),
	GB_METHOD("Inv", "Complex", Complex_Inv, NULL),
	
	GB_INTERFACE("_operator", &_operator),
	GB_INTERFACE("_convert", &_convert),
	
	GB_END_DECLARE
};
