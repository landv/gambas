/***************************************************************************

  cdrom.h

  (c) 2004,2005 Laurent Carlier <lordheavy@users.sourceforge.net>

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

#ifndef __CDROM_H
#define __CDROM_H

#include "gambas.h"

#include "SDL.h"

typedef
  struct {
    GB_BASE ob;
    SDL_CD *cdrom;
    int id;
    long index;
    }
  CCDROM;

#ifndef __CDROM_C
extern GB_DESC Cquerycdrom[];
extern GB_DESC Ctrack[];
extern GB_DESC Ctracks[];
extern GB_DESC Ccdrom[];
#else

#define CDROM   ((CCDROM *)_object)->cdrom
#define THIS    ((CCDROM *)_object)

#endif /* __CDROM_C */

#endif /* __CDROM_H */
