/***************************************************************************

  CDnsClient.c

  (c) 2003-2004 Daniel Campos Fernández <dcamposf@gmail.com>

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

#define __CDNSCLIENT_C

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/poll.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <semaphore.h>
#include <pthread.h>
#include <errno.h>
#include <assert.h>
#include "main.h"

#include "CDnsClient.h"
#ifndef _REENTRANT
#define _REENTRANT
#endif

static void **dns_object=NULL;
static int dns_count=0;
static sem_t dns_th_pipe;
static int dns_r_pipe=-1;
static int dns_w_pipe=-1;
static int dns_async_count=0; /* protected by dns_th_pipe */

DECLARE_EVENT(EVENT_Finished);

int dns_init(void)
{
	int dpipe[2];

	if (pipe(dpipe))
		return 1;
	dns_r_pipe=dpipe[0];
	dns_w_pipe=dpipe[1];
	sem_init(&dns_th_pipe,0,1);
	return 0;
}

void dns_exit(void)
{
	close(dns_r_pipe);
	close(dns_w_pipe);
	dns_r_pipe=-1;
	dns_w_pipe=-1;
}

/**********************************************************
 DnsClient pipe message protocol:

 1) (void*)DNSCLIENT*
 2) Action : '0' --> dns_get_name, '1' --> dns_get_ip
 3) Result : string finished with '\x10'
 *********************************************************/

static bool read_dns_pipe(void *data, size_t length)
{
	if (read(dns_r_pipe, data, length) != length)
	{
		fprintf(stderr, "gb.net: cannot read DNS pipe: %s\n", strerror(errno));
		return TRUE;
	}
	
	return FALSE;
}

/* Callers hold the dns_th_pipe semaphore as we change dns_async_count! */
static void dns_start_async(void)
{
	assert(dns_async_count >= 0);
	if (!dns_async_count++)
		GB.Watch(dns_r_pipe, GB_WATCH_READ, (void *) dns_callback, 0);
}

/* Callers hold the dns_th_pipe semaphore */
static void dns_stop_async(void)
{
	if (!--dns_async_count)
		GB.Watch(dns_r_pipe, GB_WATCH_NONE, (void *) dns_callback, 0);
	assert(dns_async_count >= 0);
}

void dns_callback(intptr_t lParam)
{
	/***********************************************************
	This function reads a message sent by a thread, and then
	raises "Finished" event if necessary, and fills HostName
	and HostIP properties. This function will run on the main
	thread.
	************************************************************/
	CDNSCLIENT *mythis;
	void *v_obj;
	char Action[1];
	char BufRead[1];
	char *Buf=NULL;

	int Position;
	int myloop;
	int test_id;
	struct pollfd mypoll;
	int idata=1;
	if (dns_r_pipe==-1) return;
	sem_wait(&dns_th_pipe);
	
	for(;;)
	{
		Position=0;
		BufRead[0]='\0';
		mypoll.fd=dns_r_pipe;
		mypoll.events=POLLIN;
		mypoll.revents=0;
		idata=poll(&mypoll,1,0);
		if (idata <= 0) break;
		read_dns_pipe(&v_obj,sizeof(void*));
		read_dns_pipe(&test_id,sizeof(int));
		read_dns_pipe(Action,sizeof(char));
		GB.Alloc(POINTER(&Buf),sizeof(char));

		while (BufRead[0] != '\x10')
		{
			read_dns_pipe(BufRead,sizeof(char));
			if (BufRead[0]!='\x10')
			{
				Buf[Position]=BufRead[0];
				Position++;
				GB.Realloc(POINTER(&Buf),(Position+1)*sizeof(char));
			}
			else
				Buf[Position]='\0';
		}
		Position=-1;
		for (myloop=0;myloop<dns_count;myloop++)
		{
			if ( dns_object[myloop]==v_obj)
			{
				Position=myloop;
				break;
			}
		}
		if ( Position>=0)
		{
			mythis=(CDNSCLIENT*)v_obj;
			if (mythis->iStatus && (mythis->i_id==test_id))
			{
				if (Action[0]=='1')
				{
					GB.FreeString(&mythis->sHostIP);
					mythis->sHostIP = GB.NewZeroString(Buf);
				}
				else
				{
					GB.FreeString(&mythis->sHostName);
					mythis->sHostName = GB.NewZeroString(Buf);
				}
				mythis->iStatus=0;
				if (mythis->finished_callback)
				{
				  //GB.Ref(mythis);
				  GB.Post(mythis->finished_callback, (intptr_t)mythis->CliParent);
				}
				else
				{
					GB.Ref(mythis);
					GB.Post((void (*)())dns_event, (intptr_t)mythis);
				}
			}
			/* Release one DnsClient in async mode */
			if (mythis->iAsync)
				dns_stop_async();
		}
		GB.Free(POINTER(&Buf));
	}

	sem_post(&dns_th_pipe);
}
/**************************************************
 To raise an event
 **************************************************/

void dns_event(CDNSCLIENT *mythis)
{
    GB.Raise(mythis,EVENT_Finished,0);
    GB.Unref(POINTER(&mythis));
}

static void write_dns_pipe(void *data, size_t length)
{
	if (write(dns_w_pipe, data, length) != length)
		fprintf(stderr, "gb.net: cannot write to DNS pipe: %s\n", strerror(errno));
}

void* dns_get_name(void* v_obj)
{
	/****************************************************************
	This function will run in a thread different from main thread,
	   and when it finish its proccess, it sends a message to the
	   main thread using a pipe
	*****************************************************************/
	char Buf[1];
	int myid;
	int res;
	CDNSCLIENT *mythis;

	char host[1024];
	struct sockaddr_in sa;
	
	pthread_setcancelstate(PTHREAD_CANCEL_ENABLE,NULL);
	Buf[0]='0';

	mythis=(CDNSCLIENT*)v_obj;
	sem_wait(&mythis->sem_id);
	myid=mythis->i_id;
	sem_post(&mythis->sem_id);


	//((struct sockaddr*)&sa)->sa_family=AF_INET;
	sa.sin_family = AF_INET;
	bzero(host,1024);
	sa.sin_port=0;
	inet_aton(mythis->sHostIP, &sa.sin_addr);
	
	res=getnameinfo((struct sockaddr*)&sa,sizeof(struct sockaddr_in),host,1024*sizeof(char),NULL,0,NI_NAMEREQD);
	
	pthread_setcancelstate(PTHREAD_CANCEL_DISABLE,NULL);
	sem_wait(&dns_th_pipe);
	write_dns_pipe(&v_obj,sizeof(void*)); /* object */
	write_dns_pipe(&myid,sizeof(int));    /* id */
	write_dns_pipe(Buf,sizeof(char));     /* Action */
	if (!res) write_dns_pipe(host,strlen(host)*sizeof(char));
	write_dns_pipe("\x10",sizeof(char));
	sem_post(&dns_th_pipe);
	return NULL;
}


/****************************************************************
This function will run in a thread different from main thread,
and when it finish its proccess, it sends a message to the
main thread using a pipe
*****************************************************************/

void* dns_get_ip(void* v_obj)
{	
	char Buf[1];
	char *BufData;
	int myid;
	struct addrinfo *stHost;
	struct sockaddr_in *addr;
	CDNSCLIENT *mythis;
	
	pthread_setcancelstate(PTHREAD_CANCEL_ENABLE,NULL);
	
	Buf[0]='1';

	mythis=(CDNSCLIENT*)v_obj;
	sem_wait(&mythis->sem_id);
	myid=mythis->i_id;
	sem_post(&mythis->sem_id);

	if (getaddrinfo(mythis->sHostName,NULL,NULL,&stHost)) stHost=NULL;
	if (stHost) if ( stHost[0].ai_family!=PF_INET ) stHost=NULL;

	sem_wait(&dns_th_pipe);
	pthread_setcancelstate(PTHREAD_CANCEL_DISABLE,NULL);
	
	write_dns_pipe(&v_obj, sizeof(void*)); /* object */
	write_dns_pipe(&myid, sizeof(int));    /* id */
	write_dns_pipe(Buf, sizeof(char));     /* action */
	
	if (stHost!=NULL)
	{
		addr=(struct sockaddr_in*)stHost[0].ai_addr;
		BufData=inet_ntoa(addr->sin_addr);
		if (BufData) write_dns_pipe(BufData,strlen(BufData)*sizeof(char));
	}
	write_dns_pipe("\x10",sizeof(char));
	sem_post(&dns_th_pipe);
	return NULL;
}


void dns_close_all(CDNSCLIENT *mythis)
{
	if (mythis->iStatus)
  	{
		pthread_cancel(mythis->th_id); /* cancel thread */
		pthread_join(mythis->th_id,NULL);
		sem_destroy(&mythis->sem_id);
		mythis->iStatus=0; /* inactive */
		dns_callback(dns_r_pipe); /* freeing pipe data */

	}
}
int dns_thread_getname(CDNSCLIENT *mythis)
{
	sem_wait(&mythis->sem_id);
	mythis->i_id++;
	sem_post(&mythis->sem_id);

	mythis->iStatus=1;
	/* We need to register the watch in the main thread to not anger qt */
	sem_wait(&dns_th_pipe);
	dns_start_async();
	sem_post(&dns_th_pipe);

	if (pthread_create(&mythis->th_id,NULL,dns_get_name,(void*)mythis) )
	{
		mythis->iStatus=0;
		return 1;
	}
	pthread_detach(mythis->th_id);
	return 0;
}

int dns_thread_getip(CDNSCLIENT *mythis)
{
	sem_wait(&mythis->sem_id);
	mythis->i_id++;
	sem_post(&mythis->sem_id);

	mythis->iStatus=1;
	sem_wait(&dns_th_pipe);
	dns_start_async();
	sem_post(&dns_th_pipe);

	if (pthread_create(&mythis->th_id,NULL,dns_get_ip,(void*)mythis) )
	{
		mythis->iStatus=0;
		return 1;
	}
	pthread_detach(mythis->th_id);
	return 0;
}

void dns_set_async_mode(int myval, CDNSCLIENT *mythis)
{
	mythis->iAsync = myval;
}

/*************************************************************************************
######################################################################################
	---------------------  DNSCLIENT GAMBAS INTERFACE IMPLEMENTATION -----------------
######################################################################################
**************************************************************************************/

/*********************************************
 This property gets/sets the name of a Host
 *********************************************/
BEGIN_PROPERTY ( HostName )

	if (READ_PROPERTY)
  	{
		if (THIS->iStatus){ GB.ReturnString(NULL); return; }
		GB.ReturnString(THIS->sHostName);
		return;
	}

	if (THIS->iStatus) { GB.Error("HostIP can not be changed while working"); return;}
	GB.FreeString(&THIS->sHostName);
	GB.StoreString(PROP(GB_STRING), &THIS->sHostName);

END_PROPERTY

/*************************************************
 This property gets/sets the IP address of a Host
 *************************************************/
BEGIN_PROPERTY ( HostIP )

	if (READ_PROPERTY)
  	{
		if (THIS->iStatus) { GB.ReturnString(NULL); return; }
		GB.ReturnString(THIS->sHostIP);
		return;
	}

	if (THIS->iStatus) { GB.Error("HostIP can not be changed while working"); return; }
	GB.FreeString(&THIS->sHostIP);
	GB.StoreString(PROP(GB_STRING), &THIS->sHostIP);

END_PROPERTY

/*************************************************
 This property gets/sets asynchronous state:

 FALSE : Synchronous
 TRUE  : Asynchronous
 *************************************************/


BEGIN_PROPERTY ( CDNSCLIENT_Async )


  	if (READ_PROPERTY)
	{
		GB.ReturnBoolean(THIS->iAsync);
		return;
	}
	THIS->iAsync = VPROP(GB_BOOLEAN);

END_PROPERTY

/**********************************************************
 This property gets status : 0 --> Inactive, 1 --> Working
 **********************************************************/
BEGIN_PROPERTY ( CDNSCLIENT_Status )

  GB.ReturnInteger(THIS->iStatus);

END_PROPERTY
/*************************************************
 Gambas object "Constructor"
 *************************************************/
BEGIN_METHOD_VOID(CDNSCLIENT_new)

  THIS->CliParent=NULL;
  THIS->finished_callback=NULL;
  THIS->sHostIP=NULL;
  THIS->sHostName=NULL;
  THIS->iStatus=0;
  THIS->iAsync=0;
  THIS->i_id=0;
  sem_init(&THIS->sem_id,0,1);
  dns_count++;
  if (dns_object==NULL)
  	GB.Alloc(POINTER(&dns_object),sizeof(void*));
  else
  	GB.Realloc(POINTER(&dns_object),dns_count*sizeof(void*));

  dns_object[dns_count-1]=(void*)THIS;

END_METHOD

/*************************************************
 Gambas object "Destructor"
 *************************************************/
BEGIN_METHOD_VOID(CDNSCLIENT_free)

  int myloop;
  int Position=-1;
  dns_close_all(THIS);
  GB.FreeString(&THIS->sHostIP);
  GB.FreeString(&THIS->sHostName);
  for (myloop=0;myloop<dns_count;myloop++)
  {
	if ( dns_object[myloop]==((void*)THIS) )
	{
		Position=myloop;
		break;
	}
  }
  if (Position>=0)
  {
  	for (myloop=Position;myloop< (dns_count-1);myloop++)
	{
		dns_object[myloop]=dns_object[myloop+1];
	}
	dns_count--;
	if (!dns_count)
		GB.Free(POINTER(&dns_object));
  }

END_METHOD

/*************************************************
 To cancel an asynchronous operation
 *************************************************/
BEGIN_METHOD_VOID(CDNSCLIENT_Stop)

	dns_close_all(THIS);

END_METHOD
/*****************************************************************************
 This method takes the Host IP, which was stored using HostName property,
 and trasnlates it to IP address
 *****************************************************************************/
BEGIN_METHOD_VOID(CDNSCLIENT_GetHostName)

  struct hostent *stHost=NULL;
  struct in_addr addr;


  if (THIS->iStatus)
  {
  	GB.Error("Object is already working");
	return;
  }

  if (THIS->sHostIP)
  {
  	if ( THIS->iAsync)
	{ /* Asynchronous mode */
		sem_wait(&THIS->sem_id);
		THIS->i_id++;
		sem_post(&THIS->sem_id);
		THIS->iStatus=1;
		if (dns_thread_getname(THIS))
		{
			GB.Error("No resources available to create a thread");
			return;
		}
	}
	else
	{ /* Synchronous mode */
		inet_aton(THIS->sHostIP,&addr);
#ifdef __sun__
		stHost=gethostbyaddr((const char *)&addr,sizeof(addr),AF_INET);
#else
		stHost=gethostbyaddr(&addr,sizeof(addr),AF_INET);
#endif
		if (stHost==NULL)
		{
			GB.FreeString(&THIS->sHostName);
		}
		else
		{
			GB.FreeString(&THIS->sHostName);
			THIS->sHostName = GB.NewZeroString(stHost->h_name);
		}
		GB.Raise((void*)THIS,EVENT_Finished,0);
	}
  }
  else
  {
	GB.FreeString(&THIS->sHostName);
  }


END_METHOD

/*****************************************************************************
 This method takes the Host IP, which was stored using HostIP property,
 and trasnlates it to host name
 *****************************************************************************/
BEGIN_METHOD_VOID(CDNSCLIENT_GetHostIP)

  struct hostent *stHost=NULL;
  if (THIS->iStatus)
  {
  	GB.Error("Object is already working");
	return;
  }


  if (THIS->sHostName)
  {
  	if ( THIS->iAsync )
	{ /* Asynchronous mode */

		sem_wait(&THIS->sem_id);
		THIS->i_id++;
		sem_post(&THIS->sem_id);
		THIS->iStatus=1;
		if (dns_thread_getip(THIS))
		{
			GB.Error("No resource available to create a thread");
			return;
		}
	}
	else
	{ /* Synchronous mode */
		stHost=gethostbyname(THIS->sHostName);
		if (stHost==NULL)
		{
			GB.FreeString(&THIS->sHostIP);
		}
		else
		{
			GB.FreeString(&THIS->sHostIP);
			THIS->sHostIP = GB.NewZeroString(inet_ntoa(*((struct in_addr*)stHost->h_addr)));
		}
		GB.Raise((void*)THIS,EVENT_Finished,0);
	}
  }
  else
  {
	GB.FreeString(&THIS->sHostIP);
  }



END_METHOD


/***************************************************************
 Here we declare the public interface of DnsClient class
 ***************************************************************/
GB_DESC CDnsClientDesc[] =
{

  GB_DECLARE("DnsClient", sizeof(CDNSCLIENT)),

  GB_EVENT("Finished", NULL, NULL, &EVENT_Finished),

  GB_METHOD("_new", NULL, CDNSCLIENT_new, NULL),
  GB_METHOD("_free", NULL, CDNSCLIENT_free, NULL),
  GB_METHOD("Stop", NULL, CDNSCLIENT_Stop, NULL),
  GB_METHOD("GetHostName", NULL, CDNSCLIENT_GetHostName, NULL),
  GB_METHOD("GetHostIP", NULL, CDNSCLIENT_GetHostIP, NULL),

  GB_PROPERTY("HostName", "s", HostName),
  GB_PROPERTY("HostIP", "s", HostIP),
  GB_PROPERTY("Async", "b", CDNSCLIENT_Async),
  GB_PROPERTY_READ("Status", "i", CDNSCLIENT_Status),
  
  GB_CONSTANT("_IsControl", "b", TRUE),
  GB_CONSTANT("_IsVirtual", "b", TRUE),
  GB_CONSTANT("_Group", "s", "Network"),
  GB_CONSTANT("_Properties", "s", "HostName,HostIP,Async"),
  GB_CONSTANT("_DefaultEvent", "s", "Finished"),

  GB_END_DECLARE
};


/******************************************************************************

I do not know if Solaris accepts getaddrinfo and getnameinfo, so here 
is the old implementation for that O.S.

*******************************************************************************/
/*
void* dns_get_ip(void* v_obj)
{
	char Buf[1];
	char *BufData;
	int myid;
	int herr;

	struct hostent hostbuf, *stHost;
  	size_t hstbuflen;
  	char tmphstbuf[1024];
	CDNSCLIENT *mythis;
	pthread_setcancelstate(PTHREAD_CANCEL_ENABLE,NULL);
	hstbuflen=1024;
	Buf[0]='1';

	mythis=(CDNSCLIENT*)v_obj;
	sem_wait(&mythis->sem_id);
	myid=mythis->i_id;
	sem_post(&mythis->sem_id);

	stHost=gethostbyname_r (mythis->sHostName, &hostbuf, tmphstbuf, hstbuflen, &herr);

	sem_wait(&dns_th_pipe);
	pthread_setcancelstate(PTHREAD_CANCEL_DISABLE,NULL);
	write (dns_w_pipe,&v_obj,sizeof(void*)); 
	write (dns_w_pipe,&myid,sizeof(int));    
	write (dns_w_pipe,Buf,sizeof(char));     
	if (stHost!=NULL)
	{
		BufData=inet_ntoa(*( (struct in_addr*)stHost->h_addr ));
		write (dns_w_pipe,BufData,strlen(BufData)*sizeof(char));
	}
	write (dns_w_pipe,"\x10",sizeof(char));
	sem_post(&dns_th_pipe);
	return NULL;
}


void* dns_get_name(void* v_obj)
{

	char Buf[1];
	int myid;
	int herr;
	size_t hstbuflen;
  	char tmphstbuf[2048];
	struct hostent hostbuf,*stHost=NULL;
	CDNSCLIENT *mythis;
	struct in_addr addr;

	pthread_setcancelstate(PTHREAD_CANCEL_ENABLE,NULL);
	hstbuflen=2048;
	Buf[0]='0';

	mythis=(CDNSCLIENT*)v_obj;
	sem_wait(&mythis->sem_id);
	myid=mythis->i_id;
	sem_post(&mythis->sem_id);
	inet_aton(mythis->sHostIP,&addr);
	stHost=gethostbyaddr_r((const char *)&addr, sizeof (addr), AF_INET, &hostbuf,tmphstbuf,hstbuflen,&herr);
	pthread_setcancelstate(PTHREAD_CANCEL_DISABLE,NULL);
	sem_wait(&dns_th_pipe);
	write (dns_w_pipe,&v_obj,sizeof(void*)); 
	write (dns_w_pipe,&myid,sizeof(int));    
	write (dns_w_pipe,Buf,sizeof(char));     
	if (stHost!=NULL) write (dns_w_pipe,stHost->h_name,strlen(stHost->h_name)*sizeof(char));
	write (dns_w_pipe,"\x10",sizeof(char));
	sem_post(&dns_th_pipe);
	return NULL;
}
*/
