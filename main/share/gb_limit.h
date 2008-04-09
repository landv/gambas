/***************************************************************************

  limit.h

  Compiler and interpreter limits

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

#ifndef __GB_LIMIT_H
#define __GB_LIMIT_H

/* Maximum number of parameters for a function - CANNOT CHANGE */
#define MAX_PARAM_FUNC      63

/* Maximum number of parameters for a subroutine - CANNOT CHANGE */
#define MAX_PARAM_SUBR      63

/* Maximum number of parameters for an operator - CANNOT CHANGE */
#define MAX_PARAM_OP        63

/* Maximum level of expression imbrication */
#define MAX_EXPR_LEVEL      255

/* Maximum number of patterns in an expression - CANNOT CHANGE */
#define MAX_EXPR_PATTERN    1023

/* Maximum length of a symbol */
#define MAX_SYMBOL_LEN      255

/* Maximum number of constants in the same class - CANNOT CHANGE */
#define MAX_CLASS_CONST     4096

/* Maximum number of static or dynamic symbols in the same class - CANNOT CHANGE */
#define MAX_CLASS_SYMBOL    2048

/* Maximum number of functions in the same class - CANNOT CHANGE */
#define MAX_CLASS_FUNCTION  2048

/* Maximum number of class uses in the same class - CANNOT CHANGE */
#define MAX_CLASS_CLASS     2048

/* Maximum number of class uses in the same class - CANNOT CHANGE */
#define MAX_CLASS_EXTERN    256

/* Maximum number of events in the same class - CANNOT CHANGE */
#define MAX_CLASS_EVENT     256

/* Maximum number of unknown symbols in the same class - CANNOT CHANGE */
#define MAX_CLASS_UNKNOWN   65536

/* Maximum number of array declarations in the same class - CANNOT CHANGE */
#define MAX_CLASS_ARRAY     32678

/* Maximum number of local variables in a function - CANNOT CHANGE */
#define MAX_LOCAL_SYMBOL    127

/* Maximum level of control structures imbrication */
#define MAX_CTRL_LEVEL      32

/* Maximum number of dimensions in an array */
#define MAX_ARRAY_DIM       8

/* Maximum length of a file path */
#define MAX_PATH            512

/* Maximum level of controls imbrication in a form */
#define MAX_FORM_PARENT     32

/* Maximum number of possible comparisons in one CASE instruction */
#define MAX_CASE_EXPR       32

/* Maximum number of breakpoints */
#define MAX_BREAKPOINT      255

/* Number of bytes written at once in a single input/output request */
#define MAX_IO              4096

/* Maximum level of class inheritance */
#define MAX_INHERITANCE     16

/* Maximum length of an error message */
#define MAX_ERROR_MSG       511

#endif

