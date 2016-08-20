/***************************************************************************

  gbx_c_task.h

  (c) 2000-2013 Benoît Minisini <gambas@users.sourceforge.net>

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

#ifndef __GBX_C_TASK_H
#define __GBX_C_TASK_H

#include <sys/types.h>
#include <signal.h>

#include "gambas.h"
#include "gb_file.h"
#include "gb_list.h"

#ifndef __GBX_C_TASK_C
extern GB_DESC TaskDesc[];
#endif

typedef
	struct {
		GB_BASE ob;
		LIST list;
		GB_VARIANT_VALUE ret;
		pid_t pid;
		int fd_out;
		int fd_err;
		int status;
		volatile sig_atomic_t stopped;
		unsigned child : 1;
		unsigned got_value : 1;
	}
	CTASK;

#define THIS ((CTASK *)_object)

#define RETURN_DIR_PATTERN FILE_TEMP_DIR "/task"
#define RETURN_FILE_PATTERN FILE_TEMP_DIR "/task/%d"

#define MAX_TASK 256

#endif
