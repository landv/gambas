/***************************************************************************

  gbx_stream_string.c

  (c) 2000-2013 Benoît Minisini <gambas@users.sourceforge.net>

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

#define __STREAM_IMPL_C

#include "gb_common.h"
#include "gb_common_check.h"
#include "gb_error.h"
#include "gbx_value.h"
#include "gb_limit.h"

#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>
#include <signal.h>
#include <setjmp.h>

#include "gbx_string.h"
#include "gbx_stream.h"


static int stream_open(STREAM *stream, const char *path, int mode)
{
	stream->string.buffer = NULL;
	stream->common.available_now = TRUE;
	stream->string.size = 0;
	stream->string.pos = 0;

  return FALSE;
}


static int stream_close(STREAM *stream)
{
	STRING_unref(&stream->string.buffer);
  return FALSE;
}


static int stream_read(STREAM *stream, char *buffer, int len)
{
  bool strip = FALSE;
  int max;

  max = stream->string.size - stream->string.pos;

  if (len > max)
  {
    strip = TRUE;
    len = max;
    errno = 0;
  }

  if (len > 0)
		memcpy(buffer, stream->string.buffer + stream->string.pos, len);

  stream->string.pos += len;
  STREAM_eff_read = len;

  return strip;
}

#define stream_getchar NULL

static int stream_write(STREAM *stream, char *buffer, int len)
{
	if ((stream->common.mode & ST_WRITE) == 0)
		THROW(E_ACCESS);

	stream->string.buffer = STRING_add(stream->string.buffer, buffer, len);

	stream->string.size += len;
	stream->string.pos = stream->string.size;

  return FALSE;
}


static int stream_seek(STREAM *stream, int64_t pos, int whence)
{
	int ipos = (int)pos;

	if (pos != (int64_t)ipos || ipos < 0 || ipos > stream->string.size)
		return TRUE;

	stream->string.pos = ipos;
	return FALSE;
}


static int stream_tell(STREAM *stream, int64_t *pos)
{
	*pos = (int64_t)stream->string.pos;
  return FALSE;
}


static int stream_flush(STREAM *stream)
{
  return FALSE;
}


static int stream_eof(STREAM *stream)
{
  return (stream->string.pos >= stream->string.size);
}


static int stream_lof(STREAM *stream, int64_t *len)
{
  *len = STRING_length(stream->string.buffer);
  return FALSE;
}


static int stream_handle(STREAM *stream)
{
  return -1;
}


DECLARE_STREAM(STREAM_string);
