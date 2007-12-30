/***************************************************************************

  GLcoordTransf.h

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

#ifndef __GLCOORDTRANSF_H
#define __GLCOORDTRANSF_H

#include "gambas.h"
#include "gb_common.h"
#include "main.h"

#include <GL/gl.h>

DECLARE_METHOD(GLDEPTHRANGE);
DECLARE_METHOD(GLFRUSTUM);
DECLARE_METHOD(GLLOADIDENTITY);
DECLARE_METHOD(GLLOADMATRIXF);
DECLARE_METHOD(GLMATRIXMODE);
DECLARE_METHOD(GLMULTMATRIXF);
DECLARE_METHOD(GLORTHO);
DECLARE_METHOD(GLPOPMATRIX);
DECLARE_METHOD(GLPUSHMATRIX);
DECLARE_METHOD(GLROTATEF);
DECLARE_METHOD(GLSCALEF);
DECLARE_METHOD(GLTRANSLATEF);
DECLARE_METHOD(GLVIEWPORT);

#endif /* __GLCOORDTRANSF_H */
