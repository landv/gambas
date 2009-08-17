/***************************************************************************

  gbx_c_class.h

  (c) 2000-2009 Benoît Minisini <gambas@users.sourceforge.net>

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

#ifndef __GBX_C_CLASS_H
#define __GBX_C_CLASS_H

#include "gambas.h"

#ifndef __GBX_C_CLASS_C

EXTERN GB_DESC NATIVE_ClassSymbols[];
EXTERN GB_DESC NATIVE_Component[];
EXTERN GB_DESC NATIVE_Components[];
EXTERN GB_DESC NATIVE_Class[];
EXTERN GB_DESC NATIVE_Classes[];
EXTERN GB_DESC NATIVE_Object[];
EXTERN GB_DESC NATIVE_Symbol[];

#endif

#define SYMBOL_TO_CDS(object)  ((CLASS_DESC_SYMBOL *)((char *)object - (long)&(((CLASS_DESC_SYMBOL *)0)->ref)))

#endif
