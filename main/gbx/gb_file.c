/***************************************************************************

  file.c

  The file management routines

  (c) 2000-2005 Benoît Minisini <gambas@users.sourceforge.net>

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

#include "gb_common.h"
#include "gb_error.h"

#include "gbx_value.h"
#include "gbx_exec.h"
#include "gbx_project.h"
#include "gambas.h"
#include "gbx_regexp.h"
#include "gbx_string.h"
#include "gbx_archive.h"
#include "gbx_stream.h"

#define PROJECT_EXEC

#include "gb_file_temp.h"
