/***************************************************************************

  GLframeBufferOps.c

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

#define __GLFRAMEBUFFEROPS_C

#include "GL.h"

/***************************************************************************/

BEGIN_METHOD(GLACCUM, GB_INTEGER operation; GB_FLOAT value)

	glAccum(VARG(operation), VARG(value));

END_METHOD

BEGIN_METHOD(GLALPHAFUNC, GB_INTEGER function; GB_FLOAT reference)

	glAlphaFunc(VARG(function), VARG(reference));

END_METHOD

BEGIN_METHOD(GLBLENDFUNC, GB_INTEGER sfactor; GB_INTEGER dfactor)

	glBlendFunc(VARG(sfactor), VARG(dfactor));

END_METHOD

BEGIN_METHOD(GLCLEAR, GB_INTEGER mask)

	glClear(VARG(mask));

END_METHOD

BEGIN_METHOD(GLCLEARACCUM, GB_FLOAT red; GB_FLOAT green; GB_FLOAT blue; GB_FLOAT alpha)

	glClearAccum(VARG(red), VARG(green), VARG(blue), VARG(alpha));

END_METHOD

BEGIN_METHOD(GLCLEARCOLOR, GB_FLOAT red; GB_FLOAT green; GB_FLOAT blue; GB_FLOAT alpha)

	glClearColor(VARG(red), VARG(green), VARG(blue), VARG(alpha));

END_METHOD

BEGIN_METHOD(GLCLEARDEPTH, GB_FLOAT depth)

	glClearDepth(VARG(depth));

END_METHOD

BEGIN_METHOD(GLCLEARINDEX, GB_FLOAT value)

	glClearIndex(VARG(value));

END_METHOD

BEGIN_METHOD(GLCLEARSTENCIL, GB_INTEGER value)

	glClearStencil(VARG(value));

END_METHOD

BEGIN_METHOD(GLCOLORMASK, GB_BOOLEAN red; GB_BOOLEAN green; GB_BOOLEAN blue; GB_BOOLEAN alpha)

	glColorMask(VARG(red), VARG(green), VARG(blue), VARG(alpha));

END_METHOD

BEGIN_METHOD(GLDEPTHFUNC, GB_INTEGER function)

	glDepthFunc(VARG(function));

END_METHOD

BEGIN_METHOD(GLDEPTHMASK, GB_BOOLEAN flag)

	glDepthMask(VARG(flag));

END_METHOD

BEGIN_METHOD(GLDRAWBUFFER, GB_INTEGER mode)

	glDrawBuffer(VARG(mode));

END_METHOD

BEGIN_METHOD(GLINDEXMASK, GB_INTEGER mask)

	glIndexMask(VARG(mask));

END_METHOD

BEGIN_METHOD(GLLOGICOP, GB_INTEGER opcode)

	glLogicOp(VARG(opcode));

END_METHOD

BEGIN_METHOD(GLSCISSOR, GB_INTEGER x; GB_INTEGER y; GB_INTEGER width; GB_INTEGER height)

	glScissor(VARG(x), VARG(y), VARG(width), VARG(height));

END_METHOD

BEGIN_METHOD(GLSTENCILFUNC, GB_INTEGER function; GB_INTEGER reference; GB_INTEGER mask)

	glStencilFunc(VARG(function), VARG(reference), VARG(mask));

END_METHOD

BEGIN_METHOD(GLSTENCILMASK, GB_INTEGER mask)

	glStencilMask(VARG(mask));

END_METHOD

BEGIN_METHOD(GLSTENCILOP, GB_INTEGER fail; GB_INTEGER zfail; GB_INTEGER zpass)

	glStencilOp(VARG(fail), VARG(zfail), VARG(zpass));

END_METHOD
