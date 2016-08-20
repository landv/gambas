/***************************************************************************

  GLcolorLighting.c

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

#define __GLCOLORLIGHTING_C

#include "GL.h"

BEGIN_METHOD(GLCOLOR3F, GB_FLOAT red; GB_FLOAT green; GB_FLOAT blue)

      glColor3d(VARG(red), VARG(green), VARG(blue));

END_METHOD

BEGIN_METHOD(GLCOLORF, GB_FLOAT red; GB_FLOAT green; GB_FLOAT blue; GB_FLOAT alpha)

      if (MISSING(alpha))
            glColor3d(VARG(red), VARG(green), VARG(blue));
      else
            glColor4d(VARG(red), VARG(green), VARG(blue), VARG(alpha));

END_METHOD

BEGIN_METHOD(GLCOLOR3I, GB_INTEGER red; GB_INTEGER green; GB_INTEGER blue)

      glColor3i(VARG(red), VARG(green), VARG(blue));

END_METHOD

BEGIN_METHOD(GLCOLORI, GB_INTEGER red; GB_INTEGER green; GB_INTEGER blue; GB_INTEGER alpha)

      if (MISSING(alpha))
            glColor3i(VARG(red), VARG(green), VARG(blue));
      else
            glColor4i(VARG(red), VARG(green), VARG(blue), VARG(alpha));

END_METHOD

BEGIN_METHOD(GLCOLORFV, GB_OBJECT array)

	GLdouble r,g,b,a;
	GB_ARRAY color = (GB_ARRAY) VARG(array);
	int count = GB.Array.Count(color);
	
	if (count<3)
		return;

	r = *((GLdouble *)GB.Array.Get(color,0));
	g = *((GLdouble *)GB.Array.Get(color,1));
	b = *((GLdouble *)GB.Array.Get(color,2));
	
	if (count==3)
		glColor3d(r, g, b);
	else
	{
		a = *((GLdouble *)GB.Array.Get(color,3));
		glColor4d(r, g, b, a);
	}
	  
END_METHOD

BEGIN_METHOD(GLCOLORIV, GB_OBJECT array)

	GLint r,g,b,a;
	GB_ARRAY color = (GB_ARRAY) VARG(array);
	int count = GB.Array.Count(color);
	
	if (count<3)
		return;

	r = *((GLint *)GB.Array.Get(color,0));
	g = *((GLint *)GB.Array.Get(color,1));
	b = *((GLint *)GB.Array.Get(color,2));
	
	if (count==3)
		glColor3i(r, g, b);
	else
	{
		a = *((GLint *)GB.Array.Get(color,3));
		glColor4i(r, g, b, a);
	}
	  
END_METHOD

BEGIN_METHOD(GLCOLORMATERIAL, GB_INTEGER face; GB_INTEGER mode)

	glColorMaterial(VARG(face), VARG(mode));

END_METHOD

BEGIN_METHOD(GLFRONTFACE, GB_INTEGER mode)

	glFrontFace(VARG(mode));

END_METHOD

BEGIN_METHOD(GLGETLIGHTFV, GB_INTEGER light; GB_INTEGER pname)

	GLfloat params[4];
	GB_ARRAY fArray;
	int i, count=1;

	switch(VARG(pname))
	{
		case GL_AMBIENT :
		case GL_DIFFUSE :
		case GL_SPECULAR :
		case GL_POSITION :
			count = 4;
			break;
		case GL_SPOT_DIRECTION :
			count = 3;
			break;
	}
	
	GB.Array.New(&fArray , GB_T_FLOAT , count);
	glGetLightfv(VARG(light), VARG(pname), params);
	
	for (i=0; i<count; i++)
		*((double *)GB.Array.Get(fArray, i)) = params[i];
	
	GB.ReturnObject(fArray);

END_METHOD

BEGIN_METHOD(GLGETLIGHTIV, GB_INTEGER light; GB_INTEGER pname)

	GLint params[4];
	GB_ARRAY iArray;
	int i, count=1;

	switch(VARG(pname))
	{
		case GL_AMBIENT :
		case GL_DIFFUSE :
		case GL_SPECULAR :
		case GL_POSITION :
			count = 4;
			break;
		case GL_SPOT_DIRECTION :
			count = 3;
			break;
	}
	
	GB.Array.New(&iArray , GB_T_INTEGER , count);
	glGetLightiv(VARG(light), VARG(pname), params);
	
	for (i=0; i<count; i++)
		*((GLint *)GB.Array.Get(iArray, i)) = params[i];
	
	GB.ReturnObject(iArray);

END_METHOD

BEGIN_METHOD(GLGETMATERIALFV, GB_INTEGER face; GB_INTEGER pname)

	GLfloat params[4];
	GB_ARRAY fArray;
	int i, count=1;

	switch(VARG(pname))
	{
		case GL_AMBIENT :
		case GL_DIFFUSE :
		case GL_SPECULAR :
		case GL_EMISSION :
			count = 4;
			break;
		case GL_COLOR_INDEXES :
			count = 3;
			break;
	}
	
	GB.Array.New(&fArray , GB_T_FLOAT , count);
	glGetMaterialfv(VARG(face), VARG(pname), params);
	
	for (i=0; i<count; i++)
		*((double *)GB.Array.Get(fArray, i)) = params[i];
	
	GB.ReturnObject(fArray);

END_METHOD

BEGIN_METHOD(GLGETMATERIALIV, GB_INTEGER face; GB_INTEGER pname)

	GLint params[4];
	GB_ARRAY iArray;
	int i, count=1;

	switch(VARG(pname))
	{
		case GL_AMBIENT :
		case GL_DIFFUSE :
		case GL_SPECULAR :
		case GL_EMISSION :
			count = 4;
			break;
		case GL_COLOR_INDEXES :
			count = 3;
			break;
	}
	
	GB.Array.New(&iArray , GB_T_INTEGER , count);
	glGetMaterialiv(VARG(face), VARG(pname), params);
	
	for (i=0; i<count; i++)
		*((GLint *)GB.Array.Get(iArray, i)) = params[i];
	
	GB.ReturnObject(iArray);

END_METHOD

BEGIN_METHOD(GLINDEXF, GB_FLOAT index)

	glIndexd(VARG(index));

END_METHOD

BEGIN_METHOD(GLINDEXI, GB_INTEGER index)

	glIndexi(VARG(index));

END_METHOD

BEGIN_METHOD(GLLIGHTF, GB_INTEGER light; GB_INTEGER pname; GB_FLOAT param)

	glLightf(VARG(light), VARG(pname), VARG(param));

END_METHOD

BEGIN_METHOD(GLLIGHTI, GB_INTEGER light; GB_INTEGER pname; GB_INTEGER param)

	glLighti(VARG(light), VARG(pname), VARG(param));

END_METHOD

BEGIN_METHOD(GLLIGHTFV, GB_INTEGER light; GB_INTEGER pname; GB_OBJECT params)

	GLfloat params[4];
	GB_ARRAY fArray = (GB_ARRAY) VARG(params);
	uint i, count = GB.Array.Count(fArray);
	
      count = (count > 4 ? 4 : count);

	for (i=0; i<count; i++)
		params[i]=*((double *)GB.Array.Get(fArray,i));

	glLightfv(VARG(light), VARG(pname), params);

END_METHOD

BEGIN_METHOD(GLLIGHTIV, GB_INTEGER light; GB_INTEGER pname; GB_OBJECT params)

	GLint params[4];
	GB_ARRAY iArray = (GB_ARRAY) VARG(params);
	uint i, count = GB.Array.Count(iArray);
	
      count = (count > 4 ? 4 : count);

	for (i=0; i<count; i++)
		params[i]=*((GLint *)GB.Array.Get(iArray,i));

	glLightiv(VARG(light), VARG(pname), params);

END_METHOD

BEGIN_METHOD(GLLIGHTMODELF, GB_INTEGER pname; GB_FLOAT param)

	glLightModelf(VARG(pname), VARG(param));

END_METHOD

BEGIN_METHOD(GLLIGHTMODELI, GB_INTEGER pname; GB_INTEGER param)

	glLightModeli(VARG(pname), VARG(param));

END_METHOD

BEGIN_METHOD(GLLIGHTMODELFV, GB_INTEGER pname; GB_OBJECT params)

	GLfloat params[4];
	GB_ARRAY fArray = (GB_ARRAY) VARG(params);
	uint i, count = GB.Array.Count(fArray);
	
      count = (count > 4 ? 4 : count);

	for (i=0; i<count; i++)
		params[i]=*((GLfloat *)GB.Array.Get(fArray,i));

	glLightModelfv(VARG(pname), params);

END_METHOD

BEGIN_METHOD(GLLIGHTMODELIV, GB_INTEGER pname; GB_OBJECT params)

	GLint params[4];
	GB_ARRAY iArray = (GB_ARRAY) VARG(params);
	uint i, count = GB.Array.Count(iArray);
	
      count = (count > 4 ? 4 : count);

	for (i=0; i<count; i++)
		params[i]=*((GLint *)GB.Array.Get(iArray,i));

	glLightModeliv(VARG(pname), params);

END_METHOD

BEGIN_METHOD(GLMATERIALF, GB_INTEGER face; GB_INTEGER pname; GB_FLOAT param)

	glMaterialf(VARG(face), VARG(pname), VARG(param));

END_METHOD

BEGIN_METHOD(GLMATERIALI, GB_INTEGER face; GB_INTEGER pname; GB_INTEGER param)

	glMateriali(VARG(face), VARG(pname), VARG(param));

END_METHOD

BEGIN_METHOD(GLMATERIALFV, GB_INTEGER face; GB_INTEGER pname; GB_OBJECT params)

	GLfloat params[4];
	GB_ARRAY fArray = (GB_ARRAY) VARG(params);
	uint i, count = GB.Array.Count(fArray);
	
      count = (count > 4 ? 4 : count);

	for (i=0; i<count; i++)
		params[i]=*((double *)GB.Array.Get(fArray,i));

	glMaterialfv(VARG(face), VARG(pname), params);

END_METHOD

BEGIN_METHOD(GLMATERIALIV, GB_INTEGER face; GB_INTEGER pname; GB_OBJECT params)

	GLint params[4];
	GB_ARRAY iArray = (GB_ARRAY) VARG(params);
	uint i, count = GB.Array.Count(iArray);
	
      count = (count > 4 ? 4 : count);

	for (i=0; i<count; i++)
		params[i]=*((GLint *)GB.Array.Get(iArray,i));

	glMaterialiv(VARG(face), VARG(pname), params);

END_METHOD

BEGIN_METHOD(GLNORMAL3F, GB_FLOAT nx; GB_FLOAT ny; GB_FLOAT nz)

	glNormal3d(VARG(nx), VARG(ny), VARG(nz));

END_METHOD

BEGIN_METHOD(GLNORMAL3I, GB_INTEGER nx; GB_INTEGER ny; GB_INTEGER nz)

	glNormal3i(VARG(nx), VARG(ny), VARG(nz));

END_METHOD

BEGIN_METHOD(GLNORMAL3FV, GB_OBJECT array)

	GLdouble x,y,z;
	GB_ARRAY normal = (GB_ARRAY) VARG(array);
	int count = GB.Array.Count(normal);
	
	if (count != 3)
		return;

	x = *((GLdouble *)GB.Array.Get(normal,0));
	y = *((GLdouble *)GB.Array.Get(normal,1));
	z = *((GLdouble *)GB.Array.Get(normal,2));
	
	glNormal3d(x, y, z);

END_METHOD

BEGIN_METHOD(GLNORMAL3IV, GB_OBJECT array)

	GLint x,y,z;
	GB_ARRAY normal = (GB_ARRAY) VARG(array);
	int count = GB.Array.Count(normal);
	
	if (count != 3)
		return;

	x = *((GLint *)GB.Array.Get(normal,0));
	y = *((GLint *)GB.Array.Get(normal,1));
	z = *((GLint *)GB.Array.Get(normal,2));
	
	glNormal3i(x, y, z);

END_METHOD

BEGIN_METHOD(GLSHADEMODEL, GB_INTEGER mode)

	glShadeModel(VARG(mode));

END_METHOD

