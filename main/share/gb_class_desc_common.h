/***************************************************************************

  gb_class_desc_common.h

  (c) 2000-2013 Benoît Minisini <gambas@users.sourceforge.net>

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

#ifndef __GB_CLASS_DESC_COMMON_H
#define __GB_CLASS_DESC_COMMON_H

enum
{
  CD_VARIABLE_ID          = 1,
  CD_STATIC_VARIABLE_ID   = 2,
  CD_PROPERTY_ID          = 3,
  CD_STATIC_PROPERTY_ID   = 4,
  CD_METHOD_ID            = 5,
  CD_STATIC_METHOD_ID     = 6,
  CD_CONSTANT_ID          = 7,
  CD_EVENT_ID             = 8,
  CD_EXTERN_ID            = 9,
};

#endif
