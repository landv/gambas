/***************************************************************************

  CCompress.h

  Compression Library - Compression Class

  (c) 2003-2004 Daniel Campos Fern�ndez <danielcampos@netcourrier.com>

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
#ifndef __CCOMPRESS_H
#define __CCOMPRESS_H

#include "gambas.h"
#include "gb.compress.h"

#ifndef __CCOMPRESS_C


extern GB_DESC CCompressDesc[];
extern GB_STREAM_DESC CCompressStream;

#else

#define THIS ((CCOMPRESS *)_object)

#endif
typedef  struct
{
    GB_BASE ob;
    GB_STREAM stream;
    COMPRESS_DRIVER *driver;
    
    
}  CCOMPRESS;


#endif
