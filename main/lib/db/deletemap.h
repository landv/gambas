/***************************************************************************

  deletemap.h

  Tracking deletion in an array

  (c) 2000-2005 Benoît Minisini <gambas@users.sourceforge.net>

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

#ifndef __DELETEMAP_H
#define __DELETEMAP_H

typedef
  void *DELETE_MAP;
  
long DELETE_MAP_virtual_to_real(DELETE_MAP *dmap, long vpos);
long DELETE_MAP_real_to_virtual(DELETE_MAP *dmap, long rpos);
void DELETE_MAP_add(DELETE_MAP **dmap, long vpos);
void DELETE_MAP_free(DELETE_MAP **dmap);

#endif
