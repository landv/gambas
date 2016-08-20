/***************************************************************************

  GLeval.h

  (c) 2005-2011 Laurent Carlier <lordheavy@users.sourceforge.net>

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

#ifndef __GLEVAL_H
#define __GLEVAL_H

#include "main.h"

DECLARE_METHOD(GLMAP1F);
DECLARE_METHOD(GLMAP2F);
//DECLARE_METHOD(GLGETMAPFV);
//DECLARE_METHOD(GLGETMAPIV);
DECLARE_METHOD(GLEVALCOORD1F);
DECLARE_METHOD(GLEVALCOORD2F);
DECLARE_METHOD(GLEVALCOORD2FV);
DECLARE_METHOD(GLMAPGRID1F);
DECLARE_METHOD(GLMAPGRID2F);
DECLARE_METHOD(GLEVALPOINT1);
DECLARE_METHOD(GLEVALPOINT2);
DECLARE_METHOD(GLEVALMESH1);
DECLARE_METHOD(GLEVALMESH2);

#endif /* __GLEVAL_H */


