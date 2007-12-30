/***************************************************************************

  GLrasterization.h

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

#ifndef __GLRASTERIZATION_H
#define __GLRASTERIZATION_H

#include "gambas.h"
#include "main.h"

// TODO
DECLARE_METHOD(GLBITMAP);
DECLARE_METHOD(GLPOLYGONSTIPPLE);
DECLARE_METHOD(GLGETPOLYGONSTIPPLE);
//
DECLARE_METHOD(GLCULLFACE);
DECLARE_METHOD(GLLINESTIPPLE);
DECLARE_METHOD(GLLINEWIDTH);
DECLARE_METHOD(GLPOINTSIZE);
DECLARE_METHOD(GLPOLYGONMODE);
DECLARE_METHOD(GLPOLYGONSTIPPLE);
DECLARE_METHOD(GLRASTERPOSF);
DECLARE_METHOD(GLRASTERPOSI);
DECLARE_METHOD(GLRASTERPOSFV);
DECLARE_METHOD(GLRASTERPOSIV);

#endif /* __GLRASTERIZATION_H */
