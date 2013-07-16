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

#ifndef HELEMENT_H
#define HELEMENT_H

#include "htmlmain.h"

Attribute* HTMLElement_GetClassName(const Element *elmt);
bool HTMLElement_HasClassName(const Element *elmt, const char *className, const size_t lenClassName);
void HTMLElement_SetClassName(Element *elmt, const char *className, const size_t lenClassName);

Attribute* HTMLElement_GetId(const Element *elmt);
void HTMLElement_SetId(Element *elmt, const char* value, size_t len);

void HTMLElement_GetGBChildrenByFilter(Element *elmt, char *filter, size_t lenFilter, GB_ARRAY *array, int depth = -1);

Element* HTMLElement_GetChildById(Element *elmt, char *id, size_t lenId, int depth = -1);
void HTMLElement_GetGBChildrenByClassName(Element *elmt, char* className, size_t lenClassName, GB_ARRAY *array, int depth = -1);

bool HTMLElement_IsSingle(Element *elmt);


#endif
