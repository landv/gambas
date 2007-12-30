/***************************************************************************

  mathext.h

  Mathematical extension routines

  (c) 2000-2005 Beno� Minisini <gambas@users.sourceforge.net>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 1, or (at your option)
  any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

***************************************************************************/

#ifndef __GBX_MATH_H
#define __GBX_MATH_H

#include "config.h"

PUBLIC void MATH_init(void);

PUBLIC int lsgn(long x);
PUBLIC int llsgn(long long x);
PUBLIC long long llabs(long long x);

PUBLIC double frac(double x);
PUBLIC int fsgn(double x);
PUBLIC double deg(double x);
PUBLIC double rad(double x);
PUBLIC double fix(double x);
PUBLIC double frexp10(double x, int *exp);
PUBLIC double ang(double x, double y);

PUBLIC void randomize(bool set, uint seed);
PUBLIC double rnd(void);

#ifdef OS_FREEBSD
PUBLIC double exp10(double x);
PUBLIC double log2(double x);
#endif

#endif
