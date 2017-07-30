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

#ifndef ELEMENT_H
#define ELEMENT_H

#include "gb.xml.h"


Attribute* XMLAttribute_New();
Attribute* XMLAttribute_New(const char *nattrName, const size_t nlenAttrName);
Attribute* XMLAttribute_New(const char *nattrName, const size_t nlenAttrName,
                            const char *nattrVal, const size_t nlenAttrVal);

void XMLAttribute_Free(Attribute *attr);

void XMLAttribute_SetName(Attribute *attr, const char *nattrName, const size_t nlenAttrName);
void XMLAttribute_SetValue(Attribute *attr, const char *nattrVal, const size_t nlenAttrVal);

Element* XMLElement_New();
Element* XMLElement_New(const char *ntagName, size_t nlenTagName);
void XMLElement_Free(Element *elmt);

void XMLElement_SetTagName(Element *elmt, const char *ntagName, size_t nlenTagName);
void XMLElement_SetPrefix(Element *elmt, const char *nprefix, size_t nlenPrefix);
void XMLElement_RefreshPrefix(Element *elmt);

Attribute* XMLElement_AddAttribute(Element *elmt, const char *nattrName, const size_t nlenAttrName);
Attribute* XMLElement_AddAttribute(Element *elmt, const char *nattrName, const size_t nlenAttrName,
                             const char *nattrVal, const size_t nlenAttrVal);

Attribute* XMLElement_GetAttribute(const Element *elmt, const char *nattrName, const size_t nlenAttrName, const int mode = GB_STRCOMP_BINARY);

void XMLElement_SetAttribute(Element *elmt, const char *nattrName, const size_t nlenAttrName,
                             const char *nattrVal, const size_t nlenAttrVal);

bool XMLElement_AttributeContains(const Element *elmt, const char *attrName, size_t lenAttrName, const char *value, size_t lenValue);

void XMLElement_RemoveAttribute(Element *elmt, const char *attrName, size_t lenAttrName);
void XMLElement_RemoveAttribute(Element *elmt, Attribute *attr);

void XMLElement_SetTextContent(Element *elmt, const char *content, size_t lenContent);


#endif // ELEMENT_H
