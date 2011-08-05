/***************************************************************************

	cgluquadric.c

	(c) 2000-2011 Benoît Minisini <gambas@users.sourceforge.net>
	(and Tomek Kolodziejczyk)

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

#define __CGLUQUADRATIC_C

#include "gb_common.h"
#include "cgluquadric.h"

CGLUQUADRIC *CGLUQUADRIC_create(GLUquadric *quadric)
{
	CGLUQUADRIC *ob = GB.New(GB.FindClass("GluQuadric"), NULL, NULL);
	ob->quadric = quadric;
	return ob;
};

/* This is source of glu.h procedure of gluNewQuadric()

GLUquadric * GLAPIENTRY
gluNewQuadric(void)
{
    GLUquadric *newstate;

    newstate = (GLUquadric *) malloc(sizeof(CGLUQUADRIC));
    if (newstate == NULL) {
	 Can't report an error at this point... 
	return NULL;
    }
    newstate->normals = GLU_SMOOTH;
    newstate->textureCoords = GL_FALSE;
    newstate->orientation = GLU_OUTSIDE;
    newstate->drawStyle = GLU_FILL;
    newstate->errorCallback = NULL;
    return newstate;
}*/


GB_DESC GluQuadricDesc[] =
{
	GB_DECLARE("GluQuadric", sizeof(CGLUQUADRIC)),
	GB_NOT_CREATABLE(),
	GB_END_DECLARE
};

/* If this is ok, then uncomment it please.

BEGIN_METHOD(CGLUQUADRIC_free, GB_OBJECT quadric)

	if (!quadric) 
		return;

	gluDeleteNurbsRenderer (quadric);


END_METHOD*/
