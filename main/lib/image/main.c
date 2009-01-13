/***************************************************************************

  main.c

  (c) 2008 Benoît Minisini <gambas@users.sourceforge.net>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 1, or (at your option)
  any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

***************************************************************************/

#define __MAIN_C

#include <unistd.h>
#include <stdio.h>

#include "CImageStat.h"
#include "main.h"

GB_INTERFACE GB EXPORT;


int stream_seek(IMAGE_STREAM *stream, int pos, int whence)
{
	switch (whence)
	{
		case SEEK_CUR:
			if ((stream->pos + pos) >= stream->len)
				return 1;
			if ((stream->pos + pos) < 0)
				return 1;
			stream->pos += pos;
			return 0;
		
		case SEEK_SET:
			if (pos < 0 || pos >= stream->len)
				return 1;
			stream->pos = pos;
			return 0;
		
		default:
			return 1;
	}
}

int stream_read(IMAGE_STREAM *stream, void *addr, int len)
{
	int lmax = stream->len - stream->pos;
	
	if (len > lmax)
		len = lmax;
		
	memcpy(addr, stream->addr + stream->pos, len);
	stream->pos += len;
	return len;
}


int stream_getc(IMAGE_STREAM *stream)
{
	if (stream->pos >= stream->len)
		return EOF;
		
	return (unsigned char)stream->addr[stream->pos++];
}


GB_DESC *GB_CLASSES[] EXPORT =
{
	CImageStatDesc,
  NULL
};

int EXPORT GB_INIT(void)
{
	return FALSE;
}

void EXPORT GB_EXIT()
{
}
