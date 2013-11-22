/***************************************************************************

  cmd2model.c

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

#define THIS OBJECT(CMD2MODEL)

/* Table of precalculated normals */
GLfloat anorms_table[162][3] =
{
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


CMD2MODEL *MD2MODEL_create(void)
{
	return (CMD2MODEL*)GB.New(GB.FindClass("Md2Model"), NULL, NULL);
};

int MD2MODEL_draw(CMD2MODEL *_object, double frame, int texture, float *pos, float *scale, float *rotate)
{
	int i;
	int n, n2;
	double interp;
	GLfloat v_curr[3], v_next[3], v[3], norm[3];
	GLfloat *n_curr, *n_next;
	const framemd2 *pframe1, *pframe2;
	const vertexmd2 *pvert1, *pvert2;
	bool enabled;
	int nvert = 0;

	int *pglcmds;
	glcmd *packet;

	if (texture < 0)
		return 0;

	n = (int)frame;
	interp = frame - n;

	if (n < 0 || n >= THIS->num_frames)
		return 0;

	n2 = n + 1;
	if (n2 >= THIS->num_frames)
		n2 = 0;

	enabled = glIsEnabled(GL_TEXTURE_2D);
  if (!enabled)
		glEnable(GL_TEXTURE_2D);

	glPushMatrix();

	if (pos)
		glTranslatef(pos[0], pos[1], pos[2]);

	glRotatef(-90, 1, 0, 0);
	glRotatef(-90, 0, 0, 1);

	if (rotate && rotate[0] != 0)
		glRotatef(rotate[0], rotate[1], rotate[2], rotate[3]);

	glScalef(THIS->scale[0], THIS->scale[1], THIS->scale[2]);

	if (scale)
		glScalef(scale[0], scale[1], scale[2]);

	glBindTexture(GL_TEXTURE_2D, texture);

#if 1
	// pglcmds points at the start of the command list
	pglcmds = THIS->glcmds;

	pframe1 = &THIS->frames[n];
	pframe2 = &THIS->frames[n2];

	//fprintf(stderr, "\n******** %p: n = %d / %d | %p %p\n", _object, n, THIS->num_frames, pframe1->verts, pframe2->verts);

	// Draw the model
	while ((i = *(pglcmds++)) != 0)
	{
		//fprintf(stderr, "i = %d\n", i);

		if (i < 0)
		{
			glBegin(GL_TRIANGLE_FAN);
			i = -i;
		}
		else
		{
			glBegin(GL_TRIANGLE_STRIP);
		}

		// Draw each vertex of this group
		for (/* Nothing */ ; i > 0; --i, pglcmds += 3)
		{
			packet = (glcmd *)pglcmds;

			//fprintf(stderr, "%d (%d) ", i, packet->index);

			pvert1 = &pframe1->verts[packet->index];
			pvert2 = &pframe2->verts[packet->index];
			//if (!pvert1 || !pvert2)
			//	continue;

			// Pass texture coordinates to OpenGL
			//glTexCoord2f (pGLcmd->s, 1.0f - pGLcmd->t);
			glTexCoord2f(packet->s, packet->t);

			// Compute interpolated normal vector
			n_curr = anorms_table[pvert1->normalIndex];
			n_next = anorms_table[pvert2->normalIndex];

			norm[0] = n_curr[0] + interp * (n_next[0] - n_curr[0]);
			norm[1] = n_curr[1] + interp * (n_next[1] - n_curr[1]);
			norm[2] = n_curr[2] + interp * (n_next[2] - n_curr[2]);

			// Normal vector
			glNormal3fv(norm);

			v_curr[0] = pframe1->scale[0] * pvert1->v[0] + pframe1->translate[0];
			v_curr[1] = pframe1->scale[1] * pvert1->v[1] + pframe1->translate[1];
			v_curr[2] = pframe1->scale[2] * pvert1->v[2] + pframe1->translate[2];

			v_next[0] = pframe2->scale[0] * pvert2->v[0] + pframe2->translate[0];
			v_next[1] = pframe2->scale[1] * pvert2->v[1] + pframe2->translate[1];
			v_next[2] = pframe2->scale[2] * pvert2->v[2] + pframe2->translate[2];

			v[0] = v_curr[0] + interp * (v_next[0] - v_curr[0]);
			v[1] = v_curr[1] + interp * (v_next[1] - v_curr[1]);
			v[2] = v_curr[2] + interp * (v_next[2] - v_curr[2]);

			glVertex3fv(v);
			nvert++;
		}

		//fprintf(stderr, "\n");

		glEnd();
	}
#else
	{
		int j;
		GLfloat s, t;

		glBegin (GL_TRIANGLES);

		for (i = 0; i < THIS->num_tris; ++i)
		{
			for (j = 0; j < 3; ++j)
			{
				pframe1 = &THIS->frames[n];
				pframe2 = &THIS->frames[n2];
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
				nvert++;
			}
		}

		glEnd();
	}
#endif

	//glScalef(1/THIS->scale[0], 1/THIS->scale[1], 1/THIS->scale[2]);
	glPopMatrix();

	if (!enabled)
		glDisable(GL_TEXTURE_2D);

	return nvert;
}


//---------------------------------------------------------------------------

BEGIN_METHOD_VOID(Md2Model_new)

	THIS->texture = -1;

END_METHOD

BEGIN_METHOD_VOID(Md2Model_free)

	int i;

	GB.Free ((void *) &THIS->skins);
  GB.Free ((void *) &THIS->texcoords);
  GB.Free ((void *) &THIS->triangles);

	for (i = 0; i < THIS->num_frames; ++i)
		GB.Free ((void *) &THIS->frames[i].verts);

	GB.Free ((void *) &THIS->frames);
  GB.Free ((void *) &THIS->glcmds);

END_METHOD

/*
	This method creates and loads new Md2 model. To use this in the program you should declare
	it first as ie.
	Public model as Md2Model
	Then initiate it loading
	model = Md2Model.Load(name as string)
*/

BEGIN_METHOD(Md2Model_Load, GB_STRING name)

	char *addr;
	int len;
	CMD2MODEL *mdl = NULL;
	int i;

	if (GB.LoadFile(STRING(name), LENGTH(name), &addr, &len))
		return;

	mdl = MD2MODEL_create();

	// Read header
	{
		int *p = (int *)addr;

		mdl->ident = *p++;
		mdl->version = *p++;
		mdl->skinwidth = *p++;
		mdl->skinheight = *p++;
		mdl->framesize = *p++;
		mdl->num_skins = *p++;
		mdl->num_vertices = *p++;
		mdl->num_st = *p++;
		mdl->num_tris = *p++;
		mdl->num_glcmds = *p++;
		mdl->num_frames = *p++;
		mdl->offset_skins = *p++;
		mdl->offset_st = *p++;
		mdl->offset_tris = *p++;
		mdl->offset_frames = *p++;
		mdl->offset_glcmds = *p++;
		mdl->offset_end = *p++;
	}

	if ((mdl->ident != 844121161) || (mdl->version != 8))
	{
		// Error!
		GB.Error("Bad version or identifier");
		goto __ERROR;
	}

	// Memory allocations
	GB.Alloc ((void *) &mdl->skins, sizeof (skinmd2) * mdl->num_skins );
	GB.Alloc ((void *) &mdl->texcoords, sizeof (texCoordmd2) * mdl->num_st);
	GB.Alloc ((void *) &mdl->triangles, sizeof (trianglemd2) * mdl->num_tris);
	GB.Alloc ((void *) &mdl->frames, sizeof (framemd2) * mdl->num_frames);
	GB.Alloc ((void *) &mdl->glcmds, sizeof (int) * mdl->num_glcmds);

	// Read model data
	memcpy(mdl->skins, &addr[mdl->offset_skins], sizeof(skinmd2) * mdl->num_skins);
	memcpy(mdl->texcoords, &addr[mdl->offset_st], sizeof(texCoordmd2) * mdl->num_st);
	memcpy(mdl->triangles, &addr[mdl->offset_tris], sizeof(trianglemd2) * mdl->num_tris);
	memcpy(mdl->glcmds, &addr[mdl->offset_glcmds], sizeof(int) * mdl->num_glcmds);

	// Read frames
	{
		char *p = &addr[mdl->offset_frames];

		for (i = 0; i < mdl->num_frames; ++i)
		{
			// Memory allocation for vertices of this frame
			GB.Alloc((void *) &mdl->frames[i].verts, sizeof (vertexmd2) * mdl->num_vertices);

			// Read frame data
			memcpy(mdl->frames[i].scale, p, sizeof(float) * 3); p += sizeof(float) * 3;
			memcpy(mdl->frames[i].translate, p, sizeof(float) * 3); p += sizeof(float) * 3;
			memcpy(mdl->frames[i].name, p, 16); p += 16;
			memcpy(mdl->frames[i].verts, p, sizeof(vertexmd2) * mdl->num_vertices); p += sizeof(vertexmd2) * mdl->num_vertices;
		}
	}

	mdl->scale[0] = 1;
	mdl->scale[1] = 1;
	mdl->scale[2] = 1;

	GB.ReleaseFile(addr, len);
	GB.ReturnObject(mdl);
	return;

__ERROR:

	GB.ReleaseFile(addr, len);
	GB.Unref(POINTER(&mdl));

END_METHOD

BEGIN_METHOD(Md2Model_Scale, GB_FLOAT sx; GB_FLOAT sy; GB_FLOAT sz)

	THIS->scale[0] = VARG(sx);
	THIS->scale[1] = VARG(sy);
	THIS->scale[2] = VARG(sz);

END_METHOD

BEGIN_PROPERTY(Md2Model_Count)

	GB.ReturnInteger(THIS->num_frames);

END_PROPERTY

BEGIN_METHOD(Md2Model_get, GB_INTEGER frame)

	int frame = VARG(frame);

	if (frame < 0 || frame >= THIS->num_frames)
	{
		GB.Error(GB_ERR_BOUND);
		return;
	}

	THIS->frame = frame;
	RETURN_SELF();

END_METHOD

BEGIN_PROPERTY(Md2Model_Texture)

	if (READ_PROPERTY)
		GB.ReturnInteger(THIS->texture);
	else
		THIS->texture = VPROP(GB_INTEGER);

END_PROPERTY

//---------------------------------------------------------------------------

BEGIN_METHOD(Md2Model_Frame_Draw, GB_FLOAT interp; GB_INTEGER texture)

	MD2MODEL_draw(THIS, (double)THIS->frame + VARGOPT(interp, 0.0), VARGOPT(texture, THIS->texture), NULL, NULL, NULL);

END_METHOD

BEGIN_PROPERTY(Md2Model_Frame_Name)

	GB.ReturnNewZeroString(THIS->frames[THIS->frame].name);

END_METHOD

//---------------------------------------------------------------------------

GB_DESC Md2ModelFrameDesc[] =
{
	GB_DECLARE_VIRTUAL(".Md2Model.Frame"),

	GB_PROPERTY_READ("Name", "s", Md2Model_Frame_Name),
	GB_METHOD("Draw", NULL, Md2Model_Frame_Draw, "[(Interpolation)f(Texture)i]"),

	GB_END_DECLARE
};

GB_DESC Md2ModelDesc[] =
{
	GB_DECLARE("Md2Model", sizeof(CMD2MODEL)), 	GB_NOT_CREATABLE(),

	GB_METHOD("_new", NULL, Md2Model_new, NULL),
	GB_METHOD("_free", NULL, Md2Model_free, NULL),

	GB_STATIC_METHOD("Load", "Md2Model" , Md2Model_Load, "(Name)s"),

	GB_PROPERTY_READ("Count", "i", Md2Model_Count),
	GB_METHOD("_get", ".Md2Model.Frame", Md2Model_get, "(Frame)i"),

	GB_METHOD("Scale", NULL, Md2Model_Scale, "(SX)f(SY)f(SZ)f" ),

	GB_PROPERTY("Texture", "i", Md2Model_Texture),

	GB_END_DECLARE
};


