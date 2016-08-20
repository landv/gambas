/***************************************************************************

  gbx_stream_memory.c

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

#include "gbx_stream.h"


static int stream_open(STREAM *stream, const char *path, int mode)
{
	stream->memory.addr = (char *)path;

	if (stream->memory.addr == NULL)
	{
		stream->type = NULL;
		THROW(E_ARG);
	}
	
  stream->memory.pos = 0;

	stream->common.available_now = TRUE;

  return FALSE;
}


static int stream_close(STREAM *stream)
{
  /*if (munmap(stream->memory.addr, stream->memory.size) != 0)
    return TRUE;*/

  stream->memory.addr = NULL;
  //stream->memory.size = 0;
  return FALSE;
}


static int stream_read(STREAM *stream, char *buffer, int len)
{
	/*if ((stream->common.mode & ST_READ) == 0)
		THROW(E_ACCESS);*/
	
  CHECK_enter();
  
  if (sigsetjmp(CHECK_jump, TRUE) == 0)
    memmove(buffer, stream->memory.addr + stream->memory.pos, len);

  CHECK_leave();  

  if (CHECK_got_error())
  {
    errno = EIO;  
    return TRUE;
  }
  else
  {
    stream->memory.pos += len;
    STREAM_eff_read = len;
    return FALSE;
  }
}

#define stream_getchar NULL

static int stream_write(STREAM *stream, char *buffer, int len)
{
	if ((stream->common.mode & ST_WRITE) == 0)
		THROW(E_ACCESS);
	
  CHECK_enter();
  
  if (sigsetjmp(CHECK_jump, TRUE) == 0)
    memmove(stream->memory.addr + stream->memory.pos, buffer, len);

  CHECK_leave();  
  
  if (CHECK_got_error())
  {
    errno = EIO;  
    return TRUE;
  }
  else
  {
    stream->memory.pos += len;
    return FALSE;
  }
}


static int stream_seek(STREAM *stream, int64_t pos, int whence)
{
  int64_t new_pos;

  switch(whence)
  {
    case SEEK_SET:
      new_pos = pos;
      break;

    case SEEK_CUR:
      new_pos = stream->memory.pos + pos;
      break;

    case SEEK_END:
      return TRUE;

    default:
      return TRUE;
  }

  if (new_pos < 0)
    return TRUE;

  stream->memory.pos = new_pos;
  return FALSE;
}


static int stream_tell(STREAM *stream, int64_t *pos)
{
  *pos = (int64_t)stream->memory.pos;
  return FALSE;
}


static int stream_flush(STREAM *stream)
{
  return FALSE;
}


static int stream_eof(STREAM *stream)
{
  //return (stream->memory.pos >= stream->memory.size);
  return FALSE;
}


static int stream_lof(STREAM *stream, int64_t *len)
{
  //*len = stream->memory.size;
  //return FALSE;
  *len = 0;
  return TRUE;
}


static int stream_handle(STREAM *stream)
{
  return -1;
}


DECLARE_STREAM(STREAM_memory);
