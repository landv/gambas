/***************************************************************************

  eval_analyze.h

  Syntax highlighting

  (c) 2000-2005 Beno�t Minisini <gambas@users.sourceforge.net>

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


#ifndef __EVAL_ANALYZE_H
#define __EVAL_ANALYZE_H

#include "eval.h"

#define EVAL_MAX_COLOR  256

PUBLIC void EVAL_analyze(const char *src, int len, EVAL_ANALYZE *result, bool rewrite);

#endif
