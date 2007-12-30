/***************************************************************************

  mathext.c

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

#define __GBX_MATH_C

#include <math.h>
#include <time.h>
#include <sys/time.h>

#include "gb_common.h"
#include "gbx_math.h"

/* This is a twisted generalized feedback shift register
   that generates pseudo-random numbers.

   The source code from the paper of Yann Guidon
   in March edition of GNU/Linux Magazine France
*/

#define CRC32_STD 0x04C11DB7
#define GFSR_SIZE 15

static uint GFSR_table[GFSR_SIZE];
static uint GFSR_temp;
static uint GFSR_index;

static void GFSR_init(uint seed)
{
	int i = 0, j;
	uint t = seed;

	do
	{
		t ^= (t >> 5) ^ (t << 1);
		j = t >> 31;
		t <<= 1;
		if (j)
			t ^= CRC32_STD;
		GFSR_table[i++] = t;
	}
	while (i < GFSR_SIZE);

	GFSR_temp = GFSR_table[GFSR_SIZE - 1];
	GFSR_index = 0;
}

static uint GFSR_random(void)
{
	int t;

	GFSR_temp ^= GFSR_table[GFSR_index];

	t = GFSR_temp;
	GFSR_temp <<= 1;
	if (t < 0)
		GFSR_temp ^= CRC32_STD;

	GFSR_table[GFSR_index] = GFSR_temp;

	if (++GFSR_index >= GFSR_SIZE)
		GFSR_index = 0;

	return GFSR_temp;
}

//static long seed = 1;

PUBLIC double frac(double x)
{
  x = fabs(x);
  return x - floor(x);
}

PUBLIC int lsgn(long x)
{
  return ((x > 0) ? 1 : ((x < 0) ? (-1) : 0));
}

PUBLIC int llsgn(long long x)
{
  return ((x > 0) ? 1 : ((x < 0) ? (-1) : 0));
}

PUBLIC long long llabs(long long x)
{
  return ((x < 0) ? (-x) : x);
}

PUBLIC int fsgn(double x)
{
  return ((x > 0) ? 1 : ((x < 0) ? (-1) : 0));
}

PUBLIC double deg(double x)
{
  return x * 180 / M_PI;
}

PUBLIC double rad(double x)
{
  return x * M_PI / 180;
}

PUBLIC double fix(double x)
{
  if (x >= 0)
    return floor(x);
  else
    return -floor(fabs(x));
}


PUBLIC double frexp10(double x, int *exp)
{
  long double l, f;

  if (x == 0.0)
  {
    *exp = 0;
    return x;
  }

  l = modfl(log10l(fabsl(x)), &f);

  if (f == 0.0)
    l = x;
  else
    l = powl(10, l);

  if (x < 0.0) l = -l;

  while (l >= 1.0)
  {
    l /= 10.0;
    f++;
  }

  *exp = (int)f;
  return l;
}


PUBLIC void randomize(bool set, uint seed)
{
  struct timeval tv;

  if (!set && gettimeofday(&tv, NULL) == 0)
    seed = tv.tv_sec * 1000 + tv.tv_usec / 1000;

	//GFSR_init(0x1A2B3C4D);
	GFSR_init(seed);
}


PUBLIC double rnd(void)
{
  /*seed = 16807L * (seed % 127773L) - 2836L * (seed / 127773L);
  if (seed <= 0) seed += 2147483647;

  return (double)seed / 2147483648.0;*/

  unsigned long long val;

	val = GFSR_random();
	val <<= 32;
	val |= GFSR_random();

	return (double)val / 0xFFFFFFFFFFFFFFFFULL;
}


PUBLIC double ang(double x, double y)
{
  return atan2(y, x);
}

#ifdef OS_FREEBSD

PUBLIC double exp10(double x)
{
	return pow(10, x);
}

PUBLIC double log2(double x)
{
	return log(x) / M_LN2;
}

#endif

PUBLIC void MATH_init(void)
{
	randomize(FALSE, 0);
}
