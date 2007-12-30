/***************************************************************************

  CPictureBox.cpp

  (c) 2004-2005 - Daniel Campos Fernández <danielcampos@netcourrier.com>
  
  GTK+ component
  
  Realizado para la Junta de Extremadura. 
  Consejería de Educación Ciencia y Tecnología. 
  Proyecto gnuLinEx
  
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

	CCONTAINER *Parent=(CCONTAINER*)VPROP(GB_OBJECT);
	Parent=(CCONTAINER*)GetContainer ((CWIDGET*)Parent);

	THIS->widget=new gPictureBox(Parent->widget);
	InitControl(THIS->widget,(CWIDGET*)THIS);

END_METHOD

BEGIN_METHOD_VOID(CPICTUREBOX_free)

	if (THIS->picture) { GB.Unref((void**)&THIS->picture); THIS->picture=NULL; }

END_METHOD

BEGIN_PROPERTY(CPICTUREBOX_picture)

	CPICTURE *pic=NULL;
	gPicture *buf;
	
	if (READ_PROPERTY)
	{
		GB.ReturnObject(THIS->picture);
		return;
	}
	
	pic=(CPICTURE*)VPROP(GB_OBJECT);
	if (pic) GB.Ref((void*)pic);
	if (THIS->picture) GB.Unref((void**)&THIS->picture);
	THIS->picture=pic;

	if (!pic) PBOX->setPicture(NULL); 
	else      PBOX->setPicture(pic->picture);

END_PROPERTY


BEGIN_PROPERTY(CPICTUREBOX_stretch)

	if (READ_PROPERTY) { GB.ReturnBoolean(PBOX->stretch()); return; }
	PBOX->setStretch(VPROP(GB_INTEGER));

END_PROPERTY



BEGIN_PROPERTY(CPICTUREBOX_alignment)

	if (READ_PROPERTY) { GB.ReturnInteger(PBOX->alignment()); return; }
	PBOX->setAlignment(VPROP(GB_INTEGER));


END_PROPERTY

BEGIN_PROPERTY(CPICTUREBOX_border)

	if (READ_PROPERTY) { GB.ReturnInteger(PBOX->getBorder()); return; }
	PBOX->setBorder(VPROP(GB_INTEGER));

END_PROPERTY

/**********************************************************************************


MovieBox


***********************************************************************************/
BEGIN_METHOD(CMOVIEBOX_new, GB_OBJECT parent)

	CCONTAINER *Parent=(CCONTAINER*)VPROP(GB_OBJECT);
	Parent=(CCONTAINER*)GetContainer ((CWIDGET*)Parent);

	MTHIS->widget=new gMovieBox(Parent->widget);
	InitControl(MTHIS->widget,(CWIDGET*)MTHIS);

END_METHOD

BEGIN_METHOD_VOID(CMOVIEBOX_free)

	if (MTHIS->path) GB.Free((void**)&MTHIS->path);

END_METHOD


BEGIN_PROPERTY(CMOVIEBOX_border)

	if (READ_PROPERTY) { GB.ReturnInteger(MBOX->getBorder()); return; }
	MBOX->setBorder(VPROP(GB_INTEGER));

END_PROPERTY


BEGIN_PROPERTY(CMOVIEBOX_path)

	char *addr;
	char *name;
	long len;
	
	if (READ_PROPERTY)
	{
		GB.ReturnNewString(MTHIS->path,0);
		return;
	}
	
	name=GB.ToZeroString(PROP(GB_STRING));
	if (GB.LoadFile (name,strlen(name),&addr,&len)) 
	{
		GB.Error("File or directory does not exist");
		return;
	}
	
	if (MTHIS->path) { GB.Free((void**)&MTHIS->path); MTHIS->path=NULL; }
	GB.Alloc((void**)&MTHIS->path,strlen(name)+1);
	strcpy(MTHIS->path,name);
	MBOX->loadMovie(addr,len);
	GB.ReleaseFile (&addr,len);

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


GB_DESC CPictureBoxDesc[] =
{
  GB_DECLARE("PictureBox", sizeof(CPICTUREBOX)), GB_INHERITS("Control"),

  GB_STATIC_METHOD("_init",NULL,CPICTUREBOX_init,NULL),
  GB_METHOD("_new", NULL, CPICTUREBOX_new, "(Parent)Container;"),
  GB_METHOD("_free", NULL, CPICTUREBOX_free,NULL),
  
  GB_PROPERTY("Picture", "Picture", CPICTUREBOX_picture),
  GB_PROPERTY("Stretch", "b", CPICTUREBOX_stretch),

  GB_PROPERTY("Border", "i<Border>", CPICTUREBOX_border),
  GB_PROPERTY("Alignment", "i<Align>", CPICTUREBOX_alignment),

  GB_CONSTANT("_Properties", "s", CPICTUREBOX_PROPERTIES),

  GB_END_DECLARE
};

GB_DESC CMovieBoxDesc[] =
{
  GB_DECLARE("MovieBox", sizeof(CMOVIEBOX)), GB_INHERITS("Control"),

  GB_METHOD("_new", NULL, CMOVIEBOX_new, "(Parent)Container;"),
  GB_METHOD("_free", NULL, CMOVIEBOX_free,NULL),
  GB_METHOD("Rewind", NULL, CMOVIEBOX_rewind, NULL),

  GB_PROPERTY("Path", "s", CMOVIEBOX_path),
  GB_PROPERTY("Playing", "b", CMOVIEBOX_playing),
  GB_PROPERTY("Border", "i<Border>", CMOVIEBOX_border),

  GB_CONSTANT("_Properties", "s", CMOVIEBOX_PROPERTIES),

  GB_END_DECLARE
};



