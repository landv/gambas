/***************************************************************************

  gbx_info.h

  Optimize gb.info file generation

  (c) 2000-2007 Benoit Minisini <gambas@users.sourceforge.net>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 1, or (at your option)
  any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

***************************************************************************/

#ifndef __GBX_INFO_H
#define __GBX_INFO_H

#include "gb_common.h"
#include "gambas.h"

#ifdef GBX_INFO

#undef GB_DECLARE
#define GB_DECLARE(name, size) { name, (long)GB_VERSION, 0 }

#undef GB_HOOK_NEW
#define GB_HOOK_NEW(hook)    { GB_HOOK_NEW_ID, 0 }

#undef GB_HOOK_FREE
#define GB_HOOK_FREE(hook)   { GB_HOOK_FREE_ID, 0 }

#undef GB_HOOK_CHECK
#define GB_HOOK_CHECK(hook)  { GB_HOOK_CHECK_ID, 0 }

#undef GB_PROPERTY
#define GB_PROPERTY(symbol, type, proc) { "p" symbol, (long)type, 0 }

#undef GB_PROPERTY_READ
#define GB_PROPERTY_READ(symbol, type, proc) { "r" symbol, (long)type, 0 }

#undef GB_METHOD
#define GB_METHOD(symbol, type, exec, signature) { "m" symbol, (long)type, 0, (long)signature }

#undef GB_EVENT
#define GB_EVENT(symbol, type, signature, id) { "::" symbol, (long)type, 0, (long)signature }

#undef GB_STATIC_PROPERTY
#define GB_STATIC_PROPERTY(symbol, type, proc) { "P" symbol, (long)type, 0 }

#undef GB_STATIC_PROPERTY_READ
#define GB_STATIC_PROPERTY_READ(symbol, type, proc) { "R" symbol, (long)type, 0 }

#undef GB_STATIC_METHOD
#define GB_STATIC_METHOD(symbol, type, exec, signature) { "M" symbol, (long)type, 0, (long)signature }

#endif

#endif
