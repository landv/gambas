/***************************************************************************

  GLUproject.c

  (c) 2005-2012 Laurent Carlier <lordheavy@users.sourceforge.net>

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

#define __GLUPROJECT_C

#include "GLU.h"

BEGIN_METHOD(GLUPROJECT, GB_FLOAT ObjX; GB_FLOAT ObjY; GB_FLOAT ObjZ; GB_OBJECT Model; GB_OBJECT Proj; GB_OBJECT View)

	GLdouble model[16], proj[16], win[3];
	GLint view[4];
	int i, status;
	GB_ARRAY fArray;

	if ((GB.Array.Count(VARG(Model)) != 16) || (GB.Array.Count(VARG(Proj)) != 16) || (GB.Array.Count(VARG(View)) != 4))
	{
		GB.ReturnNull();
		return;
	}

	for (i=0; i<16; i++)
            model[i] = *((GLdouble *)GB.Array.Get(VARG(Model),i));
	for (i=0; i<16; i++)
            proj[i] = *((GLdouble *)GB.Array.Get(VARG(Proj),i));
	for (i=0; i<4; i++)
            view[i] = *((GLint *)GB.Array.Get(VARG(View),i));

	status = gluProject(VARG(ObjX), VARG(ObjY), VARG(ObjZ), model, proj, view, &win[0], &win[1], &win[2]);
	
	if (status == GLU_FALSE)
	{
		GB.ReturnNull();
		return;
	}

	GB.Array.New(&fArray , GB_T_FLOAT , 3);

	for (i=0; i<3; i++)
		*((GLdouble *)GB.Array.Get(fArray, i)) = win[i];
	
	GB.ReturnObject(fArray);

END_METHOD

BEGIN_METHOD(GLUUNPROJECT, GB_FLOAT WinX; GB_FLOAT WinY; GB_FLOAT WinZ; GB_OBJECT Model; GB_OBJECT Proj; GB_OBJECT View)


	GLdouble model[16], proj[16], obj[3];
	GLint view[4];
	int i, status;
	GB_ARRAY fArray;

	if ((GB.Array.Count(VARG(Model)) != 16) || (GB.Array.Count(VARG(Proj)) != 16) || (GB.Array.Count(VARG(View)) != 4))
	{
		GB.ReturnNull();
		return;
	}

	for (i=0; i<16; i++)
            model[i] = *((GLdouble *)GB.Array.Get(VARG(Model),i));
	for (i=0; i<16; i++)
            proj[i] = *((GLdouble *)GB.Array.Get(VARG(Proj),i));
	for (i=0; i<4; i++)
            view[i] = *((GLint *)GB.Array.Get(VARG(View),i));

	status = gluUnProject(VARG(WinX), VARG(WinY), VARG(WinZ), model, proj, view, &obj[0], &obj[1], &obj[2]);
	
	if (status == GLU_FALSE)
	{
		GB.ReturnNull();
		return;
	}

	GB.Array.New(&fArray , GB_T_FLOAT , 3);

	for (i=0; i<3; i++)
		*((GLdouble *)GB.Array.Get(fArray, i)) = obj[i];
	
	GB.ReturnObject(fArray);

END_METHOD

BEGIN_METHOD(GLUUNPROJECT4, GB_FLOAT WinX; GB_FLOAT WinY; GB_FLOAT WinZ; GB_FLOAT ClipW; GB_OBJECT Model; GB_OBJECT Proj; GB_OBJECT View; GB_FLOAT NearVal; GB_FLOAT FarVal)

	GLdouble model[16], proj[16], obj[4];
	GLint view[4];
	int i, status;
	GB_ARRAY fArray;

	if ((GB.Array.Count(VARG(Model)) != 16) || (GB.Array.Count(VARG(Proj)) != 16) || (GB.Array.Count(VARG(View)) != 4))
	{
		GB.ReturnNull();
		return;
	}

	for (i=0; i<16; i++)
            model[i] = *((GLdouble *)GB.Array.Get(VARG(Model),i));
	for (i=0; i<16; i++)
            proj[i] = *((GLdouble *)GB.Array.Get(VARG(Proj),i));
	for (i=0; i<4; i++)
            view[i] = *((GLint *)GB.Array.Get(VARG(View),i));

	status = gluUnProject4(VARG(WinX), VARG(WinY), VARG(WinZ), VARG(ClipW), model, proj, view, VARG(NearVal), VARG(FarVal), &obj[0], &obj[1], &obj[2], &obj[3]);
	
	if (status == GLU_FALSE)
	{
		GB.ReturnNull();
		return;
	}

	GB.Array.New(&fArray , GB_T_FLOAT , 4);

	for (i=0; i<4; i++)
		*((GLdouble *)GB.Array.Get(fArray, i)) = obj[i];
	
	GB.ReturnObject(fArray);

END_METHOD
