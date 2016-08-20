/***************************************************************************

  matrix.h

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

#ifndef __MATRIX_H
#define __MATRIX_H

#include "gb_common.h"
#include "gb.draw.h"

typedef
	GB_MATRIX
	MATRIX;

void MATRIX_init(MATRIX *matrix);
void MATRIX_reset(MATRIX *matrix);

void MATRIX_translate(MATRIX *matrix, double dx, double dy);
void MATRIX_scale(MATRIX *matrix, double sx, double sy);
void MATRIX_rotate(MATRIX *matrix, double angle);

void MATRIX_map_point(MATRIX *matrix, int *x, int *y);
void MATRIX_map_rect(MATRIX *matrix, int *x, int *y, int *w, int *h);

int *MATRIX_map_array(MATRIX *matrix, int *coord, int npoint);
void MATRIX_free_array(int **coord);

#define MATRIX_is_identity(_matrix) ((_matrix)->identity)

#endif
