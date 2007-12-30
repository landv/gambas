/***************************************************************************

  watch.h

  Default event loop and file descriptor watch routines

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

#ifndef __GBX_WATCH_H
#define __GBX_WATCH_H

#include "gambas.h"
#include <sys/select.h>
#include <sys/time.h>

enum {
  WATCH_NONE = 0,
  WATCH_READ = 1,
  WATCH_WRITE = 2,
  WATCH_READ_WRITE = 3
  };

typedef
  struct {
    long fd;
    void (*callback)();
    long param;
    }
  WATCH_CALLBACK;

typedef
	struct {
		GB_TIMER *timer;
		struct timeval timeout;
	}
	WATCH_TIMER;


PUBLIC void WATCH_init(void);
PUBLIC void WATCH_exit(void);

PUBLIC void WATCH_watch(int fd, int flag, void *callback, long param);
PUBLIC bool WATCH_one_loop(long);
PUBLIC void WATCH_loop(void);
PUBLIC void WATCH_wait(long);
PUBLIC int WATCH_loop_signal(const sigset_t *sig);
PUBLIC int WATCH_process(int fd_end, int fd_output);
PUBLIC void WATCH_timer(void *t, int on);

#endif /* */
