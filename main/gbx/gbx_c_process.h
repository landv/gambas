/***************************************************************************

  gbx_c_process.h

  (c) 2000-2011 Benoît Minisini <gambas@users.sourceforge.net>

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

#ifndef __GBX_C_PROCESS_H
#define __GBX_C_PROCESS_H

#include <sys/types.h>
#include <signal.h>

#include "gambas.h"
#include "gbx_object.h"
#include "gbx_c_file.h"
#include "gbx_c_array.h"

#ifndef __GBX_C_PROCESS_C
extern GB_DESC NATIVE_Process[];
#else

#define THIS ((CPROCESS *)_object)

#endif

typedef
  struct _CPROCESS {
    CSTREAM ob;
    struct _CPROCESS *prev;
    struct _CPROCESS *next;
    /*char *param[];*/
    /*char *cmd;*/
    pid_t pid;
    int in;
    int out;
    int err;
    int status;
    int watch;
    volatile sig_atomic_t running;
    bool to_string;
    char *result;
    GB_VARIANT_VALUE tag;
  }
  CPROCESS;

enum
{
  PM_READ = 1,
  PM_WRITE = 2,
  PM_TERM = 4,
  PM_STRING = 8,
	PM_WAIT = 16,
  PM_SHELL = 128
};

CPROCESS *CPROCESS_create(int mode, void *cmd, char *name, CARRAY *env);
void CPROCESS_wait_for(CPROCESS *process);

#endif
