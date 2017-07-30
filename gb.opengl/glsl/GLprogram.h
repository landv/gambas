/***************************************************************************

  GLprogram.h

  (c) 2009 Laurent Carlier <lordheavy@users.sourceforge.net>

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

#ifndef __GLPROGRAM_H
#define __GLPROGRAM_H

#include "main.h"

DECLARE_METHOD(GLCREATEPROGRAM);
DECLARE_METHOD(GLDELETEPROGRAM);
DECLARE_METHOD(GLGETPROGRAMINFOLOG);
DECLARE_METHOD(GLGETPROGRAMIV);
DECLARE_METHOD(GLISPROGRAM);
DECLARE_METHOD(GLLINKPROGRAM);
DECLARE_METHOD(GLUSEPROGRAM);
DECLARE_METHOD(GLVALIDATEPROGRAM);

#endif /* __GLPROGRAM_H */
