/***************************************************************************

  gbx_math.c

  (c) 2000-2011 Benoît Minisini <gambas@users.sourceforge.net>

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

#define __GBX_MATH_C

#include <math.h>
#include <time.h>
#include <sys/time.h>

#include "gb_common.h"
#include "gb_hash.h"
#include "gbx_math.h"

/* This is a twisted generalized feedback shift register
   that generates pseudo-random numbers.

   Source code from the paper of Yann Guidon
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

double frac(double x)
{
  x = fabs(x);
  return x - floor(x);
}

int lsgn(int x)
{
  return ((x > 0) ? 1 : ((x < 0) ? (-1) : 0));
}

int llsgn(int64_t x)
{
  return ((x > 0) ? 1 : ((x < 0) ? (-1) : 0));
}

/*int64_t llabs(int64_t x)
{
  return ((x < 0) ? (-x) : x);
}*/

int fsgn(double x)
{
  return ((x > 0) ? 1 : ((x < 0) ? (-1) : 0));
}

float fixf(float x)
{
  if (x >= 0)
    return floorf(x);
  else
    return -floorf(fabsf(x));
}

double fix(double x)
{
  if (x >= 0)
    return floor(x);
  else
    return -floor(fabs(x));
}

#if 0
double frexp10_old(double x, int *exp)
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
#endif

double frexp10(double x, int *exp)
{
	int p;
	
	if (x == 0.0)
	{
		*exp = 0;
		return x;
	}
	
	p = (int)log10(x);
	x /= pow(10, p);

	if (x >= 1)
	{
		x /= 10;
		p++;
	}
	
	*exp = p;
	return x;
}


void randomize(bool set, uint seed)
{
  struct timeval tv;

  if (!set && gettimeofday(&tv, NULL) == 0)
    seed = tv.tv_sec * 1000 + tv.tv_usec / 1000;

	//GFSR_init(0x1A2B3C4D);
	GFSR_init(seed);
}


double rnd(void)
{
  /*seed = 16807L * (seed % 127773L) - 2836L * (seed / 127773L);
  if (seed <= 0) seed += 2147483647;

  return (double)seed / 2147483648.0;*/

  uint64_t val;

	val = GFSR_random();
	val <<= 32;
	val |= GFSR_random();

	return (double)val / 18446744073709551616.0; //0xFFFFFFFFFFFFFFFFULL;
}

#ifndef HAVE_EXP10
double exp10(double x)
{
	return pow(10, x);
}
#endif

#ifndef HAVE_LOG2
double log2(double x)
{
	return log(x) / M_LN2;
}
#endif

#ifndef HAVE_EXP2
double exp2(double x)
{
	return pow(2, x);
}
#endif

#ifndef HAVE_LOG10L
long double log10l(long double x)
{
	return log10((double) x);
}
#endif

#ifndef HAVE_FABSL
long double fabsl(long double x)
{
	return fabs((double) x);
}
#endif

#ifndef HAVE_POWL
long double powl(long double x, long double y)
{
	return pow((double) x, (double) y);
}
#endif

#ifndef HAVE_MODFL
long double modfl(long double x, long double *iptr)
{
	double val;
	return modf((double)x, &val);
	*iptr = val;
}
#endif

static int nbits(uint n)
{
  uint c;
  for (c = 0; n; c++) 
    n &= n - 1;
  return c;
}

void MATH_init(void)
{
	uint seed;
	
	randomize(FALSE, 0);

	// Internet condom
	do
		seed = GFSR_random();
	while (nbits(seed) < 16);
		
	HASH_seed = seed;
}
