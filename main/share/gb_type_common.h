/***************************************************************************

  type_common.h

  Common datatypes definitions

  (c) 2000-2007 Benoit Minisini <gambas@users.sourceforge.net>

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

#ifndef __GB_TYPE_COMMON_H
#define __GB_TYPE_COMMON_H

enum {
  TK_UNKNOWN     = 0,   /* External symbol */
  TK_VARIABLE    = 1,   /* Variable */
  TK_FUNCTION    = 2,   /* Function */
  TK_PROPERTY    = 3,   /* Property */
  TK_LABEL       = 4,   /* GOTO label */
  TK_EVENT       = 5,   /* Event */
  TK_EXTERN      = 6,   /* Shared library function */
  TK_CONST       = 7,   /* Constant */

  TF_OPTIONAL    = 16,

  TF_STATIC      = 32,
  TF_PUBLIC      = 64,
  };


enum {
  T_VOID         = 0,
  T_BOOLEAN      = 1,
  T_BYTE         = 2,
  T_SHORT        = 3,
  T_INTEGER      = 4,
  T_LONG         = 5,
  T_SINGLE       = 6,
  T_FLOAT        = 7,
  T_DATE         = 8,
  T_STRING       = 9,
  T_CSTRING      = 10,
  T_VARIANT      = 11,
  T_ARRAY        = 12,
  T_FUNCTION     = 13,
  T_CLASS        = 14,
  T_NULL         = 15,
  T_OBJECT       = 16,
  #if __WORDSIZE == 64
  T_POINTER      = T_LONG
  #else
  T_POINTER      = T_INTEGER
  #endif
  };
  
#define T_SIZEOF_BOOLEAN   1
#define T_SIZEOF_BYTE      1
#define T_SIZEOF_SHORT     2
#define T_SIZEOF_INTEGER   4
#define T_SIZEOF_LONG      8
#define T_SIZEOF_SINGLE    4
#define T_SIZEOF_FLOAT     8
#define T_SIZEOF_DATE      8
#define T_SIZEOF_STRING    4
#define T_SIZEOF_VARIANT   12
#define T_SIZEOF_OBJECT    4

#endif
