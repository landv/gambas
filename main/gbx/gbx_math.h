/***************************************************************************

  gbx_math.h

  (c) 2000-2012 Benoît Minisini <gambas@users.sourceforge.net>

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

#ifndef __GBX_MATH_H
#define __GBX_MATH_H

#include "config.h"

#ifdef OS_FREEBSD
#undef HAVE_EXP10
#endif

#ifndef __GBX_MATH_C
extern const double MATH_pow10[];
#endif

void MATH_init(void);

int lsgn(int x);
int llsgn(int64_t x);
//int64_t llabs(int64_t x);

double frac(double x);
int fsgn(double x);
float fixf(float x);
double fix(double x);
double frexp10(double x, int *exp);

#define pow10(_n) (((_n) >= -10 && (_n) <= 16) ? MATH_pow10[(_n) + 10] : pow(10, (_n)))

void randomize(bool set, uint seed);
double rnd(void);

#define deg(_x) ((_x) * 180 / M_PI)
#define rad(_x) ((_x) * M_PI / 180)

#ifndef HAVE_EXP10
double exp10(double x);
#endif

#ifndef HAVE_LOG2
double log2(double x);
#endif

#ifndef HAVE_EXP2
double exp2(double x);
#endif

#endif
