/***************************************************************************

  watcher.h

  (c) 2004-2007 - Daniel Campos Fernández <dcamposf@gmail.com>

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

#ifndef __WATCHER_H
#define __WATCHER_H

#include "gambas.h"
#include <gtk/gtk.h>

typedef
	GB_WATCH_CALLBACK WATCH_CALLBACK;

typedef
  struct {
		int fd;
		GIOChannel *channel_read;
    guint id_read;
    WATCH_CALLBACK callback_read;
    intptr_t param_read;
		GIOChannel *channel_write;
    guint id_write;
    WATCH_CALLBACK callback_write;
    intptr_t param_write;
    }
  WATCH;

class CWatcher
{
public:
	static void init();
	static void exit();
	static void Add(int fd, int type, void *callback, intptr_t param);
	static void Clear();
	static void Remove(int fd);
	static int count();
};

#endif
