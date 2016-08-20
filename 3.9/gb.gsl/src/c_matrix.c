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

#define __C_MATRIX_C

#include "c_complex.h"
#include "c_vector.h"
#include "c_matrix.h"

#define THIS ((CMATRIX *)_object)
#define MAT(_m) ((gsl_matrix *)(_m)->matrix)
#define CMAT(_m) ((gsl_matrix_complex *)(_m)->matrix)
#define HEIGHT(_m) ((int)(MAT(_m)->size1))
#define WIDTH(_m) ((int)(MAT(_m)->size2))
#define COMPLEX(_m) ((_m)->complex)

//---- Matrix creation ------------------------------------------------------

static CMATRIX *MATRIX_create(int width, int height, bool complex, bool init)
{
	CMATRIX *m = GB.Create(CLASS_Matrix, NULL,NULL);
	
	if (complex)
		m->matrix = init ? gsl_matrix_complex_calloc(height, width) : gsl_matrix_complex_alloc(height, width);
	else
		m->matrix = init ? gsl_matrix_calloc(height, width) : gsl_matrix_alloc(height, width);
	
	m->complex = complex;
	return m;
}

static CMATRIX *MATRIX_identity(int width, int height, bool complex)
{
	CMATRIX *m = MATRIX_create(width, height, complex, FALSE);
	
	if (complex)
		gsl_matrix_complex_set_identity(CMAT(m));
	else
		gsl_matrix_set_identity(MAT(m));
	
	return m;
}

static CMATRIX *MATRIX_create_from(void *matrix, bool complex)
{
	CMATRIX *m = GB.Create(CLASS_Matrix, NULL,NULL);
	
	m->matrix = matrix;
	m->complex = complex;
	return m;
}

static CMATRIX *MATRIX_copy(CMATRIX *_object)
{
	CMATRIX *copy = MATRIX_create(WIDTH(THIS), HEIGHT(THIS), COMPLEX(THIS), FALSE);
	if (COMPLEX(THIS))
		gsl_matrix_complex_memcpy(CMAT(copy), CMAT(THIS));
	else
		gsl_matrix_memcpy(MAT(copy), MAT(THIS));
	
	return copy;
}

#define MATRIX_make(_ma) (((_ma)->ob.ref <= 1) ? (_ma) : MATRIX_copy(_ma))

/*static CMATRIX *MATRIX_convert_to_complex(CMATRIX *_object)
{
	CMATRIX *m = MATRIX_create(WIDTH(THIS), HEIGHT(THIS), TRUE, FALSE);
	int i, j;
	
	for (i = 0; i < HEIGHT(THIS); i++)
		for (j = 0; j < WIDTH(THIS); j++)
			gsl_matrix_complex_set(CMAT(m), i, j, gsl_complex_rect(gsl_matrix_get(MAT(THIS), i, j), 0));
	
	return m;
}*/

static void MATRIX_ensure_complex(CMATRIX *_object)
{
	gsl_matrix_complex *v;
	int w = WIDTH(THIS);
	int h = HEIGHT(THIS);
	int i, j;
	
	if (COMPLEX(THIS))
		return;
	
	v = gsl_matrix_complex_alloc(h, w);
	for (i = 0; i < h; i++)
		for (j = 0; j < w; j++)
			gsl_matrix_complex_set(v, i, j, gsl_complex_rect(gsl_matrix_get(MAT(THIS), i, j), 0));
	
	gsl_matrix_free(MAT(THIS));
	THIS->matrix = v;
	THIS->complex = TRUE;
}


/*static bool MATRIX_ensure_not_complex(CMATRIX *_object)
{
	gsl_matrix *m;
	int w = WIDTH(THIS);
	int h = HEIGHT(THIS);
	int i, j;
	gsl_complex c;
	
	if (!COMPLEX(THIS))
		return FALSE;
	
	for (i = 0; i < h; i++)
		for (j = 0; j < w; j++)
		{
			c = gsl_matrix_complex_get(CMAT(THIS), i, j);
			if (GSL_IMAG(c) != 0.0)
				return TRUE;
		}
	
	m = gsl_matrix_alloc(h, w);
	
	for (i = 0; i < h; i++)
		for (j = 0; j < w; j++)
			gsl_matrix_set(m, i, j, GSL_REAL(gsl_matrix_complex_get(CMAT(THIS), i, j)));
	
	gsl_matrix_complex_free(CMAT(THIS));
	THIS->matrix = m;
	THIS->complex = FALSE;
	return FALSE;
}*/

static void matrix_negative(void *m, bool complex)
{
	uint i;
	gsl_matrix *mm = (gsl_matrix *)m;
	double *d = mm->data;
	uint n = (uint)(mm->size1 * mm->size2);
	
	if (complex)
		n *= 2;
	
	for (i = 0; i < n; i++)
		d[i] = -d[i];
}

static bool matrix_determinant(CMATRIX *m, COMPLEX_VALUE *det) 
{
  int sign = 0; 
	int size = WIDTH(m);
	
	if (size != HEIGHT(m))
		return TRUE;
	
  gsl_permutation *p = gsl_permutation_calloc(size);
	
	if (COMPLEX(m))
	{
		gsl_matrix_complex *tmp = gsl_matrix_complex_alloc(size, size);
		gsl_matrix_complex_memcpy(tmp, CMAT(m));
		gsl_linalg_complex_LU_decomp(tmp, p, &sign);
		det->z = gsl_linalg_complex_LU_det(tmp, sign);
		gsl_matrix_complex_free(tmp);
	}
	else
	{
		gsl_matrix *tmp = gsl_matrix_alloc(size, size);
		gsl_matrix_memcpy(tmp, MAT(m));
		gsl_linalg_LU_decomp(tmp, p, &sign);
		det->x = gsl_linalg_LU_det(tmp, sign);
		det->z.dat[1] = 0;
		gsl_matrix_free(tmp);
	}
	
  gsl_permutation_free(p);
  return FALSE;
}

static void *matrix_invert(void *m, bool complex) 
{
  int sign = 0; 
	int size = ((gsl_matrix *)m)->size1;
	void *result;
	
	if (size != ((gsl_matrix *)m)->size2)
		return NULL;
	
  gsl_permutation *p = gsl_permutation_calloc(size);
	
	if (!complex)
	{
		gsl_matrix *tmp = gsl_matrix_alloc(size, size);
		result = gsl_matrix_alloc(size, size);
		gsl_matrix_memcpy(tmp, (gsl_matrix *)m);
		gsl_linalg_LU_decomp(tmp, p, &sign);
		if (gsl_linalg_LU_invert(tmp, p, (gsl_matrix *)result) != GSL_SUCCESS)
		{
			gsl_matrix_free(result);
			return NULL;
		}
		gsl_matrix_free(tmp);
	}
	else
	{
		gsl_matrix_complex *tmp = gsl_matrix_complex_alloc(size, size);
		result = gsl_matrix_complex_alloc(size, size);
		gsl_matrix_complex_memcpy(tmp, (gsl_matrix_complex *)m);
		gsl_linalg_complex_LU_decomp(tmp, p, &sign);
		if (gsl_linalg_complex_LU_invert(tmp, p, (gsl_matrix_complex *)result) != GSL_SUCCESS)
		{
			gsl_matrix_complex_free(result);
			return NULL;
		}
		gsl_matrix_complex_free(tmp);
	}
	
  gsl_permutation_free(p);
  return result;
}

static void matrix_add_identity(gsl_matrix *m, double f)
{
	gsl_matrix *id = gsl_matrix_alloc(m->size1, m->size2);
	gsl_matrix_set_identity(id);
	gsl_matrix_scale(id, f);
	gsl_matrix_add(m, id);
	gsl_matrix_free(id);
}

static void matrix_complex_add_identity(gsl_matrix_complex *m, gsl_complex c)
{
	gsl_matrix_complex *id = gsl_matrix_complex_alloc(m->size1, m->size2);
	gsl_matrix_complex_set_identity(id);
	gsl_matrix_complex_scale(id, c);
	gsl_matrix_complex_add(m, id);
	gsl_matrix_complex_free(id);
}



//---- Arithmetic operators -------------------------------------------------

#define IMPLEMENT_OP(_name) \
static CMATRIX *_name(CMATRIX *a, CMATRIX *b, bool invert) \
{ \
	CMATRIX *m; \
	\
	if (COMPLEX(a) || COMPLEX(b)) \
	{ \
		MATRIX_ensure_complex(a); \
		MATRIX_ensure_complex(b); \
		m = MAKE_MATRIX(a); \
		CFUNC(CMAT(m), CMAT(a), CMAT(b)); \
	} \
	else \
	{ \
		m = MAKE_MATRIX(a); \
		FUNC(MAT(m), MAT(a), MAT(b)); \
	} \
	 \
	return m; \
}

#define IMPLEMENT_OP_FLOAT(_name) \
static CMATRIX *_name(CMATRIX *a, double f, bool invert) \
{ \
	CMATRIX *m = MAKE_MATRIX(a); \
	\
	if (COMPLEX(a)) \
	{ \
		CFUNC(CMAT(m), CMAT(a), f); \
	} \
	else \
	{ \
		FUNC(MAT(m), MAT(a), f); \
	} \
	 \
	return m; \
}

#define IMPLEMENT_OP_OTHER(_name) \
static CMATRIX *_name(CMATRIX *a, void *b, bool invert) \
{ \
	CMATRIX *m = MAKE_MATRIX(a); \
	\
	if (GB.Is(b, CLASS_Complex)) \
	{ \
		MATRIX_ensure_complex(m); \
		CFUNC(CMAT(m), CMAT(a), ((CCOMPLEX *)b)->number); \
		return m; \
	} \
	else \
		return NULL; \
}

#define MAKE_MATRIX(_a) MATRIX_make(_a)

#define FUNC(_m, _a, _b) gsl_matrix_add(_m, _b)
#define CFUNC(_m, _a, _b) gsl_matrix_complex_add(_m, _b)
IMPLEMENT_OP(_add)
#undef FUNC
#undef CFUNC

#define FUNC(_m, _a, _f) matrix_add_identity(_m, _f)
#define CFUNC(_m, _a, _f) matrix_complex_add_identity(_m, gsl_complex_rect(_f, 0))
IMPLEMENT_OP_FLOAT(_addf)
#undef FUNC
#undef CFUNC

#define CFUNC(_m, _a, _c) matrix_complex_add_identity(_m, _c)
IMPLEMENT_OP_OTHER(_addo)
#undef CFUNC

#define FUNC(_m, _a, _b) gsl_matrix_sub(_m, _b)
#define CFUNC(_m, _a, _b) gsl_matrix_complex_sub(_m, _b)
IMPLEMENT_OP(_sub)
#undef FUNC
#undef CFUNC

#define FUNC(_m, _a, _f) \
	if (invert) \
	{ \
		matrix_negative(_m, FALSE); \
		matrix_add_identity(_m, _f); \
	} \
	else \
		matrix_add_identity(_m, -(_f));

#define CFUNC(_m, _a, _f) \
	if (invert) \
	{ \
		matrix_negative(_m, TRUE); \
		matrix_complex_add_identity(_m, gsl_complex_rect(_f, 0)); \
	} \
	else \
		matrix_complex_add_identity(_m, gsl_complex_rect(-(_f), 0));

IMPLEMENT_OP_FLOAT(_subf)
#undef FUNC
#undef CFUNC

#define CFUNC(_m, _a, _c) \
	if (invert) \
		matrix_negative(_m, TRUE); \
	else \
		gsl_complex_negative(_c); \
	matrix_complex_add_identity(_m, _c);

IMPLEMENT_OP_OTHER(_subo)
#undef CFUNC

#define FUNC(_m, _a, _f) gsl_matrix_scale(_m, _f)
#define CFUNC(_m, _a, _f) gsl_matrix_complex_scale(_m, gsl_complex_rect(_f, 0))
IMPLEMENT_OP_FLOAT(_mulf)
#undef FUNC
#undef CFUNC

#define CFUNC(_m, _a, _c) gsl_matrix_complex_scale(_m, _c)
IMPLEMENT_OP_OTHER(_mulo)
#undef CFUNC

#undef MAKE_MATRIX
#define MAKE_MATRIX(_a) MATRIX_copy(_a)

#define FUNC(_m, _a, _b) gsl_blas_dgemm(CblasNoTrans, CblasNoTrans, 1.0, _a, _b, 0.0, _m);
#define CFUNC(_m, _a, _b) 	gsl_blas_zgemm(CblasNoTrans, CblasNoTrans, COMPLEX_one, _a, _b, COMPLEX_zero, _m);
IMPLEMENT_OP(_mul)
#undef FUNC
#undef CFUNC

#define FUNC(_m, _a, _b) \
{ \
	gsl_matrix *inv = matrix_invert(_b, FALSE); \
	if (!inv) \
		return NULL; \
	gsl_blas_dgemm(CblasNoTrans, CblasNoTrans, 1.0, _a, inv, 0.0, _m); \
	gsl_matrix_free(inv); \
}

#define CFUNC(_m, _a, _b) \
{ \
	gsl_matrix_complex *inv = matrix_invert(_b, TRUE); \
	if (!inv) \
	{ \
		GB.Error(GB_ERR_ZERO); \
		return NULL; \
	} \
	gsl_blas_zgemm(CblasNoTrans, CblasNoTrans, COMPLEX_one, _a, inv, COMPLEX_zero, _m); \
	gsl_matrix_complex_free(inv); \
}

IMPLEMENT_OP(_div)
#undef FUNC
#undef CFUNC

static CMATRIX *_divf(CMATRIX *a, double f, bool invert)
{
	bool complex = COMPLEX(a);
	CMATRIX *m;
	
	if (invert)
	{
		void *inv = matrix_invert(MAT(a), complex);
		
		if (!inv)
		{
			GB.Error(GB_ERR_ZERO);
			return NULL;
		}
		
		m = MATRIX_create_from(inv, complex);
	}
	else
	{
		if (f == 0.0)
		{
			GB.Error(GB_ERR_ZERO);
			return NULL;
		}
		
		f = 1 / f;
		m = MATRIX_make(a);
	}
	
	if (complex)
		gsl_matrix_complex_scale(CMAT(m), gsl_complex_rect(f, 0));
	else
		gsl_matrix_scale(MAT(m), f);
	
	return m;
}

static CMATRIX *_divo(CMATRIX *a, void *b, bool invert)
{
	bool complex = COMPLEX(a);
	CMATRIX *m;
	gsl_complex c;
	
	if (!GB.Is(b, CLASS_Complex))
		return NULL;
	
	c = ((CCOMPLEX *)b)->number;
	
	if (invert)
	{
		void *inv = matrix_invert(MAT(a), complex);
		
		if (!inv)
		{
			GB.Error(GB_ERR_ZERO);
			return NULL;
		}
		
		m = MATRIX_create_from(inv, complex);
	}
	else
	{
		if (GSL_REAL(c) == 0 && GSL_IMAG(c) == 0)
		{
			GB.Error(GB_ERR_ZERO);
			return NULL;
		}
		
		c = gsl_complex_inverse(c);
		m = MATRIX_make(a);
	}
	
	MATRIX_ensure_complex(m);
	gsl_matrix_complex_scale(CMAT(m), c);
	
	return m;
}

static int _equal(CMATRIX *a, CMATRIX *b)
{
	if (WIDTH(a) != WIDTH(b) || HEIGHT(a) != HEIGHT(b))
		return FALSE;
	
	if (COMPLEX(a) || COMPLEX(b))
	{
		MATRIX_ensure_complex(a);
		MATRIX_ensure_complex(b);
		return gsl_matrix_complex_equal(CMAT(a), CMAT(b));
	}
	else
		return gsl_matrix_equal(MAT(a), MAT(b));
}

static int _equalf(CMATRIX *a, double f)
{
	bool result;
	
	if (COMPLEX(a))
	{
		if (f == 0.0)
			return gsl_matrix_complex_isnull(CMAT(a));
		
		gsl_matrix_complex *m = gsl_matrix_complex_alloc(WIDTH(a), HEIGHT(a));
		gsl_matrix_complex_set_identity(m);
		gsl_matrix_complex_scale(m, gsl_complex_rect(f, 0));
		result = gsl_matrix_complex_equal(CMAT(a), m);
		gsl_matrix_complex_free(m);
	}
	else
	{
		if (f == 0.0)
			return gsl_matrix_isnull(MAT(a));
		
		gsl_matrix *m = gsl_matrix_alloc(WIDTH(a), HEIGHT(a));
		gsl_matrix_set_identity(m);
		gsl_matrix_scale(m, f);
		result = gsl_matrix_equal(MAT(a), m);
		gsl_matrix_free(m);
	}
	
	return result;
}

static int _equalo(CMATRIX *a, void *b)
{
	bool result;
	CCOMPLEX *c;
	
	if (!GB.Is(b, CLASS_Complex))
		return -1;
	
	c = (CCOMPLEX *)b;
	
	if (GSL_IMAG(c->number) == 0.0)
		return _equalf(a, GSL_REAL(c->number));
	
	if (!COMPLEX(a))
		return FALSE;
	
	gsl_matrix_complex *m = gsl_matrix_complex_alloc(WIDTH(a), HEIGHT(a));
	gsl_matrix_complex_set_identity(m);
	gsl_matrix_complex_scale(m, c->number);
	result = gsl_matrix_complex_equal(CMAT(a), m);
	gsl_matrix_complex_free(m);
	return result;
}

static CMATRIX *_neg(CMATRIX *a)
{
	CMATRIX *m = MATRIX_make(a);
	matrix_negative(m->matrix, m->complex);
	return m;
}

static CMATRIX *_powi(CMATRIX *m, int n)
{
	if (n == 1)
		return m;
	
	CMATRIX *m2 = _mul(m, m, FALSE);
	CMATRIX *r;
	
	if ((n & 1) == 0)
	{
		n /= 2;
		if (n > 1)
			r = _powi(m2, n);
		else
			r = m2;
	}
	else
	{
		n /= 2;
		if (n > 1)
			r = _powi(m2, n);
		else
			r = m2;
		
		m2 = _mul(r, m, FALSE);
		GB.Unref(POINTER(&r));
		r = m2;
	}
	
	GB.Unref(POINTER(&m));
	return r;
}

static CMATRIX *_powf(CMATRIX *a, double f, bool invert)
{
	if (invert || f != (double)(int)f)
		return NULL;
	
	CMATRIX *m;
	int n = (int)f;
	
	if (n == 0)
	{
		m = MATRIX_make(a);
		if (COMPLEX(m))
			gsl_matrix_complex_set_identity(CMAT(m));
		else
			gsl_matrix_set_identity(MAT(m));
	}
	else if (n == 1)
	{
		m = a;
	}
	else if (n > 1)
	{
		m = _powi(MATRIX_copy(a), n);
	}
	else if (n < 0)
	{
		void *inv = matrix_invert(a->matrix, COMPLEX(a));
		if (inv == NULL)
		{
			GB.Error(GB_ERR_ZERO);
			return NULL;
		}
		
		m = _powi(MATRIX_create_from(inv, COMPLEX(a)), (-n));
	}
	
	return m;
}

static GB_OPERATOR_DESC _operator =
{
	.equal   = (void *)_equal,
	.equalf  = (void *)_equalf,
	.equalo  = (void *)_equalo,
	.add     = (void *)_add,
	.addf    = (void *)_addf,
	.addo    = (void *)_addo,
	.sub     = (void *)_sub,
	.subf    = (void *)_subf,
	.subo    = (void *)_subo,
	.mul     = (void *)_mul,
	.mulf    = (void *)_mulf,
	.mulo    = (void *)_mulo,
	.div     = (void *)_div,
	.divf    = (void *)_divf,
	.divo    = (void *)_divo,
	.powf    = (void *)_powf,
	.neg     = (void *)_neg
};

//---- Conversions ----------------------------------------------------------

static char *_to_string(CMATRIX *_object, bool local)
{
	char *result = NULL;
	int i, j;
	int w = WIDTH(THIS);
	int h = HEIGHT(THIS);
	char *str;
	int len;
	
	result = GB.AddChar(result, '[');
	
	for (i = 0; i < h; i++)
	{
		if (i)
		{
			if (!local)
				result = GB.AddChar(result, ',');
		}
		
		result = GB.AddChar(result, '[');
	
		for (j = 0; j < w; j++)
		{
			if (j)
				result = GB.AddChar(result, local ? ' ' : ',');
			
			if (!COMPLEX(THIS))
			{
				GB.NumberToString(local, gsl_matrix_get(MAT(THIS), i, j), NULL, &str, &len);
				result = GB.AddString(result, str, len);
			}
			else
			{
				str = COMPLEX_to_string(gsl_matrix_complex_get(CMAT(THIS), i, j), local);
				result = GB.AddString(result, str, GB.StringLength(str));
				GB.FreeString(&str);
			}
		}
		
		result = GB.AddChar(result, ']');
	}
	
	result = GB.AddChar(result, ']');
	
	return result;
}

static bool _convert(CMATRIX *_object, GB_TYPE type, GB_VALUE *conv)
{
	if (THIS)
	{
		if (!COMPLEX(THIS))
		{
			switch (type)
			{
				/*case GB_T_FLOAT:
					conv->_float.value = gsl_blas_dnrm2(MAT(THIS));
					return FALSE;
					
				case GB_T_SINGLE:
					conv->_single.value = gsl_blas_dnrm2(MAT(THIS));
					return FALSE;
					
				case GB_T_INTEGER:
				case GB_T_SHORT:
				case GB_T_BYTE:
					conv->_integer.value = gsl_blas_dnrm2(MAT(THIS));
					return FALSE;
					
				case GB_T_LONG:
					conv->_long.value = gsl_blas_dnrm2(MAT(THIS));
					return FALSE;*/
					
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
				/*case GB_T_FLOAT:
					conv->_float.value = gsl_blas_dznrm2(CMAT(THIS));
					return FALSE;
					
				case GB_T_SINGLE:
					conv->_single.value = gsl_blas_dznrm2(CMAT(THIS));
					return FALSE;
					
				case GB_T_INTEGER:
				case GB_T_SHORT:
				case GB_T_BYTE:
					conv->_integer.value = gsl_blas_dznrm2(CMAT(THIS));
					return FALSE;
					
				case GB_T_LONG:
					conv->_long.value = gsl_blas_dznrm2(CMAT(THIS));
					return FALSE;*/
					
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
		/*if (type == CLASS_Complex)
		{
			CCOMPLEX *c = (CCOMPLEX *)conv->_object.value;
			CMATRIX *m = MATRIX_create(2, 2, FALSE, FALSE);
			
			gsl_matrix_set(MAT(m), 0, 0, GSL_REAL(c->number));
			gsl_matrix_set(MAT(m), 1, 1, GSL_REAL(c->number));
			gsl_matrix_set(MAT(m), 0, 1, -GSL_IMAG(c->number));
			gsl_matrix_set(MAT(m), 1, 0, GSL_IMAG(c->number));
			
			conv->_object.value = m;
			return FALSE;
		}
		else*/ if (GB.Is(conv->_object.value, CLASS_Array))
		{
			GB_ARRAY array = (GB_ARRAY)conv->_object.value;
			GB_ARRAY array2;
			int height = GB.Array.Count(array);
			int width = 0;
			GB_TYPE atype = GB.Array.Type(array);
			GB_TYPE atype2;
			int i, j, w;
			GB_VALUE temp;
			void *data;
			CMATRIX *m;
			CCOMPLEX *c;
			bool complex;
			
			if (atype >= GB_T_OBJECT)
			{
				complex = FALSE;
				
				for (i = 0; i < height; i++)
				{
					data = GB.Array.Get(array, i);
					array2 = *((void **)data);
					if (!array2 || !GB.Is(array2, CLASS_Array))
						return TRUE;
					
					w = GB.Array.Count(array2);
					if (w > width)
						width = w;
					
					atype2 = GB.Array.Type(array2);
					
					if (atype2 == GB_T_VARIANT || atype2 == CLASS_Complex)
						complex = TRUE;
					else if (!(atype2 > GB_T_BOOLEAN && atype2 <= GB_T_FLOAT))
						return TRUE;
				}
				
				//fprintf(stderr, "create: %d %d %d\n", width, height, complex);
				m = MATRIX_create(width, height, complex, TRUE);
				
				for (i = 0; i < height; i++)
				{
					array2 = *((void **)GB.Array.Get(array, i));
					atype2 = GB.Array.Type(array2);
					w = GB.Array.Count(array2);
					
					if (atype2 > GB_T_BOOLEAN && atype2 <= GB_T_FLOAT)
					{
						for (j = 0; j < w; j++)
						{
							data = GB.Array.Get(array2, j);
							GB.ReadValue(&temp, data, atype2);
							GB.Conv(&temp, GB_T_FLOAT);
							if (complex)
								gsl_matrix_complex_set(CMAT(m), i, j, gsl_complex_rect(temp._float.value, 0));
							else
								gsl_matrix_set(MAT(m), i, j, temp._float.value);
						}
					}
					else if (atype2 == GB_T_VARIANT)
					{
						for (j = 0; j < w; j++)
						{
							GB.ReadValue(&temp, GB.Array.Get(array2, j), atype2);
							GB.BorrowValue(&temp);
							GB.Conv(&temp, CLASS_Complex);
							c = temp._object.value;
							if (c)
								gsl_matrix_complex_set(CMAT(m), i, j, c->number);
							else
								gsl_matrix_complex_set(CMAT(m), i, j, COMPLEX_zero);
							GB.ReleaseValue(&temp);
						}
					}
					else if (atype2 == CLASS_Complex)
					{
						for (j = 0; j < w; j++)
						{
							c = *((CCOMPLEX **)GB.Array.Get(array2, j));
							if (c)
								gsl_matrix_complex_set(CMAT(m), i, j, c->number);
							else
								gsl_matrix_complex_set(CMAT(m), i, j, COMPLEX_zero);
						}
					}
				}
				
				conv->_object.value = m;
				return FALSE;
			}
		}
	}
	/*else if (type > GB_T_BOOLEAN && type <= GB_T_FLOAT)
	{
		CMATRIX *m = MATRIX_create(2, 2, FALSE, TRUE);
		double value;
		
		if (type == GB_T_FLOAT)
			value = conv->_float.value;
		else if (type == GB_T_SINGLE)
			value = conv->_single.value;
		else
			value = conv->_integer.value;
		
		gsl_matrix_set(MAT(m), 0, 0, value);
		gsl_matrix_set(MAT(m), 1, 1, value);
		
		conv->_object.value = m;
		return FALSE;
	}*/
	
	return TRUE;
}

//---------------------------------------------------------------------------

BEGIN_METHOD(Matrix_new, GB_INTEGER height; GB_INTEGER width; GB_BOOLEAN complex)

	bool complex = VARGOPT(complex, FALSE);
	int h = VARGOPT(height, 2);
	int w = VARGOPT(width, 2);
	
	if (h < 1) h = 1;
	if (w < 1) w = 1;
	
	THIS->complex = complex;
	
	if (!complex)
		THIS->matrix = gsl_matrix_calloc(h, w);
	else
		THIS->matrix = gsl_matrix_complex_calloc(h, w);
	
END_METHOD


BEGIN_METHOD_VOID(Matrix_free)

	if (!COMPLEX(THIS))
		gsl_matrix_free(MAT(THIS));
	else
		gsl_matrix_complex_free(CMAT(THIS));

END_METHOD


BEGIN_PROPERTY(Matrix_Width)

	GB.ReturnInteger(WIDTH(THIS));

END_PROPERTY


BEGIN_PROPERTY(Matrix_Height)

	GB.ReturnInteger(HEIGHT(THIS));

END_PROPERTY


BEGIN_METHOD_VOID(Matrix_Copy)

	GB.ReturnObject(MATRIX_copy(THIS));

END_METHOD


BEGIN_METHOD(Matrix_get, GB_INTEGER i; GB_INTEGER j)

	int w = WIDTH(THIS), h = HEIGHT(THIS);
	int i = VARG(i), j = VARG(j);
	
	if (i < 0 || i >= h || j < 0 || j >= w)
	{
		GB.Error(GB_ERR_BOUND);
		return;
	}
	
	if (!COMPLEX(THIS))
		GB.ReturnFloat(gsl_matrix_get(MAT(THIS), i, j));
	else
		GB.ReturnObject(COMPLEX_create(gsl_matrix_complex_get(CMAT(THIS), i, j)));

	GB.ReturnConvVariant();

END_METHOD


BEGIN_METHOD(Matrix_put, GB_VARIANT value; GB_INTEGER i; GB_INTEGER j)

	int w = WIDTH(THIS), h = HEIGHT(THIS);
	int i = VARG(i), j = VARG(j);
	GB_VALUE *value = (GB_VALUE *)ARG(value);
	int type;
	COMPLEX_VALUE cv;
	
	if (i < 0 || i >= h || j < 0 || j >= w)
	{
		GB.Error(GB_ERR_BOUND);
		return;
	}
	
	type = COMPLEX_get_value(value, &cv);
	
	if (type == CGV_ERR)
		return;

	if (type == CGV_COMPLEX)
	{
		MATRIX_ensure_complex(THIS);
		gsl_matrix_complex_set(CMAT(THIS), i, j, cv.z);
	}
	else
	{
		if (COMPLEX(THIS))
			gsl_matrix_complex_set(CMAT(THIS), i, j, cv.z);
		else
			gsl_matrix_set(MAT(THIS), i, j, cv.x);
	}
	
END_METHOD


BEGIN_PROPERTY(Matrix_Handle)

	GB.ReturnPointer(THIS->matrix);
	
END_PROPERTY


BEGIN_METHOD(Matrix_Identity, GB_INTEGER width; GB_INTEGER height; GB_BOOLEAN complex)

	GB.ReturnObject(MATRIX_identity(VARGOPT(width, 2), VARGOPT(height, 2), VARGOPT(complex, FALSE)));

END_METHOD


BEGIN_METHOD(Matrix_Row, GB_INTEGER row)

	int row = VARG(row);
	
	if (row < 0 || row >= HEIGHT(THIS))
	{
		GB.Error(GB_ERR_BOUND);
		return;
	}
	
	bool complex = COMPLEX(THIS);
	CVECTOR *v = VECTOR_create(WIDTH(THIS), complex, FALSE);

	if (complex)
		gsl_matrix_complex_get_row(CVEC(v), CMAT(THIS), row);
	else
		gsl_matrix_get_row(VEC(v), MAT(THIS), row);
	
	GB.ReturnObject(v);
	
END_METHOD

BEGIN_METHOD(Matrix_Column, GB_INTEGER column)

	int column = VARG(column);
	
	if (column < 0 || column >= WIDTH(THIS))
	{
		GB.Error(GB_ERR_BOUND);
		return;
	}
	
	bool complex = COMPLEX(THIS);
	CVECTOR *v = VECTOR_create(HEIGHT(THIS), complex, FALSE);

	if (complex)
		gsl_matrix_complex_get_col(CVEC(v), CMAT(THIS), column);
	else
		gsl_matrix_get_col(VEC(v), MAT(THIS), column);
	
	GB.ReturnObject(v);

END_METHOD

BEGIN_METHOD(Matrix_SetRow, GB_INTEGER row; GB_OBJECT vector)

	int row = VARG(row);
	
	if (row < 0 || row >= HEIGHT(THIS))
	{
		GB.Error(GB_ERR_BOUND);
		return;
	}
	
	CVECTOR *v = (CVECTOR *)VARG(vector);
	
	if (GB.CheckObject(v))
		return;
	
	if (SIZE(v) != WIDTH(THIS))
	{
		GB.Error("Vector size does not match matrix width");
		return;
	}
	
	bool complex = COMPLEX(THIS);
	
	if (complex)
	{
		VECTOR_ensure_complex(v);
		gsl_matrix_complex_set_row(CMAT(THIS), row, CVEC(v));
	}
	else
	{
		if (VECTOR_ensure_not_complex(v))
		{
			GB.Error(GB_ERR_TYPE, "Float", "Complex");
			return;
		}
		gsl_matrix_set_row(MAT(THIS), row, VEC(v));
	}

END_METHOD

BEGIN_METHOD(Matrix_SetColumn, GB_INTEGER column; GB_OBJECT vector)

	int column = VARG(column);
	
	if (column < 0 || column >= WIDTH(THIS))
	{
		GB.Error(GB_ERR_BOUND);
		return;
	}
	
	CVECTOR *v = (CVECTOR *)VARG(vector);
	
	if (GB.CheckObject(v))
		return;
	
	if (SIZE(v) != HEIGHT(THIS))
	{
		GB.Error("Vector size does not match matrix height");
		return;
	}
	
	bool complex = COMPLEX(THIS);
	
	if (complex)
	{
		VECTOR_ensure_complex(v);
		gsl_matrix_complex_set_col(CMAT(THIS), column, CVEC(v));
	}
	else
	{
		if (VECTOR_ensure_not_complex(v))
		{
			GB.Error(GB_ERR_TYPE, "Float", "Complex");
			return;
		}
		gsl_matrix_set_col(MAT(THIS), column, VEC(v));
	}

END_METHOD

BEGIN_METHOD_VOID(Matrix_Determinant)

	COMPLEX_VALUE cv;
	
	if (matrix_determinant(THIS, &cv))
	{
		GB.Error("Matrix is not square");
		return;
	}
	
	if (COMPLEX(THIS))
		GB.ReturnObject(COMPLEX_create(cv.z));
	else
		GB.ReturnFloat(cv.x);
	
	GB.ReturnConvVariant();

END_METHOD


BEGIN_METHOD(Matrix_call, GB_OBJECT vector)

	CVECTOR *v = VARG(vector);
	CVECTOR *result;
	
	if (GB.CheckObject(v))
		return;
	
	if (COMPLEX(THIS) || v->complex)
	{
		MATRIX_ensure_complex(THIS);
		VECTOR_ensure_complex(v);
		result = VECTOR_create(SIZE(v), TRUE, FALSE);
		gsl_blas_zgemv(CblasNoTrans, COMPLEX_one, CMAT(THIS), CVEC(v), COMPLEX_zero, CVEC(result));
		GB.ReturnObject(result);
	}
	else
	{
		result = VECTOR_create(SIZE(v), FALSE, FALSE);
		gsl_blas_dgemv(CblasNoTrans, 1.0, MAT(THIS), VEC(v), 0.0, VEC(result));
		GB.ReturnObject(result);
	}

END_METHOD


BEGIN_METHOD_VOID(Matrix_Transpose)

	if (!COMPLEX(THIS))
	{
		gsl_matrix *m = gsl_matrix_alloc(WIDTH(THIS), HEIGHT(THIS));
		gsl_matrix_transpose_memcpy(m, MAT(THIS));
		GB.ReturnObject(MATRIX_create_from(m, FALSE));
	}
	else
	{
		gsl_matrix_complex *m = gsl_matrix_complex_alloc(WIDTH(THIS), HEIGHT(THIS));
		gsl_matrix_complex_transpose_memcpy(m, CMAT(THIS));
		GB.ReturnObject(MATRIX_create_from(m, TRUE));
	}

END_METHOD


BEGIN_METHOD_VOID(Matrix_Conjugate)

	CMATRIX *m = MATRIX_copy(THIS);

	if (COMPLEX(THIS))
	{
		int i, j;
		
		for (i = 0; i < HEIGHT(m); i++)
			for (j = 0; j < WIDTH(m); j++)
				gsl_matrix_complex_set(CMAT(m), i, j, gsl_complex_conjugate(gsl_matrix_complex_get(CMAT(m), i, j)));
	}
		
	GB.ReturnObject(m);

END_METHOD


BEGIN_METHOD_VOID(Matrix_Invert)

	void *m = matrix_invert(THIS->matrix, COMPLEX(THIS));

	if (!m)
		GB.ReturnNull();
	else
		GB.ReturnObject(MATRIX_create_from(m, COMPLEX(THIS)));

END_METHOD


BEGIN_METHOD(Matrix_ToString, GB_BOOLEAN local)

	GB.ReturnString(GB.FreeStringLater(_to_string(THIS, VARGOPT(local, FALSE))));

END_METHOD


//---------------------------------------------------------------------------

GB_DESC MatrixDesc[] =
{
	GB_DECLARE("Matrix", sizeof(CMATRIX)),
	
	GB_METHOD("_new", NULL, Matrix_new, "[(Width)i(Height)i(Complex)b]"),
	GB_METHOD("_free", NULL, Matrix_free, NULL),
	//GB_STATIC_METHOD("_call", "Vector", Matrix_call, "(Value)f."),
	GB_METHOD("Copy", "Matrix", Matrix_Copy, NULL),
	GB_METHOD("ToString", "s", Matrix_ToString, "[(Local)b]"),

	GB_STATIC_METHOD("Identity", "Matrix", Matrix_Identity, "[(Width)i(Height)i(Complex)b]"),
	
	GB_PROPERTY_READ("Width", "i", Matrix_Width),
	GB_PROPERTY_READ("Height", "i", Matrix_Height),
	GB_PROPERTY_READ("Handle", "p", Matrix_Handle),
	
	GB_METHOD("Det", "v", Matrix_Determinant, NULL),
	
	GB_METHOD("_get", "v", Matrix_get, "(I)i(J)i"),
	GB_METHOD("_put", NULL, Matrix_put, "(Value)v(I)i(J)i"),

	GB_METHOD("_call", "Vector", Matrix_call, "(Vector)Vector"),
	
	//GB_METHOD("Equal", "b", Matrix_Equal, "(Matrix)Matrix;"),
	
	GB_METHOD("Row", "Vector", Matrix_Row, "(Row)i"),
	GB_METHOD("Column", "Vector", Matrix_Column, "(Column)i"),
	GB_METHOD("SetRow", NULL, Matrix_SetRow, "(Row)i(Vector)Vector;"),
	GB_METHOD("SetColumn", NULL, Matrix_SetColumn, "(Column)i(Vector)Vector;"),
	
	//GB_METHOD("Scale", "Matrix", Matrix_Scale, "(Value)v"),
	GB_METHOD("Trans", "Matrix", Matrix_Transpose, NULL),
	GB_METHOD("Conj", "Matrix", Matrix_Conjugate, NULL),
	GB_METHOD("Inv", "Matrix", Matrix_Invert, NULL),
	
	GB_INTERFACE("_convert", &_convert),
	GB_INTERFACE("_operator", &_operator),
	
	GB_END_DECLARE
};
