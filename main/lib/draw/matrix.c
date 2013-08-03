/***************************************************************************

  matrix.c

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

#define __MATRIX_C

#include "gb_common.h"
#include "matrix.h"
#include "main.h"

static INLINE int DROUND(double d)
{
	return d >= 0.0 ? (int)(d + 0.5) : (int)( d - ((int)d-1) + 0.5 ) + ((int)d-1);
}

static INLINE double DMIN(double a, double b)
{
	return (a > b ? b : a);
}

static INLINE double DMAX(double a, double b)
{
	return (a > b ? a : b);
}

void MATRIX_reset(MATRIX *matrix)
{
	matrix->m11 = matrix->m22 = 1;
	matrix->m12 = matrix->m21 = 0;
	matrix->dx = matrix->dy = 0;
	matrix->identity = TRUE;
	matrix->rotation = FALSE;
}

void MATRIX_init(MATRIX *matrix)
{
	matrix->next = NULL;
	MATRIX_reset(matrix);
}

#define MAPDOUBLE(mat, x, y, nx, ny ) \
{ \
    double fx = x; \
    double fy = y; \
    nx = mat->m11*fx + mat->m21*fy + mat->dx; \
    ny = mat->m12*fx + mat->m22*fy + mat->dy; \
}

void MATRIX_map_point(MATRIX *matrix, int *x, int *y)
{
	double nx;
	double ny;
	
	MAPDOUBLE(matrix, *x, *y, nx, ny);
	
	*x = DROUND(nx);
	*y = DROUND(ny);
}

void MATRIX_map_rect(MATRIX *matrix, int *x, int *y, int *w, int *h)
{
	int rx, ry, rw, rh;
	
	if (matrix->m12 == 0.0F && matrix->m21 == 0.0F) 
	{
		rx = DROUND(matrix->m11 * *x + matrix->dx);
		ry = DROUND(matrix->m22 * *y + matrix->dy);
		rw = DROUND(matrix->m11 * *w);
		rh = DROUND(matrix->m22 * *h);
			
		if (rw < 0) 
		{
			rw = -rw;
			rx -= rw - 1;
		}
			
		if (rh < 0) 
		{
			rh = -rh;
			ry -= rh-1;
		}
	} 
	else 
	{
		int left = *x;
		int top = *y;
		int right = *x + *w;
		int bottom = *y + *h;
		
		double x0, y0;
		double x, y;
		MAPDOUBLE(matrix, left, top, x0, y0 );
		double xmin = x0;
		double ymin = y0;
		double xmax = x0;
		double ymax = y0;
		MAPDOUBLE(matrix, right, top, x, y );
		xmin = DMIN( xmin, x );
		ymin = DMIN( ymin, y );
		xmax = DMAX( xmax, x );
		ymax = DMAX( ymax, y );
		MAPDOUBLE(matrix, right, bottom, x, y );
		xmin = DMIN( xmin, x );
		ymin = DMIN( ymin, y );
		xmax = DMAX( xmax, x );
		ymax = DMAX( ymax, y );
		MAPDOUBLE(matrix, left, bottom, x, y );
		xmin = DMIN( xmin, x );
		ymin = DMIN( ymin, y );
		xmax = DMAX( xmax, x );
		ymax = DMAX( ymax, y );
		double ww = xmax - xmin;
		double hh = ymax - ymin;
		xmin -= ( xmin - x0 ) / ww;
		ymin -= ( ymin - y0 ) / hh;
		xmax -= ( xmax - x0 ) / ww;
		ymax -= ( ymax - y0 ) / hh;
		
		rx = DROUND(xmin);
		ry = DROUND(ymin);
		rw = DROUND(xmax) - DROUND(xmin) + 1;
		rh = DROUND(ymax) - DROUND(ymin) + 1;
	}
	
	*x = rx;
	*y = ry;
	*w = rw;
	*h = rh;
}

int *MATRIX_map_array(MATRIX *matrix, int *coord, int npoint)
{
	int *map_coord, *map;
	int i;
	
	GB.Alloc(POINTER(&map_coord), sizeof(int) * npoint * 2);
	
	map = map_coord;
	for (i = 0; i < npoint; i++)
	{
		map[0] = coord[0];
		map[1] = coord[1];
		MATRIX_map_point(matrix, &map[0], &map[1]); 
		coord += 2;
		map += 2;
	}
	
	return map_coord;
}

void MATRIX_free_array(int **coord)
{
	GB.Free(POINTER(coord));
}


static void update_flag(MATRIX *matrix)
{
	matrix->identity = 
		matrix->m11 == 1.0 && matrix->m22 == 1.0 
		&& matrix->m12 == 0.0 && matrix->m21 == 0.0 
		&& matrix->dx == 0.0 && matrix->dy == 0.0;
		
	matrix->rotation = matrix->m12 != 0.0 || matrix->m21 != 0.0;
}

void MATRIX_translate(MATRIX *matrix, double dx, double dy)
{
	matrix->dx += dx * matrix->m11 + dy * matrix->m21;
	matrix->dy += dy * matrix->m22 + dx * matrix->m12;
	
	update_flag(matrix);
}

void MATRIX_scale(MATRIX *matrix, double sx, double sy)
{
	matrix->m11 *= sx;
	matrix->m12 *= sx;
	matrix->m21 *= sy;
	matrix->m22 *= sy;
	
	update_flag(matrix);
}

void MATRIX_rotate(MATRIX *matrix, double angle)
{
	double sina = sin(angle);
	double cosa = cos(angle);
		
	double m11 = cosa * matrix->m11 + sina * matrix->m21;
	double m12 = cosa * matrix->m12 + sina * matrix->m22;
	double m21 = sina * matrix->m11 + cosa * matrix->m21;
	double m22 = sina * matrix->m12 + cosa * matrix->m22;
	
	matrix->m11 = m11;
	matrix->m12 = m12;
	matrix->m21 = m21; 
	matrix->m22 = m22;
	
	update_flag(matrix);
}

