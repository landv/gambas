/***************************************************************************

  CMD2MODEL.c

  (c) 2012 Tomasz Kołodziejczyk "Tommyline"

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

#define __CMD2MODEL_C

#include "gb_common.h"
#include "cmd2model.h"
#include <stdio.h>

#define THIS OBJECT(MD2MODEL)

/* Table of precalculated normals */
	GLfloat anorms_table[162][3] = {
{ -0.525731f,  0.000000f,  0.850651f }, 
{ -0.442863f,  0.238856f,  0.864188f }, 
{ -0.295242f,  0.000000f,  0.955423f }, 
{ -0.309017f,  0.500000f,  0.809017f }, 
{ -0.162460f,  0.262866f,  0.951056f }, 
{  0.000000f,  0.000000f,  1.000000f }, 
{  0.000000f,  0.850651f,  0.525731f }, 
{ -0.147621f,  0.716567f,  0.681718f }, 
{  0.147621f,  0.716567f,  0.681718f }, 
{  0.000000f,  0.525731f,  0.850651f }, 
{  0.309017f,  0.500000f,  0.809017f }, 
{  0.525731f,  0.000000f,  0.850651f }, 
{  0.295242f,  0.000000f,  0.955423f }, 
{  0.442863f,  0.238856f,  0.864188f }, 
{  0.162460f,  0.262866f,  0.951056f }, 
{ -0.681718f,  0.147621f,  0.716567f }, 
{ -0.809017f,  0.309017f,  0.500000f }, 
{ -0.587785f,  0.425325f,  0.688191f }, 
{ -0.850651f,  0.525731f,  0.000000f }, 
{ -0.864188f,  0.442863f,  0.238856f }, 
{ -0.716567f,  0.681718f,  0.147621f }, 
{ -0.688191f,  0.587785f,  0.425325f }, 
{ -0.500000f,  0.809017f,  0.309017f }, 
{ -0.238856f,  0.864188f,  0.442863f }, 
{ -0.425325f,  0.688191f,  0.587785f }, 
{ -0.716567f,  0.681718f, -0.147621f }, 
{ -0.500000f,  0.809017f, -0.309017f }, 
{ -0.525731f,  0.850651f,  0.000000f }, 
{  0.000000f,  0.850651f, -0.525731f }, 
{ -0.238856f,  0.864188f, -0.442863f }, 
{  0.000000f,  0.955423f, -0.295242f }, 
{ -0.262866f,  0.951056f, -0.162460f }, 
{  0.000000f,  1.000000f,  0.000000f }, 
{  0.000000f,  0.955423f,  0.295242f }, 
{ -0.262866f,  0.951056f,  0.162460f }, 
{  0.238856f,  0.864188f,  0.442863f }, 
{  0.262866f,  0.951056f,  0.162460f }, 
{  0.500000f,  0.809017f,  0.309017f }, 
{  0.238856f,  0.864188f, -0.442863f }, 
{  0.262866f,  0.951056f, -0.162460f }, 
{  0.500000f,  0.809017f, -0.309017f }, 
{  0.850651f,  0.525731f,  0.000000f }, 
{  0.716567f,  0.681718f,  0.147621f }, 
{  0.716567f,  0.681718f, -0.147621f }, 
{  0.525731f,  0.850651f,  0.000000f }, 
{  0.425325f,  0.688191f,  0.587785f }, 
{  0.864188f,  0.442863f,  0.238856f }, 
{  0.688191f,  0.587785f,  0.425325f }, 
{  0.809017f,  0.309017f,  0.500000f }, 
{  0.681718f,  0.147621f,  0.716567f }, 
{  0.587785f,  0.425325f,  0.688191f }, 
{  0.955423f,  0.295242f,  0.000000f }, 
{  1.000000f,  0.000000f,  0.000000f }, 
{  0.951056f,  0.162460f,  0.262866f }, 
{  0.850651f, -0.525731f,  0.000000f }, 
{  0.955423f, -0.295242f,  0.000000f }, 
{  0.864188f, -0.442863f,  0.238856f }, 
{  0.951056f, -0.162460f,  0.262866f }, 
{  0.809017f, -0.309017f,  0.500000f }, 
{  0.681718f, -0.147621f,  0.716567f }, 
{  0.850651f,  0.000000f,  0.525731f }, 
{  0.864188f,  0.442863f, -0.238856f }, 
{  0.809017f,  0.309017f, -0.500000f }, 
{  0.951056f,  0.162460f, -0.262866f }, 
{  0.525731f,  0.000000f, -0.850651f }, 
{  0.681718f,  0.147621f, -0.716567f }, 
{  0.681718f, -0.147621f, -0.716567f }, 
{  0.850651f,  0.000000f, -0.525731f }, 
{  0.809017f, -0.309017f, -0.500000f }, 
{  0.864188f, -0.442863f, -0.238856f }, 
{  0.951056f, -0.162460f, -0.262866f }, 
{  0.147621f,  0.716567f, -0.681718f }, 
{  0.309017f,  0.500000f, -0.809017f }, 
{  0.425325f,  0.688191f, -0.587785f }, 
{  0.442863f,  0.238856f, -0.864188f }, 
{  0.587785f,  0.425325f, -0.688191f }, 
{  0.688191f,  0.587785f, -0.425325f }, 
{ -0.147621f,  0.716567f, -0.681718f }, 
{ -0.309017f,  0.500000f, -0.809017f }, 
{  0.000000f,  0.525731f, -0.850651f }, 
{ -0.525731f,  0.000000f, -0.850651f }, 
{ -0.442863f,  0.238856f, -0.864188f }, 
{ -0.295242f,  0.000000f, -0.955423f }, 
{ -0.162460f,  0.262866f, -0.951056f }, 
{  0.000000f,  0.000000f, -1.000000f }, 
{  0.295242f,  0.000000f, -0.955423f }, 
{  0.162460f,  0.262866f, -0.951056f }, 
{ -0.442863f, -0.238856f, -0.864188f }, 
{ -0.309017f, -0.500000f, -0.809017f }, 
{ -0.162460f, -0.262866f, -0.951056f }, 
{  0.000000f, -0.850651f, -0.525731f }, 
{ -0.147621f, -0.716567f, -0.681718f }, 
{  0.147621f, -0.716567f, -0.681718f }, 
{  0.000000f, -0.525731f, -0.850651f }, 
{  0.309017f, -0.500000f, -0.809017f }, 
{  0.442863f, -0.238856f, -0.864188f }, 
{  0.162460f, -0.262866f, -0.951056f }, 
{  0.238856f, -0.864188f, -0.442863f }, 
{  0.500000f, -0.809017f, -0.309017f }, 
{  0.425325f, -0.688191f, -0.587785f }, 
{  0.716567f, -0.681718f, -0.147621f }, 
{  0.688191f, -0.587785f, -0.425325f }, 
{  0.587785f, -0.425325f, -0.688191f }, 
{  0.000000f, -0.955423f, -0.295242f }, 
{  0.000000f, -1.000000f,  0.000000f }, 
{  0.262866f, -0.951056f, -0.162460f }, 
{  0.000000f, -0.850651f,  0.525731f }, 
{  0.000000f, -0.955423f,  0.295242f }, 
{  0.238856f, -0.864188f,  0.442863f }, 
{  0.262866f, -0.951056f,  0.162460f }, 
{  0.500000f, -0.809017f,  0.309017f }, 
{  0.716567f, -0.681718f,  0.147621f }, 
{  0.525731f, -0.850651f,  0.000000f }, 
{ -0.238856f, -0.864188f, -0.442863f }, 
{ -0.500000f, -0.809017f, -0.309017f }, 
{ -0.262866f, -0.951056f, -0.162460f }, 
{ -0.850651f, -0.525731f,  0.000000f }, 
{ -0.716567f, -0.681718f, -0.147621f }, 
{ -0.716567f, -0.681718f,  0.147621f }, 
{ -0.525731f, -0.850651f,  0.000000f }, 
{ -0.500000f, -0.809017f,  0.309017f }, 
{ -0.238856f, -0.864188f,  0.442863f }, 
{ -0.262866f, -0.951056f,  0.162460f }, 
{ -0.864188f, -0.442863f,  0.238856f }, 
{ -0.809017f, -0.309017f,  0.500000f }, 
{ -0.688191f, -0.587785f,  0.425325f }, 
{ -0.681718f, -0.147621f,  0.716567f }, 
{ -0.442863f, -0.238856f,  0.864188f }, 
{ -0.587785f, -0.425325f,  0.688191f }, 
{ -0.309017f, -0.500000f,  0.809017f }, 
{ -0.147621f, -0.716567f,  0.681718f }, 
{ -0.425325f, -0.688191f,  0.587785f }, 
{ -0.162460f, -0.262866f,  0.951056f }, 
{  0.442863f, -0.238856f,  0.864188f }, 
{  0.162460f, -0.262866f,  0.951056f }, 
{  0.309017f, -0.500000f,  0.809017f }, 
{  0.147621f, -0.716567f,  0.681718f }, 
{  0.000000f, -0.525731f,  0.850651f }, 
{  0.425325f, -0.688191f,  0.587785f }, 
{  0.587785f, -0.425325f,  0.688191f }, 
{  0.688191f, -0.587785f,  0.425325f }, 
{ -0.955423f,  0.295242f,  0.000000f }, 
{ -0.951056f,  0.162460f,  0.262866f }, 
{ -1.000000f,  0.000000f,  0.000000f }, 
{ -0.850651f,  0.000000f,  0.525731f }, 
{ -0.955423f, -0.295242f,  0.000000f }, 
{ -0.951056f, -0.162460f,  0.262866f }, 
{ -0.864188f,  0.442863f, -0.238856f }, 
{ -0.951056f,  0.162460f, -0.262866f }, 
{ -0.809017f,  0.309017f, -0.500000f }, 
{ -0.864188f, -0.442863f, -0.238856f }, 
{ -0.951056f, -0.162460f, -0.262866f }, 
{ -0.809017f, -0.309017f, -0.500000f }, 
{ -0.681718f,  0.147621f, -0.716567f }, 
{ -0.681718f, -0.147621f, -0.716567f }, 
{ -0.850651f,  0.000000f, -0.525731f }, 
{ -0.688191f,  0.587785f, -0.425325f }, 
{ -0.587785f,  0.425325f, -0.688191f }, 
{ -0.425325f,  0.688191f, -0.587785f }, 
{ -0.425325f, -0.688191f, -0.587785f }, 
{ -0.587785f, -0.425325f, -0.688191f }, 
{ -0.688191f, -0.587785f, -0.425325f }
};


MD2MODEL *Md2Model__create()
{
	return (MD2MODEL*)GB.New(GB.FindClass("Md2Model"), NULL, NULL);
};

/* 	
	This method creates and loads new Md2 model. To use this in the program you should declare 
	it first as ie. 
	Public model as Md2Model
	Then initiate it loading
	model = Md2Model.Load(name as string)
*/

BEGIN_METHOD(Md2Model_Load, GB_STRING name)

	MD2MODEL *mdl = Md2Model__create();
	char *filename = GB.ToZeroString (ARG(name));
	FILE *fp;
  	int i,fi;

  	fp = fopen (filename, "rb");
  	if (!fp)
    {
      	GB.Error ("Error: couldn't open \"%s\"!\n", filename);
      	return ;
    }

 	// Read header 
  	fi=fread (&mdl->ident, 1, sizeof (int), fp);
	fi=fread (&mdl->version, 1, sizeof (int), fp);
	fi=fread (&mdl->skinwidth, 1, sizeof (int), fp);
	fi=fread (&mdl->skinheight, 1, sizeof (int), fp);
	fi=fread (&mdl->framesize, 1, sizeof (int), fp);
	fi=fread (&mdl->num_skins, 1, sizeof (int), fp);
	fi=fread (&mdl->num_vertices, 1, sizeof (int), fp);
	fi=fread (&mdl->num_st, 1, sizeof (int), fp);
	fi=fread (&mdl->num_tris, 1, sizeof (int), fp);
	fi=fread (&mdl->num_glcmds, 1, sizeof (int), fp);
	fi=fread (&mdl->num_frames, 1, sizeof (int), fp);
	fi=fread (&mdl->offset_skins, 1, sizeof (int), fp);
	fi=fread (&mdl->offset_st, 1, sizeof (int), fp);
	fi=fread (&mdl->offset_tris, 1, sizeof (int), fp);
	fi=fread (&mdl->offset_frames, 1, sizeof (int), fp);
	fi=fread (&mdl->offset_glcmds, 1, sizeof (int), fp);
	fi=fread (&mdl->offset_end, 1, sizeof (int), fp);

	if ((mdl->ident != 844121161) || (mdl->version != 8))
    {
      // Error! 
      GB.Error ("Error: bad version or identifier\n");
      fclose (fp);
      return; 
    }

  	// Memory allocations 
  	GB.Alloc ((void *) &mdl->skins, sizeof (skinmd2) * mdl->num_skins );
  	GB.Alloc ((void *) &mdl->texcoords, sizeof (texCoordmd2) * mdl->num_st);
  	GB.Alloc ((void *) &mdl->triangles, sizeof (trianglemd2) * mdl->num_tris);
  	GB.Alloc ((void *) &mdl->frames, sizeof (framemd2) * mdl->num_frames);
  	GB.Alloc ((void *) &mdl->glcmds, sizeof (int) * mdl->num_glcmds);
 	// Read model data 
  	fseek (fp, mdl->offset_skins, SEEK_SET);
  	fi=fread (mdl->skins, sizeof (skinmd2), mdl->num_skins, fp);

  	fseek (fp, mdl->offset_st, SEEK_SET);
  	fi=fread (mdl->texcoords, sizeof (texCoordmd2), mdl->num_st, fp);

  	fseek (fp, mdl->offset_tris, SEEK_SET);
  	fi=fread (mdl->triangles, sizeof (trianglemd2), mdl->num_tris, fp);

  	fseek (fp, mdl->offset_glcmds, SEEK_SET);
  	fi=fread (mdl->glcmds, sizeof (int), mdl->num_glcmds, fp);

  	// Read frames 
  	fseek (fp, mdl->offset_frames, SEEK_SET);
  	for (i = 0; i < mdl->num_frames; ++i)
    {
      // Memory allocation for vertices of this frame  
      GB.Alloc ((void *) &mdl->frames[i].verts, sizeof (vertexmd2) * mdl->num_vertices);

      // Read frame data 
      fi=fread (mdl->frames[i].scale, sizeof (float)*3, 1, fp);
      fi=fread (mdl->frames[i].translate, sizeof (float)*3, 1, fp);
      fi=fread (mdl->frames[i].name, sizeof (char), 16, fp);
      fi=fread (mdl->frames[i].verts, sizeof (vertexmd2), mdl->num_vertices, fp);
    }
	i=fi;
	mdl->scale[0]=1;
	mdl->scale[1]=1;
	mdl->scale[2]=1;
  	fclose (fp);	
	
	GB.ReturnObject(mdl);

END_METHOD

BEGIN_METHOD(Md2Model_SetPosition, GB_FLOAT x; GB_FLOAT y; GB_FLOAT z)

	THIS->position[0]=VARG(x);
	THIS->position[1]=VARG(y);
	THIS->position[2]=VARG(z);
	
END_METHOD

BEGIN_METHOD(Md2Model_DrawFrame, GB_INTEGER f_no; GB_INTEGER texture)

  int n = VARG(f_no);
  int i, *pglcmds;
  GLfloat  v[3];
  framemd2 *pframe;
  vertexmd2 *pvert;
  glcmd *packet;
 	
  // Check if n is in a valid range 
  if ((n < 0) || (n >= THIS->num_frames )) n=0;
	glPushMatrix();
	glTranslatef(THIS->position[0], THIS->position[1], THIS->position[2]);
	glRotatef(-90, 1, 0, 0);
 	glRotatef(-90, 0, 0, 1);
	glScalef(THIS->scale[0], THIS->scale[1], THIS->scale[2]);
	 // Enable model's texture 
  glBindTexture (GL_TEXTURE_2D, VARG(texture));

  // pglcmds points at the start of the command list 
  pglcmds = THIS->glcmds;

  // Draw the model 
  while ((i = *(pglcmds++)) != 0)
    {
      if (i < 0)
	{
	  glBegin (GL_TRIANGLE_FAN);
	  i = -i;
	}
      else
	{
	  glBegin (GL_TRIANGLE_STRIP);
	}

      // Draw each vertex of this group 
      for (/* Nothing */ ; i > 0; --i, pglcmds += 3)
	{
	  packet = (glcmd *)pglcmds;
	  pframe = &THIS->frames[n];
	  pvert = &pframe->verts[packet->index];

	  // Pass texture coordinates to OpenGL 
	  glTexCoord2f (packet->s, packet->t);

	  // Normal vector 
	  glNormal3fv (anorms_table[pvert->normalIndex]);

	  // Calculate vertex real position 
	  v[0] = (pframe->scale[0] * pvert->v[0]) + pframe->translate[0];
	  v[1] = (pframe->scale[1] * pvert->v[1]) + pframe->translate[1];
	  v[2] = (pframe->scale[2] * pvert->v[2]) + pframe->translate[2];

	  glVertex3fv (v);
	}

      glEnd ();
    }
	glScalef(1/THIS->scale[0], 1/THIS->scale[1], 1/THIS->scale[2]);
	glPopMatrix();
	
END_METHOD

BEGIN_METHOD(Md2Model_DrawInterFrame, GB_INTEGER f_no; GB_FLOAT inter; GB_INTEGER texture)
	
 int i, j, n=VARG(f_no);
  GLfloat s, t, v_curr[3], v_next[3], v[3], norm[3];
  float *n_curr, *n_next, interp = VARG(inter);
  framemd2 *pframe1, *pframe2;
  vertexmd2 *pvert1, *pvert2;

  	if ((n < 0) || (n >= THIS->num_frames-1)) {
	n=0;
	interp = 0;
	}	
	glPushMatrix();
	glTranslatef(THIS->position[0], THIS->position[1], THIS->position[2]);
	glRotatef(-90, 1, 0, 0);
 	glRotatef(-90, 0, 0, 1);
	glScalef(THIS->scale[0], THIS->scale[1], THIS->scale[2]);
	glBindTexture (GL_TEXTURE_2D, VARG(texture));

  	glBegin (GL_TRIANGLES);
    for (i = 0; i < THIS->num_tris; ++i)
    {
		for (j = 0; j < 3; ++j)
	  	{
	    	pframe1 = &THIS->frames[n];
	    	pframe2 = &THIS->frames[n + 1];
	    	pvert1 = &pframe1->verts[THIS->triangles[i].vertex[j]];
	    	pvert2 = &pframe2->verts[THIS->triangles[i].vertex[j]];

	    	s = (GLfloat)THIS->texcoords[THIS->triangles[i].st[j]].s / THIS->skinwidth;
	    	t = (GLfloat)THIS->texcoords[THIS->triangles[i].st[j]].t / THIS->skinheight;

	    	glTexCoord2f (s, t);

	    	n_curr = anorms_table[pvert1->normalIndex];
	    	n_next = anorms_table[pvert2->normalIndex];

	    	norm[0] = n_curr[0] + interp * (n_next[0] - n_curr[0]);
	    	norm[1] = n_curr[1] + interp * (n_next[1] - n_curr[1]);
	    	norm[2] = n_curr[2] + interp * (n_next[2] - n_curr[2]);

	    	glNormal3fv (norm);

	    	v_curr[0] = pframe1->scale[0] * pvert1->v[0] + pframe1->translate[0];
	    	v_curr[1] = pframe1->scale[1] * pvert1->v[1] + pframe1->translate[1];
	    	v_curr[2] = pframe1->scale[2] * pvert1->v[2] + pframe1->translate[2];

	    	v_next[0] = pframe2->scale[0] * pvert2->v[0] + pframe2->translate[0];
	    	v_next[1] = pframe2->scale[1] * pvert2->v[1] + pframe2->translate[1];
	    	v_next[2] = pframe2->scale[2] * pvert2->v[2] + pframe2->translate[2];

	    	v[0] = v_curr[0] + interp * (v_next[0] - v_curr[0]);
	    	v[1] = v_curr[1] + interp * (v_next[1] - v_curr[1]);
	    	v[2] = v_curr[2] + interp * (v_next[2] - v_curr[2]);

	    	glVertex3fv (v);
	  	}
    }
  	glEnd ();
	glScalef(1/THIS->scale[0], 1/THIS->scale[1], 1/THIS->scale[2]);
	glPopMatrix();

END_METHOD

BEGIN_METHOD_VOID(Md2Model_GetNoFrames)

	GB.ReturnInteger(THIS->num_frames);
	
END_METHOD

BEGIN_METHOD(Md2Model_GetFrameName, GB_INTEGER fn )

	GB.ReturnNewString(THIS->frames[VARG(fn)].name, 16);
	
END_METHOD

BEGIN_METHOD(Md2Model_Scale, GB_FLOAT sx; GB_FLOAT sy; GB_FLOAT sz)

	THIS->scale[0] = VARG(sx);
	THIS->scale[1] = VARG(sy);
	THIS->scale[2] = VARG(sz);

END_METHOD


BEGIN_METHOD_VOID(Md2Model_free)
int i;
	GB.Free ((void *) &THIS->skins);
  	GB.Free ((void *) &THIS->texcoords);
  	GB.Free ((void *) &THIS->triangles);
	for (i = 0; i < THIS->num_frames; ++i) GB.Free ((void *) &THIS->frames[i].verts);
	GB.Free ((void *) &THIS->frames);
  	GB.Free ((void *) &THIS->glcmds);
	
END_METHOD

GB_DESC Md2ModelDesc[] =
{
	GB_DECLARE("Md2Model", sizeof(MD2MODEL)),
	GB_NOT_CREATABLE(),
	GB_STATIC_METHOD("Load", "Md2Model" , Md2Model_Load, "(Name)s"),
	GB_METHOD("SetPosition", NULL, Md2Model_SetPosition, "(X)f(Y)f(Z)f" ),
	GB_METHOD("DrawFrame", NULL, Md2Model_DrawFrame, "(Frame_No)i(Texture)i" ),
	GB_METHOD("DrawInterFrame", NULL, Md2Model_DrawInterFrame, "(Frame_No)i(InterFrame)f(Texture)i" ),
	GB_METHOD("GetFramesNo", "i", Md2Model_GetNoFrames, NULL ),
	GB_METHOD("GetFrameName", "s", Md2Model_GetFrameName, "(FrameNumber)i" ),
	GB_METHOD("Scale", NULL, Md2Model_Scale, "(Scale_x)f(Scale_y)f(Scale_z)f" ),
	GB_METHOD("_free", NULL, Md2Model_free, NULL ),
	GB_END_DECLARE
};


