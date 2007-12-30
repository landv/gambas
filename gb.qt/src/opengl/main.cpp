/***************************************************************************

  main.cpp

  OpenGL widget support for gb.qt

  (c) 2006 Laurent Carlier <lordheavy@users.sourceforge.net>
           Benoît Minisini <gambas@users.sourceforge.net>

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

#define __MAIN_CPP

#include "gambas.h"
#include "main.h"
#include "../gb.qt.h"

#include "CGLarea.h"

extern "C" {

GB_INTERFACE GB EXPORT;
QT_INTERFACE QT;

GB_DESC *GB_CLASSES[] EXPORT =
{
  CGlareaDesc,
  NULL
};

int EXPORT GB_INIT(void)
{
  GB.GetInterface("gb.qt", QT_INTERFACE_VERSION, &QT);

  return FALSE;
}

void EXPORT GB_EXIT()
{
}

}
