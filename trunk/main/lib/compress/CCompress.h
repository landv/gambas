/***************************************************************************

  CCompress.h

  (c) 2003-2004 Daniel Campos Fernández <danielcampos@netcourrier.com>

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
