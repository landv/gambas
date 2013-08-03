/***************************************************************************

  gb_common_swap_temp.h

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

void SWAP_int(int *val)
{
  char *p = (char *)val;
  char t1, t2;

  t1 = p[0];
  t2 = p[1];
  p[0] = p[3];
  p[1] = p[2];
  p[3] = t1;
  p[2] = t2;
}

void SWAP_ints(int *val, int n)
{
	while (n > 0)
	{
		SWAP_int(val);
		val++;
		n--;
	}
}

void SWAP_short(short *val)
{
  char *p = (char *)val;
  char t;

  t = p[0];
  p[0] = p[1];
  p[1] = t;
}

void SWAP_double(double *val)
{
  char *p = (char *)val;
  char t;
  int i, j;

  for (i = 0; i < 4; i++)
  {
    j = i ^ 7;
    t = p[i];
    p[i] = p[j];
    p[j] = t;
  }
}

