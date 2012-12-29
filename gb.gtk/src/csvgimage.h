/***************************************************************************

  csvgimage.h

  (c) 2004-2006 - Daniel Campos Fernández <dcamposf@gmail.com>

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

#ifndef __CSVGIMAGE_H
#define __CSVGIMAGE_H

#include "gambas.h"
#include "widgets.h"
#include <cairo.h>
#include <librsvg/rsvg.h>
#ifndef RSVG_CAIRO_H
#include <librsvg/rsvg-cairo.h>
#endif

typedef 
	struct 
	{
		GB_BASE ob;
		cairo_surface_t *surface;
		RsvgHandle *handle;
		char *file;
		double width;
		double height;
	} 
	CSVGIMAGE;

#ifndef __CSVGIMAGE_CPP

extern GB_DESC SvgImageDesc[];

#else

#define THIS OBJECT(CSVGIMAGE)
#define SURFACE THIS->surface
#define HANDLE THIS->handle

#endif

cairo_surface_t *SVGIMAGE_begin(CSVGIMAGE *_object);
void SVGIMAGE_end(CSVGIMAGE *_object);

#endif
