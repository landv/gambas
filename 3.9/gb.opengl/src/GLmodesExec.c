/***************************************************************************

  GLmodesExec.c

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

#define __GLMODESEXEC_C

#include "GL.h"

/**************************************************************************/

BEGIN_METHOD(GLDISABLE, GB_INTEGER capacity)

	glDisable(VARG(capacity));
  
END_METHOD

BEGIN_METHOD(GLENABLE, GB_INTEGER capacity)

	glEnable(VARG(capacity));
  
END_METHOD

BEGIN_METHOD_VOID(GLFLUSH)

	glFlush();
  
END_METHOD

BEGIN_METHOD_VOID(GLFINISH)

	glFinish();
  
END_METHOD

BEGIN_METHOD(GLHINT, GB_INTEGER target; GB_INTEGER mode)

	glHint(VARG(target), VARG(mode));
  
END_METHOD

BEGIN_METHOD(GLISENABLED, GB_INTEGER capacity)

	GB.ReturnBoolean(glIsEnabled(VARG(capacity)));
  
END_METHOD
