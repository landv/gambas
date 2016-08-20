/***************************************************************************

  garray.h

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

#ifndef __GARRAY_H
#define __GARRAY_H

#include "gb_common.h"
#include "gambas.h"

extern "C" GB_INTERFACE GB;

class GArrayImpl
{
protected:
	void **array;
	bool autoDelete;
	uint pos;
	uint _count;

public:
	GArrayImpl() { GB.NewArray((void **)(void *)&array, sizeof(void *), 0); autoDelete = false; _count = 0; }
	~GArrayImpl() { GB.FreeArray((void **)(void *)&array); }

  uint count() const { return _count; }
  void setAutoDelete(bool a) { autoDelete = a; }
  void *last() const { return count() ? array[count() - 1] : 0; }
  void append(const void *d) { *((const void **)GB.Add((void **)(void *)&array)) = d; _count++; }
  void clear();
  void *take();
  void *at(uint i) const { return array[i]; }
  void insert(uint i, const void *d);
  void remove(uint i);
  int find(const void *d);
  void *first() { pos = 0; return next(); }
  void *next() { return (pos >= count()) ? 0 : array[pos++]; }
};

template<class type>
class GArray : GArrayImpl
{
public:
	GArray() {}
	~GArray() { clear(); }

  uint count() const { return GArrayImpl::count(); }
  bool isEmpty() const { return GArrayImpl::count() == 0; }
  void setAutoDelete(bool a) { GArrayImpl::setAutoDelete(a); }
  type *last() const { return (type *)GArrayImpl::last(); }
  void append(const type *d) { GArrayImpl::append(d); }

	void clear();
  type *take() { return (type *)GArrayImpl::take(); }
  type *at(uint i) const { return (type *)GArrayImpl::at(i); }
  void insert(uint i, const type *d) { GArrayImpl::insert(i, d); }
  void remove(uint i);
  int find(const type *d) { return GArrayImpl::find(d); }
  void removeRef(const type *d);

  type *first() { return (type *)GArrayImpl::first(); }
  type *next() { return (type *)GArrayImpl::next(); }
};


template <class type> inline void GArray<type>::clear()
{
	if (autoDelete)
	{
		for (uint i = 0; i < count(); i++)
			delete (type *)array[i];
	}

	GArrayImpl::clear();
}

template <class type> inline void GArray<type>::remove(uint i)
{
	if (autoDelete)
		delete (type *)array[i];

	GArrayImpl::remove(i);
}


template <class type> inline void GArray<type>::removeRef(const type *d)
{
	int i = find(d);
	if (i >= 0)
		remove((uint )i);
}


#endif
