/***************************************************************************

  cmd2object.c

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

#define __CMD2OBJECT_C

#include "cmd2object.h"

#define THIS OBJECT(CMD2OBJECT)

//---------------------------------------------------------------------------

BEGIN_METHOD(Md2Object_new, GB_OBJECT model)

	CMD2MODEL *model = VARG(model);

	if (GB.CheckObject(model))
		return;

	THIS->model = model;
	GB.Ref(model);

	THIS->texture = -1;

	THIS->scale[0] = THIS->scale[1] = THIS->scale[2] = 1;

END_METHOD

BEGIN_METHOD_VOID(Md2Object_free)

	GB.Unref(POINTER(&THIS->model));

END_METHOD


BEGIN_METHOD(Md2Object_Move, GB_FLOAT x; GB_FLOAT y; GB_FLOAT z)

	THIS->pos[0] = VARG(x);
	THIS->pos[1] = VARG(y);
	THIS->pos[2] = VARG(z);
	
END_METHOD

BEGIN_PROPERTY(Md2Object_X)

	if (READ_PROPERTY)
		GB.ReturnFloat(THIS->pos[0]);
	else
		THIS->pos[0] = VPROP(GB_FLOAT);

END_PROPERTY

BEGIN_PROPERTY(Md2Object_Y)

	if (READ_PROPERTY)
		GB.ReturnFloat(THIS->pos[1]);
	else
		THIS->pos[1] = VPROP(GB_FLOAT);

END_PROPERTY

BEGIN_PROPERTY(Md2Object_Z)

	if (READ_PROPERTY)
		GB.ReturnFloat(THIS->pos[2]);
	else
		THIS->pos[2] = VPROP(GB_FLOAT);

END_PROPERTY

BEGIN_METHOD(Md2Object_Scale, GB_FLOAT sx; GB_FLOAT sy; GB_FLOAT sz)

	THIS->scale[0] = VARG(sx);
	THIS->scale[1] = VARG(sy);
	THIS->scale[2] = VARG(sz);

END_METHOD

BEGIN_METHOD(Md2Object_Rotate, GB_FLOAT angle; GB_FLOAT rx; GB_FLOAT ry; GB_FLOAT rz)

	THIS->rotate[0] = VARG(angle);
	THIS->rotate[1] = VARG(rx);
	THIS->rotate[2] = VARG(ry);
	THIS->rotate[3] = VARG(rz);

END_METHOD

BEGIN_PROPERTY(Md2Object_Texture)

	if (READ_PROPERTY)
		GB.ReturnInteger(THIS->texture);
	else
		THIS->texture = VPROP(GB_INTEGER);

END_PROPERTY

BEGIN_PROPERTY(Md2Object_Frame)

	if (READ_PROPERTY)
		GB.ReturnFloat(THIS->frame);
	else
		THIS->frame = VPROP(GB_FLOAT);

END_PROPERTY

BEGIN_METHOD_VOID(Md2Object_Draw)

	int texture = THIS->texture;

	if (texture < 0)
		texture = THIS->model->texture;

	GB.ReturnInteger(MD2MODEL_draw(THIS->model, THIS->frame, texture, THIS->pos, THIS->scale, THIS->rotate));

END_METHOD

BEGIN_PROPERTY(Md2Object_Model)

	GB.ReturnObject(THIS->model);

END_PROPERTY

BEGIN_PROPERTY(Md2Object_Count)

	GB.ReturnInteger(THIS->model->num_frames);

END_PROPERTY

//---------------------------------------------------------------------------

GB_DESC Md2ObjectDesc[] =
{
	GB_DECLARE("Md2Object", sizeof(CMD2OBJECT)),

	GB_METHOD("_new", NULL, Md2Object_new, "(Model)Md2Model;"),
	GB_METHOD("_free", NULL, Md2Object_free, NULL),

	GB_PROPERTY_READ("X", "f", Md2Object_X),
	GB_PROPERTY_READ("Y", "f", Md2Object_Y),
	GB_PROPERTY_READ("Z", "f", Md2Object_Z),

	GB_METHOD("Move", NULL, Md2Object_Move, "(X)f(Y)f(Z)f"),
	GB_METHOD("Scale", NULL, Md2Object_Scale, "(SX)f(SY)f(SZ)f"),
	GB_METHOD("Rotate", NULL, Md2Object_Rotate, "(Angle)f(RX)f(RY)f(RZ)f"),

	GB_PROPERTY("Texture", "i", Md2Object_Texture),
	GB_PROPERTY("Frame", "f", Md2Object_Frame),
	GB_PROPERTY_READ("Count", "i", Md2Object_Count),
	GB_METHOD("Draw", "i", Md2Object_Draw, NULL),

	GB_PROPERTY_READ("Model", "Md2Model", Md2Object_Model),

	GB_END_DECLARE
};


