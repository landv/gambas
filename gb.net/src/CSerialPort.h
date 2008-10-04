/***************************************************************************

  CSerialPort.h

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

GB_STREAM_DESC SerialStream;

typedef struct
{
	int nevent;
	int value;
	void *obj;
} serialevent;

typedef struct
{
	int s_DSR;
	int s_DTR;
	int s_RTS;
	int s_CTS;
	int s_DCD;
	int s_RNG;

} serialsignal;

typedef  struct
{
	GB_BASE ob;
	GB_STREAM stream;
	int Port;
	int iStatus;
	char *sPort;
	int Parity;
	int Speed;
	int DataBits;
	int StopBits;
	int iFlow;
	serialevent e_DTR;
	serialevent e_DSR;
	serialevent e_RTS;
	serialevent e_CTS;
	serialevent e_DCD;
	serialevent e_RNG;
	serialsignal ser_status;
	struct termios oldtio;
}  CSERIALPORT;

void CSerialPort_CallBack(long lParam);
void CSerialPort_AssignCallBack(long t_obj,int t_port);
void CSerialPort_FreeCallBack(long t_obj);
void Serial_Signal_Status(serialsignal *sdata,int iPort);

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
