/***************************************************************************

  GLUcoordTransf.h

  (c) 2005-2007 Laurent Carlier <lordheavy@users.sourceforge.net>

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

#ifndef __GLUCOORDTRANSF_H
#define __GLUCOORDTRANSF_H

#include "gambas.h"
#include "main.h"

#include <GL/glu.h>

DECLARE_METHOD(GLULOOKAT);
DECLARE_METHOD(GLUORTHO2D);
DECLARE_METHOD(GLUPERSPECTIVE);

#endif /* __GLUCOORDTRANSF_H */
