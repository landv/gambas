/***************************************************************************

  GLtextureMapping.c

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

#define __GLTEXTUREMAPPING_C

#include "GL.h"

BEGIN_METHOD(GLBINDTEXTURE, GB_INTEGER target; GB_INTEGER texture)

	glBindTexture(VARG(target), VARG(texture));

END_METHOD

BEGIN_METHOD(GLACTIVETEXTURE, GB_INTEGER texture)

	glActiveTexture(VARG(texture));

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

	GLuint *textures;
	int i, count = VARG(count);
	GB_ARRAY iArray;

	if (count <= 0)
	{
		GB.ReturnNull();
		return;
	}
	
	GB.Array.New(&iArray , GB_T_INTEGER , count);

	textures = alloca(count * sizeof(GLuint));
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

BEGIN_METHOD(GLTEXCOORD2F, GB_FLOAT S; GB_FLOAT T)

      glTexCoord2d(VARG(S), VARG(T));

END_METHOD

BEGIN_METHOD(GLTEXCOORD3F, GB_FLOAT S; GB_FLOAT T; GB_FLOAT R)

      glTexCoord3d(VARG(S), VARG(T), VARG(R));

END_METHOD

BEGIN_METHOD(GLTEXCOORDF, GB_FLOAT S; GB_FLOAT T; GB_FLOAT R; GB_FLOAT Q)

      if (MISSING(T))
      {
            glTexCoord1d(VARG(S));
            return;
      }

      if (MISSING(R))
      {
            glTexCoord2d(VARG(S), VARG(T));
            return;
      }

      if (MISSING(Q))
      {
            glTexCoord3d(VARG(S), VARG(T), VARG(R));
            return;
      }

      glTexCoord4d(VARG(S), VARG(T), VARG(R), VARG(Q));

END_METHOD

BEGIN_METHOD(GLTEXCOORD1I, GB_INTEGER S)

      glTexCoord1i(VARG(S));

END_METHOD

BEGIN_METHOD(GLTEXCOORD2I, GB_INTEGER S; GB_INTEGER T)

      glTexCoord2i(VARG(S), VARG(T));

END_METHOD

BEGIN_METHOD(GLTEXCOORD3I, GB_INTEGER S; GB_INTEGER T; GB_INTEGER R)

      glTexCoord3i(VARG(S), VARG(T), VARG(R));

END_METHOD

BEGIN_METHOD(GLTEXCOORDI, GB_INTEGER S; GB_INTEGER T; GB_INTEGER R; GB_INTEGER Q)

      if (MISSING(T))
      {
            glTexCoord1i(VARG(S));
            return;
      }

      if (MISSING(R))
      {
            glTexCoord2i(VARG(S), VARG(T));
            return;
      }

      if (MISSING(Q))
      {
            glTexCoord3i(VARG(S), VARG(T), VARG(R));
            return;
      }

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
	
      count = (count > 4 ? 4 : count);

	for (i=0; i<count; i++)
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
	
      count = (count > 4 ? 4 : count);

	for (i=0; i<count; i++)
		params[i]=*((GLint *)GB.Array.Get(iArray,i));

	glTexEnviv(VARG(Target), VARG(Pname), params);

END_METHOD

BEGIN_METHOD(GLTEXIMAGE1D, GB_OBJECT Image; GB_INTEGER Level; GB_INTEGER Border)

	GB_IMG *image;
	int format;
	
	if (IMAGE_get(ARG(Image), &image, &format))
		return;

	glTexImage1D(GL_TEXTURE_1D, VARGOPT(Level, 0), IMAGE_get_ncolors(format), image->width,  VARGOPT(Border, 0),
		format, GL_UNSIGNED_BYTE, image->data);

END_METHOD

BEGIN_METHOD(GLTEXIMAGE2D, GB_OBJECT Image; GB_INTEGER Level; GB_INTEGER Border)

	GB_IMG *image;
	int format;

	if (IMAGE_get(ARG(Image), &image, &format))
		return;

	glTexImage2D(GL_TEXTURE_2D, VARGOPT(Level, 0), IMAGE_get_ncolors(format), image->width, image->height,
		 VARGOPT(Border, 0), format, GL_UNSIGNED_BYTE, image->data);

END_METHOD

BEGIN_METHOD(GLTEXSUBIMAGE1D, GB_OBJECT Image; GB_INTEGER XOffset; GB_INTEGER Width; GB_INTEGER Level)

	GB_IMG *image;
	int format;

	if (IMAGE_get(ARG(Image), &image, &format))
		return;

	glTexSubImage1D(GL_TEXTURE_1D, VARGOPT(Level, 0), VARG(XOffset), VARG(Width),
		 format, GL_UNSIGNED_BYTE, image->data);

END_METHOD

BEGIN_METHOD(GLTEXSUBIMAGE2D, GB_OBJECT Image; GB_INTEGER XOffset; GB_INTEGER YOffset; GB_INTEGER Width; GB_INTEGER Height; GB_INTEGER Level)

	GB_IMG *image;
	int format;

	if (IMAGE_get(ARG(Image), &image, &format))
		return;

	glTexSubImage2D(GL_TEXTURE_2D, VARGOPT(Level, 0), VARG(XOffset), VARG(YOffset), VARG(Width), VARG(Height),
		 format, GL_UNSIGNED_BYTE, image->data);

END_METHOD

BEGIN_METHOD(GLTEXPARAMETERF, GB_INTEGER Target; GB_INTEGER Pname; GB_FLOAT Param)

	glTexParameterf(VARG(Target), VARG(Pname), VARG(Param));

END_METHOD

BEGIN_METHOD(GLTEXPARAMETERFV, GB_INTEGER Target; GB_INTEGER Pname; GB_OBJECT Params)

	GLfloat params[4];
	GB_ARRAY fArray = (GB_ARRAY) VARG(Params);
	int count = GB.Array.Count(fArray);
	int i;
	
      count = (count > 4 ? 4 : count);

	for (i=0; i<count; i++)
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
	
      count = (count > 4 ? 4 : count);

	for (i=0; i<count; i++)
		params[i]=*((GLint *)GB.Array.Get(iArray,i));

	glTexParameteriv(VARG(Target), VARG(Pname), params);

END_METHOD

BEGIN_METHOD(GLTEXGENI, GB_INTEGER Coord; GB_INTEGER Pname; GB_INTEGER Param)

	glTexGeni(VARG(Coord), VARG(Pname), VARG(Param));

END_METHOD

BEGIN_METHOD(GLMULTITEXCOORD2F, GB_INTEGER Target; GB_FLOAT S; GB_FLOAT T)

	glMultiTexCoord2d (VARG(Target), VARG(S), VARG(T));

END_METHOD
