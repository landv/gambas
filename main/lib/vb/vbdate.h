/***************************************************************************

  vbdate.h

  Visual Basic compatibility Component date manipulation

  (c) 2000-2003 Nigel Gerrard <nigel@gerrard1123.freeserve.co.uk>

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

#ifndef __VBDATE_H
#define __VBDATE_H

#include "main.h"

void DATE_adjust( GB_DATE *vdate, int period, int interval); /* Adjust the date by the interval period */
long int DATE_diff( GB_DATE *vdate1, GB_DATE *vdate2, int period);

#endif
