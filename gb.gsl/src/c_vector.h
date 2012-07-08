/***************************************************************************

	c_vector.h

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

#ifndef __C_VECTOR_H
#define __C_VECTOR_H

#include "main.h"

#ifndef __C_VECTOR_C
extern GB_DESC VectorDesc[];
#endif

typedef
	struct
	{
		GB_BASE ob;
		gsl_vector vector;
	}
	GSLVECTOR;

#endif /* __C_VECTOR_H */
