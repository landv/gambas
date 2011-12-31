/***************************************************************************

  CProxy.h

  (c) 2003-2008 Daniel Campos Fernández <dcamposf@gmail.com>

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

#ifndef __CPROXY_H
#define __CPROXY_H

#include "gambas.h"
#include "gbcurl.h"

#include "CCurl.h"

#ifndef __CPROXY_C

extern GB_DESC CProxyDesc[];

#else

#undef THIS
#define THIS (&((CCURL *)_object)->proxy)

#endif

#endif
