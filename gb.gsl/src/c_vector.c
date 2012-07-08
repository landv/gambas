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
#define SIZE(_ob) ((int)((GSLVECTOR *)_ob)->vector->size)


//---- Utility functions ----------------------------------------------

int gsl_vector_has_zero(gsl_vector *a)
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

double gsl_vector_abs(gsl_vector *a)
{
	int i;
	int size = (int)a->size;
	double val;
	double norm = 0;
	
	for (i = 0; i < size; i++)
	{
		val = gsl_vector_get(a, i);
		norm += val * val;
	}
	
	return sqrt(norm);
}

//---- Vector creation ------------------------------------------------------

static bool _do_not_init = FALSE;

static GSLVECTOR *VECTOR_create(int size, bool init)
{
	static GB_CLASS _klass = (GB_CLASS)NULL;

	GSLVECTOR *v;
	
	if (!_klass)
		_klass = GB.FindClass("Vector");
	
	_do_not_init = TRUE;
	v = (GSLVECTOR *)GB.New(_klass, NULL, NULL);
	v->vector = init ? gsl_vector_calloc(size) : gsl_vector_alloc(size);
	
	return v;
	
}

static GSLVECTOR *VECTOR_copy(GSLVECTOR *_object)
{
	GSLVECTOR *copy = VECTOR_create((int)THIS->vector->size, FALSE);
	gsl_vector_memcpy(copy->vector, THIS->vector);
	return copy;
}

//---- Arithmetic operators -------------------------------------------------

static GSLVECTOR *_addf(GSLVECTOR *a, double f)
{
	GSLVECTOR *r = VECTOR_copy(a);
	gsl_vector_add_constant(r->vector, f);
	return r;
}

static GSLVECTOR *_add(GSLVECTOR *a, GSLVECTOR *b)
{
	GSLVECTOR *r = VECTOR_copy(a);
	gsl_vector_add(r->vector, b->vector);
	return r;
}

static GSLVECTOR *_subf(GSLVECTOR *a, double f)
{
	GSLVECTOR *r = VECTOR_copy(a);
	gsl_vector_add_constant(r->vector, -f);
	return r;
}

static GSLVECTOR *_sub(GSLVECTOR *a, GSLVECTOR *b)
{
	GSLVECTOR *r = VECTOR_copy(a);
	gsl_vector_sub(r->vector, b->vector);
	return r;
}

static GSLVECTOR *_mulf(GSLVECTOR *a, double f)
{
	GSLVECTOR *r = VECTOR_copy(a);
	gsl_vector_scale(r->vector, f);
	return r;
}

static GSLVECTOR *_mul(GSLVECTOR *a, GSLVECTOR *b)
{
	GSLVECTOR *r = VECTOR_copy(a);
	gsl_vector_mul(r->vector, b->vector);
	return r;
}

static GSLVECTOR *_divf(GSLVECTOR *a, double f)
{
	GSLVECTOR *r;
	
	if (f == 0.0)
		return NULL;
	
	r = VECTOR_copy(a);
	gsl_vector_scale(r->vector, 1 / f);
	return r;
}

static GSLVECTOR *_idivf(GSLVECTOR *a, double f)
{
	GSLVECTOR *r;
	
	if (gsl_vector_has_zero(a->vector))
		return NULL;
	
	r = VECTOR_copy(a);
	gsl_vector_inverse(r->vector);
	gsl_vector_scale(r->vector, f);
	return r;
}

static GSLVECTOR *_div(GSLVECTOR *a, GSLVECTOR *b)
{
	GSLVECTOR *r;
	
	if (gsl_vector_has_zero(b->vector))
		return NULL;
	
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
	return gsl_vector_abs(a->vector);
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
		
		GB.NumberToString(local, gsl_vector_get(THIS->vector, i), NULL, &str, &len);
		result = GB.AddString(result, str, len);
	}
	
	result = GB.AddChar(result, ']');
	
	return result;
}

static bool _convert(GSLVECTOR *a, GB_TYPE type, GB_VALUE *conv)
{
	if (a)
	{
		switch (type)
		{
			case GB_T_FLOAT:
				conv->_float.value = gsl_vector_abs(a->vector);
				return FALSE;
				
			case GB_T_SINGLE:
				conv->_single.value = gsl_vector_abs(a->vector);
				return FALSE;
				
			case GB_T_INTEGER:
			case GB_T_SHORT:
			case GB_T_BYTE:
				conv->_integer.value = gsl_vector_abs(a->vector);
				return FALSE;
				
			case GB_T_LONG:
				conv->_long.value = gsl_vector_abs(a->vector);
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
	else if (type >= GB_T_OBJECT)
	{
		if (GB.Is(conv->_object.value, GB.FindClass("Array")))
		{
			GB_ARRAY array = (GB_ARRAY)conv->_object.value;
			int size = GB.Array.Count(array);
			GSLVECTOR *v = VECTOR_create(size, FALSE);
			int i;
			GB_VALUE temp;
			void *data;
			GB_TYPE atype = GB.Array.Type(array);
			
			if (atype > GB_T_BOOLEAN && atype <= GB_T_FLOAT)
			{
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
		}
		else if (type == GB.FindClass("Complex"))
		{
			GSLCOMPLEX *c = (GSLCOMPLEX *)conv->_object.value;
			GSLVECTOR *v = VECTOR_create(2, FALSE);
			gsl_vector_set(v->vector, 0, c->number.dat[0]);
			gsl_vector_set(v->vector, 1, c->number.dat[1]);
			conv->_object.value = v;
			return FALSE;
		}
	}
	
	return TRUE;
}

//---------------------------------------------------------------------------

BEGIN_METHOD(Vector_new, GB_INTEGER size)

	if (_do_not_init)
		_do_not_init = FALSE;
	else
		THIS->vector = gsl_vector_calloc(VARGOPT(size, 1));

END_METHOD

BEGIN_METHOD_VOID(Vector_free)

	gsl_vector_free(THIS->vector);

END_METHOD

BEGIN_PROPERTY(Vector_Count)

	GB.ReturnInteger((int)THIS->vector->size);

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
	
	GB.ReturnFloat(gsl_vector_get(THIS->vector, index));

END_METHOD

BEGIN_METHOD(Vector_put, GB_FLOAT value; GB_INTEGER index)

	int size = SIZE(THIS);
	int index = VARG(index);
	
	if (index < 0 || index >= size)
	{
		GB.Error(GB_ERR_ARG);
		return;
	}
	
	gsl_vector_set(THIS->vector, index, VARG(value));

END_METHOD

GB_DESC VectorDesc[] =
{
	GB_DECLARE("Vector", sizeof(GSLVECTOR)),
	
	GB_METHOD("_new", NULL, Vector_new, "[(Size)i]"),
	GB_METHOD("_free", NULL, Vector_free, NULL),
	//GB_STATIC_METHOD("_call", "Vector", Vector_call, "(Value)f."),
	GB_METHOD("Copy", "Vector", Vector_Copy, NULL),
	
	GB_METHOD("_get", "f", Vector_get, "(Index)i"),
	GB_METHOD("_put", NULL, Vector_put, "(Value)f(Index)i"),
	
	GB_PROPERTY_READ("Count", "i", Vector_Count),
																	
	GB_INTERFACE("_operators", &_operators),
	GB_INTERFACE("_convert", &_convert),
	
	GB_END_DECLARE
};
