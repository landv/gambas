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

static CMATRIX *MATRIX_create(int width, int height, bool complex)
{
	GB.Push(3, GB_T_INTEGER, width, GB_T_INTEGER, height, GB_T_BOOLEAN, complex);
	return (CMATRIX *)GB.New(CLASS_Matrix, NULL, (void *)(intptr_t)3);
}

static CMATRIX *MATRIX_copy(CMATRIX *_object)
{
	CMATRIX *copy = MATRIX_create(WIDTH(THIS), HEIGHT(THIS), COMPLEX(THIS));
	if (COMPLEX(THIS))
		gsl_matrix_memcpy(MAT(copy), MAT(THIS));
	else
		gsl_matrix_complex_memcpy(CMAT(copy), CMAT(THIS));
	
	return copy;
}

static CMATRIX *MATRIX_convert_to_complex(CMATRIX *_object)
{
	CMATRIX *m = MATRIX_create(WIDTH(THIS), HEIGHT(THIS), TRUE);
	int i, j;
	
	for (i = 0; i < HEIGHT(THIS); i++)
		for (j = 0; j < WIDTH(THIS); j++)
			gsl_matrix_complex_set(CMAT(m), i, j, gsl_complex_rect(gsl_matrix_get(MAT(THIS), i, j), 0));
	
	return m;
}

static void ensure_complex(CMATRIX *_object)
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


/*static bool ensure_not_complex(CMATRIX *_object)
{
	gsl_matrix *v;
	int size = SIZE(THIS);
	int i;
	gsl_complex c;
	
	if (!COMPLEX(THIS))
		return FALSE;
	
	for (i = 0; i < size; i++)
	{
		c = gsl_matrix_complex_get(CMAT(THIS), i);
		if (GSL_IMAG(c) != 0.0)
			return TRUE;
	}
	
	v = gsl_matrix_alloc(size);
	
	for (i = 0; i < size; i++)
		gsl_matrix_set(v, i, GSL_REAL(gsl_matrix_complex_get(CMAT(THIS), i)));
	
	gsl_matrix_complex_free(CMAT(THIS));
	THIS->matrix = v;
	THIS->complex = FALSE;
	return FALSE;
}*/

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
	
	for (i = 0; i < w; i++)
	{
		result = GB.AddChar(result, '[');
	
		for (j = 0; j < h; j++)
		{
			if (j)
				result = GB.AddChar(result, ' ');
			
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
		if (type == CLASS_Complex)
		{
			CCOMPLEX *c = (CCOMPLEX *)conv->_object.value;
			CMATRIX *m = MATRIX_create(2, 2, FALSE);
			
			gsl_matrix_set(MAT(m), 0, 0, GSL_REAL(c->number));
			gsl_matrix_set(MAT(m), 1, 1, GSL_REAL(c->number));
			gsl_matrix_set(MAT(m), 0, 1, -GSL_IMAG(c->number));
			gsl_matrix_set(MAT(m), 1, 0, GSL_IMAG(c->number));
			
			conv->_object.value = m;
			return FALSE;
		}
		else if (GB.Is(conv->_object.value, CLASS_Array))
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
				m = MATRIX_create(width, height, complex);
				
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
	else if (type > GB_T_BOOLEAN && type <= GB_T_FLOAT)
	{
		CMATRIX *m = MATRIX_create(2, 2, FALSE);
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
	}		
	
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
	gsl_complex z;
	double x;
	
	if (i < 0 || i >= h || j < 0 || j >= w)
	{
		GB.Error(GB_ERR_BOUND);
		return;
	}
	
	type = COMPLEX_get_value(value, &x, &z);
	
	if (type == CGV_ERR)
		return;

	if (type == CGV_COMPLEX)
	{
		ensure_complex(THIS);
		gsl_matrix_complex_set(CMAT(THIS), i, j, z);
	}
	else
	{
		if (COMPLEX(THIS))
			gsl_matrix_complex_set(CMAT(THIS), i, j, z);
		else
			gsl_matrix_set(MAT(THIS), i, j, x);
	}
	
END_METHOD


BEGIN_METHOD(Matrix_Scale, GB_VALUE value)

	GB_VALUE *value = (GB_VALUE *)ARG(value);
	int type;
	gsl_complex z;
	double x;
	
	type = COMPLEX_get_value(value, &x, &z);
	
	if (type == CGV_ERR)
		return;

	if (type == CGV_COMPLEX)
	{
		ensure_complex(THIS);
		gsl_matrix_complex_scale(CMAT(THIS), z);
	}
	else
	{
		if (COMPLEX(THIS))
			gsl_matrix_complex_scale(CMAT(THIS), z);
		else
			gsl_matrix_scale(MAT(THIS), x);
	}
	
	GB.ReturnObject(THIS);
		
END_METHOD

BEGIN_METHOD(Matrix_Equal, GB_OBJECT matrix)

	CMATRIX *m = VARG(matrix);
	bool ca, cb;
	
	if (GB.CheckObject(m))
		return;
	
	if (WIDTH(THIS) != WIDTH(m) || HEIGHT(THIS) != HEIGHT(m))
	{
		GB.ReturnBoolean(FALSE);
		return;
	}
	
	ca = !COMPLEX(THIS);
	cb = !COMPLEX(m);
	
	if (ca && cb)
	{
		GB.ReturnBoolean(gsl_matrix_equal(MAT(THIS), MAT(m)));
	}
	else
	{
		CMATRIX *a, *b;
		
		if (ca)
			a = MATRIX_convert_to_complex(THIS);
		else
			a = THIS;
		
		if (cb)
			b = MATRIX_convert_to_complex(m);
		else
			b = m;
		
		GB.ReturnBoolean(gsl_matrix_complex_equal(CMAT(a), CMAT(b)));
		
		if (ca) GB.Unref(POINTER(&a));
		if (cb) GB.Unref(POINTER(&b));
	}


END_METHOD


BEGIN_PROPERTY(Matrix_Handle)

	GB.ReturnPointer(THIS->matrix);
	
END_PROPERTY


BEGIN_METHOD(Matrix_Identity, GB_INTEGER width; GB_INTEGER height; GB_BOOLEAN complex)

	CMATRIX *m = MATRIX_create(VARGOPT(width, 2), VARGOPT(height, 2), VARGOPT(complex, FALSE));
	
	if (COMPLEX(m))
		gsl_matrix_complex_set_identity(CMAT(m));
	else
		gsl_matrix_set_identity(MAT(m));
	
	GB.ReturnObject(m);

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


GB_DESC MatrixDesc[] =
{
	GB_DECLARE("Matrix", sizeof(CMATRIX)),
	
	GB_METHOD("_new", NULL, Matrix_new, "[(Width)i(Height)i(Complex)b]"),
	GB_METHOD("_free", NULL, Matrix_free, NULL),
	//GB_STATIC_METHOD("_call", "Vector", Matrix_call, "(Value)f."),
	GB_METHOD("Copy", "Matrix", Matrix_Copy, NULL),

	GB_STATIC_METHOD("Identity", "Matrix", Matrix_Identity, "[(Width)i(Height)i(Complex)b]"),
	
	GB_PROPERTY_READ("Width", "i", Matrix_Width),
	GB_PROPERTY_READ("Height", "i", Matrix_Height),
	GB_PROPERTY_READ("Handle", "p", Matrix_Handle),
	
	GB_METHOD("_get", "v", Matrix_get, "(I)i(J)i"),
	GB_METHOD("_put", NULL, Matrix_put, "(Value)v(I)i(J)i"),

	GB_METHOD("Scale", "Matrix", Matrix_Scale, "(Value)v"),
	GB_METHOD("Equal", "b", Matrix_Equal, "(Matrix)Matrix;"),
	
	GB_METHOD("Row", "Vector", Matrix_Row, "(Row)i"),
	GB_METHOD("Column", "Vector", Matrix_Column, "(Column)i"),
	GB_METHOD("SetRow", NULL, Matrix_SetRow, "(Row)i(Vector)Vector;"),
	GB_METHOD("SetColumn", NULL, Matrix_SetColumn, "(Column)i(Vector)Vector;"),
	
	GB_INTERFACE("_convert", &_convert),
	
	GB_END_DECLARE
};
