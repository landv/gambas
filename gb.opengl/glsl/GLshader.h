/***************************************************************************

  GLshader.h

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

#ifndef __GLSHADER_H
#define __GLSHADER_H

#include "main.h"

DECLARE_METHOD(GLATTACHSHADER);
DECLARE_METHOD(GLCOMPILESHADER);
DECLARE_METHOD(GLCREATESHADER);
DECLARE_METHOD(GLDELETESHADER);
DECLARE_METHOD(GLDETACHSHADER);
DECLARE_METHOD(GLGETATTACHEDSHADERS);
DECLARE_METHOD(GLGETSHADERINFOLOG);
DECLARE_METHOD(GLGETSHADERIV);
DECLARE_METHOD(GLGETSHADERSOURCE);
DECLARE_METHOD(GLISSHADER);
DECLARE_METHOD(GLSHADERSOURCE);

#endif /* __GLSHADER_H */
