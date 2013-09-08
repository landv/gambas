/***************************************************************************

  cmd2object.h

  (c) 2013 Benoît Minisini <gambas@users.sourceforge.net>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General 		License as published by
  the Free Software Foundation; either version 2, or (at your option)
  any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General 		License for more details.

  You should have received a copy of the GNU General 		License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
  MA 02110-1301, USA.

***************************************************************************/

#ifndef __CMD2OBJECT_H
#define __CMD2OBJECT_H

#include "gambas.h"
#include "main.h"
#include "cmd2model.h"

#ifndef __CMD2OBJECT_C
extern GB_DESC Md2ObjectDesc[];
#endif

typedef
	struct {
		GB_BASE ob;
		struct CMD2MODEL *model;
		float pos[3];
		float scale[3];
		float rotate[4];
		double frame;
		GLuint texture;
		GB_VARIANT tag;
		}
	CMD2OBJECT;

#endif
