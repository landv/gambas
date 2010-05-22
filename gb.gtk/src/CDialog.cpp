/***************************************************************************

  CDialog.cpp

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
  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

***************************************************************************/

#define __CDIALOG_CPP

#include "CDialog.h"
#include "CFont.h"

static GB_ARRAY dialog_filter = NULL;


BEGIN_METHOD_VOID(CDIALOG_exit)

  GB.StoreObject(NULL, POINTER(&dialog_filter));

END_METHOD


BEGIN_PROPERTY(CDIALOG_title)

	if (READ_PROPERTY)
		GB.ReturnNewZeroString(gDialog::title());
	else
		gDialog::setTitle(GB.ToZeroString(PROP(GB_STRING)));

END_PROPERTY


BEGIN_PROPERTY(CDIALOG_filter)

  char **filters;
  char *filter;
  int i;

  if (READ_PROPERTY)
    GB.ReturnObject(dialog_filter);
  else
  {
    GB.StoreObject(PROP(GB_OBJECT), POINTER(&dialog_filter));
    GB.NewArray(&filters, sizeof(char *), 0);
    if (dialog_filter)
    {
			for (i = 0; i < (GB.Array.Count(dialog_filter) - 1); i += 2)
			{
				filter = *((char **)GB.Array.Get(dialog_filter, i));
				if (filter && !strcmp(filter, "*"))
					continue;
				*((char **)GB.Add(&filters)) = filter;
				filter = *((char **)GB.Array.Get(dialog_filter, i + 1));
				*((char **)GB.Add(&filters)) = filter;
			}
		}
    
    *((char **)GB.Add(&filters)) = (char *)"*";
    *((char **)GB.Add(&filters)) = GB.Translate("All Files");
      
    gDialog::setFilter(filters, GB.Count(filters));
    GB.FreeArray(&filters);
  }

END_PROPERTY

#if 0
BEGIN_PROPERTY(CDIALOG_filter)

	GB_ARRAY Array=NULL;
	char **buf=NULL;
	char *ctmp;
	long count=0;
	long bucle;
	long tmp=0;
	
	if (READ_PROPERTY)
	{
		buf=gDialog::filter(&count);
		if (buf)
		{
			GB.Array.New(&Array,GB_T_STRING,count);
			for (bucle=0;bucle<count;bucle++)
			{
				ctmp=NULL;
				GB.NewString(&ctmp,buf[bucle],strlen(buf[bucle]));
				*((char **)GB.Array.Get(Array,bucle)) = ctmp;
			}
			GB.ReturnObject(Array);
		}
		return;
	}
	
	if (!VPROP(GB_OBJECT))
	{
		gDialog::setFilter(NULL,0);
		return;
	}
	
	GB.StoreObject(PROP(GB_OBJECT),(void**)&Array);
	count=GB.Array.Count(Array);
	for (bucle=0;bucle<count;bucle++)
	{
		ctmp=*((char **)GB.Array.Get(Array,bucle));
		if (ctmp)
			if (strlen(ctmp))
				tmp++;
	}
	
	if (tmp)
	{
		GB.Alloc((void**)&buf,sizeof(char*)*tmp);
		tmp=0;
		for (bucle=0;bucle<count;bucle++)
		{
			ctmp=*((char **)GB.Array.Get(Array,bucle));
			if (ctmp)
				if (strlen(ctmp))
				{
					GB.Alloc((void**)&buf[tmp],sizeof(char)*( strlen(ctmp)+1 ));
					strcpy(buf[tmp],ctmp);
					tmp++;
				}
		}
		
	}

	gDialog::setFilter(buf,tmp);
	
	if (buf)
	{
		for (bucle=0;bucle<tmp;bucle++) GB.Free((void**)&buf[bucle]);
		GB.Free((void**)&buf);
	}
	

END_PROPERTY
#endif

BEGIN_PROPERTY(CDIALOG_paths)

	GB_ARRAY Array=NULL;
	char **buf=NULL;
	char *ctmp;
	long b=0;
	
	buf=gDialog::paths();
	if (buf)
	{
		while (buf[b++]);
		GB.Array.New(&Array,GB_T_STRING,b-1);
		
		b=0;
		while (buf[b])
		{
			ctmp=NULL;
			GB.NewZeroString(&ctmp,buf[b]);
			*((char **)GB.Array.Get(Array,b++)) = ctmp;
		}
		GB.ReturnObject(Array);
	}
	
END_PROPERTY

BEGIN_PROPERTY(CDIALOG_path)

	if (READ_PROPERTY) { GB.ReturnNewString( gDialog::path(),0); return; }
	gDialog::setPath(GB.ToZeroString(PROP(GB_STRING)));

END_PROPERTY


BEGIN_PROPERTY(CDIALOG_font)

	CFONT *ft;
	
	if (READ_PROPERTY)
	{
    gFont *font = gDialog::font();
    if (font)
    	ft = CFONT_create(font->copy());
    else
    	ft = NULL;
    	
    GB.ReturnObject(ft);
		return;
	}
	
	ft=(CFONT*)VPROP(GB_OBJECT);
	if (ft && ft->font)
		gDialog::setFont(ft->font);

END_PROPERTY


BEGIN_PROPERTY(CDIALOG_color)

	if (READ_PROPERTY) { GB.ReturnInteger( gDialog::color() ); return; }
	gDialog::setColor(VPROP(GB_INTEGER));

END_PROPERTY


BEGIN_METHOD(CDIALOG_open_file,GB_BOOLEAN Multi;)

	bool Multi=false;
	
	if (!MISSING(Multi)) Multi=VARG(Multi);

 	GB.ReturnBoolean( gDialog::openFile(Multi) );

END_METHOD


BEGIN_METHOD_VOID(CDIALOG_save_file)

 	GB.ReturnBoolean( gDialog::saveFile() );

END_METHOD


BEGIN_METHOD_VOID(CDIALOG_get_directory)

 	GB.ReturnBoolean( gDialog::selectFolder() );

END_METHOD


BEGIN_METHOD_VOID(CDIALOG_get_color)

	GB.ReturnBoolean ( gDialog::selectColor() );

END_METHOD


BEGIN_METHOD_VOID(CDIALOG_select_font)

	GB.ReturnBoolean ( gDialog::selectFont() );

END_METHOD


GB_DESC CDialogDesc[] =
{
  GB_DECLARE("Dialog", 0), GB_VIRTUAL_CLASS(),

  GB_STATIC_METHOD("_exit", 0, CDIALOG_exit, 0),

  GB_STATIC_METHOD("OpenFile", "b", CDIALOG_open_file, "[(Multi)b]"),
  GB_STATIC_METHOD("SaveFile", "b", CDIALOG_save_file, 0),
  GB_STATIC_METHOD("SelectDirectory", "b", CDIALOG_get_directory, 0),
  GB_STATIC_METHOD("SelectColor", "b", CDIALOG_get_color, 0),
  GB_STATIC_METHOD("SelectFont", "b", CDIALOG_select_font, 0),

  GB_STATIC_PROPERTY_READ("Paths", "String[]", CDIALOG_paths),
  
  GB_STATIC_PROPERTY("Title", "s", CDIALOG_title),
  GB_STATIC_PROPERTY("Path", "s", CDIALOG_path),
  GB_STATIC_PROPERTY("Filter", "String[]", CDIALOG_filter),
  GB_STATIC_PROPERTY("Color", "i", CDIALOG_color),
  GB_STATIC_PROPERTY("Font", "Font", CDIALOG_font),
  
  GB_END_DECLARE
};


