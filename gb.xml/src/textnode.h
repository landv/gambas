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

#ifndef TEXTNODE_H
#define TEXTNODE_H

#include "main.h"

void XMLText_unEscapeContent(const char *src, const size_t lenSrc, char *&dst, size_t &lenDst);
void XMLText_escapeContent(const char *src, const size_t lenSrc, char *&dst, size_t &lenDst);
void XMLText_escapeAttributeContent(const char *src, const size_t lenSrc, char *&dst, size_t &lenDst);

TextNode* XMLTextNode_New();
TextNode* XMLTextNode_New(const char *ncontent, const size_t nlen);
void XMLTextNode_Free(TextNode *node);

void XMLTextNode_checkEscapedContent(TextNode *node);
void XMLTextNode_checkContent(TextNode *node);
void XMLTextNode_setEscapedTextContent(TextNode *node, const char *ncontent, const size_t nlen);

void XMLTextNode_TrimContent(TextNode *node);


CommentNode* XMLComment_New();
CommentNode* XMLComment_New(const char *ncontent, const size_t nlen);

CDATANode* XMLCDATA_New();
CDATANode* XMLCDATA_New(const char *ncontent, const size_t nlen);

#endif // TEXTNODE_H
