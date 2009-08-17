/***************************************************************************

  GLUcoordTransf.c

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
  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

***************************************************************************/

#define __GLUCOORDTRANSF_C

#include "gambas.h"
#include "main.h"

#include <GL/glu.h>

/**************************************************************************/

BEGIN_METHOD(GLULOOKAT, GB_FLOAT eyex; GB_FLOAT eyey; GB_FLOAT eyez; GB_FLOAT centerx; GB_FLOAT centery; GB_FLOAT centerz; GB_FLOAT upx; GB_FLOAT upy; GB_FLOAT upz)

	gluLookAt(VARG(eyex),VARG(eyey),VARG(eyez),VARG(centerx),VARG(centery),VARG(centerz),VARG(upx),VARG(upy),VARG(upz));

END_METHOD

BEGIN_METHOD(GLUORTHO2D, GB_FLOAT left; GB_FLOAT right; GB_FLOAT bottom; GB_FLOAT top)

	gluOrtho2D(VARG(left),VARG(right),VARG(bottom),VARG(top));

END_METHOD

BEGIN_METHOD(GLUPERSPECTIVE, GB_FLOAT fovy; GB_FLOAT aspect; GB_FLOAT znear; GB_FLOAT zfar)

	gluPerspective(VARG(fovy),VARG(aspect),VARG(znear),VARG(zfar));

END_METHOD

