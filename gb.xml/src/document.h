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

#ifndef DOCUMENT_H
#define DOCUMENT_H

#include "main.h"
#include "utils.h"

Document* XMLDocument_New();
Document* XMLDocument_NewFromFile(const char *fileName, const size_t lenFileName, const DocumentType docType = XMLDocumentType);
void XMLDocument_Release(Document *doc);

void XMLDocument_Open(Document *doc, const char *fileName, const size_t lenFileName) throw(XMLParseException);
void XMLDocument_SetContent(Document *doc, const char *content, size_t len) throw(XMLParseException);
void XMLDocument_Save(Document *doc, const char *fileName, bool indent = false);

void XMLDocument_SetRoot(Document *doc, Element *newRoot);

#endif // DOCUMENT_H
