/***************************************************************************

  watcher.cpp

  (c) 2004-2006 - Daniel Campos Fernández <dcamposf@gmail.com>
  
  GTK+ component
  
  Realizado para la Junta de Extremadura. 
  Consejería de Educación Ciencia y Tecnología. 
  Proyecto gnuLinEx
  
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

#include <stdio.h>
#include <stdlib.h>
#include <gtk/gtk.h>
#include "main.h"
#include "gambas.h"
#include "watcher.h"

static WATCH *watch = NULL;

static gboolean watch_adaptor(GIOChannel *source, GIOCondition condition, gpointer param)
{
	WATCH *data = (WATCH *)param;

	if (!data) return true;
	
	switch (condition)
	{
		case G_IO_IN:
			(*data->callback_read)( g_io_channel_unix_get_fd(source), GB_WATCH_READ, data->param_read); break; 
		case G_IO_OUT:
			(*data->callback_write)( g_io_channel_unix_get_fd(source), GB_WATCH_WRITE, data->param_write); break; 
		default: break;
	}

	return true;
}

void CWatcher::init()
{
	GB.NewArray(POINTER(&watch), sizeof(WATCH), 0);
}

void CWatcher::exit()
{
	GB.FreeArray(POINTER(&watch));
}

void CWatcher::Clear()
{
	while (count()) 
	{
		CWatcher::Add(watch[0].fd, GB_WATCH_NONE, NULL, 0);
	}
}

void CWatcher::Remove(int fd)
{
	CWatcher::Add(fd,GB_WATCH_NONE,NULL,0);
}

void CWatcher::Add(int fd, int type, void *callback, intptr_t param)
{
	WATCH *data = NULL;
	int i;

	//fprintf(stderr, "CWatcher::Add(%d, %d, %p, %d)\n", fd, type, callback, param);

	for (i = 0; i < count(); i++)
	{
		data = &watch[i];
		if (data->fd == fd)
			break;
	}
	
	if (!data)
	{
		if (type == GB_WATCH_NONE || !callback)
			return;
		
		data = (WATCH *)GB.Add(&watch);
		data->fd = fd;
		data->channel = g_io_channel_unix_new(fd);
		data->callback_read = data->callback_write = 0;
	}

	if (data->callback_read && (type == GB_WATCH_NONE || type == GB_WATCH_READ))
	{
		g_source_remove(data->id_read);
		data->callback_read = 0;
	}
	
	if (data->callback_write && (type == GB_WATCH_NONE || type == GB_WATCH_WRITE))
	{
		g_source_remove(data->id_write);
		data->callback_write = 0;
	}
	
	if (callback)
	{
		if (type == GB_WATCH_READ)
		{
			data->callback_read = (WATCH_CALLBACK)callback;
			data->id_read = g_io_add_watch_full(data->channel, G_PRIORITY_LOW, G_IO_IN, watch_adaptor, (void*)data, NULL);
		}
		else if (type == GB_WATCH_READ)
		{
			data->callback_write = (WATCH_CALLBACK)callback;
			data->id_write = g_io_add_watch_full(data->channel, G_PRIORITY_LOW, G_IO_OUT, watch_adaptor, (void*)data, NULL);
		}
	}

	if (!data->callback_read && !data->callback_write)
	{
		g_io_channel_unref(data->channel);
		GB.Remove(&watch, i, 1);
	}
}

int CWatcher::count()
{
	return GB.Count(watch);
}
