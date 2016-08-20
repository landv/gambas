/***************************************************************************

  GLrasterization.h

  (c) 2005-2007 Laurent Carlier <lordheavy@users.sourceforge.net>

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

#ifndef __GLRASTERIZATION_H
#define __GLRASTERIZATION_H

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
DECLARE_METHOD(GLPOLYGONOFFSET);
DECLARE_METHOD(GLRASTERPOS2F);
DECLARE_METHOD(GLRASTERPOS3F);
DECLARE_METHOD(GLRASTERPOSF);
DECLARE_METHOD(GLRASTERPOS2I);
DECLARE_METHOD(GLRASTERPOS3I);
DECLARE_METHOD(GLRASTERPOSI);
DECLARE_METHOD(GLRASTERPOSFV);
DECLARE_METHOD(GLRASTERPOSIV);

#endif /* __GLRASTERIZATION_H */
