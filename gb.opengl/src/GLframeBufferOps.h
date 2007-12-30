/***************************************************************************

  GLframeBufferOps.h

  (c) 2005 Laurent Carlier <lordheavy@users.sourceforge.net>
           Beno� Minisini <gambas@users.sourceforge.net>
  
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

#ifndef __GLFRAMEBUFFEROPS_H
#define __GLFRAMEBUFFEROPS_H

#include "gambas.h"
#include "main.h"

#include <GL/gl.h>

DECLARE_METHOD(GLACCUM);
DECLARE_METHOD(GLALPHAFUNC);
DECLARE_METHOD(GLBLENDFUNC);
DECLARE_METHOD(GLCLEAR);
DECLARE_METHOD(GLCLEARACCUM);
DECLARE_METHOD(GLCLEARCOLOR);
DECLARE_METHOD(GLCLEARDEPTH);
DECLARE_METHOD(GLCLEARINDEX);
DECLARE_METHOD(GLCLEARSTENCIL);
DECLARE_METHOD(GLCOLORMASK);
DECLARE_METHOD(GLDEPTHFUNC);
DECLARE_METHOD(GLDEPTHMASK);
DECLARE_METHOD(GLDRAWBUFFER);
DECLARE_METHOD(GLINDEXMASK);
DECLARE_METHOD(GLLOGICOP);
DECLARE_METHOD(GLSCISSOR);
DECLARE_METHOD(GLSTENCILFUNC);
DECLARE_METHOD(GLSTENCILMASK);
DECLARE_METHOD(GLSTENCILOP);

#endif /* __GLFRAMEBUFFEROPS_H */
