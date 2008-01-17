/***************************************************************************

  CDnsClient.h

  Network component

  (c) 2003-2004 Daniel Campos Fernández <dcamposf@gmail.com>

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
#ifndef __CDNSCLIENT_H
#define __CDNSCLIENT_H

#include <pthread.h>
#include <semaphore.h>
#include "gambas.h"

#ifndef __CDNSCLIENT_C

extern GB_DESC CDnsClientDesc[];

#else

#define THIS ((CDNSCLIENT *)_object)

#endif
typedef  struct
{
    GB_BASE ob;
    char* sHostName;
    char* sHostIP;
    int iStatus;
    int iAsync;
    int i_id;
    pthread_t th_id;
    sem_t sem_id;

    void (*finished_callback)(void*);
    void *CliParent;
}  CDNSCLIENT;


void dns_callback(long lParam);
void* dns_get_name(void* v_obj);
void* dns_get_ip(void* v_obj);
void dns_event(CDNSCLIENT *mythis);
void dns_close_all(CDNSCLIENT *mythis);
int dns_thread_getname(CDNSCLIENT *mythis);
int dns_thread_getip(CDNSCLIENT *mythis);
int dns_set_async_mode(int myval,CDNSCLIENT *mythis);




#endif
