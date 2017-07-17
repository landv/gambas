/***************************************************************************

  GLuniform.h

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

#ifndef __GLUNIFORM_H
#define __GLUNIFORM_H

#include "main.h"

DECLARE_METHOD(GLGETUNIFORMLOCATION);
DECLARE_METHOD(GLUNIFORM1F);
DECLARE_METHOD(GLUNIFORM2F);
DECLARE_METHOD(GLUNIFORM3F);
DECLARE_METHOD(GLUNIFORM4F);
DECLARE_METHOD(GLUNIFORM1I);
DECLARE_METHOD(GLUNIFORM2I);
DECLARE_METHOD(GLUNIFORM3I);
DECLARE_METHOD(GLUNIFORM4I);
DECLARE_METHOD(GLUNIFORM1FV);
DECLARE_METHOD(GLUNIFORM2FV);
DECLARE_METHOD(GLUNIFORM3FV);
DECLARE_METHOD(GLUNIFORM4FV);
DECLARE_METHOD(GLUNIFORM1IV);
DECLARE_METHOD(GLUNIFORM2IV);
DECLARE_METHOD(GLUNIFORM3IV);
DECLARE_METHOD(GLUNIFORM4IV);
DECLARE_METHOD(GLUNIFORMMATRIX2FV);
DECLARE_METHOD(GLUNIFORMMATRIX3FV);
DECLARE_METHOD(GLUNIFORMMATRIX4FV);
DECLARE_METHOD(GLUNIFORMMATRIX2X3FV);
DECLARE_METHOD(GLUNIFORMMATRIX3X2FV);
DECLARE_METHOD(GLUNIFORMMATRIX2X4FV);
DECLARE_METHOD(GLUNIFORMMATRIX4X2FV);
DECLARE_METHOD(GLUNIFORMMATRIX3X4FV);
DECLARE_METHOD(GLUNIFORMMATRIX4X3FV);

#endif /* __GLUNIFORM_H */
