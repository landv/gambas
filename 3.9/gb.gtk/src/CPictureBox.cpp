/***************************************************************************

  CPictureBox.cpp

  (c) 2004-2006 - Daniel Campos Fernández <dcamposf@gmail.com>

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

#define __CPICTUREBOX_CPP


#include "main.h"
#include "gambas.h"
#include "widgets.h"


#include "CPictureBox.h"
#include "CPicture.h"
#include "CContainer.h"

#include <string.h>

/**********************************************************************************


PictureBox


***********************************************************************************/
GB_CLASS CPICTURE_Class;

BEGIN_METHOD_VOID (CPICTUREBOX_init)

	CPICTURE_Class=GB.FindClass("Picture");

END_METHOD


BEGIN_METHOD(CPICTUREBOX_new, GB_OBJECT parent)

	InitControl(new gPictureBox(CONTAINER(VARG(parent))),(CWIDGET*)THIS);
	
END_METHOD

BEGIN_METHOD_VOID(CPICTUREBOX_free)

	if (THIS->picture) { GB.Unref(POINTER(&THIS->picture)); THIS->picture=NULL; }

END_METHOD

BEGIN_PROPERTY(CPICTUREBOX_picture)

	CPICTURE *pic=NULL;
	
	if (READ_PROPERTY)
	{
		GB.ReturnObject(THIS->picture);
		return;
	}
	
	pic=(CPICTURE*)VPROP(GB_OBJECT);
	if (pic) GB.Ref((void*)pic);
	if (THIS->picture) GB.Unref(POINTER(&THIS->picture));
	THIS->picture=pic;

	if (!pic) PBOX->setPicture(NULL); 
	else      PBOX->setPicture(pic->picture);

END_PROPERTY


BEGIN_PROPERTY(CPICTUREBOX_stretch)

	if (READ_PROPERTY)
		GB.ReturnBoolean(PBOX->stretch());
	else
		PBOX->setStretch(VPROP(GB_BOOLEAN));

END_PROPERTY

BEGIN_PROPERTY(CPICTUREBOX_auto_resize)

	if (READ_PROPERTY)
		GB.ReturnBoolean(PBOX->isAutoResize());
	else
		PBOX->setAutoResize(VPROP(GB_BOOLEAN));

END_PROPERTY

BEGIN_PROPERTY(CPICTUREBOX_alignment)

	if (READ_PROPERTY)
		GB.ReturnInteger(PBOX->alignment());
	else
		PBOX->setAlignment(VPROP(GB_INTEGER));

END_PROPERTY

BEGIN_PROPERTY(CPICTUREBOX_border)

	if (READ_PROPERTY) { GB.ReturnInteger(PBOX->getBorder()); return; }
	PBOX->setBorder(VPROP(GB_INTEGER));

END_PROPERTY

BEGIN_PROPERTY(PictureBox_Padding)

	if (READ_PROPERTY)
		GB.ReturnInteger(PBOX->padding());
	else
		PBOX->setPadding(VPROP(GB_INTEGER));

END_PROPERTY

/**********************************************************************************


MovieBox


***********************************************************************************/

BEGIN_METHOD(CMOVIEBOX_new, GB_OBJECT parent)

	InitControl(new gMovieBox(CONTAINER(VARG(parent))),(CWIDGET*)MTHIS);
	
END_METHOD

BEGIN_METHOD_VOID(CMOVIEBOX_free)

	if (MTHIS->path) GB.Free(POINTER(&MTHIS->path));

END_METHOD


BEGIN_PROPERTY(CMOVIEBOX_border)

	if (READ_PROPERTY) { GB.ReturnInteger(MBOX->getBorder()); return; }
	MBOX->setBorder(VPROP(GB_INTEGER));

END_PROPERTY


BEGIN_PROPERTY(CMOVIEBOX_path)

	char *addr;
	char *name;
	int len;
	
	if (READ_PROPERTY)
	{
		GB.ReturnNewZeroString(MTHIS->path);
		return;
	}
	
	name=GB.ToZeroString(PROP(GB_STRING));
	if (GB.LoadFile (name,strlen(name),&addr,&len)) 
	{
		GB.Error("File or directory does not exist");
		return;
	}
	
	if (MTHIS->path) { GB.Free(POINTER(&MTHIS->path)); MTHIS->path=NULL; }
	GB.Alloc(POINTER(&MTHIS->path),strlen(name)+1);
	strcpy(MTHIS->path,name);
	MBOX->loadMovie(addr,len);
	GB.ReleaseFile(addr,len);

END_PROPERTY


BEGIN_PROPERTY(CMOVIEBOX_playing)

	if (READ_PROPERTY) { GB.ReturnBoolean(MBOX->playing()); return; }
	MBOX->setPlaying(VPROP(GB_BOOLEAN));

END_PROPERTY


BEGIN_METHOD_VOID(CMOVIEBOX_rewind)

	if (MBOX->playing())
	{
		MBOX->setPlaying(false);
		MBOX->setPlaying(true);
	}

END_METHOD

BEGIN_PROPERTY(MovieBox_Alignment)

	if (READ_PROPERTY)
		GB.ReturnInteger(MBOX->alignment());
	else
		MBOX->setAlignment(VPROP(GB_INTEGER));

END_PROPERTY



GB_DESC CPictureBoxDesc[] =
{
  GB_DECLARE("PictureBox", sizeof(CPICTUREBOX)), GB_INHERITS("Control"),

  GB_STATIC_METHOD("_init",0,CPICTUREBOX_init,0),
  GB_METHOD("_new", 0, CPICTUREBOX_new, "(Parent)Container;"),
  GB_METHOD("_free", 0, CPICTUREBOX_free,0),
  
  GB_PROPERTY("Picture", "Picture", CPICTUREBOX_picture),
  GB_PROPERTY("Stretch", "b", CPICTUREBOX_stretch),
  GB_PROPERTY("AutoResize", "b", CPICTUREBOX_auto_resize),
  GB_PROPERTY("Padding", "i", PictureBox_Padding),

  GB_PROPERTY("Border", "i", CPICTUREBOX_border),
  GB_PROPERTY("Alignment", "i", CPICTUREBOX_alignment),

	PICTUREBOX_DESCRIPTION,

  GB_END_DECLARE
};

GB_DESC CMovieBoxDesc[] =
{
  GB_DECLARE("MovieBox", sizeof(CMOVIEBOX)), GB_INHERITS("Control"),

  GB_METHOD("_new", 0, CMOVIEBOX_new, "(Parent)Container;"),
  GB_METHOD("_free", 0, CMOVIEBOX_free,0),
  GB_METHOD("Rewind", 0, CMOVIEBOX_rewind, 0),

  GB_PROPERTY("Path", "s", CMOVIEBOX_path),
  GB_PROPERTY("Playing", "b", CMOVIEBOX_playing),
  GB_PROPERTY("Border", "i", CMOVIEBOX_border),
  GB_PROPERTY("Alignment", "i", MovieBox_Alignment),

	MOVIEBOX_DESCRIPTION,

  GB_END_DECLARE
};



