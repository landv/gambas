/*
 * c_graphmatrix.h
 *
 * Copyright (C) 2014 Tobias Boege <tobias@gambas-buch.de>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301, USA.
 */

#ifndef __C_GRAPHMATRIX_H
#define __C_GRAPHMATRIX_H

#include "gambas.h"

extern GB_INTERFACE GB;

#ifndef __C_GRAPHMATRIX_C
extern GB_DESC CGraphMatrix[], CMatrixVertices[], CMatrixEdges[],
	       CMatrixVertex[], CMatrixEdge[];
#endif

#endif /* __C_GRAPHMATRIX_H */
