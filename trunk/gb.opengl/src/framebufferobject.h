/***************************************************************************

  framebufferobject.h

  (c) 2011 Laurent Carlier <lordheavy@users.sourceforge.net>

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

#ifndef __FBO_H
#define __FBO_H

#include "main.h"
DECLARE_METHOD(GLBINDFRAMEBUFFEREXT);
DECLARE_METHOD(GLBINDRENDERBUFFEREXT);
DECLARE_METHOD(GLCHECKFRAMEBUFFERSTATUSEXT);
DECLARE_METHOD(GLDELETEFRAMEBUFFERSEXT);
DECLARE_METHOD(GLDELETERENDERBUFFERSEXT);
DECLARE_METHOD(GLFRAMEBUFFERRENDERBUFFEREXT);
DECLARE_METHOD(GLFRAMEBUFFERTEXTURE1DEXT);
DECLARE_METHOD(GLFRAMEBUFFERTEXTURE2DEXT);
DECLARE_METHOD(GLFRAMEBUFFERTEXTURE3DEXT);
DECLARE_METHOD(GLGENERATEMIPMAPEXT);
DECLARE_METHOD(GLGENFRAMEBUFFERSEXT);
DECLARE_METHOD(GLGENRENDERBUFFERSEXT);
DECLARE_METHOD(GLGETFRAMEBUFFERATTACHMENTPARAMETERIVEXT);
DECLARE_METHOD(GLGETRENDERBUFFERPARAMETERIVEXT);
DECLARE_METHOD(GLISFRAMEBUFFEREXT);
DECLARE_METHOD(GLISRENDERBUFFEREXT);
DECLARE_METHOD(GLRENDERBUFFERSTORAGEEXT);

#endif /* __FBO_H */
