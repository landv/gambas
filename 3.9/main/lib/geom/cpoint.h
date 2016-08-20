/***************************************************************************

  cpoint.h

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

#ifndef __CPOINT_H
#define __CPOINT_H

#include "gambas.h"
#include "main.h"

#ifndef __CRECT_C
extern GB_DESC PointDesc[];
extern GB_DESC PointFDesc[];
#endif

typedef
	struct {
		GB_BASE ob;
		int x;
		int y;
		}
	CPOINT;

typedef
	struct {
		GB_BASE ob;
		double x;
		double y;
		}
	CPOINTF;

#endif

CPOINT * CPOINT_create(int x, int y);
CPOINTF * CPOINTF_create(double x, double y);
