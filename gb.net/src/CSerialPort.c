/***************************************************************************

	CSerialPort.c

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

#define __CSERIALPORT_C

#include <stdio.h>
#include <termios.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/poll.h>
#include <time.h>
#include <errno.h>

#ifdef __CYGWIN__
/* Cygwin defines FIONREAD in <sys/socket.h>. */
#include <sys/socket.h>
/* TIOCOUTQ is not implemented on Cygwin */
#define TIOCOUTQ ((unsigned int) -1)
#endif /* __CYGWIN__ */

#ifndef TIOCINQ
#define TIOCINQ FIONREAD
#endif

#include "main.h"
#include "tools.h"

#include "CSerialPort.h"

#define MAX_SERIAL_BUFFER_SIZE 65536


GB_STREAM_DESC SerialStream = 
{
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

DECLARE_EVENT(EVENT_Read);
DECLARE_EVENT(EVENT_DTR);
DECLARE_EVENT(EVENT_DSR);
DECLARE_EVENT(EVENT_RTS);
DECLARE_EVENT(EVENT_CTS);
DECLARE_EVENT(EVENT_DCD);
DECLARE_EVENT(EVENT_RNG);

static SERIAL_SIGNAL get_signals(CSERIALPORT *_object)
{
	int ist = 0;
	SERIAL_SIGNAL signals = { 0 };
	
	ioctl(THIS->port, TIOCMGET, &ist);
	
	signals.DSR = (ist & TIOCM_DSR) != 0;
	signals.DTR = (ist & TIOCM_DTR) != 0;
	signals.RTS = (ist & TIOCM_RTS) != 0;
	signals.CTS = (ist & TIOCM_CTS) != 0;
	signals.DCD = (ist & TIOCM_CAR) != 0;
	signals.RNG = (ist & TIOCM_RNG) != 0;
	
	return signals;
}

static void raise_event(CSERIALPORT *_object, intptr_t event)
{
	int val = 0;
	
	if (event == EVENT_DSR)
		val = THIS->signals.DSR;
	else if (event == EVENT_DTR)
		val = THIS->signals.DTR;
	else if (event == EVENT_RTS)
		val = THIS->signals.RTS;
	else if (event == EVENT_CTS)
		val = THIS->signals.CTS;
	else if (event == EVENT_DCD)
		val = THIS->signals.DCD;
	else if (event == EVENT_RNG)
		val = THIS->signals.RNG;
	
	GB.Raise(THIS, (int)event, 1, GB_T_BOOLEAN, val);
	GB.Unref(POINTER(&_object));
}

#define CHECK_SIGNAL(_signal) \
if (THIS->signals._signal != new_signals._signal) \
{ \
	THIS->signals._signal = new_signals._signal; \
	GB.Ref(THIS); \
	GB.Post2(raise_event, (intptr_t)THIS, (intptr_t)EVENT_##_signal); \
}
	

static int cb_change(intptr_t _object)
{
	SERIAL_SIGNAL new_signals;
	//struct timespec mywait;

	// Just sleeping a bit to reduce CPU waste
	//mywait.tv_sec = 0;
	//mywait.tv_nsec = 1000000; // 1 ms
	//nanosleep(&mywait, NULL);

	/* Serial port signals status */
	new_signals = get_signals(THIS);
	
	CHECK_SIGNAL(DSR);
	CHECK_SIGNAL(DTR);
	CHECK_SIGNAL(RTS);
	CHECK_SIGNAL(CTS);
	CHECK_SIGNAL(DCD);
	CHECK_SIGNAL(RNG);
	
	return FALSE;
}

static void cb_read(int fd, int type, CSERIALPORT *_object)
{
	GB.Raise(THIS, EVENT_Read, 0);
}

static void assign_callback(CSERIALPORT *_object, int polling)
{
	int port = THIS->port;

	if (GB.CanRaise(THIS, EVENT_Read))
		GB.Watch(port, GB_WATCH_READ, (void *)cb_read, (intptr_t)THIS);
	
	if (GB.CanRaise(THIS, EVENT_DTR)
			|| GB.CanRaise(THIS, EVENT_CTS)
			|| GB.CanRaise(THIS, EVENT_DCD)
			|| GB.CanRaise(THIS, EVENT_DSR)
			|| GB.CanRaise(THIS, EVENT_RNG)
			|| GB.CanRaise(THIS, EVENT_RTS))
	{
		// Polling is 50ms by default
		GB.Every(polling, cb_change, (intptr_t)THIS);
	}
}

static void release_callback(CSERIALPORT *_object)
{
	GB.Watch(THIS->port, GB_WATCH_NONE, 0, 0);
}

static void close_serial_port(CSERIALPORT *_object)
{
	if (THIS->status)
	{
		release_callback(THIS);
		THIS->stream.desc = NULL;
		CloseSerialPort(THIS->port, &THIS->oldtio);
		THIS->status = 0;
	}
}

/****************************************************************************

	Stream implementation

****************************************************************************/

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
	void *_object = stream->tag;
	return tcdrain(THIS->port);
}

int CSerialPort_stream_handle(GB_STREAM *stream)
{
	void *_object = stream->tag;
	return THIS->port; /* OK */
}

int CSerialPort_stream_close(GB_STREAM *stream)
{
	void *_object = stream->tag;

	if (!_object) 
		return -1;
	
	close_serial_port(THIS);
	return 0;
}

int CSerialPort_stream_lof(GB_STREAM *stream, int64_t *len)
{
	void *_object = stream->tag;
	int bytes;

	*len=0;
	if (!_object) return -1;
	
	if (ioctl(THIS->port,FIONREAD,&bytes)) return -1;
	*len = bytes;
	return 0;
}

int CSerialPort_stream_eof(GB_STREAM *stream)
{
	void *_object = stream->tag;
	int bytes;

	if (!_object) return -1;
	
	if (ioctl(THIS->port,FIONREAD,&bytes)) return -1;
	if (!bytes) return -1;
	return 0;
}

int CSerialPort_stream_read(GB_STREAM *stream, char *buffer, int len)
{
	void *_object = stream->tag;
	int npos = -1;
	int no_block = 0;
	int bytes;

	if (!_object) 
		return -1;
	
	if (ioctl(THIS->port, FIONREAD, &bytes)) 
		return -1;
	
	if (bytes < len) 
		return -1;
	
	ioctl(THIS->port, FIONBIO, &no_block);
	
	npos = read(THIS->port, (void*)buffer, len);
	
	no_block++;
	ioctl(THIS->port, FIONBIO, &no_block);
	
	if (npos != len) 
		return -1;
	
	return 0;
}

int CSerialPort_stream_write(GB_STREAM *stream, char *buffer, int len)
{
	void *_object = stream->tag;
	int npos = -1;
	int no_block = 0;

	if (!_object) 
		return -1;
	
	ioctl(THIS->port, FIONBIO, &no_block);
	
	npos = write(THIS->port, (void*)buffer, len);
	
	no_block++;
	ioctl(THIS->port, FIONBIO, &no_block);
	
	if (npos < 0) 
		return -1;
	
	return 0;
}

static bool check_open(CSERIALPORT *_object)
{
	if (!THIS->status)
	{
		GB.Error("Port is closed");
		return TRUE;
	}
	else
		return FALSE;
}

static bool check_close(CSERIALPORT *_object)
{
	if (THIS->status)
	{
		GB.Error("Port must be closed first");
		return TRUE;
	}
	else
		return FALSE;
}

/****************************************************************************

	SerialPort

****************************************************************************/

BEGIN_PROPERTY(SerialPort_Status)

	GB.ReturnInteger(THIS->status);

END_PROPERTY

// Data Set Ready

BEGIN_PROPERTY(SerialPort_DSR)

	if (!THIS->status)
		GB.ReturnBoolean(0);
	else
	{
		THIS->signals = get_signals(THIS);
		GB.ReturnBoolean(THIS->signals.DSR);
	}

END_PROPERTY

// Data Transmission Ready

BEGIN_PROPERTY(SerialPort_DTR)

	int ist;

	if (READ_PROPERTY)
	{
		if (!THIS->status)
			GB.ReturnBoolean(0);
		else
		{
			THIS->signals = get_signals(THIS);
			GB.ReturnBoolean(THIS->signals.DTR);
		}
	}
	else
	{
		if (check_open(THIS))
			return;

		ioctl(THIS->port, TIOCMGET, &ist);
		if (!VPROP(GB_BOOLEAN))
			ist &= ~TIOCM_DTR;
		else
			ist = ist | TIOCM_DTR;
		ioctl(THIS->port, TIOCMSET, &ist);
	}

END_PROPERTY

// Ready to send

BEGIN_PROPERTY(SerialPort_RTS)

	int ist;
	
	if (READ_PROPERTY)
	{
		if (!THIS->status)
			GB.ReturnBoolean(0);
		else
		{
			THIS->signals = get_signals(THIS);
			GB.ReturnBoolean(THIS->signals.RTS);
		}
	}
	else
	{
		if (check_open(THIS))
			return;

		ioctl(THIS->port, TIOCMGET, &ist);
		if(!VPROP(GB_BOOLEAN))
			ist &= ~TIOCM_RTS;
		else
			ist = ist | TIOCM_RTS;
		ioctl(THIS->port, TIOCMSET, &ist);
	}

END_PROPERTY

// Clear to send

BEGIN_PROPERTY(SerialPort_CTS)

	if (!THIS->status)
		GB.ReturnBoolean(0);
	else
	{
		THIS->signals = get_signals(THIS);
		GB.ReturnBoolean(THIS->signals.CTS);
	}

END_PROPERTY

// Data Carrier Detect

BEGIN_PROPERTY(SerialPort_DCD)

	if (!THIS->status)
		GB.ReturnBoolean(0);
	else
	{
		THIS->signals = get_signals(THIS);
		GB.ReturnBoolean(THIS->signals.DCD);
	}

END_PROPERTY

// Ring

BEGIN_PROPERTY(SerialPort_RNG)

	if (!THIS->status)
		GB.ReturnBoolean(0);
	else
	{
		THIS->signals = get_signals(THIS);
		GB.ReturnBoolean(THIS->signals.RNG);
	}

END_PROPERTY

// Gets / Sets serial port name (on Linux /dev/ttyS0, etc)

BEGIN_PROPERTY(SerialPort_Port)

	if (READ_PROPERTY)
		GB.ReturnString(THIS->portName);
	else
	{
		if (check_close(THIS))
			return;
		GB.StoreString(PROP(GB_STRING), &THIS->portName);
	}

END_PROPERTY

// FlowControl : 1->CRTSCTS , 2-> XON/XOFF , 3-> XON/OFF plus CRTSCTS, 0 --> NONE

BEGIN_PROPERTY(SerialPort_FlowControl)

	int flow;

	if (READ_PROPERTY)
		GB.ReturnInteger(THIS->flow);
	else
	{
		if (check_close(THIS))
			return;
		
		flow = VPROP(GB_INTEGER);
		if (flow < 0 || flow > 3)
		{
			GB.Error("Invalid flow control value");
			return;
		}
		
		THIS->flow = VPROP(GB_INTEGER);
	}

END_PROPERTY

// Gets / Sets serial parity (E,O,N)

BEGIN_PROPERTY(SerialPort_Parity)

	int parity;
	
	if (READ_PROPERTY)
		GB.ReturnInteger(THIS->parity);
	else
	{
		if (check_close(THIS))
			return;

		parity = VPROP(GB_INTEGER);
		if (parity < 0 || parity > 2)
		{
			GB.Error("Invalid parity");
			return;
		}
		
		THIS->parity = parity;
	}

END_PROPERTY

// Gets / Sets serial port Speed

BEGIN_PROPERTY(SerialPort_Speed)

	int speed;
	
	if (READ_PROPERTY)
		GB.ReturnInteger(THIS->speed);
	else
	{
		if (check_close(THIS))
			return;
		
		speed = VPROP(GB_INTEGER);
		
		if (ConvertBaudRate(speed) == -1)
			GB.Error("Invalid speed value");
		else
			THIS->speed = speed;
	}

END_PROPERTY

// Gets / Sets serial port Data Bits

BEGIN_PROPERTY(SerialPort_DataBits)

	int value;
	
	if (READ_PROPERTY)
		GB.ReturnInteger(THIS->dataBits);
	else
	{
		if (check_close(THIS))
			return;
		
		value = VPROP(GB_INTEGER);
		
		if (ConvertDataBits(value) == -1)
			GB.Error("Invalid data bits value");
		else
			THIS->dataBits = value;
	}

END_PROPERTY

// Gets / Sets serial port Stop Bits

BEGIN_PROPERTY(SerialPort_StopBits)

	int value;
	
	if (READ_PROPERTY)
		GB.ReturnInteger(THIS->stopBits);
	else
	{
		if (check_close(THIS))
			return;
		
		value = VPROP(GB_INTEGER);
		
		if (ConvertStopBits(value) == -1)
			GB.Error("Invalid stop bits value");
		else
			THIS->stopBits = value;
	}

END_PROPERTY

// Gambas object "Constructor"

BEGIN_METHOD_VOID(SerialPort_new)

	THIS->portName = GB.NewZeroString("/dev/ttyS0");
	THIS->speed = 19200;
	THIS->parity = 0;
	THIS->dataBits = 8;
	THIS->stopBits = 1;
	THIS->flow = 1;

END_METHOD

// Gambas object "Destructor"

BEGIN_METHOD_VOID(SerialPort_free)

	close_serial_port(THIS);
	GB.FreeString(&THIS->portName);

END_METHOD

// To open the port

BEGIN_METHOD(SerialPort_Open, GB_INTEGER polling)

	int err;
	char buffer[8];
	int polling = VARGOPT(polling, 50);

	if (THIS->status)
	{
		GB.Error("Port is already opened");
		return;
	}
	
	if ((err = OpenSerialPort(&THIS->port, THIS->flow, &THIS->oldtio, THIS->portName, THIS->speed, THIS->parity, THIS->dataBits, THIS->stopBits)))
	{
		sprintf(buffer, "#%d", err);
		GB.Error("Cannot open serial port (&1)", buffer);
		return;
	}
	
	THIS->signals = get_signals(THIS);
	THIS->stream.desc = &SerialStream;
	THIS->stream.tag = THIS;
	assign_callback(THIS, polling);

	THIS->status = 1;

END_METHOD

BEGIN_PROPERTY(SerialPort_InputBufferSize)

	int ret = 0;

	if (THIS->status)
	{
		if (ioctl(THIS->port, TIOCINQ, &ret))
			GB.Error("Unable to read input buffer size: &1", strerror(errno));
	}

	GB.ReturnInteger(ret);

END_PROPERTY

BEGIN_PROPERTY(SerialPort_OutputBufferSize)

	int ret = 0;

	if (THIS->status)
	{
		if (ioctl(THIS->port, TIOCOUTQ, &ret))
			GB.Error("Unable to read output buffer size: &1", strerror(errno));
	}

	GB.ReturnInteger(ret);

END_PROPERTY

// Here we declare the public interface of SerialPort class

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
	
	GB_EVENT("Read", NULL, NULL, &EVENT_Read),
	GB_EVENT("DTRChange", NULL, "(CurrentValue)b", &EVENT_DTR),
	GB_EVENT("DSRChange", NULL, "(CurrentValue)b", &EVENT_DSR),
	GB_EVENT("RTSChange", NULL, "(CurrentValue)b", &EVENT_RTS),
	GB_EVENT("CTSChange", NULL, "(CurrentValue)b", &EVENT_CTS),
	GB_EVENT("DCDChange", NULL, "(CurrentValue)b", &EVENT_DCD),
	GB_EVENT("RNGChange", NULL, "(CurrentValue)b", &EVENT_RNG),

	GB_METHOD("_new", NULL, SerialPort_new, NULL),
	GB_METHOD("_free", NULL, SerialPort_free, NULL),
	GB_METHOD("Open", NULL, SerialPort_Open, "[(Polling)i]"),

	GB_PROPERTY("FlowControl","i",SerialPort_FlowControl),
	GB_PROPERTY("PortName", "s", SerialPort_Port),
	GB_PROPERTY("Parity", "i", SerialPort_Parity),
	GB_PROPERTY("Speed", "i", SerialPort_Speed),
	
	GB_PROPERTY("DataBits", "i", SerialPort_DataBits),
	GB_PROPERTY("StopBits", "i", SerialPort_StopBits),
	GB_PROPERTY("DTR", "b", SerialPort_DTR),
	GB_PROPERTY("RTS", "b", SerialPort_RTS),
	GB_PROPERTY_READ("Status", "i", SerialPort_Status),
	GB_PROPERTY_READ("DSR", "b", SerialPort_DSR),
	GB_PROPERTY_READ("CTS", "b", SerialPort_CTS),
	GB_PROPERTY_READ("DCD", "b", SerialPort_DCD),
	GB_PROPERTY_READ("RNG", "b", SerialPort_RNG),

	GB_PROPERTY_READ("InputBufferSize", "i", SerialPort_InputBufferSize),
	GB_PROPERTY_READ("OutputBufferSize", "i", SerialPort_OutputBufferSize),

	GB_CONSTANT("_IsControl", "b", TRUE),
	GB_CONSTANT("_IsVirtual", "b", TRUE),
	GB_CONSTANT("_Group", "s", "Network"),
	GB_CONSTANT("_Properties", "s", "FlowControl{SerialPort.None;Hardware;Software;Both}=Hardware,PortName,Parity{SerialPort.None;Even;Odd}=None,Speed=19200,DataBits{SerialPort.Bits5;Bits6;Bits7;Bits8}=Bits8,StopBits{SerialPort.Bits1;Bits2}=Bits1"),
	GB_CONSTANT("_DefaultEvent", "s", "Read"),

	GB_END_DECLARE
};


