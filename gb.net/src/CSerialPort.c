/***************************************************************************

  CSerialPort.c

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

#define __CSERIALPORT_C

#include <stdio.h>
#include <termios.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/poll.h>
#include <time.h>

#ifdef __CYGWIN__
/* Cygwin defines FIONREAD in <sys/socket.h>. */
#include <sys/socket.h>
#endif /* __CYGWIN__ */

#include "main.h"
#include "tools.h"

#include "CSerialPort.h"

#define MAX_SERIAL_BUFFER_SIZE 65536



long *ser_objwatch=NULL;
long *ser_portwatch=NULL;
long ser_numwatch=0;

GB_STREAM_DESC SerialStream = {
	open: CSerialPort_stream_open,
	close: CSerialPort_stream_close,
	read: CSerialPort_stream_read,
	write: CSerialPort_stream_write,
	seek: CSerialPort_stream_seek,
	tell: CSerialPort_stream_tell,
	flush: CSerialPort_stream_flush,
	eof: CSerialPort_stream_eof,
	lof: CSerialPort_stream_lof,
	handle: CSerialPort_stream_handle
};


DECLARE_EVENT (Serial_Read);
DECLARE_EVENT (Serial_DTR);
DECLARE_EVENT (Serial_DSR);
DECLARE_EVENT (Serial_RTS);
DECLARE_EVENT (Serial_CTS);
DECLARE_EVENT (Serial_DCD);
DECLARE_EVENT (Serial_RNG);

void Serial_Signal_Status(serialsignal *sdata,int iPort)
{
	int ist;
	sdata->s_DSR=0;
	sdata->s_DTR=0;
	sdata->s_RTS=0;
	sdata->s_CTS=0;
	sdata->s_DCD=0;
	sdata->s_RNG=0;
	ioctl(iPort,TIOCMGET,&ist);
	if ( ist & TIOCM_DSR ) sdata->s_DSR=1;
	if ( ist & TIOCM_DTR ) sdata->s_DTR=1;
	if ( ist & TIOCM_RTS ) sdata->s_RTS=1;
	if ( ist & TIOCM_CTS ) sdata->s_CTS=1;
	if ( ist & TIOCM_CAR ) sdata->s_DCD=1;
	if ( ist & TIOCM_RNG ) sdata->s_RNG=1;
}

void CSerialPort_Event(long param)
{
	serialevent *eparam;
	eparam=(serialevent*)param;
	switch(eparam->nevent)
	{
		case 0: GB.Raise(eparam->obj,Serial_DTR,1,GB_T_BOOLEAN,eparam->value); break;
    		case 1: GB.Raise(eparam->obj,Serial_DSR,1,GB_T_BOOLEAN,eparam->value); break;
    		case 2: GB.Raise(eparam->obj,Serial_RTS,1,GB_T_BOOLEAN,eparam->value); break;
    		case 3: GB.Raise(eparam->obj,Serial_CTS,1,GB_T_BOOLEAN,eparam->value); break;
    		case 4: GB.Raise(eparam->obj,Serial_DCD,1,GB_T_BOOLEAN,eparam->value); break;
    		case 5: GB.Raise(eparam->obj,Serial_RNG,1,GB_T_BOOLEAN,eparam->value); break;
	}
	GB.Unref((void**)&eparam->obj);
}

void CSerialPort_CallBack(long lParam)
{
	int position=0;
	serialsignal newstatus;
	struct pollfd mypoll;
	int numpoll;
	CSERIALPORT *mythis;
	/*	Just sleeping a bit to reduce CPU waste	*/
	struct timespec mywait;
	mywait.tv_sec=0;
	mywait.tv_nsec=1000000;
	nanosleep(&mywait,NULL);

	if ((position=search_by_integer(ser_portwatch,ser_numwatch,(int)lParam))==-1) return;

	mythis=(CSERIALPORT*)ser_objwatch[position];
	/* Serial port signals status */
	Serial_Signal_Status(&newstatus,lParam);
	mythis->e_DTR.value=newstatus.s_DTR;
	mythis->e_DSR.value=newstatus.s_DSR;
	mythis->e_RTS.value=newstatus.s_RTS;
	mythis->e_CTS.value=newstatus.s_CTS;
	mythis->e_DCD.value=newstatus.s_DCD;
	mythis->e_RNG.value=newstatus.s_RNG;
	if (mythis->ser_status.s_DTR != newstatus.s_DTR ) {
		mythis->ser_status.s_DTR = newstatus.s_DTR;
		GB.Ref(mythis);
		GB.Post(CSerialPort_Event,(long)&mythis->e_DTR);
	}
	if (mythis->ser_status.s_DSR != newstatus.s_DSR ) {
		mythis->ser_status.s_DSR = newstatus.s_DSR;
		GB.Ref(mythis);
		GB.Post(CSerialPort_Event,(long)&mythis->e_DSR);
	}
	if (mythis->ser_status.s_RTS != newstatus.s_RTS ) {
		mythis->ser_status.s_RTS = newstatus.s_RTS;
		GB.Ref(mythis);
		GB.Post(CSerialPort_Event,(long)&mythis->e_RTS);
	}
	if (mythis->ser_status.s_CTS != newstatus.s_CTS ) {
		mythis->ser_status.s_CTS = newstatus.s_CTS;
		GB.Ref(mythis);
		GB.Post(CSerialPort_Event,(long)&mythis->e_CTS);
	}
	if (mythis->ser_status.s_DCD != newstatus.s_DCD ) {
		mythis->ser_status.s_DCD = newstatus.s_DCD;
		GB.Ref(mythis);
		GB.Post(CSerialPort_Event,(long)&mythis->e_DCD);
	}
	if (mythis->ser_status.s_RNG != newstatus.s_RNG ) {
		mythis->ser_status.s_RNG = newstatus.s_RNG;
		GB.Ref(mythis);
		GB.Post(CSerialPort_Event,(long)&mythis->e_RNG);
	}

	/* Data Available */
	mypoll.fd=lParam;
	mypoll.events=POLLIN;
	mypoll.revents=0;
	numpoll=poll(&mypoll,1,0);
	if (numpoll)
	{
		GB.Raise((void*)mythis,Serial_Read,0);
	}


}


void CSerialPort_AssignCallBack(long t_obj,int t_port)
{
	int position=0;
	CSERIALPORT *mythis;

	mythis=(CSERIALPORT*)t_obj;
	position=search_by_integer(ser_objwatch,ser_numwatch,t_obj);
	if (position>=0)
		GB.Watch (t_port , GB_WATCH_NONE , (void *)CSerialPort_CallBack,0);
	if (position<0)
	{
		position=ser_numwatch++;
		Alloc_CallBack_Pointers(ser_numwatch,&ser_objwatch,&ser_portwatch);
	}
	ser_objwatch[position]=t_obj;
	ser_portwatch[position]=t_port;
	GB.Watch (t_port , GB_WATCH_WRITE , (void *)CSerialPort_CallBack,0);
}
void CSerialPort_FreeCallBack(long t_obj)
{
	int myloop;
	int position;

	position=search_by_integer(ser_objwatch,ser_numwatch,t_obj);
	if ( position==-1 ) return;
	GB.Watch (ser_portwatch[position] , GB_WATCH_NONE , (void *)CSerialPort_CallBack,0);
	for ( myloop=position;myloop < (ser_numwatch-1);myloop++ )
	{
		ser_objwatch[myloop]=ser_objwatch[myloop+1];
		ser_portwatch[myloop]=ser_portwatch[myloop+1];
	}
	ser_numwatch--;
	Alloc_CallBack_Pointers(ser_numwatch,&ser_objwatch,&ser_portwatch);
}
////////////////////////////////////////////////////////////////////////////////////
//
//
//**********************************************************************************
//===================================================================================
//##################STREAM RELATED FUNCTIONS#########################################
//===================================================================================
//***********************************************************************************/
//
//XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
//
/////////////////////////////////////////////////////////////////////////////////////

int CSerialPort_stream_open(GB_STREAM *stream, const char *path, int mode, void *data)
{
	return -1; /* not allowed */
}
int CSerialPort_stream_seek(GB_STREAM *stream, int64_t pos, int whence)
{
	return -1; /* not allowed */
}
int CSerialPort_stream_tell(GB_STREAM *stream, int64_t *pos)
{
	return -1; /* not allowed */
}
int CSerialPort_stream_flush(GB_STREAM *stream)
{
	return 0; /* OK */
}
int CSerialPort_stream_handle(GB_STREAM *stream)
{
	return 0; /* OK */
}
int CSerialPort_stream_close(GB_STREAM *stream)
{
	void *_object = STREAM_TO_SERIALPORT(stream);

	if (!_object) return -1;	
	
	if (THIS->iStatus)
	{
		CSerialPort_FreeCallBack((long)THIS);
		THIS->stream.desc=NULL;
		CloseSerialPort(THIS->Port,&THIS->oldtio);
		THIS->iStatus=0;
	}
	return 0;
}
int CSerialPort_stream_lof(GB_STREAM *stream, int64_t *len)
{
	void *_object = STREAM_TO_SERIALPORT(stream);
	int bytes;

	*len=0;
	if (!_object) return -1;
	
	if (ioctl(THIS->Port,FIONREAD,&bytes)) return -1;
	*len=bytes;
	return 0;
}
int CSerialPort_stream_eof(GB_STREAM *stream)
{
	void *_object = STREAM_TO_SERIALPORT(stream);
	int bytes;

	if (!_object) return -1;
	
	if (ioctl(THIS->Port,FIONREAD,&bytes)) return -1;
	if (!bytes) return -1;
	return 0;
}

int CSerialPort_stream_read(GB_STREAM *stream, char *buffer, int len)
{
	void *_object = STREAM_TO_SERIALPORT(stream);
	int npos=-1;
	int NoBlock=0;
	int bytes;

  	if (!_object) return -1;
	
	if (ioctl(THIS->Port,FIONREAD,&bytes)) return -1;
	if (bytes < len) return -1;
	ioctl(THIS->Port,FIONBIO,&NoBlock);
	npos=read(THIS->Port,(void*)buffer,len);
	NoBlock++;
  	ioctl(THIS->Port,FIONBIO,&NoBlock);
  	if (npos==len) return 0;
  	return -1;
}

int CSerialPort_stream_write(GB_STREAM *stream, char *buffer, int len)
{
	void *_object = STREAM_TO_SERIALPORT(stream);
	int npos=-1;
	int NoBlock=0;

	if (!_object) return -1;
	
	ioctl(THIS->Port,FIONBIO,&NoBlock);
	npos=write(THIS->Port,(void*)buffer,len);
	NoBlock++;
	ioctl(THIS->Port,FIONBIO,&NoBlock);
	if (npos>=0) return 0;
	return -1;
}
/************************************************************************
##########################################################################
				GAMBAS OBJECT PUBLIC INTERFACE
##########################################################################
**************************************************************************/
/********************************************************************
Returns current Status of the port (0->Closed, 1 --> Opened)
*********************************************************************/
BEGIN_PROPERTY ( CSERIALPORT_Status )

  GB.ReturnInteger(THIS->iStatus);

END_PROPERTY



/*************************************
 Data Set Ready
 *************************************/
BEGIN_PROPERTY ( CSERIALPORT_DSR )

  if ( !THIS->iStatus )
  {
  	GB.ReturnBoolean(0);
	return;
  }

  Serial_Signal_Status(&THIS->ser_status,THIS->Port);
  GB.ReturnBoolean(THIS->ser_status.s_DSR);

END_PROPERTY

/***************************************
 Data Transmission Ready
 ***************************************/
BEGIN_PROPERTY ( CSERIALPORT_DTR )

	int ist;
	if (READ_PROPERTY)
	{
		if ( !THIS->iStatus )
		{
			GB.ReturnBoolean(0);
			return;
		}
		Serial_Signal_Status(&THIS->ser_status,THIS->Port);
		GB.ReturnBoolean(THIS->ser_status.s_DTR);
		return;
	}
  
	if (!THIS->iStatus )
	{
		GB.Error ("Port is closed");
		return;
	}
	ioctl(THIS->Port,TIOCMGET,&ist);
	if (!VPROP(GB_BOOLEAN))
		ist &= ~TIOCM_DTR;
	else
		ist = ist | TIOCM_DTR;
	ioctl(THIS->Port,TIOCMSET,&ist);


END_PROPERTY

/****************************************
 Ready to send
 ****************************************/
BEGIN_PROPERTY ( CSERIALPORT_RTS )

	int ist;
	if (READ_PROPERTY)
	{
		if ( !THIS->iStatus )
		{
			GB.ReturnBoolean(0);
			return;
		}
	
		Serial_Signal_Status(&THIS->ser_status,THIS->Port);
		GB.ReturnBoolean(THIS->ser_status.s_RTS);
		return;
	}
  
  	if (!THIS->iStatus )
	{
		GB.Error ("Port is closed");
		return;
	}
	ioctl(THIS->Port,TIOCMGET,&ist);
	if(!VPROP(GB_BOOLEAN))
		ist &= ~TIOCM_RTS;
	else
		ist = ist | TIOCM_RTS;
	ioctl(THIS->Port,TIOCMSET,&ist);


END_PROPERTY

/****************************************
 Clear to send
 ****************************************/
BEGIN_PROPERTY ( CSERIALPORT_CTS )

  if ( !THIS->iStatus )
  {
  	GB.ReturnBoolean(0);
	return;
  }

  Serial_Signal_Status(&THIS->ser_status,THIS->Port);
  GB.ReturnBoolean(THIS->ser_status.s_CTS);

END_PROPERTY

/***************************************
 Data Carrier Detect
 ***************************************/
BEGIN_PROPERTY ( CSERIALPORT_DCD )

  if ( !THIS->iStatus )
  {
  	GB.ReturnBoolean(0);
	return;
  }

  Serial_Signal_Status(&THIS->ser_status,THIS->Port);
  GB.ReturnBoolean(THIS->ser_status.s_DCD);

END_PROPERTY

/***************************************
 Ring
 ***************************************/
BEGIN_PROPERTY ( CSERIALPORT_RNG )

  if ( !THIS->iStatus )
  {
  	GB.ReturnBoolean(0);
	return;
  }

  Serial_Signal_Status(&THIS->ser_status,THIS->Port);
  GB.ReturnBoolean(THIS->ser_status.s_RNG);

END_PROPERTY

/*********************************************************************
 Gets / Sets serial port name (on Linux /dev/ttyS0, etc)
**********************************************************************/
BEGIN_PROPERTY ( CSERIALPORT_Port )

	if (READ_PROPERTY)
	{
		GB.ReturnString(THIS->sPort);
		return;
	}
  	if ( THIS->iStatus )
	{
		GB.Error("Current port must be closed first");
		return;
	}
	GB.StoreString(PROP(GB_STRING), &THIS->sPort);

END_PROPERTY


/*******************************************************************************
 FlowControl : 1->CRTSCTS , 2-> XON/XOFF , 3-> XON/OFF plus CRTSCTS, 4 --> NONE
********************************************************************************/
BEGIN_PROPERTY ( CSERIALPORT_FlowControl )

	if (READ_PROPERTY)
	{
		GB.ReturnInteger(THIS->iFlow);
		return;
	}
  	if ( THIS->iStatus )
	{
		GB.Error("Current port must be closed first");
		return;
	}
	if ( (VPROP(GB_INTEGER)<0) || (VPROP(GB_INTEGER)>3) )
	{
		GB.Error("Invalid flow control value");
		return;
	}
	THIS->iFlow=VPROP(GB_INTEGER);


END_PROPERTY

/*********************************************************************
 Gets / Sets serial parity (E,O,N)
**********************************************************************/
BEGIN_PROPERTY ( CSERIALPORT_Parity )

  int parity;
  
  if (READ_PROPERTY)
  {
	GB.ReturnInteger(THIS->Parity);
  }
  else
  {
  	if ( THIS->iStatus )
		GB.Error("Current port must be closed first");
	else
	{
          parity = VPROP(GB_INTEGER);
          if (parity < 0 || parity > 2)
            GB.Error("Invalid parity");
          else
            THIS->Parity = parity;
                /*
		GB.StoreString(PROP(GB_STRING), &tmpstring);
		if ( GB.StringLength (tmpstring) != 1 )
		{
			myok=0;
		}
		else
		{
		        if (ConvertParity(tmpstring[0]) == -1) myok=0;
		}
		GB.FreeString(&tmpstring);
		if (!myok)
			GB.Error("Invalid parity value.");
		else
			GB.StoreString(PROP(GB_STRING), &THIS->Parity);
                */
	}
  }

END_PROPERTY

/*********************************************************************
 Gets / Sets serial port Speed
**********************************************************************/
BEGIN_PROPERTY ( CSERIALPORT_Speed )

	int myok=1;
	if (READ_PROPERTY)
	{
		GB.ReturnInteger(THIS->Speed);
		return;
	}
  	if ( THIS->iStatus ){ GB.Error("Current port must be closed first");return; }
	if ( !VPROP(GB_INTEGER) ) myok=0;
	if ( ConvertBaudRate(VPROP(GB_INTEGER)) == -1) myok=0;
	if (!myok)
		GB.Error("Invalid speed value");
	else
		THIS->Speed=VPROP(GB_INTEGER);



END_PROPERTY

/*********************************************************************
 Gets / Sets serial port Data Bits
**********************************************************************/
BEGIN_PROPERTY ( CSERIALPORT_DataBits )

	int myok=1;
	if (READ_PROPERTY)
	{
		GB.ReturnInteger(THIS->DataBits);
		return;
	}
  	if ( THIS->iStatus ) { GB.Error("Current port must be closed first"); return; }
	if ( ConvertDataBits(VPROP(GB_INTEGER)) == -1) myok=0;
	if (!myok)
		GB.Error("Invalid data bits value");
	else
		THIS->DataBits=VPROP(GB_INTEGER);



END_PROPERTY

/*********************************************************************
 Gets / Sets serial port Stop Bits
**********************************************************************/
BEGIN_PROPERTY ( CSERIALPORT_StopBits )

	int myok=1;
	if (READ_PROPERTY)
	{
		GB.ReturnInteger(THIS->StopBits);
		return;
	}
  	if ( THIS->iStatus ){GB.Error("Current port must be closed first");return;}
	if ( ConvertStopBits(VPROP(GB_INTEGER)) == -1) myok=0;
	if (!myok)
		GB.Error("Invalid stop bits value");
	else
		THIS->StopBits=VPROP(GB_INTEGER);



END_PROPERTY

/*************************************************
 Gambas object "Constructor"
 *************************************************/
BEGIN_METHOD_VOID(CSERIALPORT_new)

	THIS->Port=0;
	THIS->iStatus=0;
	THIS->sPort=NULL;
	GB.NewString(&THIS->sPort,"/dev/ttyS0",10);
	THIS->Speed=19200;
	THIS->Parity=0;
	THIS->DataBits=8;
	THIS->StopBits=1;
	THIS->iFlow=1;

END_METHOD

/*************************************************
 Gambas object "Destructor"
 *************************************************/
BEGIN_METHOD_VOID(CSERIALPORT_free)

	if (THIS->iStatus)
	{
		CSerialPort_FreeCallBack((long)THIS);
		GB.Stream.Init(&THIS->stream,-1);
		CloseSerialPort(THIS->Port,&THIS->oldtio);
		THIS->iStatus=0;
	}
	GB.FreeString(&THIS->sPort);

END_METHOD

/*************************************************
 To open the port
 *************************************************/
BEGIN_METHOD_VOID(CSERIALPORT_Open)

	void *stream;
	int err;
	char buffer[8];

	if (THIS->iStatus)
	{
		GB.Error("Port is already opened");
		return;
	}
	if ((err = OpenSerialPort(&THIS->Port, THIS->iFlow, &THIS->oldtio, THIS->sPort, THIS->Speed, THIS->Parity, THIS->DataBits, THIS->StopBits)))
	{
		sprintf(buffer, "#%d", err);
		GB.Error("Cannot open serial port (&1)", buffer);
		return;
	}
	THIS->e_DTR.nevent=0;
	THIS->e_DSR.nevent=1;
	THIS->e_RTS.nevent=2;
	THIS->e_CTS.nevent=3;
	THIS->e_DCD.nevent=4;
	THIS->e_RNG.nevent=5;
	THIS->e_DTR.obj=THIS;
	THIS->e_DSR.obj=THIS;
	THIS->e_RTS.obj=THIS;
	THIS->e_CTS.obj=THIS;
	THIS->e_DCD.obj=THIS;
	THIS->e_RNG.obj=THIS;
	Serial_Signal_Status(&THIS->ser_status,THIS->Port);
	CSerialPort_AssignCallBack((long)THIS,THIS->Port);
	//CSerialPort_stream_init(&THIS->stream,THIS->Port);
	THIS->stream.desc=&SerialStream;
	THIS->iStatus=1;

	stream = &THIS->stream;
	STREAM_TO_SERIALPORT(stream) = THIS;

END_METHOD



/***************************************************************
 Here we declare the public interface of SerialPort class
 ***************************************************************/
GB_DESC CSerialPortDesc[] =
{

  GB_DECLARE("SerialPort", sizeof(CSERIALPORT)),

  GB_INHERITS("Stream"),

  GB_CONSTANT("None", "i", 0),
  
  GB_CONSTANT("Hardware", "i", 1),
  GB_CONSTANT("Software", "i", 2),
  GB_CONSTANT("Both", "i", 3),

  GB_CONSTANT("Even", "i", 1),
  GB_CONSTANT("Odd", "i", 2),
  
  GB_CONSTANT("Bits1", "i", 1),
  GB_CONSTANT("Bits2", "i", 2),
  
  GB_CONSTANT("Bits5", "i", 5),
  GB_CONSTANT("Bits6", "i", 6),
  GB_CONSTANT("Bits7", "i", 7),
  GB_CONSTANT("Bits8", "i", 8),
  
  GB_EVENT("Read", NULL, NULL, &Serial_Read),
  GB_EVENT("DTRChange", NULL, "(CurrentValue)b", &Serial_DTR),
  GB_EVENT("DSRChange", NULL, "(CurrentValue)b", &Serial_DSR),
  GB_EVENT("RTSChange", NULL, "(CurrentValue)b", &Serial_RTS),
  GB_EVENT("CTSChange", NULL, "(CurrentValue)b", &Serial_CTS),
  GB_EVENT("DCDChange", NULL, "(CurrentValue)b", &Serial_DCD),
  GB_EVENT("RNGChange", NULL, "(CurrentValue)b", &Serial_RNG),

  GB_METHOD("_new", NULL, CSERIALPORT_new, NULL),
  GB_METHOD("_free", NULL, CSERIALPORT_free, NULL),
  GB_METHOD("Open", NULL, CSERIALPORT_Open, NULL),
  //GB_METHOD("Close", NULL, CSERIALPORT_Close, NULL),

  GB_PROPERTY("FlowControl","i",CSERIALPORT_FlowControl),
  GB_PROPERTY("PortName", "s", CSERIALPORT_Port),
  GB_PROPERTY("Parity", "i", CSERIALPORT_Parity),
  GB_PROPERTY("Speed", "i", CSERIALPORT_Speed),
  
  GB_PROPERTY("DataBits", "i", CSERIALPORT_DataBits),
  GB_PROPERTY("StopBits", "i", CSERIALPORT_StopBits),
  GB_PROPERTY("DTR", "b", CSERIALPORT_DTR),
  GB_PROPERTY("RTS", "b", CSERIALPORT_RTS),
  GB_PROPERTY_READ("Status", "i", CSERIALPORT_Status),
  GB_PROPERTY_READ("DSR", "b", CSERIALPORT_DSR),
  GB_PROPERTY_READ("CTS", "b", CSERIALPORT_CTS),
  GB_PROPERTY_READ("DCD", "b", CSERIALPORT_DCD),
  GB_PROPERTY_READ("RNG", "b", CSERIALPORT_RNG),
  
  GB_CONSTANT("_Properties", "s", "FlowControl{SerialPort.None;Hardware;Software;Both}=Hardware,PortName,Parity{SerialPort.None;Even;Odd}=None,Speed=19200,DataBits{SerialPort.Bits5;Bits6;Bits7;Bits8}=Bits8,StopBits{SerialPort.Bits1;Bits2}=Bits1"),
  GB_CONSTANT("_DefaultEvent", "s", "Read"),

  GB_END_DECLARE
};


