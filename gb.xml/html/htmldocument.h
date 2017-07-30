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

#ifndef HDOCUMENT_H
#define HDOCUMENT_H

#include "htmlmain.h"

Document* HtmlDocument_New();
Document* HtmlDocument_NewFromFile(const char *fileName, const size_t lenFileName);
Element* HtmlDocument_GetBody(Document *doc);
Element* HtmlDocument_GetHead(Document *doc);

Element* HtmlDocument_GetElementById(Document *doc, const char *id, const size_t lenId, int depth = -1);
void HtmlDocument_GetElementsByClassName(Document *doc, const char *className, const size_t lenClassName, GB_ARRAY *array, int depth = -1);

Element* HtmlDocument_GetTitle(Document *doc);
Attribute* HtmlDocument_GetFavicon(Document *doc);
Attribute* HtmlDocument_GetBase(Document *doc);
Attribute* HtmlDocument_GetLang(Document *doc);

void HtmlDocument_SetHTML(Document *doc, const bool isHtml);

void HtmlDocument_AddStyleSheet(Document *doc, const char *src, size_t lenSrc,
                                 const char *media, size_t lenMedia);

void HtmlDocument_AddStyleSheetIfIE(Document *doc, const char *src, size_t lenSrc,
                                     const char *cond, size_t lenCond,
                                     const char *media, size_t lenMedia);

void HtmlDocument_AddStyleSheetIfNotIE(Document *doc, const char *src, size_t lenSrc,
                                        const char *media, size_t lenMedia);

void HtmlDocument_AddScript(Document *doc, const char *src, size_t lenSrc);


void HtmlDocument_AddScriptIfIE(Document *doc, const char *src, size_t lenSrc,
                                 const char *cond, size_t lenCond);

void HtmlDocument_AddScriptIfNotIE(Document *doc, const char *src, size_t lenSrc);


#endif
