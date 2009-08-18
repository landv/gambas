/***************************************************************************

  GLinfo.c

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
  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

***************************************************************************/

#define __GLINFO_C

#include "main.h"

#include <GL/gl.h>

// max size of an array
#define MAXSIZE 16

// Arrays returned with glGet calls
static GLboolean *boolArray;
static int boolSize = 0;
static GLdouble *floatArray;
static int floatSize = 0;
static GLint *intArray;
static int intSize = 0;

static void resizeBool(int size)
{
	if (size < boolSize)
		return;

	if (!boolSize)
		GB.Alloc(POINTER(&boolArray), sizeof(GLboolean)*size);
	else
		GB.Realloc(POINTER(&boolArray), sizeof(GLboolean)*size);

	boolSize = size;
}

static void resizeFloat(int size)
{
	if (size < floatSize)
		return;

	if (!floatSize)
		GB.Alloc(POINTER(&floatArray), sizeof(GLdouble)*size);
	else
		GB.Realloc(POINTER(&floatArray), sizeof(GLdouble)*size);

	floatSize = size;
}

static void resizeInt(int size)
{
	if (size < intSize)
		return;

	if (!intSize)
		GB.Alloc(POINTER(&intArray), sizeof(GLint)*size);
	else
		GB.Realloc(POINTER(&intArray), sizeof(GLint)*size);

	intSize = size;
}

static int checkSize(GLenum value)
{
	int retSize = 0;

	switch(value)
	{
		case GL_ACCUM_ALPHA_BITS:
		case GL_ACCUM_BLUE_BITS:
		case GL_ACCUM_GREEN_BITS:
		case GL_ACCUM_RED_BITS:
		case GL_ALPHA_BIAS:
		case GL_ALPHA_TEST:
		case GL_ALPHA_BITS:
		case GL_ALPHA_SCALE:
		case GL_ALPHA_TEST_FUNC:
		case GL_ALPHA_TEST_REF:
		case GL_ATTRIB_STACK_DEPTH:
		case GL_AUTO_NORMAL:
		case GL_AUX_BUFFERS:
		case GL_BLEND:
		case GL_BLEND_DST:
		case GL_BLEND_SRC:
		case GL_BLUE_BIAS:
		case GL_BLUE_BITS:
		case GL_BLUE_SCALE:
		case GL_CLIP_PLANE0:
		case GL_CLIP_PLANE1:
		case GL_CLIP_PLANE2:
		case GL_CLIP_PLANE3:
		case GL_CLIP_PLANE4:
		case GL_CLIP_PLANE5:
		case GL_COLOR_MATERIAL:
		case GL_COLOR_MATERIAL_FACE:
		case GL_COLOR_MATERIAL_PARAMETER:
		case GL_CULL_FACE:
		case GL_CULL_FACE_MODE:
		case GL_CURRENT_INDEX:
		case GL_CURRENT_RASTER_DISTANCE:
		case GL_CURRENT_RASTER_INDEX:
		case GL_CURRENT_RASTER_POSITION_VALID:
		case GL_DEPTH_BIAS:
		case GL_DEPTH_BITS:
		case GL_DEPTH_CLEAR_VALUE:
		case GL_DEPTH_FUNC:
		case GL_DEPTH_SCALE:
		case GL_DEPTH_TEST:
		case GL_DEPTH_WRITEMASK:
		case GL_DITHER:
		case GL_DOUBLEBUFFER:
		case GL_DRAW_BUFFER:
		case GL_EDGE_FLAG:
		case GL_FOG:
		case GL_FOG_DENSITY:
		case GL_FOG_END:
		case GL_FOG_HINT:
		case GL_FOG_INDEX:
		case GL_FOG_MODE:
		case GL_FOG_START:
		case GL_FRONT_FACE:
		case GL_GREEN_BIAS:
		case GL_GREEN_BITS:
		case GL_GREEN_SCALE:
		case GL_INDEX_BITS:
		case GL_INDEX_CLEAR_VALUE:
		case GL_INDEX_MODE:
		case GL_INDEX_OFFSET:
		case GL_INDEX_SHIFT:
		case GL_INDEX_WRITEMASK:
		case GL_LIGHT0:
		case GL_LIGHT1:
		case GL_LIGHT2:
		case GL_LIGHT3:
		case GL_LIGHT4:
		case GL_LIGHT5:
		case GL_LIGHT6:
		case GL_LIGHT7:
		case GL_LIGHTING:
		case GL_LIGHT_MODEL_LOCAL_VIEWER:
		case GL_LIGHT_MODEL_TWO_SIDE:
		case GL_LINE_SMOOTH:
		case GL_LINE_SMOOTH_HINT:
		case GL_LINE_STIPPLE:
		case GL_LINE_STIPPLE_PATTERN:
		case GL_LINE_STIPPLE_REPEAT:
		case GL_LINE_WIDTH:
		case GL_LINE_WIDTH_GRANULARITY:
		case GL_LIST_BASE:
		case GL_LIST_INDEX:
		case GL_LIST_MODE:
		case GL_LOGIC_OP:
		case GL_LOGIC_OP_MODE:
		case GL_MAP1_COLOR_4:
		case GL_MAP1_GRID_SEGMENTS:
		case GL_MAP1_INDEX:
		case GL_MAP1_NORMAL:
		case GL_MAP1_TEXTURE_COORD_1:
		case GL_MAP1_TEXTURE_COORD_2:
		case GL_MAP1_TEXTURE_COORD_3:
		case GL_MAP1_TEXTURE_COORD_4:
		case GL_MAP1_VERTEX_3:
		case GL_MAP1_VERTEX_4:
		case GL_MAP2_COLOR_4:
		case GL_MAP2_INDEX:
		case GL_MAP2_NORMAL:
		case GL_MAP2_TEXTURE_COORD_1:
		case GL_MAP2_TEXTURE_COORD_2:
		case GL_MAP2_TEXTURE_COORD_3:
		case GL_MAP2_TEXTURE_COORD_4:
		case GL_MAP2_VERTEX_3:
		case GL_MAP2_VERTEX_4:
		case GL_MAP_COLOR:
		case GL_MAP_STENCIL:
		case GL_MATRIX_MODE:
		case GL_MAX_ATTRIB_STACK_DEPTH:
		case GL_MAX_CLIP_PLANES:
		case GL_MAX_EVAL_ORDER:
		case GL_MAX_LIGHTS:
		case GL_MAX_LIST_NESTING:
		case GL_MAX_MODELVIEW_STACK_DEPTH:
		case GL_MAX_NAME_STACK_DEPTH:
		case GL_MAX_PIXEL_MAP_TABLE:
		case GL_MAX_PROJECTION_STACK_DEPTH:
		case GL_MAX_TEXTURE_SIZE:
		case GL_MAX_TEXTURE_STACK_DEPTH:
		case GL_MODELVIEW_STACK_DEPTH:
		case GL_NAME_STACK_DEPTH:
		case GL_NORMALIZE:
		case GL_PACK_ALIGNMENT:
		case GL_PACK_LSB_FIRST:
		case GL_PACK_ROW_LENGTH:
		case GL_PACK_SKIP_PIXELS:
		case GL_PACK_SKIP_ROWS:
		case GL_PACK_SWAP_BYTES:
		case GL_PERSPECTIVE_CORRECTION_HINT:
		case GL_PIXEL_MAP_A_TO_A_SIZE:
		case GL_PIXEL_MAP_B_TO_B_SIZE:
		case GL_PIXEL_MAP_G_TO_G_SIZE:
		case GL_PIXEL_MAP_I_TO_A_SIZE:
		case GL_PIXEL_MAP_I_TO_B_SIZE:
		case GL_PIXEL_MAP_I_TO_G_SIZE:
		case GL_PIXEL_MAP_I_TO_I_SIZE:
		case GL_PIXEL_MAP_I_TO_R_SIZE:
		case GL_PIXEL_MAP_R_TO_R_SIZE:
		case GL_PIXEL_MAP_S_TO_S_SIZE:
		case GL_POINT_SIZE:
		case GL_POINT_SIZE_GRANULARITY:
		case GL_POINT_SMOOTH:
		case GL_POINT_SMOOTH_HINT:
		case GL_POLYGON_SMOOTH:
		case GL_POLYGON_SMOOTH_HINT:
		case GL_POLYGON_STIPPLE:
		case GL_PROJECTION_STACK_DEPTH:
		case GL_READ_BUFFER:
		case GL_RED_BIAS:
		case GL_RED_BITS:
		case GL_RED_SCALE:
		case GL_RENDER_MODE:
		case GL_RGBA_MODE:
		case GL_SCISSOR_TEST:
		case GL_SHADE_MODEL:
		case GL_STENCIL_BITS:
		case GL_STENCIL_CLEAR_VALUE:
		case GL_STENCIL_FAIL:
		case GL_STENCIL_FUNC:
		case GL_STENCIL_PASS_DEPTH_FAIL:
		case GL_STENCIL_PASS_DEPTH_PASS:
		case GL_STENCIL_REF:
		case GL_STENCIL_TEST:
		case GL_STENCIL_VALUE_MASK:
		case GL_STENCIL_WRITEMASK:
		case GL_STEREO:
		case GL_SUBPIXEL_BITS:
		case GL_TEXTURE_1D:
		case GL_TEXTURE_2D:
		case GL_TEXTURE_ENV_MODE:
		case GL_TEXTURE_GEN_S:
		case GL_TEXTURE_GEN_R:
		case GL_TEXTURE_GEN_T:
		case GL_TEXTURE_GEN_Q:
		case GL_TEXTURE_STACK_DEPTH:
		case GL_UNPACK_ALIGNMENT:
		case GL_UNPACK_LSB_FIRST:
		case GL_UNPACK_ROW_LENGTH:
		case GL_UNPACK_SKIP_PIXELS:
		case GL_UNPACK_SKIP_ROWS:
		case GL_UNPACK_SWAP_BYTES:
		case GL_ZOOM_X:
		case GL_ZOOM_Y:
			retSize = 1;
			break;
		case GL_DEPTH_RANGE:
		case GL_LINE_WIDTH_RANGE:
		case GL_MAP1_GRID_DOMAIN:
		case GL_MAP2_GRID_SEGMENTS:
		case GL_MAX_VIEWPORT_DIMS:
		case GL_POINT_SIZE_RANGE:
		case GL_POLYGON_MODE:
			retSize = 2;
			break;
		case GL_CURRENT_NORMAL:
			retSize = 3;
			break;
		case GL_ACCUM_CLEAR_VALUE:
		case GL_COLOR_WRITEMASK:
		case GL_CURRENT_COLOR:
		case GL_CURRENT_RASTER_COLOR:
		case GL_CURRENT_RASTER_POSITION:
		case GL_CURRENT_RASTER_TEXTURE_COORDS:
		case GL_CURRENT_TEXTURE_COORDS:
		case GL_FOG_COLOR:
		case GL_LIGHT_MODEL_AMBIENT:
		case GL_MAP2_GRID_DOMAIN:
		case GL_SCISSOR_BOX:
		case GL_TEXTURE_ENV_COLOR:
		case GL_VIEWPORT:
			retSize = 4;
			break;
		case GL_MODELVIEW_MATRIX:
		case GL_PROJECTION_MATRIX:
		case GL_TEXTURE_MATRIX:
			retSize = 16;
			break;
	}

	return retSize;
}

void freeGetsAllocs(void )
{
	if (boolSize)
		GB.Free(POINTER(&boolArray));

	if (floatSize)
		GB.Free(POINTER(&floatArray));

	if (intSize)
		GB.Free(POINTER(&intArray));

	boolSize = 0;
	floatSize = 0;
	intSize = 0;
}

/**************************************************************************/

/*
   First, we check if the parameter is of a known size.
   If not (0 is returned), we check the size method parameter.
   If size is always 0 (size parameter is equal to 0 or is missing)
   we return a null object.
   If size is too big (see MAXSIZE) an error is raised.

   Finally we return an object of the defined size.
*/


BEGIN_METHOD(GLGETBOOLEANV, GB_INTEGER parameter; GB_INTEGER size)

	GB_ARRAY bArray;
	int i;
	int size = checkSize(VARG(parameter));

	if (!size)
		size = VARGOPT(size, 0);

	if (size >= MAXSIZE)
	{
		GB.Error("Parameter size is too big");
		return;
	}

	if (!size)
	{
		GB.ReturnNull();
		return;
	}

	resizeBool(size);

	GB.Array.New(&bArray , GB_T_BOOLEAN , size);
	glGetBooleanv(VARG(parameter), boolArray);
	
	for (i=0;i<size; i++)
		*((int *)GB.Array.Get(bArray, i)) = boolArray[i];
	
	GB.ReturnObject(bArray);



END_METHOD

BEGIN_METHOD(GLGETFLOATV, GB_INTEGER parameter; GB_INTEGER size)

	GB_ARRAY fArray;
	int i;
	int size = checkSize(VARG(parameter));

	if (!size)
		size = VARGOPT(size, 0);

	if (size >= MAXSIZE)
	{
		GB.Error("Parameter size is too big");
		return;
	}

	if (!size)
	{
		GB.ReturnNull();
		return;
	}

	resizeFloat(size);

	GB.Array.New(&fArray , GB_T_FLOAT , size);
	glGetDoublev(VARG(parameter), floatArray);
	
	for (i=0;i<size; i++)
		*((double *)GB.Array.Get(fArray, i)) = floatArray[i];
	
	GB.ReturnObject(fArray);

END_METHOD

BEGIN_METHOD(GLGETINTEGERV, GB_INTEGER parameter; GB_INTEGER size)

	GB_ARRAY iArray;
	int i;
	int size = checkSize(VARG(parameter));

	if (!size)
		size = VARGOPT(size, 0);

	if (size >= MAXSIZE)
	{
		GB.Error("Parameter size is too big");
		return;
	}

	if (!size)
	{
		GB.ReturnNull();
		return;
	}

	resizeInt(size);

	GB.Array.New(&iArray , GB_T_INTEGER , size);
	glGetIntegerv(VARG(parameter), intArray);
	
	for (i=0;i<size; i++)
		*((int *)GB.Array.Get(iArray, i)) = intArray[i];
	
	GB.ReturnObject(iArray);

END_METHOD

