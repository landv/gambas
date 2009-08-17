/***************************************************************************

  CPdfDocument.h

  (C) 2005-2007 Daniel Campos Fernández <dcamposf@gmail.com>

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
#ifndef __CPDFDOCUMENT_H
#define __CPDFDOCUMENT_H

#include "gambas.h"

#include <PDFDoc.h>
#include <SplashOutputDev.h>
#include <Page.h>
#include <goo/GooList.h>
#include <stdint.h>

#ifndef __CPDFDOCUMENT_C

extern GB_DESC PdfDocumentDesc[];
extern GB_DESC PdfPageDesc[];
extern GB_DESC PdfResultDesc[];
extern GB_DESC PdfResultItemDesc[];
extern GB_DESC PdfLinkDesc[];
extern GB_DESC PdfLinkDataDesc[];
extern GB_DESC PdfIndexDesc[];
extern GB_DESC PdfDocumentInfo[];
extern GB_DESC PdfLayoutDesc[];
extern GB_DESC PdfModeDesc[];

#else

#define THIS ((CPDFDOCUMENT *)_object)

#endif


typedef	struct
{
	GB_BASE ob;

	char *buf;
	int32_t len;

	PDFDoc *doc;
	SplashOutputDev *dev;
	Page *page;
	uint32_t currpage;
		
	void **pindex;           // Parent of current index entries
	GooList *index;          // Current entries
	uint32_t currindex; // Current entry
 	uint32_t *oldindex; // Parent entry

	Links *links;            // Page bookmarks
	uint32_t lcurrent;  // Current bookmark

	void *Found;             // Found text elements
	uint32_t fcurrent;  // Current found text

	LinkAction *action;      // Current link action 

	double scale;
	int32_t rotation;

} CPDFDOCUMENT;



#endif
