/***************************************************************************

  getoptions.h

  (c) 2000-2009 Chintan Rao <chintanraoh@gmail.com>

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

#ifndef __GETOPTIONS_H
#define __GETOPTIONS_H

#include "gambas.h"


#ifndef __GETOPTIONS_C

extern GB_DESC GetOptionsDesc[];

#else

typedef
struct {
  GB_BASE ob;
  char *options;
  char **argv;
  int arg_count;
  char **opt_found;
  char **opt_arg;
  char **invalid;
  GB_ARRAY rest;
  GB_ARRAY cmdline;
  GB_ARRAY return_temp;
  int index;
}
COPTIONS;

#define THIS  OBJECT(COPTIONS)
	
#endif

#endif
