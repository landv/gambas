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

BEGIN_METHOD(GLBINDFRAMEBUFFEREXT, GB_INTEGER target; GB_INTEGER framebuffer)

	glBindFramebufferEXT(VARG(target), VARG(framebuffer));

END_METHOD

BEGIN_METHOD(GLBINDRENDERBUFFEREXT, GB_INTEGER target; GB_INTEGER renderbuffer)

	glBindRenderbufferEXT(VARG(target), VARG(renderbuffer));

END_METHOD

BEGIN_METHOD(GLCHECKFRAMEBUFFERSTATUSEXT, GB_INTEGER target)

	GB.ReturnInteger(glCheckFramebufferStatusEXT(VARG(target)));

END_METHOD

BEGIN_METHOD(GLDELETEFRAMEBUFFERSEXT, GB_OBJECT buffers)


	GB_ARRAY iArray = (GB_ARRAY) VARG(buffers);
	int i,count = GB.Array.Count(iArray);
	GLuint buffer[1];
	
	if (count<=0)
		return;

	for (i=0;i<count; i++)
	{
		buffer[0]=*((GLuint *)GB.Array.Get(iArray,i));
		glDeleteFramebuffersEXT(1, buffer);
	}

END_METHOD

BEGIN_METHOD(GLDELETERENDERBUFFERSEXT, GB_OBJECT buffers)


	GB_ARRAY iArray = (GB_ARRAY) VARG(buffers);
	int i,count = GB.Array.Count(iArray);
	GLuint buffer[1];
	
	if (count<=0)
		return;

	for (i=0;i<count; i++)
	{
		buffer[0]=*((GLuint *)GB.Array.Get(iArray,i));
		glDeleteRenderbuffersEXT(1, buffer);
	}

END_METHOD

BEGIN_METHOD(GLFRAMEBUFFERRENDERBUFFEREXT, GB_INTEGER target; GB_INTEGER attachment; GB_INTEGER rbtarget; GB_INTEGER buffer)

	 glFramebufferRenderbufferEXT(VARG(target), VARG(attachment), VARG(rbtarget), VARG(buffer));

END_METHOD

BEGIN_METHOD(GLFRAMEBUFFERTEXTURE1DEXT, GB_INTEGER target; GB_INTEGER attachment; GB_INTEGER textarget; GB_INTEGER texture; GB_INTEGER level)

	 glFramebufferTexture1DEXT(VARG(target), VARG(attachment), VARG(textarget), VARG(texture), VARG(level));

END_METHOD

BEGIN_METHOD(GLFRAMEBUFFERTEXTURE2DEXT, GB_INTEGER target; GB_INTEGER attachment; GB_INTEGER textarget; GB_INTEGER texture; GB_INTEGER level)

	 glFramebufferTexture2DEXT(VARG(target), VARG(attachment), VARG(textarget), VARG(texture), VARG(level));

END_METHOD

BEGIN_METHOD(GLFRAMEBUFFERTEXTURE3DEXT, GB_INTEGER target; GB_INTEGER attachment; GB_INTEGER textarget; GB_INTEGER texture; GB_INTEGER level; GB_INTEGER zoffset)

	 glFramebufferTexture3DEXT(VARG(target), VARG(attachment), VARG(textarget), VARG(texture), VARG(level), VARG(zoffset));

END_METHOD

BEGIN_METHOD(GLGENERATEMIPMAPEXT, GB_INTEGER target)

	glGenerateMipmapEXT(VARG(target));

END_METHOD

BEGIN_METHOD(GLGENFRAMEBUFFERSEXT, GB_INTEGER count)

	GLuint framebuffers[VARG(count)];
	int i, count = VARG(count);
	GB_ARRAY iArray;

	if (count<=0)
		return;

	GB.Array.New(&iArray , GB_T_INTEGER , count);
	glGenFramebuffersEXT(VARG(count), framebuffers);

	for (i=0;i<count; i++)
		*((GLuint *)GB.Array.Get(iArray, i)) = framebuffers[i];
	
	GB.ReturnObject(iArray);

END_METHOD

BEGIN_METHOD(GLGENRENDERBUFFERSEXT, GB_INTEGER count)

	GLuint renderbuffers[VARG(count)];
	int i, count = VARG(count);
	GB_ARRAY iArray;

	if (count<=0)
		return;

	GB.Array.New(&iArray , GB_T_INTEGER , count);
	glGenRenderbuffersEXT(VARG(count), renderbuffers);

	for (i=0;i<count; i++)
		*((GLuint *)GB.Array.Get(iArray, i)) = renderbuffers[i];
	
	GB.ReturnObject(iArray);

END_METHOD

BEGIN_METHOD(GLGETFRAMEBUFFERATTACHMENTPARAMETERIVEXT, GB_INTEGER target; GB_INTEGER attachment; GB_INTEGER pname)

	int params[1];
	GB_ARRAY iArray;

	GB.Array.New(&iArray , GB_T_INTEGER , 1);
	glGetFramebufferAttachmentParameterivEXT(VARG(target), VARG(attachment), VARG(pname), params);

	*((GLuint *)GB.Array.Get(iArray, 0)) = params[0];
	GB.ReturnObject(iArray);

END_METHOD

BEGIN_METHOD(GLGETRENDERBUFFERPARAMETERIVEXT, GB_INTEGER target; GB_INTEGER pname)

	int params[1];
	GB_ARRAY iArray;

	GB.Array.New(&iArray , GB_T_INTEGER , 1);
	glGetRenderbufferParameterivEXT(VARG(target), VARG(pname), params);

	*((GLuint *)GB.Array.Get(iArray, 0)) = params[0];
	GB.ReturnObject(iArray);

END_METHOD

BEGIN_METHOD(GLISFRAMEBUFFEREXT, GB_INTEGER buffer)

	GB.ReturnBoolean(glIsFramebufferEXT(VARG(buffer)));

END_METHOD

BEGIN_METHOD(GLISRENDERBUFFEREXT, GB_INTEGER buffer)

	GB.ReturnBoolean(glIsRenderbufferEXT(VARG(buffer)));

END_METHOD

BEGIN_METHOD(GLRENDERBUFFERSTORAGEEXT, GB_INTEGER target; GB_INTEGER format; GB_INTEGER width; GB_INTEGER height)

	 glRenderbufferStorageEXT(VARG(target), VARG(format), VARG(width), VARG(height));

END_METHOD

// 