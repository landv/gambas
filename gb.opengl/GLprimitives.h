/***************************************************************************

  GLprimitives.h

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

#ifndef __GLPRIMITIVES_H
#define __GLPRIMITIVES_H

#include "main.h"

DECLARE_METHOD(GLBEGIN);
DECLARE_METHOD(GLEDGEFLAG);
DECLARE_METHOD(GLEND);
DECLARE_METHOD(GLRECTF);
DECLARE_METHOD(GLRECTI);
DECLARE_METHOD(GLVERTEX2F);
DECLARE_METHOD(GLVERTEX3F);
DECLARE_METHOD(GLVERTEXF);
DECLARE_METHOD(GLVERTEX2I);
DECLARE_METHOD(GLVERTEX3I);
DECLARE_METHOD(GLVERTEXI);
DECLARE_METHOD(GLVERTEXFV);
DECLARE_METHOD(GLVERTEXIV);

#endif /* __GLPRIMITIVES_H */
