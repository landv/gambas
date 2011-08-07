/***************************************************************************

  framebufferobject.c

  (c) 2011 Laurent Carlier <lordheavy@users.sourceforge.net>

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

#define __FBO_C

#include "GL.h"

BEGIN_METHOD(GLFRAMEBUFFERTEXTURE2DEXT, GB_INTEGER Target; GB_INTEGER Attachment; GB_INTEGER Textarget; GB_INTEGER Texture; GB_INTEGER Level)

	 glFramebufferTexture2D(VARG(Target), VARG(Attachment), VARG(Textarget), VARG(Texture), VARG(Level));

END_METHOD

BEGIN_METHOD(GLGENFRAMEBUFFERSEXT, GB_INTEGER N)

	GLuint framebuffers;
	glGenFramebuffersEXT(VARG(N), &framebuffers);
	GB.ReturnInteger(framebuffers);

END_METHOD

BEGIN_METHOD(GLBINDFRAMEBUFFERSEXT, GB_INTEGER Target; GB_INTEGER Framebuffer)

	glBindFramebufferEXT(VARG(Target), VARG(Framebuffer));

END_METHOD

BEGIN_METHOD(GLCHECKFRAMEBUFFERSTATUSEXT, GB_INTEGER Target)

	GLuint result;
	result = glCheckFramebufferStatusEXT (VARG(Target));
	GB.ReturnInteger(result);

END_METHOD

// 