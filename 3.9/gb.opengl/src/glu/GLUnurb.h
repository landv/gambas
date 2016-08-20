/***************************************************************************

  GLUnurb.h

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
  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
  MA 02110-1301, USA.

***************************************************************************/

#ifndef __GLUNURB_H
#define __GLUNURB_H

#include "main.h"
DECLARE_METHOD(GLUBEGINCURVE);
DECLARE_METHOD(GLUBEGINSURFACE);
DECLARE_METHOD(GLUBEGINTRIM);
DECLARE_METHOD(GLUDELETENURBSRENDERER);
DECLARE_METHOD(GLUENDCURVE);
DECLARE_METHOD(GLUENDSURFACE);
DECLARE_METHOD(GLUENDTRIM);
/*skip ***DECLARE_METHOD(GLUGETNURBSPROPERTY);
skip ***DECLARE_METHOD(GLULOADSAMPLINGMATRICES);
skip ***DECLARE_METHOD(GLUNURBSCALLBACK);
skip ***DECLARE_METHOD(GLUNURBSCALLBACKDATA);
skip ***DECLARE_METHOD(GLUNURBSCALLBACKDATAEXT);*/
DECLARE_METHOD(GLUNURBSCURVE);
DECLARE_METHOD(GLUNURBSPROPERTY);
DECLARE_METHOD(GLUNURBSSURFACE);
DECLARE_METHOD(GLUNEWNURBSRENDERER);
//DECLARE_METHOD(GLUPWLCURVE);


#endif /* __GLUNURB_H */
