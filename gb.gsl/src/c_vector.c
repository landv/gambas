/***************************************************************************

	c_vector.c

	gb.gsl component

	(c) 2012 Benoît Minisini <gambas@users.sourceforge.net>

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
#include "c_vector.h"

#define THIS ((GSLVECTOR *)_object)
#define VECTOR THIS->vector
#define VECTORC ((gsl_vector_complex *)VECTOR)

#define SIZE(_ob) ((int)((GSLVECTOR *)_ob)->vector->size)


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

static bool _do_not_init = FALSE;

static GSLVECTOR *VECTOR_create(GB_TYPE type, int size, bool init)
{
	GB_CLASS klass;
	GSLVECTOR *v;
	
	if (type == GB_T_FLOAT)
		klass = GB.FindClass("FloatVector");
	else if (type == GB.FindClass("Complex"))
		klass = GB.FindClass("ComplexVector");
	else
		return NULL;
	
	_do_not_init = TRUE;
	v = (GSLVECTOR *)GB.New(klass, NULL, NULL);
	
	v->type = type;
	
	if (type == GB_T_FLOAT)
		v->vector = init ? gsl_vector_calloc(size) : gsl_vector_alloc(size);
	else
		v->vector = (gsl_vector *)(init ? gsl_vector_complex_calloc(size) : gsl_vector_complex_alloc(size));
	
	return v;
}

static GSLVECTOR *VECTOR_copy(GSLVECTOR *_object)
{
	GSLVECTOR *copy = VECTOR_create(THIS->type, (int)VECTOR->size, FALSE);
	if (THIS->type == GB_T_FLOAT)
		gsl_vector_memcpy(copy->vector, VECTOR);
	else
		gsl_vector_complex_memcpy((gsl_vector_complex *)copy->vector, VECTORC);
	
	return copy;
}

//---- Arithmetic operators -------------------------------------------------

#if 0
static GSLVECTOR *_addf(GSLVECTOR *a, double f)
{
	GSLVECTOR *r = VECTOR_copy(a);
	if (r->type == GB_T_FLOAT)
		gsl_vector_add_constant(r->vector, f);
	else
		gsl_vector_complex_add_constant((gsl_vector_complex *)r->vector, gsl_complex_rect(f, 0));
	return r;
}

static GSLVECTOR *_add(GSLVECTOR *a, GSLVECTOR *b)
{
	GSLVECTOR *r = VECTOR_copy(a);
	
	if (r->type == GB_T_FLOAT)
		gsl_vector_add(r->vector, b->vector);
	else
		gsl_vector_complex_add((gsl_vector_complex *)r->vector, (gsl_vector_complex *)b->vector);
	
	return r;
}

static GSLVECTOR *_subf(GSLVECTOR *a, double f)
{
	GSLVECTOR *r = VECTOR_copy(a);
	if (r->type == GB_T_FLOAT)
		gsl_vector_add_constant(r->vector, -f);
	else
		gsl_vector_complex_add_constant((gsl_vector_complex *)r->vector, gsl_complex_rect(-f, 0));
	return r;
}

static GSLVECTOR *_sub(GSLVECTOR *a, GSLVECTOR *b)
{
	GSLVECTOR *r = VECTOR_copy(a);

	if (r->type == GB_T_FLOAT)
		gsl_vector_sub(r->vector, b->vector);
	else
		gsl_vector_complex_sub((gsl_vector_complex *)r->vector, (gsl_vector_complex *)b->vector);
	
	return r;
}

static GSLVECTOR *_mulf(GSLVECTOR *a, double f)
{
	GSLVECTOR *r = VECTOR_copy(a);
	
	if (r->type == GB_T_FLOAT)
		gsl_vector_scale(r->vector, f);
	else
		gsl_vector_complex_scale((gsl_vector_complex *)r->vector, gsl_complex_rect(f, 0));
	
	return r;
}

static GSLVECTOR *_mul(GSLVECTOR *a, GSLVECTOR *b)
{
	GSLVECTOR *r = VECTOR_copy(a);

	if (r->type == GB_T_FLOAT)
		gsl_vector_mul(r->vector, b->vector);
	else
		gsl_vector_complex_mul((gsl_vector_complex *)r->vector, (gsl_vector_complex *)b->vector);
	
	return r;
}

static GSLVECTOR *_divf(GSLVECTOR *a, double f)
{
	GSLVECTOR *r;
	
	if (f == 0.0)
		return NULL;
	
	r = VECTOR_copy(a);
	if (r->type == GB_T_FLOAT)
		gsl_vector_scale(r->vector, 1 / f);
	else
		gsl_vector_complex_scale((gsl_vector_complex *)r->vector, gsl_complex_rect(1 / f, 0));
	
	return r;
}

static GSLVECTOR *_idivf(GSLVECTOR *a, double f)
{
	GSLVECTOR *r;
	
	if (a->type == GB_T_FLOAT)
	{
		if (gsl_vector_has_zero(a->vector))
			return NULL;
	}
	else
	{
		if (gsl_vector_complex_has_zero((gsl_vector_complex *)a->vector))
			return NULL;
	}
	
	r = VECTOR_copy(a);

	if (a->type == GB_T_FLOAT)
	{
		gsl_vector_inverse(r->vector);
		gsl_vector_scale(r->vector, f);
	}
	else
	{
		gsl_vector_complex_inverse((gsl_vector_complext *)r->vector);
		gsl_vector_complex_scale((gsl_vector_complex *)r->vector, gsl_complex_rect(f, 0));
	}
	
	return r;
}

static GSLVECTOR *_div(GSLVECTOR *a, GSLVECTOR *b)
{
	GSLVECTOR *r;
	
	if (a->type == GB_T_FLOAT)
	{
		if (gsl_vector_has_zero(a->vector))
			return NULL;
	}
	else
	{
		if (gsl_vector_complex_has_zero((gsl_vector_complex *)a->vector))
			return NULL;
	}
	
	r = VECTOR_copy(a);
	gsl_vector_div(r->vector, b->vector);
	return r;
}

static int _equal(GSLVECTOR *a, GSLVECTOR *b)
{
	return gsl_vector_equal(a->vector, b->vector);
}

static int _equalf(GSLVECTOR *a, double f)
{
	int i;
	int size = (int)a->vector->size;
	
	for (i = 0; i < size; i++)
	{
		if (gsl_vector_get(a->vector, i) != f)
			return FALSE;
	}
	
	return TRUE;
}

static GSLVECTOR *_neg(GSLVECTOR *a)
{
	GSLVECTOR *r = VECTOR_copy(a);
	gsl_vector_negative(r->vector);
	return r;
}

static double _abs(GSLVECTOR *a)
{
	return gsl_blas_dnrm2(a->vector);
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

static char *_to_string(GSLVECTOR *_object, bool local)
{
	char *result = NULL;
	int i;
	int size = (int)THIS->vector->size;
	char *str;
	int len;
	
	result = GB.AddChar(result, '[');
	
	for (i = 0; i < size; i++)
	{
		if (i)
			result = GB.AddChar(result, ' ');
		
		if (THIS->type == GB_T_FLOAT)
		{
			GB.NumberToString(local, gsl_vector_get(VECTOR, i), NULL, &str, &len);
			result = GB.AddString(result, str, len);
		}
		else
		{
			str = COMPLEX_to_string(gsl_vector_complex_get(VECTORC, i), local);
			result = GB.AddString(result, str, GB.StringLength(str));
			GB.FreeString(&str);
		}
	}
	
	result = GB.AddChar(result, ']');
	
	return result;
}

static bool _convert(GSLVECTOR *_object, GB_TYPE type, GB_VALUE *conv)
{
	if (THIS)
	{
		if (THIS->type == GB_T_FLOAT)
		{
			switch (type)
			{
				case GB_T_FLOAT:
					conv->_float.value = gsl_blas_dnrm2(VECTOR);
					return FALSE;
					
				case GB_T_SINGLE:
					conv->_single.value = gsl_blas_dnrm2(VECTOR);
					return FALSE;
					
				case GB_T_INTEGER:
				case GB_T_SHORT:
				case GB_T_BYTE:
					conv->_integer.value = gsl_blas_dnrm2(VECTOR);
					return FALSE;
					
				case GB_T_LONG:
					conv->_long.value = gsl_blas_dnrm2(VECTOR);
					return FALSE;
					
				case GB_T_STRING:
				case GB_T_CSTRING:
					conv->_string.value.addr = _to_string(THIS, type == GB_T_CSTRING);
					conv->_string.value.start = 0;
					conv->_string.value.len = GB.StringLength(conv->_string.value.addr);
					return FALSE;
					
				default:
					return TRUE;
			}
		}
		else
		{
			switch (type)
			{
				case GB_T_FLOAT:
					conv->_float.value = gsl_blas_dznrm2(VECTORC);
					return FALSE;
					
				case GB_T_SINGLE:
					conv->_single.value = gsl_blas_dznrm2(VECTORC);
					return FALSE;
					
				case GB_T_INTEGER:
				case GB_T_SHORT:
				case GB_T_BYTE:
					conv->_integer.value = gsl_blas_dznrm2(VECTORC);
					return FALSE;
					
				case GB_T_LONG:
					conv->_long.value = gsl_blas_dznrm2(VECTORC);
					return FALSE;
					
				case GB_T_STRING:
				case GB_T_CSTRING:
					conv->_string.value.addr = _to_string(THIS, type == GB_T_CSTRING);
					conv->_string.value.start = 0;
					conv->_string.value.len = GB.StringLength(conv->_string.value.addr);
					return FALSE;
					
				default:
					return TRUE;
			}
		}
	}
	else if (type >= GB_T_OBJECT)
	{
		if (GB.Is(conv->_object.value, GB.FindClass("Array")))
		{
			GB_ARRAY array = (GB_ARRAY)conv->_object.value;
			int size = GB.Array.Count(array);
			GSLVECTOR *v;
			int i;
			GB_VALUE temp;
			void *data;
			GB_TYPE atype = GB.Array.Type(array);
			
			if (atype > GB_T_BOOLEAN && atype <= GB_T_FLOAT)
			{
				v = VECTOR_create(GB_T_FLOAT, size, FALSE);
				
				for (i = 0; i < size; i++)
				{
					data = GB.Array.Get(array, i);
					GB.ReadValue(&temp, data, atype);
					GB.Conv(&temp, GB_T_FLOAT);
					gsl_vector_set(v->vector, i, temp._float.value);
				}
				
				conv->_object.value = v;
				return FALSE;
			}
			else if (atype == GB.FindClass("Complex"))
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
			}
		}
		else if (type == GB.FindClass("Complex"))
		{
			GSLCOMPLEX *c = (GSLCOMPLEX *)conv->_object.value;
			GSLVECTOR *v = VECTOR_create(type, 1, FALSE);
			gsl_vector_complex_set((gsl_vector_complex *)v->vector, 0, c->number);
			conv->_object.value = v;
			return FALSE;
		}
	}
	
	return TRUE;
}

//---------------------------------------------------------------------------

BEGIN_METHOD_VOID(Vector_free)

	if (THIS->type == GB_T_FLOAT)
		gsl_vector_free(VECTOR);
	else
		gsl_vector_complex_free(VECTORC);

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
		GB.Error(GB_ERR_ARG);
		return;
	}
	
	if (THIS->type == GB_T_FLOAT)
		GB.ReturnFloat(gsl_vector_get(VECTOR, index));
	else
		GB.ReturnObject(COMPLEX_create(gsl_vector_complex_get(VECTORC, index)));

	GB.ReturnConvVariant();

END_METHOD

BEGIN_METHOD(Vector_put, GB_VARIANT value; GB_INTEGER index)

	int size = SIZE(THIS);
	int index = VARG(index);
	
	if (index < 0 || index >= size)
	{
		GB.Error(GB_ERR_ARG);
		return;
	}
	
	GB.Conv((GB_VALUE *)ARG(value), THIS->type);
	
	if (THIS->type == GB_T_FLOAT)
		gsl_vector_set(VECTOR, index, ((GB_FLOAT *)ARG(value))->value);
	else
	{
		GSLCOMPLEX *c = (GSLCOMPLEX *)((GB_OBJECT *)ARG(value))->value;
		if (GB.CheckObject(c))
			gsl_vector_complex_set(VECTORC, index, gsl_complex_rect(0, 0));
		else
			gsl_vector_complex_set(VECTORC, index, c->number);
	}

END_METHOD

//---------------------------------------------------------------------------

BEGIN_METHOD(FloatVector_new, GB_INTEGER size)

	if (_do_not_init)
		_do_not_init = FALSE;
	else
	{
		THIS->type = GB_T_FLOAT;
		THIS->vector = gsl_vector_calloc(VARGOPT(size, 1));
	}

END_METHOD

BEGIN_METHOD(FloatVector_get, GB_INTEGER index)

	int size = SIZE(THIS);
	int index = VARG(index);
	
	if (index < 0 || index >= size)
	{
		GB.Error(GB_ERR_ARG);
		return;
	}
	
	GB.ReturnFloat(gsl_vector_get(VECTOR, index));

END_METHOD

/*BEGIN_METHOD(FloatVector_put, GB_FLOAT value; GB_INTEGER index)

	int size = SIZE(THIS);
	int index = VARG(index);
	
	if (index < 0 || index >= size)
	{
		GB.Error(GB_ERR_ARG);
		return;
	}
	
	gsl_vector_set(VECTOR, index, VARG(value));

END_METHOD*/

//---------------------------------------------------------------------------

BEGIN_METHOD(ComplexVector_new, GB_INTEGER size)

	if (_do_not_init)
		_do_not_init = FALSE;
	else
	{
		THIS->type = GB.FindClass("Complex");
		THIS->vector = (gsl_vector *)gsl_vector_complex_calloc(VARGOPT(size, 1));
	}

END_METHOD

BEGIN_METHOD(ComplexVector_get, GB_INTEGER index)

	int size = SIZE(THIS);
	int index = VARG(index);
	
	if (index < 0 || index >= size)
	{
		GB.Error(GB_ERR_ARG);
		return;
	}
	
	GB.ReturnObject(COMPLEX_create(gsl_vector_complex_get(VECTORC, index)));

END_METHOD

/*BEGIN_METHOD(ComplexVector_put, GB_OBJECT value; GB_INTEGER index)

	int size = SIZE(THIS);
	int index = VARG(index);
	GSLCOMPLEX *value;
	
	if (index < 0 || index >= size)
	{
		GB.Error(GB_ERR_ARG);
		return;
	}
	
	value = (GSLCOMPLEX *)VARG(value);
	if (GB.CheckObject(value))
		return;
	
	gsl_vector_complex_set(VECTORC, index, value->number);

END_METHOD*/

GB_DESC VectorDesc[] =
{
	GB_DECLARE("Vector", sizeof(GSLVECTOR)), GB_NOT_CREATABLE(),
	
	//GB_METHOD("_new", NULL, Vector_new, "[(Size)i]"),
	GB_METHOD("_free", NULL, Vector_free, NULL),
	//GB_STATIC_METHOD("_call", "Vector", Vector_call, "(Value)f."),
	GB_METHOD("Copy", "Vector", Vector_Copy, NULL),
	
	GB_PROPERTY_READ("Count", "i", Vector_Count),
	
	GB_METHOD("_get", "v", Vector_get, "(Index)i"),
	GB_METHOD("_put", NULL, Vector_put, "(Value)v(Index)i"),

	GB_INTERFACE("_convert", &_convert),
	
	GB_END_DECLARE
};

GB_DESC FloatVectorDesc[] =
{
	GB_DECLARE("FloatVector", sizeof(GSLVECTOR)),
	GB_INHERITS("Vector"),
	
	GB_METHOD("_new", NULL, FloatVector_new, "[(Size)i]"),
	
	GB_METHOD("_get", "f", FloatVector_get, "(Index)i"),
	GB_METHOD("_put", NULL, Vector_put, "(Value)f(Index)i"),
	
	GB_END_DECLARE
};

GB_DESC ComplexVectorDesc[] =
{
	GB_DECLARE("ComplexVector", sizeof(GSLVECTOR)),
	GB_INHERITS("Vector"),
	
	GB_METHOD("_new", NULL, ComplexVector_new, "[(Size)i]"),
	
	GB_METHOD("_get", "Complex", ComplexVector_get, "(Index)i"),
	GB_METHOD("_put", NULL, Vector_put, "(Value)Complex(Index)i"),
	
	GB_END_DECLARE
};
