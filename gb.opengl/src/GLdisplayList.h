/***************************************************************************

  GLdisplayList.h

  The Gambas openGL component

  (c) 2005-2007 Laurent Carlier <lordheavy@users.sourceforge.net>
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

#ifndef __GLDISPLAYLIST_H
#define __GLDISPLAYLIST_H

#include "gambas.h"
#include "main.h"

#include <GL/gl.h>

DECLARE_METHOD(GLCALLLIST);
DECLARE_METHOD(GLCALLLISTS);
DECLARE_METHOD(GLDELETELISTS);
DECLARE_METHOD(GLENDLIST);
DECLARE_METHOD(GLGENLISTS);
DECLARE_METHOD(GLISLIST);
DECLARE_METHOD(GLLISTBASE);
DECLARE_METHOD(GLNEWLIST);

#endif /* __GLDISPLAYLIST_H */
