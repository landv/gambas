/***************************************************************************

	c_polynomial.c

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

#define __C_POLYNOMIAL_C

#include "c_polynomial.h"
#include "c_complex.h"

#define THIS ((CPOLYNOMIAL *)_object)
#define DATA(_p) ((double *)(_p)->data)
#define CDATA(_p) ((gsl_complex *)(_p)->data)
#define COUNT(_p) ((_p)->size)
#define COMPLEX(_p) ((_p)->complex)

//---- Utility methods ------------------------------------------------------

static CPOLYNOMIAL *POLYNOMIAL_create(int size, bool complex)
{
	GB.Push(2, GB_T_INTEGER, size, GB_T_BOOLEAN, complex);
	return (CPOLYNOMIAL *)GB.New(CLASS_Polynomial, NULL, (void *)(intptr_t)2);
}

static int get_degree(CPOLYNOMIAL *_object)
{
	int i;
	
	if (COMPLEX(THIS))
	{
		gsl_complex *d = CDATA(THIS);
		
		for (i = COUNT(THIS) - 1; i >= 0; i--)
		{
			if (GSL_REAL(d[i]) != 0.0 || GSL_IMAG(d[i]) != 0.0)
				return i;
		}
	}
	else
	{
		double *d = DATA(THIS);
		
		for (i = COUNT(THIS) - 1; i >= 0; i--)
		{
			if (d[i] != 0.0)
				return i;
		}
	}
	
	return 0;
}

static void ensure_size(CPOLYNOMIAL *_object, int size)
{
	if (size > COUNT(THIS))
	{
		GB.Insert(POINTER(&THIS->data), -1, size - COUNT(THIS));
		THIS->size = size;
	}
}

static void ensure_complex(CPOLYNOMIAL *_object)
{
	gsl_complex *d;
	int size, i;
	
	if (COMPLEX(THIS))
		return;
	
	if (THIS->data)
	{
		size = COUNT(THIS);
		GB.NewArray(POINTER(&d), sizeof(gsl_complex), size);
		for (i = 0; i < size; i++)
			d[i].dat[0] = DATA(THIS)[i];
		GB.FreeArray(POINTER(&THIS->data));
		THIS->data = d;
	}
	
	THIS->complex = TRUE;
}

static bool ensure_not_complex(CPOLYNOMIAL *_object)
{
	gsl_complex *cd;
	double *d;
	int size, i;
	
	if (!COMPLEX(THIS))
		return FALSE;
	
	if (THIS->data)
	{
		size = COUNT(THIS);
		cd = CDATA(THIS);
		
		for (i = 0; i < size; i++)
		{
			if (GSL_IMAG(cd[i]) != 0.0)
				return TRUE;
		}
		
		GB.NewArray(POINTER(&d), sizeof(double), size);
		
		for (i = 0; i < size; i++)
			d[i] = GSL_REAL(cd[i]);
		
		GB.FreeArray(POINTER(&THIS->data));
		THIS->data = d;
	}
	
	THIS->complex = FALSE;
	return FALSE;
}

//---- Arithmetic operators -------------------------------------------------

#if 0
static CPOLYNOMIAL *_addf(CPOLYNOMIAL *a, double f)
{
	return COMPLEX_create(gsl_complex_add_real(a->number, f));
}

static CPOLYNOMIAL *_add(CPOLYNOMIAL *a, CPOLYNOMIAL *b)
{
	return COMPLEX_create(gsl_complex_add(a->number, b->number));
}

static CPOLYNOMIAL *_subf(CPOLYNOMIAL *a, double f)
{
	return COMPLEX_create(gsl_complex_sub_real(a->number, f));
}

static CPOLYNOMIAL *_sub(CPOLYNOMIAL *a, CPOLYNOMIAL *b)
{
	return COMPLEX_create(gsl_complex_sub(a->number, b->number));
}

static CPOLYNOMIAL *_mulf(CPOLYNOMIAL *a, double f)
{
	return COMPLEX_create(gsl_complex_mul_real(a->number, f));
}

static CPOLYNOMIAL *_mul(CPOLYNOMIAL *a, CPOLYNOMIAL *b)
{
	return COMPLEX_create(gsl_complex_mul(a->number, b->number));
}

static CPOLYNOMIAL *_divf(CPOLYNOMIAL *a, double f)
{
	gsl_complex c = gsl_complex_div_real(a->number, f);
	
	if (isfinite(c.dat[0]) && isfinite(c.dat[1]))
		return COMPLEX_create(c);
	else
		return NULL;
}

static CPOLYNOMIAL *_idivf(CPOLYNOMIAL *a, double f)
{
	gsl_complex c = gsl_complex_inverse(a->number);
	
	if (isfinite(c.dat[0]) && isfinite(c.dat[1]))
		return COMPLEX_create(gsl_complex_mul_real(c, f));
	else
		return NULL;
}

static CPOLYNOMIAL *_div(CPOLYNOMIAL *a, CPOLYNOMIAL *b)
{
	gsl_complex c = gsl_complex_div(a->number, b->number);
	
	if (isfinite(c.dat[0]) && isfinite(c.dat[1]))
		return COMPLEX_create(c);
	else
		return NULL;
}

static int _equal(CPOLYNOMIAL *a, CPOLYNOMIAL *b)
{
	return a->number.dat[0] == b->number.dat[0] && a->number.dat[1] == b->number.dat[1];
}

static int _equalf(CPOLYNOMIAL *a, double f)
{
	return a->number.dat[0] == f && a->number.dat[1] == 0.0;
}

static CPOLYNOMIAL *_neg(CPOLYNOMIAL *a)
{
	return COMPLEX_create(gsl_complex_negative(a->number));
}

static double _abs(CPOLYNOMIAL *a)
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
#endif

//---- Conversions ----------------------------------------------------------

char *POLYNOMIAL_to_string(CPOLYNOMIAL *p, bool local)
{
	int i;
	int size = COUNT(p);
	char *result = NULL;
	char *str;
	int len;
	char buffer[16];
	double re, im;
	bool add = FALSE;
	bool complex = COMPLEX(p);
	gsl_complex c;
	
	i = size;
	while (i > 0)
	{
		i--;
		
		if (complex)
		{
			c = CDATA(p)[i];
			re = GSL_REAL(c);
			im = GSL_IMAG(c);
		}
		else
		{
			re = DATA(p)[i];
			im = 0.0;
		}
		
		if (re == 0.0 && im == 0.0)
			continue;
		
		if (!add)
			add = TRUE;
		else if (re > 0.0 || (re == 0.0 && im > 0.0))
			result = GB.AddChar(result, '+');
		
		if (i == 0 || re != 1.0 || im != 0.0)
		{
			if (re != 0.0)
			{
				GB.NumberToString(local, re, NULL, &str, &len);
				result = GB.AddString(result, str, len);
			}
			if (im != 0.0)
			{
				if (re != 0.0 && im > 0.0)
					result = GB.AddChar(result, '+');
				
				if (im != 1.0)
				{
					GB.NumberToString(local, im, NULL, &str, &len);
					result = GB.AddString(result, str, len);
				}
				result = GB.AddChar(result, 'i');
			}
		}
		
		if (i > 0)
		{
			result = GB.AddChar(result, 'x');
			if (i > 1)
			{
				result = GB.AddChar(result, '^');
				len = sprintf(buffer, "%d", i);
				result = GB.AddString(result, buffer, len);
			}
		}
	}
	
	if (!result)
		result = GB.NewString("0", 1);
	
	return result;
}

static bool _convert(CPOLYNOMIAL *a, GB_TYPE type, GB_VALUE *conv)
{
	if (a)
	{
		switch (type)
		{
			case GB_T_STRING:
			case GB_T_CSTRING:
				conv->_string.value.addr = POLYNOMIAL_to_string(a, type == GB_T_CSTRING);
				conv->_string.value.start = 0;
				conv->_string.value.len = GB.StringLength(conv->_string.value.addr);
				return FALSE;
				
			default:
				return TRUE;
		}
	}
	else
	{
		double coef;
		
		switch(type)
		{
			case GB_T_FLOAT: coef = conv->_float.value; break;
			case GB_T_SINGLE: coef = conv->_single.value; break;
			case GB_T_INTEGER: case GB_T_SHORT: 	case GB_T_BYTE: coef = conv->_integer.value;
				return FALSE;
				
			default:
				if (type >= GB_T_OBJECT)
				{
					if (GB.Is(conv->_object.value, GB.FindClass("Array")))
					{
						CPOLYNOMIAL *p;
						CCOMPLEX *c;
						GB_ARRAY array = (GB_ARRAY)conv->_object.value;
						int size = GB.Array.Count(array);
						int i;
						GB_VALUE temp;
						void *data;
						GB_TYPE atype = GB.Array.Type(array);
						
						if (atype > GB_T_BOOLEAN && atype <= GB_T_FLOAT)
						{
							p = POLYNOMIAL_create(size, FALSE);
							
							for (i = 0; i < size; i++)
							{
								data = GB.Array.Get(array, i);
								GB.ReadValue(&temp, data, atype);
								GB.Conv(&temp, GB_T_FLOAT);
								DATA(p)[i] = temp._float.value;
							}
							
							conv->_object.value = p;
							return FALSE;
						}
						else if (atype == GB_T_VARIANT)
						{
							p = POLYNOMIAL_create(size, TRUE);
							
							for (i = 0; i < size; i++)
							{
								GB.ReadValue(&temp, GB.Array.Get(array, i), atype);
								GB.BorrowValue(&temp);
								GB.Conv(&temp, CLASS_Complex);
								c = temp._object.value;
								CDATA(p)[i] = c->number;
								GB.ReleaseValue(&temp);
							}
							
							conv->_object.value = p;
							return FALSE;
						}
						else if (atype == CLASS_Complex)
						{
							p = POLYNOMIAL_create(size, TRUE);
							
							for (i = 0; i < size; i++)
							{
								c = *(CCOMPLEX **)GB.Array.Get(array, i);
								if (c)
									CDATA(p)[i] = c->number;
								else
									CDATA(p)[i] = COMPLEX_zero;
							}
							
							conv->_object.value = p;
							return FALSE;
						}
					}
				}
				
				return TRUE;
		}
		
		a = POLYNOMIAL_create(1, FALSE);
		*DATA(a) = coef;
		conv->_object.value = a;
		return FALSE;
	}
}

//---------------------------------------------------------------------------

BEGIN_METHOD(Polynomial_new, GB_INTEGER size; GB_BOOLEAN complex)

	bool complex = VARGOPT(complex, FALSE);
	int size = VARGOPT(size, 0);
	
	if (size > 0)
		GB.NewArray(POINTER(&THIS->data), complex ? sizeof(gsl_complex) : sizeof(double), size);
	
	THIS->size = size;
	THIS->complex = complex;
	
END_METHOD


BEGIN_METHOD_VOID(Polynomial_free)

	GB.FreeArray(POINTER(&THIS->data));

END_METHOD


BEGIN_PROPERTY(Polynomial_Count)

	GB.ReturnInteger(COUNT(THIS));

END_PROPERTY


BEGIN_PROPERTY(Polynomial_Degree)

	GB.ReturnInteger(get_degree(THIS));

END_PROPERTY


BEGIN_METHOD_VOID(Polynomial_ToString)

	GB.ReturnString(POLYNOMIAL_to_string(THIS, FALSE));

END_METHOD


BEGIN_METHOD(Polynomial_get, GB_INTEGER index)

	int index = VARG(index);
	
	if (index < 0 || index >= COUNT(THIS))
	{
		if (COMPLEX(THIS))
			GB.ReturnObject(COMPLEX_create(COMPLEX_zero));
		else
			GB.ReturnFloat(0.0);
	}
	else
	{
		if (COMPLEX(THIS))
			GB.ReturnObject(COMPLEX_create(CDATA(THIS)[index]));
		else
			GB.ReturnFloat(DATA(THIS)[index]);
	}

	GB.ReturnConvVariant();

END_METHOD


BEGIN_METHOD(Polynomial_put, GB_VARIANT value; GB_INTEGER index)

	int index = VARG(index);
	GB_VALUE *value = (GB_VALUE *)ARG(value);
	int type;
	gsl_complex z;
	double x;
	
	if (index < 0 || index > 65535)
	{
		GB.Error(GB_ERR_ARG);
		return;
	}
	
	type = COMPLEX_get_value(value, &x, &z);
	
	if (type == CGV_ERR)
		return;
	
	ensure_size(THIS, index + 1);
	
	if (type == CGV_COMPLEX)
	{
		ensure_complex(THIS);
		CDATA(THIS)[index] = z;
	}
	else
	{
		if (COMPLEX(THIS))
			CDATA(THIS)[index] = gsl_complex_rect(x, 0);
		else
			DATA(THIS)[index] = x;
	}
	
END_METHOD


BEGIN_METHOD(Polynomial_Eval, GB_VARIANT value)

	GB_VALUE *value = (GB_VALUE *)ARG(value);
	int type;
	double x;
	gsl_complex z;

	type = COMPLEX_get_value(value, &x, &z);
	if (type == CGV_ERR)
		return;
									 
	if (COMPLEX(THIS))
	{
		GB.ReturnObject(COMPLEX_create(gsl_complex_poly_complex_eval(CDATA(THIS), COUNT(THIS), z)));
	}
	else
	{
		if (type == CGV_COMPLEX)
			GB.ReturnObject(COMPLEX_create(gsl_poly_complex_eval(DATA(THIS), COUNT(THIS), z)));
		else
			GB.ReturnFloat(gsl_poly_eval(DATA(THIS), COUNT(THIS), x));
	}

END_METHOD


BEGIN_METHOD(Polynomial_Solve, GB_BOOLEAN want_croot)

	bool want_croot = VARGOPT(want_croot, FALSE);
	bool complex = COMPLEX(THIS);
	
	double *data = DATA(THIS);
	gsl_complex *cdata = CDATA(THIS);
	
	int nr = 0, i, dg, ret;
	
	GB_ARRAY result;
	
	double r[3];
	gsl_complex cr[3];
	
	double *z = NULL;
	gsl_poly_complex_workspace *work;
	
	double *root;
	CCOMPLEX **croot;
	
	dg = get_degree(THIS) + 1;
	
	if (dg > 2)
	{
		if (complex && ensure_not_complex(THIS))
		{
			GB.Error("Cannot solve polynomial with complex coefficients");
			return;
		}
		
		data = DATA(THIS);
	}
	
	switch(dg)
	{
		case 1:
			GB.ReturnNull();
			return;
			
		case 2:
			nr = 1;
			if (complex)
			{
				cr[0] = gsl_complex_div(gsl_complex_negative(cdata[0]), cdata[1]);
				if (!want_croot)
				{
					if (GSL_IMAG(cr[0]) == 0.0)
						r[0] = GSL_REAL(cr[0]);
					else
						nr = 0;
				}
			}
			else
			{
				r[0] = -data[0] / data[1];
				if (want_croot)
					cr[0] = gsl_complex_rect(r[0], 0);
			}
			
			break;
		
		case 3:
			if (want_croot)
				nr = gsl_poly_complex_solve_quadratic(data[2], data[1], data[0], &cr[0], &cr[1]);
			else
				nr = gsl_poly_solve_quadratic(data[2], data[1], data[0], &r[0], &r[1]);
			
			break;
			
		case 4:
			if (data[3] == 1.0)
			{
				if (want_croot)
					nr = gsl_poly_complex_solve_cubic(data[2], data[1], data[0], &cr[0], &cr[1], &cr[2]);
				else
					nr = gsl_poly_solve_cubic(data[2], data[1], data[0], &r[0], &r[1], &r[2]);
				break;
			}
			
		default:
			work = gsl_poly_complex_workspace_alloc(dg);
			GB.Alloc(POINTER(&z), sizeof(double) * (dg - 1) * 2);
			
			ret = gsl_poly_complex_solve(data, dg, work, z);
			
			gsl_poly_complex_workspace_free(work);
			
			if (ret != GSL_SUCCESS)
			{
				GB.Free(POINTER(&z));
				return;
			}
			
			if (!want_croot)
			{
				nr = 0;
				for (i = 0; i < (dg - 1); i++)
				{
					if (z[i * 2 + 1] == 0.0)
						nr++;
				}
			}
			else
				nr = dg - 1;
	}
	
	if (!want_croot)
	{
		GB.Array.New(&result, GB_T_FLOAT, nr);
		
		if (nr > 0)
			root = (double *)GB.Array.Get(result, 0);
		
		if (!z)
		{
			if (nr >= 1) root[0] = r[0];
			if (nr >= 2) root[1] = r[1];
			if (nr >= 3) root[2] = r[2];
		}
		else
		{
			if (nr > 0)
			{
				nr = 0;
				
				for (i = 0; i < (dg - 1); i++)
				{
					if (z[i * 2 + 1] == 0.0)
						root[nr++] = z[i * 2];
				}
			}
			
			GB.Free(POINTER(&z));
		}
	}
	else
	{
		GB.Array.New(&result, CLASS_Complex, nr);
		
		if (nr > 0)
			croot = (CCOMPLEX **)GB.Array.Get(result, 0);
		else
			croot = NULL;
		
		if (!z)
		{
			if (nr >= 1) croot[0] = COMPLEX_create(cr[0]);
			if (nr >= 2) croot[1] = COMPLEX_create(cr[1]);
			if (nr >= 3) croot[2] = COMPLEX_create(cr[2]);
		}
		else
		{
			for (i = 0; i < nr; i++)
				croot[i] = COMPLEX_create(gsl_complex_rect(z[i * 2], z[i * 2 + 1]));
			
			GB.Free(POINTER(&z));
		}
		
		for (i = 0; i < nr; i++)
			GB.Ref(croot[i]);
	}
			
	GB.ReturnObject(result);

END_METHOD

//---------------------------------------------------------------------------

GB_DESC PolynomialDesc[] =
{
	GB_DECLARE("Polynomial", sizeof(CPOLYNOMIAL)),

	GB_METHOD("_new", NULL, Polynomial_new, "[(Size)i(Complex)b]"),
	GB_METHOD("_free", NULL, Polynomial_free, NULL),
	
	GB_METHOD("ToString", "s", Polynomial_ToString, NULL),

	GB_PROPERTY_READ("Degree", "i", Polynomial_Degree),
	GB_PROPERTY_READ("Count", "i", Polynomial_Count),
							
	GB_METHOD("_get", "v", Polynomial_get, "(Index)i"),
	GB_METHOD("_put", NULL, Polynomial_put, "(Value)v(Index)i"),
										 
	GB_METHOD("_call", "v", Polynomial_Eval, "(X)v"),
	GB_METHOD("Eval", "v", Polynomial_Eval, "(X)v"),
	GB_METHOD("Solve", "Array", Polynomial_Solve, "[(Complex)b]"),

	//GB_INTERFACE("_operators", &_operators),
	GB_INTERFACE("_convert", &_convert),
	
	GB_END_DECLARE
};
