/***************************************************************************

  cmd2model.h

  (c) 2012 Tomasz Kolodziejczyk "Tommyline" 

  Based on David HENRY's md2.c md2 model loader and renderer
=========================================================================
 *
 * md2.c -- md2 model loader
 * last modification: aug. 14, 2007
 *
 * Copyright (c) 2005-2007 David HENRY
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use,
 * copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR
 * ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF
 * CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 * WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * gcc -Wall -ansi -lGL -lGLU -lglut md2.c -o md2
 *
=========================================================================
  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General 		License as published by
  the Free Software Foundation; either version 2, or (at your option)
  any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General 		License for more details.

  You should have received a copy of the GNU General 		License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
  MA 02110-1301, USA.

***************************************************************************/

#ifndef __CMD2MODEL_H
#define __CMD2MODEL_H

#include "gambas.h"
#include "main.h"
#include "cmd2object.h"

#ifndef __CMD2MODEL_C
extern GB_DESC Md2ModelFrameDesc[];
extern GB_DESC Md2ModelDesc[];
#endif

/* Vector */
typedef float vec3[3];


/* Texture name */
typedef
struct md2_skin
{
  char name[68];
}skinmd2;

/* Texture coords */
typedef
struct md2_texCoordmd2
{
  short s;
  short t;
}texCoordmd2;

/* Triangle info */
typedef
struct md2_trianglemd2
{
  unsigned short vertex[3];
  unsigned short st[3];
}trianglemd2;

/* Compressed vertexmd2 */
typedef
struct md2_vertexmd2
{
  unsigned char v[3];
  unsigned char normalIndex;
}vertexmd2;

/* Model frame */
typedef
struct md2_frame
{
  float scale[3];
  float translate[3];
  char name[16];
  vertexmd2 *verts;
}framemd2;

/* GL command packet */
typedef
struct md2_glcmd
{
  float s;
  float t;
  int index;
}glcmd;


typedef
	struct CMD2MODEL {
		GB_BASE ob;
		//Header
		int ident;
		int version;

		int skinwidth;
		int skinheight;

		int framesize;

		int num_skins;
		int num_vertices;
		int num_st;
		int num_tris;
		int num_glcmds;
		int num_frames;

		int offset_skins;
		int offset_st;
		int offset_tris;
		int offset_frames;
		int offset_glcmds;
		int offset_end;
		//End header

		skinmd2 *skins;
		texCoordmd2 *texcoords;
		trianglemd2 *triangles;
		framemd2 *frames;
		int *glcmds;

		//Model specific data
		float scale[3];
		int frame; // frame being accessed with the [] operator
		GLuint texture;
		//End model specific data
		}
	CMD2MODEL;


CMD2MODEL *MD2MODEL_create(void);
int MD2MODEL_draw(CMD2MODEL *_object, double frame, int texture, float *pos, float *rotate, float *scale);

#endif
