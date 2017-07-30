/***************************************************************************

  (c) 2012 Adrien Prokopowicz <prokopy@users.sourceforge.net>

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

#ifndef CXMLREADER_H
#define CXMLREADER_H

#include "gbinterface.h"

class Reader;

typedef struct CReader
{
    GB_BASE ob;
    Reader *reader;
} CReader;

extern GB_DESC CReaderDesc[];
extern GB_DESC CReaderNodeDesc[];
extern GB_DESC CReaderNodeTypeDesc[];
extern GB_DESC CReaderNodeAttributesDesc[];
extern GB_DESC CReaderReadFlagsDesc[];


#endif // CXMLREADER_H
