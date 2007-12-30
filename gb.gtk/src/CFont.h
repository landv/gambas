/***************************************************************************

  CFont.h

  (c) 2004-2006 - Daniel Campos Fernández <dcamposf@gmail.com>
  
  GTK+ component
  
  Realizado para la Junta de Extremadura. 
  Consejería de Educación Ciencia y Tecnología. 
  Proyecto gnuLinEx
  
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
#ifndef __CFONT_H
#define __CFONT_H

#include "main.h"
#include "gfont.h"

#ifndef __CFONT_CPP
extern GB_DESC CFontDesc[];
extern GB_DESC CFontsDesc[];

#else

#define THIS ((CFONT *)_object)
#define FONT (THIS->font)

#endif

typedef
  void (*FONT_FUNC)(gFont *, void *);

typedef struct 
  {
    GB_BASE ob;
    gFont *font;
    FONT_FUNC func;
    void *object;
    enum { Name, Size, Grade, Bold, Italic, Underline, StrikeOut };
  }  
  CFONT;

CFONT *CFONT_create(gFont *font, FONT_FUNC func = 0, void *object = 0);

#endif
