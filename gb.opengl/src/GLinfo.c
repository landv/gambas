/***************************************************************************

  GLinfo.c

  openGL component

  (c) 2005 Laurent Carlier <lordheavy@users.sourceforge.net>
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

#define __GLINFO_C

#include "gambas.h"
#include "gb_common.h"
#include "main.h"

#include <GL/gl.h>

/**************************************************************************/

BEGIN_METHOD_VOID(GLGETACCUMALPHABITS)

	GLint bits[1];

	glGetIntegerv(GL_ACCUM_ALPHA_BITS, bits);
	GB.ReturnInteger(bits[0]);

END_METHOD

BEGIN_METHOD_VOID(GLGETACCUMCLEARVALUE)

	GLfloat params[4];
	GB_ARRAY fArray;
	int i;

	GB.Array.New(&fArray , GB_T_FLOAT , 4);
	glGetFloatv(GL_ACCUM_CLEAR_VALUE, params);
	
	for (i=0;i<4; i++)
		*((double *)GB.Array.Get(fArray, i)) = params[i];
	
	GB.ReturnObject(fArray);

END_METHOD

BEGIN_METHOD_VOID(GLGETACCUMBLUEBITS)

	GLint bits[1];

	glGetIntegerv(GL_ACCUM_BLUE_BITS, bits);
	GB.ReturnInteger(bits[0]);

END_METHOD

BEGIN_METHOD_VOID(GLGETACCUMGREENBITS)

	GLint bits[1];

	glGetIntegerv(GL_ACCUM_GREEN_BITS, bits);
	GB.ReturnInteger(bits[0]);

END_METHOD

BEGIN_METHOD_VOID(GLGETACCUMREDBITS)

	GLint bits[1];

	glGetIntegerv(GL_ACCUM_RED_BITS, bits);
	GB.ReturnInteger(bits[0]);

END_METHOD

BEGIN_METHOD_VOID(GLGETALPHABITS)

	GLint bits[1];

	glGetIntegerv(GL_ALPHA_BITS, bits);
	GB.ReturnInteger(bits[0]);

END_METHOD

BEGIN_METHOD_VOID(GLGETALPHATESTFUNC)

	GLint func[1];

	glGetIntegerv(GL_ALPHA_TEST_FUNC, func);
	GB.ReturnInteger(func[0]);

END_METHOD

BEGIN_METHOD_VOID(GLGETALPHATESTREF)

	GLdouble ref[1];

 	glGetDoublev(GL_ALPHA_TEST_REF, ref);
	GB.ReturnFloat(ref[0]);

END_METHOD

BEGIN_METHOD_VOID(GLGETBLENDDST)

	GLint blend[1];

	glGetIntegerv(GL_BLEND_DST, blend);
	GB.ReturnInteger(blend[0]);

END_METHOD

BEGIN_METHOD_VOID(GLGETBLENDSRC)

	GLint blend[1];

	glGetIntegerv(GL_BLEND_SRC, blend);
	GB.ReturnInteger(blend[0]);

END_METHOD

BEGIN_METHOD_VOID(GLGETCOLORMATERIALFACE)

	GLint face[1];

	glGetIntegerv(GL_COLOR_MATERIAL_FACE, face);
	GB.ReturnInteger(face[0]);

END_METHOD

BEGIN_METHOD_VOID(GLGETCOLORMATERIALPARAMETER)

	GLint param[1];

	glGetIntegerv(GL_COLOR_MATERIAL_PARAMETER, param);
	GB.ReturnInteger(param[0]);

END_METHOD

BEGIN_METHOD_VOID(GLGETCULLFACEMODE)

	GLint mode[1];

	glGetIntegerv(GL_CULL_FACE_MODE, mode);
	GB.ReturnInteger(mode[0]);

END_METHOD

BEGIN_METHOD_VOID(GLGETDEPTHBITS)

	GLint bits[1];

	glGetIntegerv(GL_DEPTH_BITS, bits);
	GB.ReturnInteger(bits[0]);

END_METHOD

BEGIN_METHOD_VOID(GLGETDEPTHCLEARVALUE)

	GLdouble value[1];

 	glGetDoublev(GL_DEPTH_CLEAR_VALUE, value);
	GB.ReturnFloat(value[0]);

END_METHOD

BEGIN_METHOD_VOID(GLGETDEPTHFUNC)

	GLint func[1];

	glGetIntegerv(GL_DEPTH_FUNC, func);
	GB.ReturnInteger(func[0]);

END_METHOD

BEGIN_METHOD_VOID(GLGETDEPTHRANGE)

	GLfloat params[2];
	GB_ARRAY fArray;
	int i;

	GB.Array.New(&fArray , GB_T_FLOAT , 2);
	glGetFloatv(GL_DEPTH_RANGE, params);
	
	for (i=0;i<2; i++)
		*((double *)GB.Array.Get(fArray, i)) = params[i];
	
	GB.ReturnObject(fArray);

END_METHOD

BEGIN_METHOD_VOID(GLGETDEPTHWRITEMASK)

	GLboolean mask[1];

	glGetBooleanv(GL_DEPTH_WRITEMASK, mask);
	GB.ReturnBoolean(mask[0]);

END_METHOD

BEGIN_METHOD_VOID(GLGETEDGEFLAG)

	GLboolean flag[1];

 	glGetBooleanv(GL_EDGE_FLAG, flag);
	GB.ReturnBoolean(flag[0]);

END_METHOD

BEGIN_METHOD_VOID(GLGETFRONTFACE)

	GLint face[1];

	glGetIntegerv(GL_FRONT_FACE, face);
	GB.ReturnInteger(face[0]);

END_METHOD

BEGIN_METHOD_VOID(GLGETFOGCOLOR)

	GLfloat params[4];
	GB_ARRAY fArray;
	int i;

	GB.Array.New(&fArray , GB_T_FLOAT , 4);
	glGetFloatv(GL_FOG_COLOR, params);
	
	for (i=0;i<4; i++)
		*((double *)GB.Array.Get(fArray, i)) = params[i];
	
	GB.ReturnObject(fArray);

END_METHOD

BEGIN_METHOD_VOID(GLGETFOGDENSITY)

	GLdouble fog[1];

	glGetDoublev(GL_FOG_DENSITY, fog);
	GB.ReturnFloat(fog[0]);

END_METHOD

BEGIN_METHOD_VOID(GLGETFOGEND)

	GLdouble fog[1];

	glGetDoublev(GL_FOG_DENSITY, fog);
	GB.ReturnFloat(fog[0]);

END_METHOD

BEGIN_METHOD_VOID(GLGETFOGHINT)

	GLint hint[1];

	glGetIntegerv(GL_FOG_HINT, hint);
	GB.ReturnInteger(hint[0]);

END_METHOD

BEGIN_METHOD_VOID(GLGETFOGINDEX)

	GLdouble fog[1];

	glGetDoublev(GL_FOG_INDEX, fog);
	GB.ReturnFloat(fog[0]);

END_METHOD

BEGIN_METHOD_VOID(GLGETFOGMODE)

	GLint fog[1];

	glGetIntegerv(GL_FOG_MODE, fog);
	GB.ReturnInteger(fog[0]);

END_METHOD

BEGIN_METHOD_VOID(GLGETFOGSTART)

	GLdouble fog[1];

	glGetDoublev(GL_FOG_START, fog);
	GB.ReturnFloat(fog[0]);

END_METHOD

BEGIN_METHOD_VOID(GLGETLIGHTMODELAMBIENT)

	GLfloat params[4];
	GB_ARRAY fArray;
	int i;

	GB.Array.New(&fArray , GB_T_FLOAT , 4);
	glGetFloatv(GL_LIGHT_MODEL_AMBIENT, params);
	
	for (i=0;i<4; i++)
		*((double *)GB.Array.Get(fArray, i)) = params[i];
	
	GB.ReturnObject(fArray);

END_METHOD

BEGIN_METHOD_VOID(GLGETLIGHTMODELLOCALVIEWER)

	GLboolean model[1];

	glGetBooleanv(GL_LIGHT_MODEL_LOCAL_VIEWER, model);
	GB.ReturnBoolean(model[0]);

END_METHOD

BEGIN_METHOD_VOID(GLGETLIGHTMODELTWOSIDE)

	GLdouble model[1];

	glGetDoublev(GL_LIGHT_MODEL_TWO_SIDE, model);
	GB.ReturnFloat(model[0]);

END_METHOD

BEGIN_METHOD_VOID(GLGETLINESMOOTHHINT)

	GLint hint[1];

	glGetIntegerv(GL_LINE_SMOOTH_HINT, hint);
	GB.ReturnInteger(hint[0]);

END_METHOD

BEGIN_METHOD_VOID(GLGETLINESTIPPLEPATTERN)

	GLint pattern[1];

	glGetIntegerv(GL_LINE_STIPPLE_PATTERN, pattern);
	GB.ReturnInteger(pattern[0]);

END_METHOD

BEGIN_METHOD_VOID(GLGETLINESTIPPLEREPEAT)

	GLint repeat[1];

	glGetIntegerv(GL_LINE_STIPPLE_REPEAT, repeat);
	GB.ReturnInteger(repeat[0]);

END_METHOD

BEGIN_METHOD_VOID(GLGETLINEWIDTH)

	GLdouble width[1];

	glGetDoublev(GL_LINE_WIDTH, width);
	GB.ReturnFloat(width[0]);

END_METHOD

BEGIN_METHOD_VOID(GLGETLINEWIDTHGRANULARITY)

	GLdouble width[1];

	glGetDoublev(GL_LINE_WIDTH_GRANULARITY, width);
	GB.ReturnFloat(width[0]);

END_METHOD

BEGIN_METHOD_VOID(GLGETLINEWIDTHRANGE)

	GLfloat params[2];
	GB_ARRAY fArray;
	int i;

	GB.Array.New(&fArray , GB_T_FLOAT , 2);
	glGetFloatv(GL_LINE_WIDTH_RANGE, params);
	
	for (i=0;i<2; i++)
		*((double *)GB.Array.Get(fArray, i)) = params[i];
	
	GB.ReturnObject(fArray);

END_METHOD

BEGIN_METHOD_VOID(GLGETLISTBASE)

	GLint list[1];

	glGetIntegerv(GL_LIST_BASE, list);
	GB.ReturnInteger(list[0]);

END_METHOD

BEGIN_METHOD_VOID(GLGETLISTINDEX)

	GLint list[1];

	glGetIntegerv(GL_LIST_INDEX, list);
	GB.ReturnInteger(list[0]);

END_METHOD

BEGIN_METHOD_VOID(GLGETLISTMODE)

	GLint list[1];

	glGetIntegerv(GL_LIST_MODE, list);
	GB.ReturnInteger(list[0]);

END_METHOD

BEGIN_METHOD_VOID(GLGETMATRIXMODE)

	GLint matrix[1];

	glGetIntegerv(GL_MATRIX_MODE, matrix);
	GB.ReturnInteger(matrix[0]);

END_METHOD

BEGIN_METHOD_VOID(GLGETPERSPECTIVECORRECTIONHINT)

	GLint hint[1];

	glGetIntegerv(GL_PERSPECTIVE_CORRECTION_HINT, hint);
	GB.ReturnInteger(hint[0]);

END_METHOD

BEGIN_METHOD_VOID(GLGETPOINTSMOOTHHINT)

	GLint hint[1];

	glGetIntegerv(GL_POINT_SMOOTH_HINT, hint);
	GB.ReturnInteger(hint[0]);

END_METHOD

BEGIN_METHOD_VOID(GLGETPOINTSIZE)

	GLdouble size[1];

	glGetDoublev(GL_POINT_SIZE, size);
	GB.ReturnFloat(size[0]);

END_METHOD

BEGIN_METHOD_VOID(GLGETPOINTSIZEGRANULARITY)

	GLdouble size[1];

	glGetDoublev(GL_POINT_SIZE_GRANULARITY, size);
	GB.ReturnFloat(size[0]);

END_METHOD

BEGIN_METHOD_VOID(GLGETPOINTSIZERANGE)

	GLfloat params[2];
	GB_ARRAY fArray;
	int i;

	GB.Array.New(&fArray , GB_T_FLOAT , 2);
	glGetFloatv(GL_POINT_SIZE_RANGE, params);
	
	for (i=0;i<2; i++)
		*((double *)GB.Array.Get(fArray, i)) = params[i];
	
	GB.ReturnObject(fArray);

END_METHOD

BEGIN_METHOD_VOID(GLGETPOLYGONMODE)

	GLint params[2];
	GB_ARRAY iArray;
	int i;

	GB.Array.New(&iArray , GB_T_INTEGER , 2);
	glGetIntegerv(GL_POLYGON_MODE, params);
	
	for (i=0;i<2; i++)
		*((int *)GB.Array.Get(iArray, i)) = params[i];
	
	GB.ReturnObject(iArray);

END_METHOD

BEGIN_METHOD_VOID(GLGETPOLYGONSMOOTHHINT)

	GLint hint[1];

	glGetIntegerv(GL_POLYGON_SMOOTH_HINT, hint);
	GB.ReturnInteger(hint[0]);

END_METHOD

BEGIN_METHOD_VOID(GLGETSCISSORBOX)

	GLint params[4];
	GB_ARRAY iArray;
	int i;

	GB.Array.New(&iArray , GB_T_INTEGER , 4);
	glGetIntegerv(GL_SCISSOR_BOX, params);
	
	for (i=0;i<4; i++)
		*((int *)GB.Array.Get(iArray, i)) = params[i];
	
	GB.ReturnObject(iArray);

END_METHOD

BEGIN_METHOD_VOID(GLGETSHADEMODEL)

	GLint model[1];

	glGetIntegerv(GL_SHADE_MODEL, model);
	GB.ReturnInteger(model[0]);

END_METHOD

BEGIN_METHOD_VOID(GLMAXLISTNESTING)

	GLint max[1];

	glGetIntegerv(GL_MAX_LIST_NESTING, max);
	GB.ReturnInteger(max[0]);

END_METHOD

BEGIN_METHOD_VOID(GLMAXATTRIBSTACKDEPTH)

	GLint max[1];

	glGetIntegerv(GL_MAX_ATTRIB_STACK_DEPTH, max);
	GB.ReturnInteger(max[0]);

END_METHOD

BEGIN_METHOD_VOID(GLMAXMODELVIEWSTACKDEPTH)

	GLint max[1];

	glGetIntegerv(GL_MAX_MODELVIEW_STACK_DEPTH, max);
	GB.ReturnInteger(max[0]);

END_METHOD

BEGIN_METHOD_VOID(GLMAXNAMESTACKDEPTH)

	GLint max[1];

	glGetIntegerv(GL_MAX_NAME_STACK_DEPTH, max);
	GB.ReturnInteger(max[0]);

END_METHOD

BEGIN_METHOD_VOID(GLMAXPROJECTIONSTACKDEPTH)

	GLint max[1];

	glGetIntegerv(GL_MAX_PROJECTION_STACK_DEPTH, max);
	GB.ReturnInteger(max[0]);

END_METHOD

BEGIN_METHOD_VOID(GLMAXTEXTURESTACKDEPTH)

	GLint max[1];

	glGetIntegerv(GL_MAX_TEXTURE_STACK_DEPTH, max);
	GB.ReturnInteger(max[0]);

END_METHOD

BEGIN_METHOD_VOID(GLMAXEVALORDER)

	GLint max[1];

	glGetIntegerv(GL_MAX_EVAL_ORDER, max);
	GB.ReturnInteger(max[0]);

END_METHOD

BEGIN_METHOD_VOID(GLMAXLIGHTS)

	GLint max[1];

	glGetIntegerv(GL_MAX_LIGHTS, max);
	GB.ReturnInteger(max[0]);

END_METHOD

BEGIN_METHOD_VOID(GLMAXCLIPPLANES)

	GLint max[1];

	glGetIntegerv(GL_MAX_CLIP_PLANES, max);
	GB.ReturnInteger(max[0]);

END_METHOD

BEGIN_METHOD_VOID(GLMAXTEXTURESIZE)

	GLint max[1];

	glGetIntegerv(GL_MAX_TEXTURE_SIZE, max);
	GB.ReturnInteger(max[0]);

END_METHOD

BEGIN_METHOD_VOID(GLMAXVIEWPORTDIMS)

	GLint params[2];
	GB_ARRAY iArray;
	int i;

	GB.Array.New(&iArray , GB_T_INTEGER , 2);
	glGetIntegerv(GL_MAX_VIEWPORT_DIMS, params);
	
	for (i=0;i<2; i++)
		*((int *)GB.Array.Get(iArray, i)) = params[i];
	
	GB.ReturnObject(iArray);

END_METHOD

BEGIN_METHOD_VOID(GLMAXCLIENTATTRIBSTACKDEPTH)

	GLint max[1];

	glGetIntegerv(GL_MAX_CLIENT_ATTRIB_STACK_DEPTH, max);
	GB.ReturnInteger(max[0]);

END_METHOD

