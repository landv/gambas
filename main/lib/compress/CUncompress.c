/***************************************************************************

  CUncompress.c

  Compression Library - Decompression Class

  (c) 2003-2004 Daniel Campos Fernández <danielcampos@netcourrier.com>

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
#define __CUNCOMPRESS_C


#include "CUncompress.h"
#include "main.h"
#include <stdio.h>

#define Check_Driver()  if (!THIS->driver) { GB.Error("No driver specified"); return; }


//*************************************************************************
//#################### INITIALIZATION AND DESTRUCTION #####################
//*************************************************************************
/*************************************************
 Class "Constructor"
 *************************************************/
BEGIN_METHOD_VOID(CUNCOMPRESS_init)

	

END_METHOD
/*************************************************
 Class "Destructor"
 *************************************************/
BEGIN_METHOD_VOID(CUNCOMPRESS_exit)



END_METHOD
/*************************************************
 Gambas object "Constructor"
 *************************************************/
BEGIN_METHOD_VOID(CUNCOMPRESS_new)

	THIS->driver=NULL;
	THIS->stream.desc=NULL;

END_METHOD
/*************************************************
 Gambas object "Destructor"
 *************************************************/
BEGIN_METHOD_VOID(CUNCOMPRESS_free)

	if (THIS->stream.desc && THIS->driver) THIS->driver->Compress.Close(&THIS->stream);

END_METHOD

BEGIN_METHOD (CUNCOMPRESS_File,GB_STRING Source;GB_STRING Target;)

	Check_Driver();
	THIS->driver->Uncompress.File(STRING(Source),STRING(Target));

END_METHOD

BEGIN_METHOD (CUNCOPMPRESS_String,GB_STRING Source;)

	char *target=NULL;
	long lent=0;

	Check_Driver();
	if (!LENGTH(Source)) { GB.ReturnNewString(NULL,0); return; }
	
	THIS->driver->Uncompress.String(&target,&lent,STRING(Source),LENGTH(Source));
	
	if (!lent) GB.ReturnNewString(NULL,0);
	GB.ReturnNewString (target,lent);
	GB.Free((void**)&target);

END_METHOD

BEGIN_METHOD (CUNCOMPRESS_Open,GB_STRING Path;)

	
	Check_Driver();
	
	if (THIS->stream.desc) { GB.Error ("File is already opened"); return; }
	
	THIS->driver->Uncompress.Open(STRING(Path),&THIS->stream);

END_METHOD

BEGIN_PROPERTY (CUNCOMPRESS_Type)

	if (READ_PROPERTY)
	{
		if (!THIS->driver)
		{	
			GB.ReturnNewString(NULL,0);
			return;
		}
		GB.ReturnNewString(THIS->driver->name,0);
		return;
	}
	
	if (THIS->stream.desc) { GB.Error("Type can not be changed while the stream is opened"); return; }
	if (!PROP(GB_STRING)) { GB.Error("Invalid driver name"); return; }
	
	if (!(THIS->driver=COMPRESS_GetDriver(GB.ToZeroString(PROP(GB_STRING)))) )
	{
		GB.Error("Cannot find driver &1", GB.ToZeroString(PROP(GB_STRING)));
	}

END_PROPERTY

/*******************************************************************
 Interface declaration
 *******************************************************************/
GB_DESC CUncompressDesc[] =
{

  GB_DECLARE("Uncompress", sizeof(CUNCOMPRESS)),
  
  GB_INHERITS("Stream"),
  
  GB_PROPERTY("Type","s",CUNCOMPRESS_Type),

  GB_STATIC_METHOD("_init", NULL, CUNCOMPRESS_init, NULL),
  GB_STATIC_METHOD("_exit", NULL, CUNCOMPRESS_exit, NULL),
  GB_METHOD("_new", NULL, CUNCOMPRESS_new, NULL),
  GB_METHOD("_free", NULL, CUNCOMPRESS_free, NULL),
  
  GB_METHOD("String","s",CUNCOPMPRESS_String,"(Source)s"),
  GB_METHOD("File",NULL,CUNCOMPRESS_File,"(Source)s(Target)s"),
  GB_METHOD("Open",NULL,CUNCOMPRESS_Open,"(Path)s"),
    
  GB_END_DECLARE
};

