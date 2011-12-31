/***************************************************************************

  main.cpp

  (c) 2006 Laurent Carlier <lordheavy@users.sourceforge.net>

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

#define __MAIN_CPP

#include "main.h"

#include "CGLarea.h"

extern "C" {

GB_INTERFACE GB EXPORT;
QT_INTERFACE QT;
GL_INTERFACE GL;

GB_DESC *GB_CLASSES[] EXPORT =
{
  CGlareaDesc,
  NULL
};

int EXPORT GB_INIT(void)
{
  GB.GetInterface("gb.qt4", QT_INTERFACE_VERSION, &QT);
  GB.GetInterface("gb.opengl", GL_INTERFACE_VERSION, &GL);

  return 0;
}

void EXPORT GB_EXIT()
{
}

}
