/***************************************************************************

  GLattributes.h

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
  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

***************************************************************************/

#ifndef __GLATTRIBUTES_H
#define __GLATTRIBUTES_H

#include "main.h"
DECLARE_METHOD(GLBINDATTRIBLOCATION);
DECLARE_METHOD(GLVERTEXATTRIB1F);
DECLARE_METHOD(GLVERTEXATTRIB1FV);
DECLARE_METHOD(GLVERTEXATTRIB2F);
DECLARE_METHOD(GLVERTEXATTRIB2FV);
DECLARE_METHOD(GLVERTEXATTRIB3F);
DECLARE_METHOD(GLVERTEXATTRIB3FV);
DECLARE_METHOD(GLVERTEXATTRIB4F);
DECLARE_METHOD(GLVERTEXATTRIB4FV);
DECLARE_METHOD(GLGENFRAMEBUFFERSEXT);
DECLARE_METHOD(GLFRAMEBUFFERTEXTURE2D);
DECLARE_METHOD(GLBINDFRAMEBUFFERSEXT);
DECLARE_METHOD(GLCHECKFRAMEBUFFERSTATUSEXT);

#endif /* __GLATTRIBUTES_H */
