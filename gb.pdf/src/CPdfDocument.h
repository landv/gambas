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
  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
  MA 02110-1301, USA.

***************************************************************************/

#ifndef __CPDFDOCUMENT_H
#define __CPDFDOCUMENT_H

#include "gambas.h"

#include <PDFDoc.h>
#include <SplashOutputDev.h>
#include <Page.h>
#if POPPLER_VERSION_0_76
#include <vector>
#include <Outline.h>
#else
#include <goo/GooList.h>
#endif
#include <stdint.h>

#if POPPLER_VERSION_0_76
#define const_LinkAction const LinkAction
#define const_LinkDest const LinkDest
#define const_GooList const std::vector<OutlineItem*>
#define GooList std::vector<OutlineItem*>
#define const_GooString const GooString
#elif POPPLER_VERSION_0_64
#define const_LinkAction const LinkAction
#define const_LinkDest const LinkDest
#define const_GooList const GooList
#define const_GooString const GooString
#else
#define const_LinkAction LinkAction
#define const_LinkDest LinkDest
#define const_GooList GooList
#define const_GooString GooString
#endif

#ifndef __CPDFDOCUMENT_C

extern GB_DESC PdfRectDesc[];
extern GB_DESC PdfDocumentDesc[];
extern GB_DESC PdfPageDesc[];
extern GB_DESC PdfResultDesc[];
extern GB_DESC PdfLinkDesc[];
extern GB_DESC PdfLinkDataDesc[];
extern GB_DESC PdfIndexDesc[];
extern GB_DESC PdfDocumentInfo[];
extern GB_DESC PdfLayoutDesc[];
extern GB_DESC PdfModeDesc[];

#else

#define THIS ((CPDFDOCUMENT *)_object)
#define THIS_RECT ((CPDFRECT *)_object)

#endif

#if POPPLER_VERSION_0_76

#define CPDF_list_get(_list, _i) ((_list)->at(_i))
#define CPDF_list_count(_list) ((_list)->size())

#else

#define CPDF_list_get(_list, _i) ((OutlineItem *)(_list)->get(_i))
#define CPDF_list_count(_list) ((_list)->getLength())

#endif

#define CPDF_index_get(_i) CPDF_list_get(THIS->index, _i)
#define CPDF_index_count() CPDF_list_count(THIS->index)


typedef
	struct {
		GB_BASE ob;
		double x, y, w, h;
	}
	CPDFRECT;

typedef 
	struct {
		double x0;
		double y0;
		double x1;
		double y1;
	} 
	CPDFFIND;

typedef
	struct {
		GB_BASE ob;

		char *buf;
		int len;

		PDFDoc *doc;
		SplashOutputDev *dev;
		Page *page;
		uint currpage;

		void **pindex;            // Parent of current index entries
		const_GooList *index;     // Current entries
		
		uint currindex;           // Current entry
		uint *oldindex;           // Parent entry

		Links *links;             // Page bookmarks
		uint lcurrent;            // Current bookmark

		CPDFFIND *Found;          // Found text elements

		const_LinkAction *action; // Current link action

		double scale;
		int rotation;
	}
	CPDFDOCUMENT;



#endif
