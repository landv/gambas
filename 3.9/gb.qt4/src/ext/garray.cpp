/***************************************************************************

  garray.cpp

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

#define __G_ARRAY_CPP

#include "garray.h"

void GArrayImpl::clear()
{
  GB.FreeArray((void **)(void *)&array);
  GB.NewArray((void **)(void *)&array, sizeof(void *), 0);
	_count = 0;
}

void *GArrayImpl::take()
{
	void *d;

	if (count() == 0)
		return 0;

	d = array[count() - 1];
	GB.Remove((void **)(void *)&array, count() - 1, 1);
	_count--;
	return d;
}

void GArrayImpl::insert(uint i, const void *d)
{
	GB.Insert((void **)(void *)&array, i, 1);
	array[i] = (void *)d;
	_count++;
}


void GArrayImpl::remove(uint i)
{
	GB.Remove((void **)(void *)&array, i, 1);
	_count--;
}

int GArrayImpl::find(const void *d)
{
	int i;

	for (i = 0; i < (int)count(); i++)
		if (array[i] == d)
			return i;

	return (-1);
}
