/***************************************************************************

  CSerialPort.h

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

#ifndef __CSERIALPORT_H
#define __CSERIALPORT_H

#include "gambas.h"
#include <termios.h>

#ifndef __CSERIALPORT_C

extern GB_DESC CSerialPortDesc[];
extern GB_STREAM_DESC SerialStream;

#else

#define THIS ((CSERIALPORT *)_object)

#endif

typedef 
	struct
	{
		unsigned DSR : 1;
		unsigned DTR : 1;
		unsigned RTS : 1;
		unsigned CTS : 1;
		unsigned DCD : 1;
		unsigned RNG : 1;
	}
	SERIAL_SIGNAL;

typedef
	struct
	{
		GB_BASE ob;
		GB_STREAM stream;
		int port;
		int status;
		char *portName;
		int parity;
		int speed;
		int dataBits;
		int stopBits;
		int flow;
		int polling;
		SERIAL_SIGNAL signals;
		struct termios oldtio;
	}  
	CSERIALPORT;

int CSerialPort_stream_read(GB_STREAM *stream, char *buffer, int len);
int CSerialPort_stream_write(GB_STREAM *stream, char *buffer, int len);
int CSerialPort_stream_eof(GB_STREAM *stream);
int CSerialPort_stream_lof(GB_STREAM *stream, int64_t *len);
int CSerialPort_stream_open(GB_STREAM *stream, const char *path, int mode, void *data);
int CSerialPort_stream_seek(GB_STREAM *stream, int64_t pos, int whence);
int CSerialPort_stream_tell(GB_STREAM *stream, int64_t *pos);
int CSerialPort_stream_flush(GB_STREAM *stream);
int CSerialPort_stream_close(GB_STREAM *stream);
int CSerialPort_stream_handle(GB_STREAM *stream);

#endif
