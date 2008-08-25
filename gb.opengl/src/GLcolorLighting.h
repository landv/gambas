/***************************************************************************

  GLcolorLighting.h

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

#ifndef __GLCOLORLIGHTING_H
#define __GLCOLORLIGHTING_H

#include "gambas.h"
#include "main.h"

#include <GL/gl.h>

DECLARE_METHOD(GLCOLORF);
DECLARE_METHOD(GLCOLORI);
DECLARE_METHOD(GLCOLORFV);
DECLARE_METHOD(GLCOLORIV);
DECLARE_METHOD(GLCOLORMATERIAL);
DECLARE_METHOD(GLFRONTFACE);
DECLARE_METHOD(GLGETLIGHTFV);
DECLARE_METHOD(GLGETLIGHTIV);
DECLARE_METHOD(GLGETMATERIALFV);
DECLARE_METHOD(GLGETMATERIALIV);
DECLARE_METHOD(GLINDEXF);
DECLARE_METHOD(GLINDEXI);
DECLARE_METHOD(GLLIGHTF);
DECLARE_METHOD(GLLIGHTI);
DECLARE_METHOD(GLLIGHTFV);
DECLARE_METHOD(GLLIGHTIV);
DECLARE_METHOD(GLLIGHTMODELF);
DECLARE_METHOD(GLLIGHTMODELI);
DECLARE_METHOD(GLLIGHTMODELFV);
DECLARE_METHOD(GLLIGHTMODELIV);
DECLARE_METHOD(GLMATERIALF);
DECLARE_METHOD(GLMATERIALI);
DECLARE_METHOD(GLNORMAL3FV);
DECLARE_METHOD(GLNORMAL3IV);
DECLARE_METHOD(GLMATERIALFV);
DECLARE_METHOD(GLMATERIALIV);
DECLARE_METHOD(GLNORMAL3F);
DECLARE_METHOD(GLNORMAL3I);
DECLARE_METHOD(GLSHADEMODEL);

#endif /* __GLCOLORLIGHTING_H */
