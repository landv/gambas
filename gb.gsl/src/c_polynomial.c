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

#include "c_complex.h"
#include "c_vector.h"
#include "c_polynomial.h"

#define THIS ((CPOLYNOMIAL *)_object)
#define DATA(_p) ((double *)(_p)->data)
#define CDATA(_p) ((gsl_complex *)(_p)->data)
#define COUNT(_p) ((_p)->size)
#define COMPLEX(_p) ((_p)->complex)

//---- Utility methods ------------------------------------------------------

static CPOLYNOMIAL *POLYNOMIAL_create(int size, bool complex)
{
	CPOLYNOMIAL *p = (CPOLYNOMIAL *)GB.Create(CLASS_Polynomial, NULL, NULL);
	
	GB.NewArray(POINTER(&p->data), complex ? sizeof(gsl_complex) : sizeof(double), size);
	
	p->size = size;
	p->complex = complex;
	
	return p;
}

static CPOLYNOMIAL *POLYNOMIAL_copy(CPOLYNOMIAL *_object)
{
	CPOLYNOMIAL *p = POLYNOMIAL_create(COUNT(THIS), COMPLEX(THIS));
	memcpy(p->data, THIS->data, THIS->size * (COMPLEX(THIS) ? sizeof(gsl_complex) : sizeof(double)));
	return p;
}

#define POLYNOMIAL_make(_a) (((_a)->ob.ref <= 1) ? (_a) : POLYNOMIAL_copy(_a))

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

static CPOLYNOMIAL *POLYNOMIAL_make_size(CPOLYNOMIAL *a, int min_size)
{
	if (a->size >= min_size)
		return POLYNOMIAL_make(a);
	
	a = POLYNOMIAL_copy(a);
	ensure_size(a, min_size);
	return a;
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

static void poly_negative(CPOLYNOMIAL *_object)
{
	int i;
	
	if (COMPLEX(THIS))
	{
		for (i = 0; i < COUNT(THIS); i++)
			DATA(THIS)[i] = (- DATA(THIS)[i]);
	}
	else
	{
		for (i = 0; i < COUNT(THIS); i++)
			CDATA(THIS)[i] = gsl_complex_negative(CDATA(THIS)[i]);
	}
}

//---- Arithmetic operators -------------------------------------------------

static CPOLYNOMIAL *_addf(CPOLYNOMIAL *a, double f, bool invert)
{
	CPOLYNOMIAL *p = POLYNOMIAL_make(a);
	
	DATA(p)[0] += f;
	return p;
}

static CPOLYNOMIAL *_add(CPOLYNOMIAL *a, CPOLYNOMIAL *b, bool invert)
{
	int da = get_degree(a);
	int db = get_degree(b);
	int d = Max(da, db);
	int dm = Min(d, db);
	int i;
	
	CPOLYNOMIAL *p = POLYNOMIAL_make_size(a, d + 1);

	if (COMPLEX(a) || COMPLEX(b))
	{
		ensure_complex(p);
		ensure_complex(b);
		
		for (i = 0; i <= dm; i++)
			CDATA(p)[i] = gsl_complex_add(CDATA(p)[i], CDATA(b)[i]);
	}
	else
	{
		for (i = 0; i <= dm; i++)
			DATA(p)[i] += DATA(b)[i];
	}
		
	return p;
}

static CPOLYNOMIAL *op_array(CPOLYNOMIAL *a, void *b, bool invert, CPOLYNOMIAL *(*func)(CPOLYNOMIAL *, CPOLYNOMIAL *, bool))
{
	GB_VALUE conv;
	bool err;
	CPOLYNOMIAL *p;
	
	conv._object.type = (GB_TYPE)GB.GetClass(b);
	conv._object.value = b;
	
	GB.Ref(b);
	err = GB.Conv(&conv, CLASS_Polynomial);
	
	if (!err)
	{
		if (invert)
		{
			GB.Ref(conv._object.value);
			p = (*func)(conv._object.value, a, FALSE);
			GB.Unref(&conv._object.value);
		}
		else
		{
			p = (*func)(a, conv._object.value, FALSE);
		}
		
		GB.Unref(&conv._object.value);
		
		return p;
	}
	else
		return NULL;
}

static CPOLYNOMIAL *_addo(CPOLYNOMIAL *a, void *b, bool invert)
{
	CPOLYNOMIAL *p;
	if (GB.Is(b, CLASS_Complex))
	{
		p = POLYNOMIAL_make(a);
		
		ensure_complex(p);
		CDATA(p)[0] = gsl_complex_add(CDATA(p)[0], ((CCOMPLEX *)b)->number);
		return p;
	}
	else if (GB.Is(b, CLASS_Array))
	{
		return op_array(a, b, invert, _add);
	}
	
	return NULL;
}

static CPOLYNOMIAL *_subf(CPOLYNOMIAL *a, double f, bool invert)
{
	CPOLYNOMIAL *p = POLYNOMIAL_make(a);
	
	if (invert)
		poly_negative(p);
	else
		f = -f;
	
	DATA(p)[0] += f;
	return p;
}

static CPOLYNOMIAL *_sub(CPOLYNOMIAL *a, CPOLYNOMIAL *b, bool invert)
{
	int da = get_degree(a);
	int db = get_degree(b);
	int d = Max(da, db);
	int dm = Min(d, db);
	int i;
	
	CPOLYNOMIAL *p = POLYNOMIAL_make_size(a, d + 1);

	if (COMPLEX(a) || COMPLEX(b))
	{
		ensure_complex(p);
		ensure_complex(b);
		
		for (i = 0; i <= dm; i++)
			CDATA(p)[i] = gsl_complex_sub(CDATA(p)[i], CDATA(b)[i]);
	}
	else
	{
		for (i = 0; i <= dm; i++)
			DATA(p)[i] -= DATA(b)[i];
	}
		
	return p;
}

static CPOLYNOMIAL *_subo(CPOLYNOMIAL *a, void *b, bool invert)
{
	CPOLYNOMIAL *p;
	
	if (GB.Is(b, CLASS_Complex))
	{
		p = POLYNOMIAL_make(a);
		
		if (invert)
		{
			poly_negative(p);
			ensure_complex(p);
			CDATA(p)[0] = gsl_complex_add(CDATA(p)[0], ((CCOMPLEX *)b)->number);
		}
		else
		{
			ensure_complex(p);
			CDATA(p)[0] = gsl_complex_sub(CDATA(p)[0], ((CCOMPLEX *)b)->number);
		}
		
		return p;
	}
	else if (GB.Is(b, CLASS_Array))
	{
		return op_array(a, b, invert, _sub);
	}
	
	return NULL;
}

static CPOLYNOMIAL *_neg(CPOLYNOMIAL *a)
{
	CPOLYNOMIAL *p = POLYNOMIAL_make(a);
	poly_negative(p);
	return p;
}

static int _equal(CPOLYNOMIAL *a, CPOLYNOMIAL *b, bool invert)
{
	int da = get_degree(a);
	int db = get_degree(b);
	int i;
	
	if (da != db)
		return FALSE;
	
	if (COMPLEX(a) || COMPLEX(b))
	{
		ensure_complex(a);
		ensure_complex(b);
		
		for (i = 0; i <= da; i++)
			if (GSL_REAL(CDATA(a)[i]) != GSL_REAL(CDATA(b)[i]) || GSL_IMAG(CDATA(a)[i]) != GSL_IMAG(CDATA(b)[i]))
				return FALSE;
	}
	else
	{
		for (i = 0; i <= da; i++)
			if (DATA(a)[i] != DATA(b)[i])
				return FALSE;
	}
	
	return TRUE;
}


static GB_OPERATOR_DESC _operator =
{
	add: (void *)_add,
	addf: (void *)_addf,
	addo: (void *)_addo,
	sub: (void *)_sub,
	subf: (void *)_subf,
	subo: (void *)_subo,
	/*mul: (void *)_mul,
	mulf: (void *)_mulf,
	div: (void *)_div,
	divf: (void *)_divf,
	idivf: (void *)_idivf,*/
	equal: (void *)_equal,
	/*equalf: (void *)_equalf,
	abs: (void *)_abs,*/
	neg: (void *)_neg
};

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
	bool par;
	
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
		
		par = i > 0 && re != 0.0 && im != 0.0;
		
		if (!add)
			add = TRUE;
		else if (re > 0.0 || (re == 0.0 && im > 0.0) || par)
			result = GB.AddChar(result, '+');
		
		//l = GB.StringLength(result);
		
		if (par)
			result = GB.AddChar(result, '(');
		
		if (i == 0 || re != 1.0 || im != 0.0)
		{
			if (re != 0.0)
			{
				if (re == -1.0 && i > 0)
					result = GB.AddChar(result, '-');
				else
				{
					GB.NumberToString(local, re, NULL, &str, &len);
					result = GB.AddString(result, str, len);
				}
			}
			if (im != 0.0)
			{
				if (re != 0.0 && im > 0.0)
					result = GB.AddChar(result, '+');
				
				if (local && im == -1.0)
					result = GB.AddChar(result, '-');
				else if (!local || im != 1.0)
				{
					GB.NumberToString(local, im, NULL, &str, &len);
					result = GB.AddString(result, str, len);
				}
				result = GB.AddChar(result, 'i');
			}
		}
		
		if (par)
			result = GB.AddChar(result, ')');
		
		if (i > 0)
		{
			if (!local && ((im == 0 && re != 0 && re != 1 && re != -1) || (im != 0)))
				result = GB.AddChar(result, '*');
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

bool POLYNOMIAL_convert(CPOLYNOMIAL *a, GB_TYPE type, GB_VALUE *conv)
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
					if (GB.Is(conv->_object.value, CLASS_Array))
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


BEGIN_METHOD(Polynomial_ToString, GB_BOOLEAN local)

	GB.ReturnString(GB.FreeStringLater(POLYNOMIAL_to_string(THIS, VARGOPT(local, FALSE))));

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
	COMPLEX_VALUE cv;
	
	if (index < 0 || index > 65535)
	{
		GB.Error(GB_ERR_ARG);
		return;
	}
	
	type = COMPLEX_get_value(value, &cv);
	
	if (type == CGV_ERR)
		return;
	
	ensure_size(THIS, index + 1);
	
	if (type == CGV_COMPLEX)
	{
		ensure_complex(THIS);
		CDATA(THIS)[index] = cv.z;
	}
	else
	{
		if (COMPLEX(THIS))
			CDATA(THIS)[index] = cv.z;
		else
			DATA(THIS)[index] = cv.x;
	}
	
END_METHOD


BEGIN_METHOD(Polynomial_Eval, GB_VARIANT value)

	GB_VALUE *value = (GB_VALUE *)ARG(value);
	int type;
	COMPLEX_VALUE cv;

	type = COMPLEX_get_value(value, &cv);
	if (type == CGV_ERR)
		return;
									 
	if (COMPLEX(THIS))
	{
		GB.ReturnObject(COMPLEX_create(gsl_complex_poly_complex_eval(CDATA(THIS), COUNT(THIS), cv.z)));
	}
	else
	{
		if (type == CGV_COMPLEX)
			GB.ReturnObject(COMPLEX_create(gsl_poly_complex_eval(DATA(THIS), COUNT(THIS), cv.z)));
		else
			GB.ReturnFloat(gsl_poly_eval(DATA(THIS), COUNT(THIS), cv.x));
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
	
	GB_METHOD("ToString", "s", Polynomial_ToString, "[(Local)b]"),

	GB_PROPERTY_READ("Degree", "i", Polynomial_Degree),
	GB_PROPERTY_READ("Count", "i", Polynomial_Count),
							
	GB_METHOD("_get", "v", Polynomial_get, "(Index)i"),
	GB_METHOD("_put", NULL, Polynomial_put, "(Value)v(Index)i"),
										 
	GB_METHOD("_call", "v", Polynomial_Eval, "(X)v"),
	GB_METHOD("Eval", "v", Polynomial_Eval, "(X)v"),
	GB_METHOD("Solve", "Array", Polynomial_Solve, "[(Complex)b]"),

	GB_INTERFACE("_operator", &_operator),
	GB_INTERFACE("_convert", &POLYNOMIAL_convert),
	
	GB_END_DECLARE
};
