/***************************************************************************

  gb_common_swap_temp.h

  common useful routines template

  Datatype management routines. Conversions between each Gambas datatype,
  and conversions between Gambas datatypes and native datatypes.

  (c) 2000-2004 Beno� Minisini <gambas@users.sourceforge.net>

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

PUBLIC void SWAP_long(long *val)
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

PUBLIC void SWAP_longs(long *val, int n)
{
	while (n > 0)
	{
		SWAP_long(val);
		val++;
		n--;
	}
}



PUBLIC void SWAP_short(short *val)
{
  char *p = (char *)val;
  char t;

  t = p[0];
  p[0] = p[1];
  p[1] = t;
}


PUBLIC void SWAP_double(double *val)
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

