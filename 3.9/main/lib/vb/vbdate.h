/***************************************************************************

  vbdate.h

  (c) 2000-2003 Nigel Gerrard <nigel@gerrard1123.freeserve.co.uk>

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

#ifndef __VBDATE_H
#define __VBDATE_H

#include "main.h"

void DATE_adjust( GB_DATE *vdate, int period, int interval); /* Adjust the date by the interval period */
int DATE_diff( GB_DATE *vdate1, GB_DATE *vdate2, int period);

#endif
