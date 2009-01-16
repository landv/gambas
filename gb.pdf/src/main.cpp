/***************************************************************************

  main.cpp

  gb.pdf Component

  (C) 2005 Daniel Campos Fernández <danielcampos@netcourrier.com>


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

#define __MAIN_C

#include "CPdfDocument.h"
#include "main.h"

#include <stdio.h>

#include <GlobalParams.h>

extern "C" {

GB_INTERFACE GB EXPORT;
IMAGE_INTERFACE IMAGE EXPORT;

GB_DESC *GB_CLASSES[] EXPORT =
{
  PdfDocumentDesc,
  PdfPageDesc,
  PdfResultDesc,
  PdfResultItemDesc,
  PdfIndexDesc,
  PdfLinkDesc,
  PdfLinkDataDesc,
  PdfDocumentInfo,
  PdfLayoutDesc,
  PdfModeDesc,
  NULL
};



int EXPORT GB_INIT(void)
{
	if (!globalParams)
	{
#if POPPLER_VERSION_0_6
		globalParams = new GlobalParams();
#else
		globalParams = new GlobalParams("/etc/xpdfrc");
#endif

#if POPPLER_VERSION_0_5
#else
		globalParams->setupBaseFontsFc(NULL);
#endif
	}

	GB.GetInterface("gb.image", IMAGE_INTERFACE_VERSION, &IMAGE);
	
	return 0;
}



void EXPORT GB_EXIT()
{

}


}


