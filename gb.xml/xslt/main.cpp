/***************************************************************************

  main.c

  (c) 2004 Daniel Campos Fernández <danielcampos@netcourrier.com>
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

#include "CXSLT.h"
#include "../gb.xml.h"

GB_INTERFACE GB EXPORT;
XML_INTERFACE XML EXPORT;

extern "C"
{
    GB_DESC *GB_CLASSES[] EXPORT =
    {
        CXsltDesc ,0
    };

    int EXPORT GB_INIT(void)
    {
        GB.GetInterface("gb.xml", XML_INTERFACE_VERSION, &XML);
        return -1;
    }

    void EXPORT GB_EXIT()
    {

    }
}
