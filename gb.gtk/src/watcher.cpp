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
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include "main.h"
#include "gambas.h"
#include "watcher.h"

static Watcher **watch;
static long    nwatch;

void CWatcher::Init()
{
	watch=NULL;
	nwatch=0;
}

void CWatcher::Clear()
{
	while (nwatch) CWatcher::Remove(watch[0]->fd);
}

void CWatcher::Remove(int fd)
{
	long bucle,b2;
	
	for (bucle=0;bucle<nwatch;bucle++)
	{
		if ( watch[bucle]->fd == fd )
		{
			GB.Free((void**)&watch[bucle]);
			for (b2=bucle;b2<(nwatch-1);b2++)
			{
				watch[b2]=watch[b2+1];
			}
			nwatch--;
			if (!nwatch)
			{
				GB.Free((void**)&watch);
				watch=NULL;
			}
			else
			{
				GB.Realloc((void**)&watch,sizeof(Watcher*)*nwatch);
			}
			break;
		}
	}
}

void CWatcher::Perform()
{
	int myfd;
	long bucle;
	fd_set rfds,wfds;
    struct timeval tv;
    int valret;
	GB_WATCH_CALLBACK func;
	
	if (!nwatch) return;
	
	for (bucle=0;bucle<nwatch;bucle++)
	{	
		tv.tv_sec = 0;
		tv.tv_usec = 0;
		
		if (watch[bucle]->type)
		{
			myfd=watch[bucle]->fd;
			FD_ZERO(&rfds);
			FD_ZERO(&wfds);
			if (watch[bucle]->type & GB_WATCH_READ) FD_SET(watch[bucle]->fd, &rfds);
			if (watch[bucle]->type & GB_WATCH_WRITE) FD_SET(watch[bucle]->fd, &wfds);
			
			valret = select(watch[bucle]->fd+1, &rfds, &wfds, NULL, &tv);
			
			if (valret==-1) { printf("WARNING: invalid descriptor\n"); }
			else
			{
				if (FD_ISSET(watch[bucle]->fd, &rfds)) 
				{
					func=(GB_WATCH_CALLBACK)watch[bucle]->callback;
					func(watch[bucle]->fd,GB_WATCH_READ,watch[bucle]->param);
				}
				
				if (!watch) return;
				
				if ( nwatch>bucle )
				{
					if ( (watch[bucle]->fd == myfd) && (watch[bucle]->type & GB_WATCH_WRITE) )
					{
						if (FD_ISSET(watch[bucle]->fd, &wfds))
						{
							func=(GB_WATCH_CALLBACK)watch[bucle]->callback;
							func(watch[bucle]->fd,GB_WATCH_WRITE,watch[bucle]->param);
						}
					}
				}
				
			}
		}
	
		
		
	}
	
}

void CWatcher::Add(int fd,int type,void *callback,long param)
{
	long pos=-1,bucle;
	
	if (type==GB_WATCH_NONE) { CWatcher::Remove(fd); return; }
	
	if ( (type==GB_WATCH_READ) || (type==GB_WATCH_WRITE) || (type==GB_WATCH_READ_WRITE) )
	{
	
		for(bucle=0;bucle<nwatch;bucle++)
			if (watch[bucle]->fd==fd) { pos=bucle; break; }
	
		if (pos==-1)
		{
			if (!watch)
				GB.Alloc((void**)&watch,sizeof(Watcher*));
			else
				GB.Realloc((void**)&watch,sizeof(Watcher*)*(nwatch+1));
			
			GB.Alloc ((void**)&watch[nwatch],sizeof(Watcher));
			pos=nwatch;
			nwatch++;
		}
		
		watch[pos]->fd=fd;
		watch[pos]->type=type;
		watch[pos]->callback=callback;
		watch[pos]->param=param;
		
	}
}