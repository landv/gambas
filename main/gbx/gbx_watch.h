/***************************************************************************

  gbx_watch.h

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

#ifndef __GBX_WATCH_H
#define __GBX_WATCH_H

#include "gambas.h"
#include <sys/select.h>
#include <sys/time.h>

#ifdef OS_OPENBSD
#include <signal.h>
#endif

enum {
  WATCH_NONE = 0,
  WATCH_READ = 1,
  WATCH_WRITE = 2,
  };

typedef
  struct {
    int fd;
    void (*callback_read)();
    void (*callback_write)();
    intptr_t param_read;
    intptr_t param_write;
    }
  WATCH_CALLBACK;

typedef
	struct {
		GB_TIMER *timer;
		struct timeval timeout;
	}
	WATCH_TIMER;


void WATCH_init(void);
void WATCH_exit(void);

void WATCH_watch(int fd, int flag, void *callback, intptr_t param);
bool WATCH_one_loop(int);
void WATCH_loop(void);
void WATCH_wait(int);
int WATCH_loop_signal(const sigset_t *sig);
int WATCH_process(int fd_end, int fd_output);
void WATCH_timer(void *t, int on);
double WATCH_get_timeout(GB_TIMER *timer);

#endif /* */
