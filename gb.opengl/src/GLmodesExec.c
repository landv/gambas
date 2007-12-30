/***************************************************************************

  GLmodesExec.c

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

#define __GLMODESEXEC_C

#include "gambas.h"
#include "gb_common.h"
#include "main.h"

#include <GL/gl.h>

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
