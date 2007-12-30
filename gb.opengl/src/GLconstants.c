/***************************************************************************

  GLconstants.c

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

#define __GLCONSTANTS_C

#include "gambas.h"
#include "gb_common.h"
#include "main.h"

#include "GLconstants.h"

#include <GL/gl.h>
#include <GL/glu.h>

/**************************************************************************

  GL_xxxxx constants

***************************************************************************/

GB_DESC CglConstants[] =
{
	GB_DECLARE("Glconst",0), GB_VIRTUAL_CLASS(),



/* Vertex Arrays */

  GB_CONSTANT("VertexArray", "i", GL_VERTEX_ARRAY),
  GB_CONSTANT("NormalArray", "i", GL_NORMAL_ARRAY),
  GB_CONSTANT("ColorArray", "i", GL_COLOR_ARRAY),
  GB_CONSTANT("IndexArray", "i", GL_INDEX_ARRAY),
  GB_CONSTANT("TextureCoordArray", "i", GL_TEXTURE_COORD_ARRAY),
  GB_CONSTANT("EdgeFlagArray", "i", GL_EDGE_FLAG_ARRAY),
  GB_CONSTANT("VertexArraySize", "i", GL_VERTEX_ARRAY_SIZE),
  GB_CONSTANT("VertexArrayType", "i", GL_VERTEX_ARRAY_TYPE),
  GB_CONSTANT("VertexArrayStride", "i", GL_VERTEX_ARRAY_STRIDE),
  GB_CONSTANT("NormalArrayType", "i", GL_NORMAL_ARRAY_TYPE),
  GB_CONSTANT("NormalArrayStride", "i", GL_NORMAL_ARRAY_STRIDE),
  GB_CONSTANT("ColorArraySize", "i", GL_COLOR_ARRAY_SIZE),
  GB_CONSTANT("ColorArrayType", "i", GL_COLOR_ARRAY_TYPE),
  GB_CONSTANT("ColorArrayStride", "i", GL_COLOR_ARRAY_STRIDE),
  GB_CONSTANT("IndexArrayType", "i", GL_INDEX_ARRAY_TYPE),
  GB_CONSTANT("IndexArrayStride", "i", GL_INDEX_ARRAY_STRIDE),
  GB_CONSTANT("TextureCoordArraySize", "i", GL_TEXTURE_COORD_ARRAY_SIZE),
  GB_CONSTANT("TextureCoordArrayType", "i", GL_TEXTURE_COORD_ARRAY_TYPE),
  GB_CONSTANT("TextureCoordArrayStride", "i", GL_TEXTURE_COORD_ARRAY_STRIDE),
  GB_CONSTANT("EdgeFlagArrayStride", "i", GL_EDGE_FLAG_ARRAY_STRIDE),
  GB_CONSTANT("VertexArrayPointer", "i", GL_VERTEX_ARRAY_POINTER),
  GB_CONSTANT("NormalArrayPointer", "i", GL_NORMAL_ARRAY_POINTER),
  GB_CONSTANT("ColorArrayPointer", "i", GL_COLOR_ARRAY_POINTER),
  GB_CONSTANT("IndexArrayPointer", "i", GL_INDEX_ARRAY_POINTER),
  GB_CONSTANT("TextureCoordArrayPointer", "i", GL_TEXTURE_COORD_ARRAY_POINTER),
  GB_CONSTANT("EdgeFlagArrayPointer", "i", GL_EDGE_FLAG_ARRAY_POINTER),
  GB_CONSTANT("V2f", "i", GL_V2F),
  GB_CONSTANT("V3f", "i", GL_V3F),
  GB_CONSTANT("C4ubV2f", "i", GL_C4UB_V2F),
  GB_CONSTANT("C4ubV3f", "i", GL_C4UB_V3F),
  GB_CONSTANT("C3fV3f", "i", GL_C3F_V3F),
  GB_CONSTANT("N3fV3f", "i", GL_N3F_V3F),
  GB_CONSTANT("C4fN3fV3f", "i", GL_C4F_N3F_V3F),
  GB_CONSTANT("T2fV3f", "i", GL_T2F_V3F),
  GB_CONSTANT("T4fV4f", "i", GL_T4F_V4F),
  GB_CONSTANT("T2fC4ubV3f", "i", GL_T2F_C4UB_V3F),
  GB_CONSTANT("T2fC3fV3f", "i", GL_T2F_C3F_V3F),
  GB_CONSTANT("T2fN3fV3f", "i", GL_T2F_N3F_V3F),
  GB_CONSTANT("T2fC4fN3fV3f", "i", GL_T2F_C4F_N3F_V3F),
  GB_CONSTANT("T4fC4fN3fV4f", "i", GL_T4F_C4F_N3F_V4F),













/* Feedback */

  GB_CONSTANT("n2d", "i", GL_2D),
  GB_CONSTANT("n3d", "i", GL_3D),
  GB_CONSTANT("n3dColor", "i", GL_3D_COLOR),
  GB_CONSTANT("n3dColorTexture", "i", GL_3D_COLOR_TEXTURE),
  GB_CONSTANT("n4dColorTexture", "i", GL_4D_COLOR_TEXTURE),
  GB_CONSTANT("PointToken", "i", GL_POINT_TOKEN),
  GB_CONSTANT("LineToken", "i", GL_LINE_TOKEN),
  GB_CONSTANT("LineResetToken", "i", GL_LINE_RESET_TOKEN),
  GB_CONSTANT("PolygonToken", "i", GL_POLYGON_TOKEN),
  GB_CONSTANT("BitmapToken", "i", GL_BITMAP_TOKEN),
  GB_CONSTANT("DrawPixelToken", "i", GL_DRAW_PIXEL_TOKEN),
  GB_CONSTANT("CopyPixelToken", "i", GL_COPY_PIXEL_TOKEN),
  GB_CONSTANT("PassThroughToken", "i", GL_PASS_THROUGH_TOKEN),
  GB_CONSTANT("FeedbackBufferPointer", "i", GL_FEEDBACK_BUFFER_POINTER),
  GB_CONSTANT("FeedbackBufferSize", "i", GL_FEEDBACK_BUFFER_SIZE),
  GB_CONSTANT("FeedbackBufferType", "i", GL_FEEDBACK_BUFFER_TYPE),


/* Selection */

  GB_CONSTANT("SelectionBufferPointer", "i", GL_SELECTION_BUFFER_POINTER),
  GB_CONSTANT("SelectionBufferSize", "i", GL_SELECTION_BUFFER_SIZE),


/* Logic Ops */

  GB_CONSTANT("LogicOp", "i", GL_LOGIC_OP),
  GB_CONSTANT("IndexLogicOp", "i", GL_INDEX_LOGIC_OP),
  GB_CONSTANT("ColorLogicOp", "i", GL_COLOR_LOGIC_OP),
  GB_CONSTANT("LogicOpMode", "i", GL_LOGIC_OP_MODE),
  GB_CONSTANT("Clear", "i", GL_CLEAR),
  GB_CONSTANT("Set", "i", GL_SET),
  GB_CONSTANT("Copy", "i", GL_COPY),
  GB_CONSTANT("CopyInverted", "i", GL_COPY_INVERTED),
  GB_CONSTANT("Noop", "i", GL_NOOP),
  GB_CONSTANT("Invert", "i", GL_INVERT),
  GB_CONSTANT("And", "i", GL_AND),
  GB_CONSTANT("Nand", "i", GL_NAND),
  GB_CONSTANT("Or", "i", GL_OR),
  GB_CONSTANT("Nor", "i", GL_NOR),
  GB_CONSTANT("Xor", "i", GL_XOR),
  GB_CONSTANT("Equiv", "i", GL_EQUIV),
  GB_CONSTANT("AndReverse", "i", GL_AND_REVERSE),
  GB_CONSTANT("AndInverted", "i", GL_AND_INVERTED),
  GB_CONSTANT("OrReverse", "i", GL_OR_REVERSE),
  GB_CONSTANT("OrInverted", "i", GL_OR_INVERTED),


/* Stencil */

  GB_CONSTANT("StencilTest", "i", GL_STENCIL_TEST),
  GB_CONSTANT("StencilWritemask", "i", GL_STENCIL_WRITEMASK),
  GB_CONSTANT("StencilBits", "i", GL_STENCIL_BITS),
  GB_CONSTANT("StencilFunc", "i", GL_STENCIL_FUNC),
  GB_CONSTANT("StencilValueMask", "i", GL_STENCIL_VALUE_MASK),
  GB_CONSTANT("StencilRef", "i", GL_STENCIL_REF),
  GB_CONSTANT("StencilFail", "i", GL_STENCIL_FAIL),
  GB_CONSTANT("StencilPassDepthPass", "i", GL_STENCIL_PASS_DEPTH_PASS),
  GB_CONSTANT("StencilPassDepthFail", "i", GL_STENCIL_PASS_DEPTH_FAIL),
  GB_CONSTANT("StencilClearValue", "i", GL_STENCIL_CLEAR_VALUE),
  GB_CONSTANT("StencilIndex", "i", GL_STENCIL_INDEX),
  GB_CONSTANT("Keep", "i", GL_KEEP),
  GB_CONSTANT("Replace", "i", GL_REPLACE),
  GB_CONSTANT("Incr", "i", GL_INCR),
  GB_CONSTANT("Decr", "i", GL_DECR),



/*GL_FRONT_AND_BACK				0x0408 */

  GB_CONSTANT("FrontLeft", "i", GL_FRONT_LEFT),
  GB_CONSTANT("FrontRight", "i", GL_FRONT_RIGHT),
  GB_CONSTANT("BackLeft", "i", GL_BACK_LEFT),
  GB_CONSTANT("BackRight", "i", GL_BACK_RIGHT),
  GB_CONSTANT("Aux0", "i", GL_AUX0),
  GB_CONSTANT("Aux1", "i", GL_AUX1),
  GB_CONSTANT("Aux2", "i", GL_AUX2),
  GB_CONSTANT("Aux3", "i", GL_AUX3),
  GB_CONSTANT("ColorIndex", "i", GL_COLOR_INDEX),
  GB_CONSTANT("Red", "i", GL_RED),
  GB_CONSTANT("Green", "i", GL_GREEN),
  GB_CONSTANT("Blue", "i", GL_BLUE),
  GB_CONSTANT("Alpha", "i", GL_ALPHA),
  GB_CONSTANT("Luminance", "i", GL_LUMINANCE),
  GB_CONSTANT("LuminanceAlpha", "i", GL_LUMINANCE_ALPHA),
  GB_CONSTANT("AlphaBits", "i", GL_ALPHA_BITS),
  GB_CONSTANT("RedBits", "i", GL_RED_BITS),
  GB_CONSTANT("GreenBits", "i", GL_GREEN_BITS),
  GB_CONSTANT("BlueBits", "i", GL_BLUE_BITS),
  GB_CONSTANT("IndexBits", "i", GL_INDEX_BITS),
  GB_CONSTANT("SubpixelBits", "i", GL_SUBPIXEL_BITS),
  GB_CONSTANT("AuxBuffers", "i", GL_AUX_BUFFERS),
  GB_CONSTANT("ReadBuffer", "i", GL_READ_BUFFER),
  GB_CONSTANT("DrawBuffer", "i", GL_DRAW_BUFFER),
  GB_CONSTANT("Doublebuffer", "i", GL_DOUBLEBUFFER),
  GB_CONSTANT("Stereo", "i", GL_STEREO),
  GB_CONSTANT("Bitmap", "i", GL_BITMAP),
  GB_CONSTANT("Color", "i", GL_COLOR),
  GB_CONSTANT("Depth", "i", GL_DEPTH),
  GB_CONSTANT("Stencil", "i", GL_STENCIL),
  GB_CONSTANT("Dither", "i", GL_DITHER),
  GB_CONSTANT("Rgb", "i", GL_RGB),
  GB_CONSTANT("Rgba", "i", GL_RGBA),




/* Gets */

  GB_CONSTANT("AttribStackDepth", "i", GL_ATTRIB_STACK_DEPTH),
  GB_CONSTANT("ClientAttribStackDepth", "i", GL_CLIENT_ATTRIB_STACK_DEPTH),
  GB_CONSTANT("ColorClearValue", "i", GL_COLOR_CLEAR_VALUE),
  GB_CONSTANT("ColorWritemask", "i", GL_COLOR_WRITEMASK),
  GB_CONSTANT("CurrentIndex", "i", GL_CURRENT_INDEX),
  GB_CONSTANT("CurrentColor", "i", GL_CURRENT_COLOR),
  GB_CONSTANT("CurrentNormal", "i", GL_CURRENT_NORMAL),
  GB_CONSTANT("CurrentRasterColor", "i", GL_CURRENT_RASTER_COLOR),
  GB_CONSTANT("CurrentRasterDistance", "i", GL_CURRENT_RASTER_DISTANCE),
  GB_CONSTANT("CurrentRasterIndex", "i", GL_CURRENT_RASTER_INDEX),
  GB_CONSTANT("CurrentRasterPosition", "i", GL_CURRENT_RASTER_POSITION),
  GB_CONSTANT("CurrentRasterTextureCoords", "i", GL_CURRENT_RASTER_TEXTURE_COORDS),
  GB_CONSTANT("CurrentRasterPositionValid", "i", GL_CURRENT_RASTER_POSITION_VALID),
  GB_CONSTANT("CurrentTextureCoords", "i", GL_CURRENT_TEXTURE_COORDS),
  GB_CONSTANT("IndexClearValue", "i", GL_INDEX_CLEAR_VALUE),
  GB_CONSTANT("IndexMode", "i", GL_INDEX_MODE),
  GB_CONSTANT("IndexWritemask", "i", GL_INDEX_WRITEMASK),
  GB_CONSTANT("ModelviewMatrix", "i", GL_MODELVIEW_MATRIX),
  GB_CONSTANT("ModelviewStackDepth", "i", GL_MODELVIEW_STACK_DEPTH),
  GB_CONSTANT("NameStackDepth", "i", GL_NAME_STACK_DEPTH),
  GB_CONSTANT("ProjectionMatrix", "i", GL_PROJECTION_MATRIX),
  GB_CONSTANT("ProjectionStackDepth", "i", GL_PROJECTION_STACK_DEPTH),
  GB_CONSTANT("RenderMode", "i", GL_RENDER_MODE),
  GB_CONSTANT("RgbaMode", "i", GL_RGBA_MODE),
  GB_CONSTANT("TextureMatrix", "i", GL_TEXTURE_MATRIX),
  GB_CONSTANT("TextureStackDepth", "i", GL_TEXTURE_STACK_DEPTH),
  GB_CONSTANT("Viewport", "i", GL_VIEWPORT),


/* Evaluators */

  GB_CONSTANT("AutoNormal", "i", GL_AUTO_NORMAL),
  GB_CONSTANT("Map1Color4", "i", GL_MAP1_COLOR_4),
  GB_CONSTANT("Map1Index", "i", GL_MAP1_INDEX),
  GB_CONSTANT("Map1Normal", "i", GL_MAP1_NORMAL),
  GB_CONSTANT("Map1TextureCoord1", "i", GL_MAP1_TEXTURE_COORD_1),
  GB_CONSTANT("Map1TextureCoord2", "i", GL_MAP1_TEXTURE_COORD_2),
  GB_CONSTANT("Map1TextureCoord3", "i", GL_MAP1_TEXTURE_COORD_3),
  GB_CONSTANT("Map1TextureCoord4", "i", GL_MAP1_TEXTURE_COORD_4),
  GB_CONSTANT("Map1Vertex3", "i", GL_MAP1_VERTEX_3),
  GB_CONSTANT("Map1Vertex4", "i", GL_MAP1_VERTEX_4),
  GB_CONSTANT("Map2Color4", "i", GL_MAP2_COLOR_4),
  GB_CONSTANT("Map2Index", "i", GL_MAP2_INDEX),
  GB_CONSTANT("Map2Normal", "i", GL_MAP2_NORMAL),
  GB_CONSTANT("Map2TextureCoord1", "i", GL_MAP2_TEXTURE_COORD_1),
  GB_CONSTANT("Map2TextureCoord2", "i", GL_MAP2_TEXTURE_COORD_2),
  GB_CONSTANT("Map2TextureCoord3", "i", GL_MAP2_TEXTURE_COORD_3),
  GB_CONSTANT("Map2TextureCoord4", "i", GL_MAP2_TEXTURE_COORD_4),
  GB_CONSTANT("Map2Vertex3", "i", GL_MAP2_VERTEX_3),
  GB_CONSTANT("Map2Vertex4", "i", GL_MAP2_VERTEX_4),
  GB_CONSTANT("Map1GridDomain", "i", GL_MAP1_GRID_DOMAIN),
  GB_CONSTANT("Map1GridSegments", "i", GL_MAP1_GRID_SEGMENTS),
  GB_CONSTANT("Map2GridDomain", "i", GL_MAP2_GRID_DOMAIN),
  GB_CONSTANT("Map2GridSegments", "i", GL_MAP2_GRID_SEGMENTS),
  GB_CONSTANT("Coeff", "i", GL_COEFF),
  GB_CONSTANT("Domain", "i", GL_DOMAIN),
  GB_CONSTANT("Order", "i", GL_ORDER),





/* Pixel Mode / Transfer */

  GB_CONSTANT("MapColor", "i", GL_MAP_COLOR),
  GB_CONSTANT("MapStencil", "i", GL_MAP_STENCIL),
  GB_CONSTANT("IndexShift", "i", GL_INDEX_SHIFT),
  GB_CONSTANT("IndexOffset", "i", GL_INDEX_OFFSET),
  GB_CONSTANT("RedScale", "i", GL_RED_SCALE),
  GB_CONSTANT("RedBias", "i", GL_RED_BIAS),
  GB_CONSTANT("GreenScale", "i", GL_GREEN_SCALE),
  GB_CONSTANT("GreenBias", "i", GL_GREEN_BIAS),
  GB_CONSTANT("BlueScale", "i", GL_BLUE_SCALE),
  GB_CONSTANT("BlueBias", "i", GL_BLUE_BIAS),
  GB_CONSTANT("AlphaScale", "i", GL_ALPHA_SCALE),
  GB_CONSTANT("AlphaBias", "i", GL_ALPHA_BIAS),
  GB_CONSTANT("DepthScale", "i", GL_DEPTH_SCALE),
  GB_CONSTANT("DepthBias", "i", GL_DEPTH_BIAS),
  GB_CONSTANT("PixelMapSToSSize", "i", GL_PIXEL_MAP_S_TO_S_SIZE),
  GB_CONSTANT("PixelMapIToISize", "i", GL_PIXEL_MAP_I_TO_I_SIZE),
  GB_CONSTANT("PixelMapIToRSize", "i", GL_PIXEL_MAP_I_TO_R_SIZE),
  GB_CONSTANT("PixelMapIToGSize", "i", GL_PIXEL_MAP_I_TO_G_SIZE),
  GB_CONSTANT("PixelMapIToBSize", "i", GL_PIXEL_MAP_I_TO_B_SIZE),
  GB_CONSTANT("PixelMapIToASize", "i", GL_PIXEL_MAP_I_TO_A_SIZE),
  GB_CONSTANT("PixelMapRToRSize", "i", GL_PIXEL_MAP_R_TO_R_SIZE),
  GB_CONSTANT("PixelMapGToGSize", "i", GL_PIXEL_MAP_G_TO_G_SIZE),
  GB_CONSTANT("PixelMapBToBSize", "i", GL_PIXEL_MAP_B_TO_B_SIZE),
  GB_CONSTANT("PixelMapAToASize", "i", GL_PIXEL_MAP_A_TO_A_SIZE),
  GB_CONSTANT("PixelMapSToS", "i", GL_PIXEL_MAP_S_TO_S),
  GB_CONSTANT("PixelMapIToI", "i", GL_PIXEL_MAP_I_TO_I),
  GB_CONSTANT("PixelMapIToR", "i", GL_PIXEL_MAP_I_TO_R),
  GB_CONSTANT("PixelMapIToG", "i", GL_PIXEL_MAP_I_TO_G),
  GB_CONSTANT("PixelMapIToB", "i", GL_PIXEL_MAP_I_TO_B),
  GB_CONSTANT("PixelMapIToA", "i", GL_PIXEL_MAP_I_TO_A),
  GB_CONSTANT("PixelMapRToR", "i", GL_PIXEL_MAP_R_TO_R),
  GB_CONSTANT("PixelMapGToG", "i", GL_PIXEL_MAP_G_TO_G),
  GB_CONSTANT("PixelMapBToB", "i", GL_PIXEL_MAP_B_TO_B),
  GB_CONSTANT("PixelMapAToA", "i", GL_PIXEL_MAP_A_TO_A),
  GB_CONSTANT("PackAlignment", "i", GL_PACK_ALIGNMENT),
  GB_CONSTANT("PackLsbFirst", "i", GL_PACK_LSB_FIRST),
  GB_CONSTANT("PackRowLength", "i", GL_PACK_ROW_LENGTH),
  GB_CONSTANT("PackSkipPixels", "i", GL_PACK_SKIP_PIXELS),
  GB_CONSTANT("PackSkipRows", "i", GL_PACK_SKIP_ROWS),
  GB_CONSTANT("PackSwapBytes", "i", GL_PACK_SWAP_BYTES),
  GB_CONSTANT("UnpackAlignment", "i", GL_UNPACK_ALIGNMENT),
  GB_CONSTANT("UnpackLsbFirst", "i", GL_UNPACK_LSB_FIRST),
  GB_CONSTANT("UnpackRowLength", "i", GL_UNPACK_ROW_LENGTH),
  GB_CONSTANT("UnpackSkipPixels", "i", GL_UNPACK_SKIP_PIXELS),
  GB_CONSTANT("UnpackSkipRows", "i", GL_UNPACK_SKIP_ROWS),
  GB_CONSTANT("UnpackSwapBytes", "i", GL_UNPACK_SWAP_BYTES),
  GB_CONSTANT("ZoomX", "i", GL_ZOOM_X),
  GB_CONSTANT("ZoomY", "i", GL_ZOOM_Y),






  GB_END_DECLARE
};

