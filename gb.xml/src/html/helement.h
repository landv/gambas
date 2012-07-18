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

#include "main.h"
#include "../element.h"


//class Element:
//{

    Attribute* getClassName();
    //void getClassNames(char ** &names, size_t* &lenNames, size_t &namesCount);
    bool hasClassName(const char *className, const size_t lenClassName);
    void setClassName(const char *value, const size_t len);

    Attribute* getId();
    void setId(const char *value, size_t len);

    bool matchSubFilter(const char *filter, size_t lenFilter);
    bool matchFilter(const char *filter, size_t lenFilter);
    void addGBChildrenByFilter(char *filter, size_t lenFilter, GB_ARRAY *array, int depth = -1);
    void getGBChildrenByFilter(char *filter, size_t lenFilter, GB_ARRAY *array, int depth = -1);

    Element* getChildById(char *id, size_t lenId, int depth = -1);
    void getGBChildrenByClassName(char* className, size_t lenClassName, GB_ARRAY *array, int depth = -1);


};


#endif
