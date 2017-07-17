/***************************************************************************

  GLUnurb.c

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

#define __GLUNURB_C

#include "cglunurb.h"
#include "GLU.h"

static GLUnurbsObj *get_nurb(void *ob)
{
	if (GB.CheckObject(ob))
		return NULL;
	else
		return ((CGLUNURB *)ob)->nurb;
}

#define GET_NURB() \
	GLUnurbsObj *thenurb = get_nurb(VARG(nurb)); \
	if (!thenurb) return;

BEGIN_METHOD(GLUBEGINCURVE, GB_OBJECT nurb)

	GET_NURB();
	gluBeginCurve(thenurb);

END_METHOD

BEGIN_METHOD(GLUBEGINSURFACE, GB_OBJECT nurb)

	GET_NURB();
	gluBeginSurface(thenurb);

END_METHOD

BEGIN_METHOD(GLUBEGINTRIM, GB_OBJECT nurb)

	GET_NURB();
	gluBeginTrim(thenurb);

END_METHOD

BEGIN_METHOD(GLUDELETENURBSRENDERER, GB_OBJECT nurb)

	GET_NURB();
	gluDeleteNurbsRenderer(thenurb);
	// Make the nurb Gambas object invalid
	((CGLUNURB *)VARG(nurb))->nurb = NULL;

END_METHOD

BEGIN_METHOD(GLUENDCURVE, GB_OBJECT nurb)

	GET_NURB();
	gluEndCurve(thenurb);

END_METHOD

BEGIN_METHOD(GLUENDSURFACE, GB_OBJECT nurb)

	GET_NURB();
	gluEndSurface(thenurb);

END_METHOD

BEGIN_METHOD(GLUENDTRIM, GB_OBJECT nurb)

	GET_NURB();
	gluEndTrim(thenurb);

END_METHOD

BEGIN_METHOD(GLUNURBSCURVE, GB_OBJECT nurb; GB_INTEGER knotCount; GB_OBJECT knots; GB_INTEGER stride; GB_OBJECT control; GB_INTEGER order; GB_INTEGER type)
	
	GET_NURB();
	
	GB_ARRAY knot = (GB_ARRAY) VARG(knots);
	GB_ARRAY controll = (GB_ARRAY) VARG(control);
	int i; 
	int count1 = GB.Array.Count(knot);
	int count2 = GB.Array.Count(controll);
	GLfloat param1[count1], param2[count2];	

	for (i=0; i<count1; i++)
		param1[i] = *((float *)GB.Array.Get(knot,i));
	for (i=0; i<count2; i++)
		param2[i] = *((float *)GB.Array.Get(controll,i));
	
	gluNurbsCurve(thenurb, VARG(knotCount),param1, VARG(stride), param2, VARG(order),VARG(type));

END_METHOD

BEGIN_METHOD(GLUNURBSPROPERTY, GB_OBJECT nurb; GB_INTEGER property; GB_FLOAT value)
	
	GET_NURB();
	gluNurbsProperty(thenurb, VARG(property), VARG(value));

END_METHOD


//gluNurbsSurface (GLUnurbs* nurb, GLint sKnotCount, GLfloat* sKnots, GLint tKnotCount, GLfloat* tKnots, GLint sStride, GLint tStride, GLfloat* control, GLint sOrder, GLint tOrder, GLenum type);

BEGIN_METHOD(GLUNURBSSURFACE, GB_OBJECT nurb; GB_INTEGER sKnotCount; GB_OBJECT sKnots; GB_INTEGER tKnotCount; GB_OBJECT tKnots; GB_INTEGER sStride; GB_INTEGER tStride; GB_INTEGER sOrder; GB_INTEGER tOrder; GB_INTEGER type; GB_OBJECT control)

	GET_NURB();
	
	GB_ARRAY sknot = (GB_ARRAY) VARG(sKnots);
	GB_ARRAY tknot = (GB_ARRAY) VARG(tKnots);
	GB_ARRAY controll = (GB_ARRAY) VARG(control);
	int i; 
	int count1 = GB.Array.Count(sknot);
	int count2 = GB.Array.Count(tknot);
	int count3 = GB.Array.Count(controll);
	GLfloat param1[count1], param2[count2],param3[count3];
	
	for (i=0; i<count1; i++)
		param1[i] = *((float *)GB.Array.Get(sknot,i));
	for (i=0; i<count2; i++)
		param2[i] = *((float *)GB.Array.Get(tknot,i));
	for (i=0; i<count3; i++)
		param3[i] = *((float *)GB.Array.Get(controll,i));
	
	gluNurbsSurface(thenurb, VARG(sKnotCount), param1, VARG(tKnotCount), param2, VARG(sStride),VARG(tStride),param3, VARG(sOrder), VARG(tOrder), VARG(type));

END_METHOD

BEGIN_METHOD_VOID(GLUNEWNURBSRENDERER)

	CGLUNURB *ob = CGLUNURB_create();
	GB.ReturnObject(ob);

END_METHOD


