/***************************************************************************

	c_vector.c

	gb.gsl component

	(c) 2000-2013 Benoît Minisini <gambas@users.sourceforge.net>

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

#define __C_VECTOR_C

#include "c_complex.h"
#include "c_polynomial.h"
#include "c_vector.h"

#define THIS ((CVECTOR *)_object)
#define COMPLEX(_v) ((_v)->complex)

//---- Utility functions ----------------------------------------------

/*int gsl_vector_has_zero(gsl_vector *a)
{
	int i;
	int size = (int)a->size;
	
	for (i = 0; i < size; i++)
	{
		if (gsl_vector_get(a, i) == 0.0)
			return TRUE;
	}
	
	return FALSE;
}

int gsl_vector_complex_has_zero(gsl_vector *a)
{
	int i;
	int size = (int)a->size;
	gsl_complex *p:
	
	for (i = 0; i < size; i++)
	{
		p = gsl_vector_complex_ptr((gsl_vector_complext *)a, i);
		if (p->dat[0] == 0.0 && p->dat[1] == 0.0)
			return TRUE;
	}
	
	return FALSE;
}

void gsl_vector_inverse(gsl_vector *a)
{
	int i;
	int size = (int)a->size;
	double *p;
	
	for (i = 0; i < size; i++)
	{
		p = gsl_vector_ptr(a, i);
		*p = 1.0 / *p;
	}
}

void gsl_vector_complex_inverse(gsl_vector *a)
{
	int i;
	int size = (int)a->size;
	gsl_complex *p;
	
	for (i = 0; i < size; i++)
	{
		p = gsl_vector_complex_ptr((gsl_vector_complex *)a, i);
		*p = gsl_complex_inverse(*p);
	}
}

void gsl_vector_negative(gsl_vector *a)
{
	int i;
	int size = (int)a->size;
	double *p;
	
	for (i = 0; i < size; i++)
	{
		p = gsl_vector_ptr(a, i);
		*p = (- *p);
	}
}

void gsl_vector_complex_negative(gsl_vector *a)
{
	int i;
	int size = (int)a->size;
	gsl_complex *p;
	
	for (i = 0; i < size; i++)
	{
		p = gsl_vector_complex_ptr((gsl_vector_complext *)a, i);
		*p = gsl_complex_negative(*p);
	}
}*/

//---- Vector creation ------------------------------------------------------

//static bool _do_not_init = FALSE;

CVECTOR *VECTOR_create(int size, bool complex, bool init)
{
	CVECTOR *v = (CVECTOR *)GB.Create(CLASS_Vector, NULL, NULL);
	
	v->complex = complex;
	
	if (!complex)
		v->vector = init ? gsl_vector_calloc(size) : gsl_vector_alloc(size);
	else
		v->vector = init ? gsl_vector_complex_calloc(size) : gsl_vector_complex_alloc(size);
	
	//GB.Push(2, GB_T_INTEGER, size, GB_T_BOOLEAN, complex);
	//return (CVECTOR *)GB.New(CLASS_Vector, NULL, (void *)(intptr_t)2);
		
	return v;
}

/*CVECTOR *VECTOR_create_from(void *vector, bool complex)
{
	int size = ((gsl_vector *)vector)->size;
	CVECTOR *v = VECTOR_create(size, complex, FALSE);
	
	if (complex)
		gsl_vector_complex_free(CVEC(v));
	else
		gsl_vector_free(VEC(v));
	
	v->vector = vector;
	return v;
}*/

static CVECTOR *VECTOR_copy(CVECTOR *_object)
{
	CVECTOR *copy = VECTOR_create(SIZE(THIS), COMPLEX(THIS), FALSE);
	if (!COMPLEX(THIS))
		gsl_vector_memcpy(VEC(copy), VEC(THIS));
	else
		gsl_vector_complex_memcpy(CVEC(copy), CVEC(THIS));
	
	return copy;
}

#define VECTOR_make(_a) (((_a)->ob.ref <= 1) ? (_a) : VECTOR_copy(_a))

static CVECTOR *VECTOR_convert_to_complex(CVECTOR *_object)
{
	CVECTOR *v = VECTOR_create(SIZE(THIS), TRUE, FALSE);
	int i;
	
	for (i = 0; i < SIZE(THIS); i++)
		gsl_vector_complex_set((gsl_vector_complex *)v->vector, i, gsl_complex_rect(gsl_vector_get(VEC(THIS), i), 0));
	
	return v;
}

void VECTOR_ensure_complex(CVECTOR *_object)
{
	gsl_vector_complex *v;
	int size = SIZE(THIS);
	int i;
	
	if (COMPLEX(THIS))
		return;
	
	v = gsl_vector_complex_alloc(size);
	for (i = 0; i < size; i++)
		gsl_vector_complex_set(v, i, gsl_complex_rect(gsl_vector_get(VEC(THIS), i), 0));
	
	gsl_vector_free(VEC(THIS));
	THIS->vector = v;
	THIS->complex = TRUE;
}


bool VECTOR_ensure_not_complex(CVECTOR *_object)
{
	gsl_vector *v;
	int size = SIZE(THIS);
	int i;
	gsl_complex c;
	
	if (!COMPLEX(THIS))
		return FALSE;
	
	for (i = 0; i < size; i++)
	{
		c = gsl_vector_complex_get(CVEC(THIS), i);
		if (GSL_IMAG(c) != 0.0)
			return TRUE;
	}
	
	v = gsl_vector_alloc(size);
	
	for (i = 0; i < size; i++)
		gsl_vector_set(v, i, GSL_REAL(gsl_vector_complex_get(CVEC(THIS), i)));
	
	gsl_vector_complex_free(CVEC(THIS));
	THIS->vector = v;
	THIS->complex = FALSE;
	return FALSE;
}

//---- Arithmetic operators -------------------------------------------------

static CVECTOR *_add(CVECTOR *a, CVECTOR *b, bool invert)
{
	CVECTOR *v = VECTOR_make(a);
	
	if (COMPLEX(v) || COMPLEX(b))
	{
		VECTOR_ensure_complex(v);
		VECTOR_ensure_complex(b);
		gsl_vector_complex_add(CVEC(v), CVEC(b));
	}
	else
		gsl_vector_add(VEC(v), VEC(b));
	
	return v;
}

static CVECTOR *_sub(CVECTOR *a, CVECTOR *b, bool invert)
{
	CVECTOR *v = VECTOR_make(a);
	
	if (COMPLEX(v) || COMPLEX(b))
	{
		VECTOR_ensure_complex(v);
		VECTOR_ensure_complex(b);
		gsl_vector_complex_sub(CVEC(v), CVEC(b));
	}
	else
		gsl_vector_sub(VEC(v), VEC(b));
	
	return v;
}

static CVECTOR *_mulf(CVECTOR *a, double f, bool invert)
{
	CVECTOR *v = VECTOR_make(a);
	
	if (COMPLEX(v))
		gsl_vector_complex_scale(CVEC(v), gsl_complex_rect(f, 0));
	else
		gsl_vector_scale(VEC(v), f);
	
	return v;
}

static CVECTOR *_mulo(CVECTOR *a, void *b, bool invert)
{
	CVECTOR *v = VECTOR_make(a);
	
	if (!GB.Is(b, CLASS_Complex))
		return NULL;
	
	VECTOR_ensure_complex(v);
	gsl_vector_complex_scale(CVEC(v), ((CCOMPLEX *)b)->number);
	
	return v;
}

static CVECTOR *_divf(CVECTOR *a, double f, bool invert)
{
	if (invert)
		return NULL;
		
	if (f == 0.0)
	{
		GB.Error(GB_ERR_ZERO);
		return NULL;
	}

	return _mulf(a, 1 / f, FALSE);
}

static CVECTOR *_divo(CVECTOR *a, void *b, bool invert)
{
	if (!GB.Is(b, CLASS_Complex))
		return NULL;
	
	CCOMPLEX *c = (CCOMPLEX *)b;
	
	if (invert)
		return NULL;
	
	if (GSL_REAL(c->number) == 0 && GSL_IMAG(c->number) == 0)
	{
		GB.Error(GB_ERR_ZERO);
		return NULL;
	}

	CVECTOR *v = VECTOR_make(a);

	VECTOR_ensure_complex(v);
	gsl_vector_complex_scale(CVEC(v), gsl_complex_inverse(c->number));
	
	return v;
}

static int _equal(CVECTOR *a, CVECTOR *b, bool invert)
{
	if (COMPLEX(a) || COMPLEX(b))
	{
		VECTOR_ensure_complex(a);
		VECTOR_ensure_complex(b);
		return gsl_vector_complex_equal(CVEC(a), CVEC(b));
	}
	else
		return gsl_vector_equal(VEC(a), VEC(b));
}

static CVECTOR *_neg(CVECTOR *a)
{
	return _mulf(a, -1, FALSE);
}

static GB_OPERATOR_DESC _operator =
{
	.equal   = (void *)_equal,
	.add     = (void *)_add,
	.sub     = (void *)_sub,
	.mulf    = (void *)_mulf,
	.mulo    = (void *)_mulo,
	.divf    = (void *)_divf,
	.divo    = (void *)_divo,
	.neg     = (void *)_neg
};

//---- Conversions ----------------------------------------------------------

static char *_to_string(CVECTOR *_object, bool local)
{
	char *result = NULL;
	int i;
	int size = SIZE(THIS);
	char *str;
	int len;
	
	result = GB.AddChar(result, '[');
	
	for (i = 0; i < size; i++)
	{
		if (i)
			result = GB.AddChar(result, local ? ' ' : ',');
		
		if (!COMPLEX(THIS))
		{
			GB.NumberToString(local, gsl_vector_get(VEC(THIS), i), NULL, &str, &len);
			result = GB.AddString(result, str, len);
		}
		else
		{
			str = COMPLEX_to_string(gsl_vector_complex_get(CVEC(THIS), i), local);
			result = GB.AddString(result, str, GB.StringLength(str));
			GB.FreeString(&str);
		}
	}
	
	result = GB.AddChar(result, ']');
	
	return result;
}

static bool _convert(CVECTOR *_object, GB_TYPE type, GB_VALUE *conv)
{
	if (THIS)
	{
		if (!COMPLEX(THIS))
		{
			switch (type)
			{
				case GB_T_FLOAT:
					conv->_float.value = gsl_blas_dnrm2(VEC(THIS));
					return FALSE;
					
				case GB_T_SINGLE:
					conv->_single.value = gsl_blas_dnrm2(VEC(THIS));
					return FALSE;
					
				case GB_T_INTEGER:
				case GB_T_SHORT:
				case GB_T_BYTE:
					conv->_integer.value = gsl_blas_dnrm2(VEC(THIS));
					return FALSE;
					
				case GB_T_LONG:
					conv->_long.value = gsl_blas_dnrm2(VEC(THIS));
					return FALSE;
					
				case GB_T_STRING:
				case GB_T_CSTRING:
					conv->_string.value.addr = _to_string(THIS, type == GB_T_CSTRING);
					conv->_string.value.start = 0;
					conv->_string.value.len = GB.StringLength(conv->_string.value.addr);
					return FALSE;
					
				default:
					break;
			}
		}
		else
		{
			switch (type)
			{
				case GB_T_FLOAT:
					conv->_float.value = gsl_blas_dznrm2(CVEC(THIS));
					return FALSE;
					
				case GB_T_SINGLE:
					conv->_single.value = gsl_blas_dznrm2(CVEC(THIS));
					return FALSE;
					
				case GB_T_INTEGER:
				case GB_T_SHORT:
				case GB_T_BYTE:
					conv->_integer.value = gsl_blas_dznrm2(CVEC(THIS));
					return FALSE;
					
				case GB_T_LONG:
					conv->_long.value = gsl_blas_dznrm2(CVEC(THIS));
					return FALSE;
					
				case GB_T_STRING:
				case GB_T_CSTRING:
					conv->_string.value.addr = _to_string(THIS, type == GB_T_CSTRING);
					conv->_string.value.start = 0;
					conv->_string.value.len = GB.StringLength(conv->_string.value.addr);
					return FALSE;
					
				default:
					break;
			}
		}
		
		// Vector ---> Float[]
		if ((type == GB.FindClass("Float[]") || type == CLASS_Polynomial) && !COMPLEX(THIS))
		{
			GB_ARRAY a;
			int i;
			double *data;
			
			GB.Array.New(&a, GB_T_FLOAT, SIZE(THIS));
			data = (double *)GB.Array.Get(a, 0);
			for(i = 0; i < SIZE(THIS); i++)
				data[i] = gsl_vector_get(VEC(THIS), i);
			
			conv->_object.value = a;
			if (type != CLASS_Polynomial)
				return FALSE;
		}
		// Vector ---> Complex[]
		else if (type == GB.FindClass("Complex[]") || type == CLASS_Polynomial)
		{
			GB_ARRAY a;
			int i;
			void **data;
			CCOMPLEX *c;
			
			GB.Array.New(&a, CLASS_Complex, SIZE(THIS));
			data = (void **)GB.Array.Get(a, 0);
			for(i = 0; i < SIZE(THIS); i++)
			{
				c = COMPLEX_create(COMPLEX(THIS) ? gsl_vector_complex_get(CVEC(THIS), i) : gsl_complex_rect(gsl_vector_get(VEC(THIS), i), 0));
				data[i] = c;
				GB.Ref(c);
			}
			
			conv->_object.value = a;
			if (type != CLASS_Polynomial)
				return FALSE;
		}
		else
			return TRUE;
		
		// Vector ---> Polynomial
		if (type == CLASS_Polynomial)
		{
			void *unref = conv->_object.value;
			GB.Ref(unref); // Will be unref by the next GB.Conv()
			POLYNOMIAL_convert(FALSE, type, conv);
			GB.Unref(&unref); // Will be unref by the next GB.Conv()
			//GB.Conv(conv, type);
			//GB.UnrefKeep(&conv->_object.value, FALSE); // Will be ref again after the current GB.Conv()
			return FALSE;
		}
		
	}
	else if (type >= GB_T_OBJECT)
	{
		if (GB.Is(conv->_object.value, CLASS_Array))
		{
			GB_ARRAY array = (GB_ARRAY)conv->_object.value;
			int size = GB.Array.Count(array);
			CVECTOR *v;
			int i;
			GB_VALUE temp;
			void *data;
			GB_TYPE atype = GB.Array.Type(array);
			
			// Float[] Integer[] ... ---> Vector
			if (atype > GB_T_BOOLEAN && atype <= GB_T_FLOAT)
			{
				v = VECTOR_create(size, FALSE, FALSE);
				
				for (i = 0; i < size; i++)
				{
					data = GB.Array.Get(array, i);
					GB.ReadValue(&temp, data, atype);
					GB.Conv(&temp, GB_T_FLOAT);
					gsl_vector_set(VEC(v), i, temp._float.value);
				}
				
				conv->_object.value = v;
				return FALSE;
			}
			// Variant[] ---> Vector
			else if (atype == GB_T_VARIANT)
			{
				CCOMPLEX *c;
				v = VECTOR_create(size, TRUE, FALSE);
				
				for (i = 0; i < size; i++)
				{
					GB.ReadValue(&temp, GB.Array.Get(array, i), atype);
					GB.BorrowValue(&temp);
					GB.Conv(&temp, CLASS_Complex);
					c = temp._object.value;
					if (c)
						gsl_vector_complex_set(CVEC(v), i, c->number);
					else
						gsl_vector_complex_set(CVEC(v), i, COMPLEX_zero);
					GB.ReleaseValue(&temp);
				}
				
				conv->_object.value = v;
				return FALSE;
			}
			// Complex[] ---> Vector
			else if (atype == CLASS_Complex)
			{
				CCOMPLEX *c;
				v = VECTOR_create(size, TRUE, FALSE);
				
				for (i = 0; i < size; i++)
				{
					c = *((CCOMPLEX **)GB.Array.Get(array, i));
					if (c)
						gsl_vector_complex_set(CVEC(v), i, c->number);
					else
						gsl_vector_complex_set(CVEC(v), i, COMPLEX_zero);
				}
				
				conv->_object.value = v;
				return FALSE;
			}
		}
		// Float Integer... ---> Vector
		else if (type > GB_T_BOOLEAN && type <= GB_T_FLOAT)
		{
			CVECTOR *v = VECTOR_create(1, FALSE, FALSE);
			if (type == GB_T_FLOAT)
				gsl_vector_set(VEC(v), 0, conv->_float.value);
			else if (type == GB_T_SINGLE)
				gsl_vector_set(VEC(v), 0, conv->_single.value);
			else
				gsl_vector_set(VEC(v), 0, conv->_integer.value);
			conv->_object.value = v;
			return FALSE;
		}
		// Complex ---> Vector
		else if (type == CLASS_Complex)
		{
			CCOMPLEX *c = (CCOMPLEX *)conv->_object.value;
			CVECTOR *v = VECTOR_create(1, TRUE, FALSE);
			gsl_vector_complex_set(CVEC(v), 0, c->number);
			conv->_object.value = v;
			return FALSE;
		}
	}
	
	return TRUE;
}

//---------------------------------------------------------------------------

BEGIN_METHOD(Vector_new, GB_INTEGER size; GB_BOOLEAN complex)

	bool complex = VARGOPT(complex, FALSE);
	int size = VARGOPT(size, 1);
	
	if (size < 1) size = 1;
	
	THIS->complex = complex;
	
	if (!complex)
		THIS->vector = gsl_vector_calloc(size);
	else
		THIS->vector = gsl_vector_complex_calloc(size);
	
END_METHOD


BEGIN_METHOD_VOID(Vector_free)

	if (!COMPLEX(THIS))
		gsl_vector_free(VEC(THIS));
	else
		gsl_vector_complex_free(CVEC(THIS));

END_METHOD


BEGIN_PROPERTY(Vector_Count)

	GB.ReturnInteger(SIZE(THIS));

END_PROPERTY


BEGIN_METHOD_VOID(Vector_Copy)

	GB.ReturnObject(VECTOR_copy(THIS));

END_METHOD


BEGIN_METHOD(Vector_get, GB_INTEGER index)

	int size = SIZE(THIS);
	int index = VARG(index);
	
	if (index < 0 || index >= size)
	{
		GB.Error(GB_ERR_BOUND);
		return;
	}
	
	if (!COMPLEX(THIS))
		GB.ReturnFloat(gsl_vector_get(VEC(THIS), index));
	else
		GB.ReturnObject(COMPLEX_create(gsl_vector_complex_get(CVEC(THIS), index)));

	GB.ReturnConvVariant();

END_METHOD


BEGIN_METHOD(Vector_put, GB_VARIANT value; GB_INTEGER index)

	int index = VARG(index);
	int size = SIZE(THIS);
	GB_VALUE *value = (GB_VALUE *)ARG(value);
	int type;
	COMPLEX_VALUE cv;
	
	if (index < 0 || index > size)
	{
		GB.Error(GB_ERR_BOUND);
		return;
	}
	
	type = COMPLEX_get_value(value, &cv);
	
	if (type == CGV_ERR)
		return;

	if (type == CGV_COMPLEX)
	{
		VECTOR_ensure_complex(THIS);
		gsl_vector_complex_set(CVEC(THIS), index, cv.z);
	}
	else
	{
		if (COMPLEX(THIS))
			gsl_vector_complex_set(CVEC(THIS), index, cv.z);
		else
			gsl_vector_set(VEC(THIS), index, cv.x);
	}
	
END_METHOD


static void do_dot(CVECTOR *_object, CVECTOR *v, bool conj)
{
	bool ca, cb;
	
	if (GB.CheckObject(v))
		return;
	
	ca = !COMPLEX(THIS);
	cb = !COMPLEX(v);
	
	if (ca && cb)
	{
		double result;
		gsl_blas_ddot(VEC(THIS), v->vector, &result);
		GB.ReturnFloat(result);
	}
	else
	{
		CVECTOR *a, *b;
		gsl_complex result;
		
		if (ca)
			a = VECTOR_convert_to_complex(THIS);
		else
			a = THIS;
		
		if (cb)
			b = VECTOR_convert_to_complex(v);
		else
			b = v;
		
		if (conj)
			gsl_blas_zdotc(CVEC(a), CVEC(b), &result);
		else
			gsl_blas_zdotu(CVEC(a), CVEC(b), &result);
		
		GB.ReturnObject(COMPLEX_create(result));
		
		if (ca) GB.Unref(POINTER(&a));
		if (cb) GB.Unref(POINTER(&b));
	}

	GB.ReturnConvVariant();
}


BEGIN_METHOD(Vector_Dot, GB_OBJECT vector)

	do_dot(THIS, VARG(vector), FALSE);

END_METHOD


BEGIN_METHOD(Vector_ConjDot, GB_OBJECT vector)

	do_dot(THIS, VARG(vector), TRUE);
	
END_METHOD


BEGIN_METHOD_VOID(Vector_Norm)

	if (!COMPLEX(THIS))
		GB.ReturnFloat(gsl_blas_dnrm2(VEC(THIS)));
	else
		GB.ReturnFloat(gsl_blas_dznrm2(CVEC(THIS)));

END_METHOD


BEGIN_PROPERTY(Vector_Handle)

	GB.ReturnPointer(THIS->vector);
	
END_PROPERTY


BEGIN_METHOD(Vector_ToString, GB_BOOLEAN local)

	GB.ReturnString(GB.FreeStringLater(_to_string(THIS, VARGOPT(local, FALSE))));

END_METHOD


//---------------------------------------------------------------------

GB_DESC VectorDesc[] =
{
	GB_DECLARE("Vector", sizeof(CVECTOR)),
	
	GB_METHOD("_new", NULL, Vector_new, "[(Size)i(Complex)b]"),
	GB_METHOD("_free", NULL, Vector_free, NULL),
	//GB_STATIC_METHOD("_call", "Vector", Vector_call, "(Value)f."),
	GB_METHOD("Copy", "Vector", Vector_Copy, NULL),
	GB_METHOD("ToString", "s", Vector_ToString, "[(Local)b]"),
	
	GB_PROPERTY_READ("Count", "i", Vector_Count),
	GB_PROPERTY_READ("Handle", "p", Vector_Handle),
	
	GB_METHOD("_get", "v", Vector_get, "(Index)i"),
	GB_METHOD("_put", NULL, Vector_put, "(Value)v(Index)i"),

	//GB_METHOD("Scale", "Vector", Vector_Scale, "(Value)v"),
	GB_METHOD("Dot", "v", Vector_Dot, "(Vector)Vector"),
	GB_METHOD("ConjDot", "v", Vector_ConjDot, "(Vector)Vector"),
	GB_METHOD("Norm", "f", Vector_Norm, NULL),
	//GB_METHOD("Equal", "b", Vector_Equal, "(Vector)Vector;"),
	
	GB_INTERFACE("_convert", &_convert),
	GB_INTERFACE("_operator", &_operator),
	
	GB_END_DECLARE
};
