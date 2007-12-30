/***************************************************************************

  GLtextureMapping.h

  openGL component

  (c) 2005 Laurent Carlier <lordheavy@users.sourceforge.net>
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

#ifndef __GLTEXTUREMAPPING_H
#define __GLTEXTUREMAPPING_H

#include "gambas.h"
#include "gb_common.h"
#include "main.h"

#include <GL/gl.h>

DECLARE_METHOD(GLBINDTEXTURE);
DECLARE_METHOD(GLCOPYTEXIMAGE1D);
DECLARE_METHOD(GLCOPYTEXIMAGE2D);
DECLARE_METHOD(GLDELETETEXTURES);
DECLARE_METHOD(GLGENTEXTURES);
DECLARE_METHOD(GLISTEXTURE);
DECLARE_METHOD(GLTEXCOORD1F);
DECLARE_METHOD(GLTEXCOORD1I);
DECLARE_METHOD(GLTEXCOORD2F);
DECLARE_METHOD(GLTEXCOORD2I);
DECLARE_METHOD(GLTEXCOORD3F);
DECLARE_METHOD(GLTEXCOORD3I);
DECLARE_METHOD(GLTEXCOORD4F);
DECLARE_METHOD(GLTEXCOORD4I);
DECLARE_METHOD(GLTEXENVF);
DECLARE_METHOD(GLTEXENVFV);
DECLARE_METHOD(GLTEXENVI);
DECLARE_METHOD(GLTEXENVIV);
DECLARE_METHOD(GLTEXIMAGE1D);
DECLARE_METHOD(GLTEXIMAGE2D);
DECLARE_METHOD(GLTEXPARAMETERF);
DECLARE_METHOD(GLTEXPARAMETERFV);
DECLARE_METHOD(GLTEXPARAMETERI);
DECLARE_METHOD(GLTEXPARAMETERIV);

#endif /* __GLTEXTUREMAPPING_H */
