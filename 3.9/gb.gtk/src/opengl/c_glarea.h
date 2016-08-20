/***************************************************************************

  c_glarea.h

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

#ifndef __C_GLAREA_H
#define __C_GLAREA_H

#include "main.h"

typedef
  struct {
    GTK_CONTROL control;
		GtkWidget *widget;
		bool init;
   }
  CGLAREA;

#ifndef __C_GLAREA_C
extern GB_DESC GLAreaDesc[];
#else

#define THIS    ((CGLAREA *)_object)
#define WIDGET  ((GLarea *)((GTK_CONTROL *)_object)->widget)

//#define CGLAREA_PROPERTIES QT_WIDGET_PROPERTIES

#endif /* __CGLAREA_CPP */

#endif
