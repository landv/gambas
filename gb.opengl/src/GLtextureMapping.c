/***************************************************************************

  GLtextureMapping.c

  openGL component

  (c) 2005 Laurent Carlier <lordheavym@gmail.com>
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

#define __GLTEXTUREMAPPING_C

#include "gambas.h"
#include "gb_common.h"
#include "main.h"

#include <GL/gl.h>

BEGIN_METHOD(GLBINDTEXTURE, GB_INTEGER target; GB_INTEGER texture)

	glBindTexture(VARG(target), VARG(texture));

END_METHOD

BEGIN_METHOD(GLCOPYTEXIMAGE1D, GB_INTEGER Ta; GB_INTEGER Le; GB_INTEGER Fo; GB_INTEGER X; GB_INTEGER Y; GB_INTEGER Wi; GB_INTEGER Bo)

	glCopyTexImage1D(VARG(Ta), VARG(Le), VARG(Fo), VARG(X), VARG(Y), VARG(Wi), VARG(Bo));

END_METHOD

BEGIN_METHOD(GLCOPYTEXIMAGE2D, GB_INTEGER Ta; GB_INTEGER Le; GB_INTEGER Fo; GB_INTEGER X; GB_INTEGER Y; GB_INTEGER Wi; GB_INTEGER He; GB_INTEGER Bo)

	glCopyTexImage2D(VARG(Ta), VARG(Le), VARG(Fo), VARG(X), VARG(Y), VARG(Wi), VARG(He), VARG(Bo));

END_METHOD

BEGIN_METHOD(GLDELETETEXTURES, GB_OBJECT textures)


	GB_ARRAY iArray = (GB_ARRAY) VARG(textures);
	int i,count = GB.Array.Count(iArray);
	GLuint texture[1];
	
	if (count<=0)
		return;

	for (i=0;i<count; i++)
	{
		texture[0]=*((GLuint *)GB.Array.Get(iArray,i));
		glDeleteTextures(1, texture);
	}

END_METHOD

BEGIN_METHOD(GLGENTEXTURES, GB_INTEGER count)

	GLuint textures[VARG(count)];
	int i, count = VARG(count);
	GB_ARRAY iArray;

	if (count<=0)
		return;

	GB.Array.New(&iArray , GB_T_INTEGER , count);
	glGenTextures(VARG(count), textures);

	for (i=0;i<count; i++)
		*((GLuint *)GB.Array.Get(iArray, i)) = textures[i];
	
	GB.ReturnObject(iArray);

END_METHOD

BEGIN_METHOD(GLISTEXTURE, GB_INTEGER texture)

	GB.ReturnBoolean(glIsTexture(VARG(texture)));

END_METHOD

BEGIN_METHOD(GLTEXCOORD1F, GB_FLOAT S)

	glTexCoord1d(VARG(S));

END_METHOD

BEGIN_METHOD(GLTEXCOORD1I, GB_INTEGER S)

	glTexCoord1i(VARG(S));

END_METHOD

BEGIN_METHOD(GLTEXCOORD2F, GB_FLOAT S; GB_FLOAT T)

	glTexCoord2d(VARG(S), VARG(T));

END_METHOD

BEGIN_METHOD(GLTEXCOORD2I, GB_INTEGER S; GB_INTEGER T)

	glTexCoord2i(VARG(S), VARG(T));

END_METHOD

BEGIN_METHOD(GLTEXCOORD3F, GB_FLOAT S; GB_FLOAT T; GB_FLOAT R)

	glTexCoord3d(VARG(S), VARG(T), VARG(R));

END_METHOD

BEGIN_METHOD(GLTEXCOORD3I, GB_INTEGER S; GB_INTEGER T; GB_INTEGER R)

	glTexCoord3i(VARG(S), VARG(T), VARG(R));

END_METHOD

BEGIN_METHOD(GLTEXCOORD4F, GB_FLOAT S; GB_FLOAT T; GB_FLOAT R; GB_FLOAT Q)

	glTexCoord4d(VARG(S), VARG(T), VARG(R), VARG(Q));

END_METHOD

BEGIN_METHOD(GLTEXCOORD4I, GB_INTEGER S; GB_INTEGER T; GB_INTEGER R; GB_INTEGER Q)

	glTexCoord4i(VARG(S), VARG(T), VARG(R), VARG(Q));

END_METHOD

BEGIN_METHOD(GLTEXENVF, GB_INTEGER Target; GB_INTEGER Pname; GB_FLOAT Param)

	glTexEnvf(VARG(Target), VARG(Pname), VARG(Param));

END_METHOD

BEGIN_METHOD(GLTEXENVFV, GB_INTEGER Target; GB_INTEGER Pname; GB_OBJECT Params)

	GLfloat params[4];
	GB_ARRAY fArray = (GB_ARRAY) VARG(Params);
	int count = GB.Array.Count(fArray);
	int i;
	
	if (count>4)
		count==4;

	for (i=0;i<count; i++)
		params[i]=*((GLfloat *)GB.Array.Get(fArray,i));

	glTexParameterfv(VARG(Target), VARG(Pname), params);

END_METHOD

BEGIN_METHOD(GLTEXENVI, GB_INTEGER Target; GB_INTEGER Pname; GB_INTEGER Param)

	glTexEnvi(VARG(Target), VARG(Pname), VARG(Param));

END_METHOD

BEGIN_METHOD(GLTEXENVIV, GB_INTEGER Target; GB_INTEGER Pname; GB_OBJECT Params)

	GLint params[4];
	GB_ARRAY iArray = (GB_ARRAY) VARG(Params);
	int count = GB.Array.Count(iArray);
	int i;
	
	if (count>4)
		count==4;

	for (i=0;i<count; i++)
		params[i]=*((GLint *)GB.Array.Get(iArray,i));

	glTexEnviv(VARG(Target), VARG(Pname), params);

END_METHOD

BEGIN_METHOD(GLTEXIMAGE1D, GB_OBJECT Image; GB_INTEGER Level; GB_INTEGER Border)

	GB_IMAGE *image = VARG(Image);
	GB_IMAGE_INFO info;
	GLint format = 0;

	GB.Image.Info(image, &info);

	if ((info.format == GB_IMAGE_RGBA) || (info.format == GB_IMAGE_RGBX))
		format = GL_RGBA;

	if ((info.format == GB_IMAGE_BGRA) || (info.format == GB_IMAGE_BGRX))
		format = GL_BGRA;

	glTexImage1D(GL_TEXTURE_1D, VARGOPT(Level, 0), format, info.width,  VARGOPT(Border, 0),
		format, GL_UNSIGNED_BYTE, info.data);

END_METHOD

BEGIN_METHOD(GLTEXIMAGE2D, GB_OBJECT Image; GB_INTEGER Level; GB_INTEGER Border)

	GB_IMAGE *image = VARG(Image);
	GB_IMAGE_INFO info;
	GLint format = 0;

	GB.Image.Info(image, &info);

	if ((info.format == GB_IMAGE_RGBA) || (info.format == GB_IMAGE_RGBX))
		format = GL_RGBA;

	if ((info.format == GB_IMAGE_BGRA) || (info.format == GB_IMAGE_BGRX))
		format = GL_BGRA;

	glTexImage2D(GL_TEXTURE_2D, VARGOPT(Level, 0), format, info.width, info.height,
		 VARGOPT(Border, 0), format, GL_UNSIGNED_BYTE, info.data);

END_METHOD

BEGIN_METHOD(GLTEXPARAMETERF, GB_INTEGER Target; GB_INTEGER Pname; GB_FLOAT Param)

	glTexParameterf(VARG(Target), VARG(Pname), VARG(Param));

END_METHOD

BEGIN_METHOD(GLTEXPARAMETERFV, GB_INTEGER Target; GB_INTEGER Pname; GB_OBJECT Params)

	GLfloat params[4];
	GB_ARRAY fArray = (GB_ARRAY) VARG(Params);
	int count = GB.Array.Count(fArray);
	int i;
	
	if (count>4)
		count==4;

	for (i=0;i<count; i++)
		params[i]=*((GLfloat *)GB.Array.Get(fArray,i));

	glTexParameterfv(VARG(Target), VARG(Pname), params);

END_METHOD

BEGIN_METHOD(GLTEXPARAMETERI, GB_INTEGER Target; GB_INTEGER Pname; GB_INTEGER Param)

	glTexParameteri(VARG(Target), VARG(Pname), VARG(Param));

END_METHOD

BEGIN_METHOD(GLTEXPARAMETERIV, GB_INTEGER Target; GB_INTEGER Pname; GB_OBJECT Params)

	GLint params[4];
	GB_ARRAY iArray = (GB_ARRAY) VARG(Params);
	int count = GB.Array.Count(iArray);
	int i;
	
	if (count>4)
		count==4;

	for (i=0;i<count; i++)
		params[i]=*((GLint *)GB.Array.Get(iArray,i));

	glTexParameteriv(VARG(Target), VARG(Pname), params);

END_METHOD
