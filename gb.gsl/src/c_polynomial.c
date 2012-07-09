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
#define DATA(_p) ((double *)(_p)->array.data)
#define COUNT(_p) ((_p)->array.count)

//---- Utility methods ------------------------------------------------------

static CPOLYNOMIAL *POLYNOMIAL_create(int size)
{
	if (size > 0) GB.Push(1, GB_T_INTEGER, size);
	return (CPOLYNOMIAL *)GB.New(GB.FindClass("Polynomial"), NULL, (void *)(intptr_t)((size > 0) ? 1 : 0));
}

static CPOLYNOMIAL *POLYNOMIAL_copy(CPOLYNOMIAL *_object)
{
	int size = COUNT(THIS);
	CPOLYNOMIAL *p = POLYNOMIAL_create(size);
	
	if (size > 0)
		memcpy(DATA(p), DATA(THIS), size * sizeof(double));
	
	return p;
}

static int get_degree(CPOLYNOMIAL *_object)
{
	int i;
	double *d = DATA(THIS);
	
	for (i = COUNT(THIS) - 1; i >= 0; i--)
	{
		if (d[i] != 0.0)
			return i;
	}
	
	return 0;
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
	double coef;
	bool add = FALSE;
	
	i = size;
	while (i > 0)
	{
		i--;
		
		coef = DATA(p)[i];
		if (coef != 0.0)
		{
			if (!add)
				add = TRUE;
			else if (coef > 0.0)
				result = GB.AddChar(result, '+');
			
			GB.NumberToString(local, coef, NULL, &str, &len);
			result = GB.AddString(result, str, len);
			
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
						GB_ARRAY array = (GB_ARRAY)conv->_object.value;
						int size = GB.Array.Count(array);
						int i;
						GB_VALUE temp;
						void *data;
						GB_TYPE atype = GB.Array.Type(array);
						
						if (atype > GB_T_BOOLEAN && atype <= GB_T_FLOAT)
						{
							p = POLYNOMIAL_create(size);
							
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
						/*else if (atype == CLASS_Complex)
						{
							GSLCOMPLEX *c;
							v = VECTOR_create(atype, size, FALSE);
							
							for (i = 0; i < size; i++)
							{
								c = *((GSLCOMPLEX **)GB.Array.Get(array, i));
								if (c)
									gsl_vector_complex_set((gsl_vector_complex *)v->vector, i, c->number);
								else
									gsl_vector_complex_set((gsl_vector_complex *)v->vector, i, gsl_complex_rect(0, 0));
							}
							
							conv->_object.value = v;
							return FALSE;
						}*/
					}
				}
				
				return TRUE;
		}
		
		a = POLYNOMIAL_create(1);
		*DATA(a) = coef;
		conv->_object.value = a;
		return FALSE;
	}
}

//---------------------------------------------------------------------------

BEGIN_METHOD_VOID(Polynomial_new)

	if (THIS->array.dim)
		GB.Error("Too many dimensions");
	
END_METHOD

BEGIN_PROPERTY(Polynomial_Degree)

	GB.ReturnInteger(get_degree(THIS));

END_PROPERTY

BEGIN_METHOD_VOID(Polynomial_ToString)

	GB.ReturnString(POLYNOMIAL_to_string(THIS, FALSE));

END_METHOD

BEGIN_METHOD(Polynomial_Eval, GB_FLOAT x)

	GB.ReturnFloat(gsl_poly_eval(DATA(THIS), COUNT(THIS), VARG(x)));

END_METHOD

BEGIN_METHOD(Polynomial_EvalComplex, GB_OBJECT x)

	GSLCOMPLEX *c = VARG(x);
	
	if (GB.CheckObject(c))
		return;

	GB.ReturnObject(COMPLEX_create(gsl_poly_complex_eval(DATA(THIS), COUNT(THIS), c->number)));

END_METHOD


BEGIN_METHOD_VOID(Polynomial_Solve)

	double *data = DATA(THIS);
	int nr, i, dg, ret;
	GB_ARRAY result;
	double r[3];
	double *z = NULL;
	gsl_poly_complex_workspace *work;
	
	dg = get_degree(THIS) + 1;
	
	switch(dg)
	{
		case 1:
			GB.ReturnNull();
			return;
			
		case 2:
			r[0] = -data[0] / data[1];
			nr = 1;
			break;
			
		case 3:
			nr = gsl_poly_solve_quadratic(data[2], data[1], data[0], &r[0], &r[1]);
			break;
			
		case 4:
			if (data[3] == 1.0)
			{
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
			
			nr = 0;
			for (i = 0; i < (dg - 1); i++)
			{
				if (z[i * 2 + 1] == 0.0)
					nr++;
			}
	}
	
	GB.Array.New(&result, GB_T_FLOAT, nr);
	
	if (nr > 0)
		data = (double *)GB.Array.Get(result, 0);
	
	if (!z)
	{
		if (nr >= 1) data[0] = r[0];
		if (nr >= 2) data[1] = r[1];
		if (nr >= 3) data[2] = r[2];
	}
	else
	{
		if (nr > 0)
		{
			nr = 0;
			
			for (i = 0; i < (dg - 1); i++)
			{
				if (z[i * 2 + 1] == 0.0)
					data[nr++] = z[i * 2];
			}
		}
		
		GB.Free(POINTER(&z));
	}
			
	GB.ReturnObject(result);

END_METHOD

//---------------------------------------------------------------------------

GB_DESC CPolynomialDesc[] =
{
	GB_DECLARE("Polynomial", sizeof(CPOLYNOMIAL)), GB_INHERITS("Float[]"),

	GB_METHOD("_new", NULL, Polynomial_new, NULL),
	GB_METHOD("ToString", "s", Polynomial_ToString, NULL),

	GB_PROPERTY("Degree", "i", Polynomial_Degree),
										 
	// Implementation Methods
	GB_METHOD("Eval", "f", Polynomial_Eval, "(X)f"),
	GB_METHOD("EvalComplex", "Complex", Polynomial_EvalComplex, "(Z)Complex;"),
	GB_METHOD("Solve", "Float[]", Polynomial_Solve, NULL),
	//GB_METHOD("Eval", "Complex", ComplexPolynomial_Eval, "(Z)Complex"),
	//GB_METHOD("Solve", "Complex[];", ComplexPolynomial_Solve, NULL),

	//GB_INTERFACE("_operators", &_operators),
	GB_INTERFACE("_convert", &_convert),
	
	GB_END_DECLARE
};


