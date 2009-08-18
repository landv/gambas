/***************************************************************************

  GLprimitives.h

  The Gambas openGL component

  (c) 2005-2007 Laurent Carlier <lordheavy@users.sourceforge.net>
                Benoît Minisini <gambas@users.sourceforge.net>

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

#ifndef __GLPRIMITIVES_H
#define __GLPRIMITIVES_H

#include "gambas.h"
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
