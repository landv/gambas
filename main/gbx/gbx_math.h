/***************************************************************************

  mathext.h

  Mathematical extension routines

  (c) 2000-2007 Benoit Minisini <gambas@users.sourceforge.net>

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

void MATH_init(void);

int lsgn(int x);
int llsgn(int64_t x);
//int64_t llabs(int64_t x);

double frac(double x);
int fsgn(double x);
double deg(double x);
double rad(double x);
double fix(double x);
double frexp10(double x, int *exp);
double ang(double x, double y);

void randomize(bool set, uint seed);
double rnd(void);

#if defined(OS_FREEBSD) || defined(OS_OPENBSD) || defined(OS_CYGWIN)
double exp10(double x);
#ifdef log2
#undef log2
#endif
double log2(double x);
#endif

#if defined(OS_OPENBSD) || defined(OS_CYGWIN)
double exp2(double x);
long double log10l(long double x);
long double fabsl(long double x);
long double powl(long double x, long double y);
#endif

#ifdef OS_CYGWIN
long double modfl(long double x, long double *p);
#endif

#endif
