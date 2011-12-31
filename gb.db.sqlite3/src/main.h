/***************************************************************************

  main.h

  (c) 2000-2012 Benoît Minisini <gambas@users.sourceforge.net>

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

#ifndef __MAIN_H
#define __MAIN_H

extern "C"
{
#include "gambas.h"
#include "gb_common.h"
#include "../gb.db.h"

#ifndef __MAIN_C
	extern GB_INTERFACE GB;
	extern DB_INTERFACE DB;
#endif
}

#define QUOTE_STRING "'"

#define MAX_PATH 132						/* MAX LENGTH OF FILNAME PATH */
#define TRUE 1
#define FALSE 0

/* Prototypes Required to allow cpp compilation */
#include "gb.db.proto.h"

#endif /* __MAIN_H */
