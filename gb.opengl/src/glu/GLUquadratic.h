/***************************************************************************

  GLUquadratic.h

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

#ifndef __GLUQUADRATIC_H
#define __GLUQUADRATIC_H

#include "main.h"
#include "cgluquadric.h"

DECLARE_METHOD(GLUNEWQUADRIC);
DECLARE_METHOD(GLUQUADRICNORMALS);
DECLARE_METHOD(GLUQUADRICTEXTURE);
DECLARE_METHOD(GLUQUADRICORIENTATION);
DECLARE_METHOD(GLUQUADRICDRAWSTYLE);
DECLARE_METHOD(GLUSPHERE);
DECLARE_METHOD(GLUCYLINDER);
DECLARE_METHOD(GLUDISK);
DECLARE_METHOD(GLUPARTIALDISK);

#endif /* __GLUQUADRATIC_H */
