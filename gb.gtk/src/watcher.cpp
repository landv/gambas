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

static watchData **watch = NULL;
static int nwatch = 0;

gboolean watch_adaptor (GIOChannel *source, GIOCondition condition, gpointer param)
{
	void (*cb)(int,int,long);
	watchData *data=(watchData*)param;
	

	if (!data) return true;
	
	cb=(void(*)(int,int,long))data->func;
	if (!cb) return true;
	
	switch (condition)
	{
		case G_IO_IN:
			cb ( g_io_channel_unix_get_fd(source),GB_WATCH_READ,(long)data->data); break; 
		case G_IO_OUT:
			cb ( g_io_channel_unix_get_fd(source),GB_WATCH_WRITE,(long)data->data); break; 

		default: break;
	}
		

	return true;
}

void CWatcher::Clear()
{
	int fd;

	while (nwatch) 
	{
		fd=g_io_channel_unix_get_fd((GIOChannel*)(watch[0]->channel));
		CWatcher::Add(fd,GB_WATCH_NONE,NULL,0);
	}
}

void CWatcher::Remove(int fd)
{
	CWatcher::Add(fd,GB_WATCH_NONE,NULL,0);
}

void CWatcher::Add(int fd,int type,void *callback,long param)
{
	GIOChannel *channel=NULL;
	watchData *data;
	int bc,bc2;

	//fprintf(stderr, "CWatcher::Add(%d, %d, %p, %d)\n", fd, type, callback, param);

	for (bc=0; bc<nwatch; bc++)
	{
		if ( g_io_channel_unix_get_fd( (GIOChannel*)(watch[bc]->channel) ) == fd ) break;
	}
	
	if (bc>=nwatch)
	{
		if ( type == GB_WATCH_NONE ) return;
		if (!watch) { nwatch=1; GB.Alloc(POINTER(&watch),sizeof(watchData)); }
		else        { nwatch++; GB.Realloc(POINTER(&watch),sizeof(watchData)*nwatch); }
		GB.Alloc (POINTER(&watch[nwatch-1]),sizeof(watchData));
		data=watch[nwatch-1]; 
	}
	else
	{
		channel=(GIOChannel*)watch[bc]->channel;
		// Apparently GLib does not remove the event source from the event loop when the channel is destroyed.
		g_source_remove(watch[bc]->id);
		
		//fprintf(stderr, "g_io_channel_unref(%p)\n", channel);		
		g_io_channel_unref(channel);
		
		if ( type == GB_WATCH_NONE )
		{
			GB.Free (POINTER(&watch[bc]));
			for (bc2=bc+1;bc2<nwatch;bc2++) watch[bc2-1]=watch[bc2];
			nwatch--;
			if (!nwatch) { GB.Free(POINTER(&watch)); watch=NULL; }
			else         { GB.Realloc(POINTER(&watch),sizeof(watchData)*nwatch); }
			return;
		}
		data=watch[bc];
	}

	channel=g_io_channel_unix_new(fd);
	//fprintf(stderr, "g_io_channel_unix_new: %p\n", channel);
	data->channel=(void*)channel;
	data->func=(void*)callback;
	data->data=(void*)param;
	switch (type)
	{
		case GB_WATCH_READ:
			data->id = g_io_add_watch(channel,G_PRIORITY_LOW,G_IO_IN,watch_adaptor,(void*)data,NULL);
			//fprintf(stderr, "g_io_add_watch: %p G_IO_IN\n", channel);
			break;
		case GB_WATCH_WRITE:
			data->id = g_io_add_watch(channel,G_PRIORITY_LOW,G_IO_OUT,watch_adaptor,(void*)data,NULL);
			//fprintf(stderr, "g_io_add_watch: %p G_IO_OUT\n", channel);
			break;
		case GB_WATCH_READ_WRITE:
			data->id = g_io_add_watch(channel,G_PRIORITY_LOW,(GIOCondition)(G_IO_IN | G_IO_OUT),watch_adaptor,(void*)data,NULL); 
			//fprintf(stderr, "g_io_add_watch: %p G_IO_IN | G_IO_OUT\n", channel);			
			break;
		default:
			data->id = 0;
	}

}

int CWatcher::count()
{
	return nwatch;
}
