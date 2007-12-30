/***************************************************************************

  stream_memory.c

  The memory stream management routines

  (c) 2000-2005 Beno�t Minisini <gambas@users.sourceforge.net>

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


#define FD (stream->buffer.file)

static int stream_open(STREAM *stream, const char *path, int mode)
{
  /*
  int fd;
  struct stat info;
  */

  /*
  if (path != NULL)
  {
    if ((mode & ST_MODE) != ST_READ)
      return TRUE;

    fd = open(path, O_RDONLY);
    if (fd < 0)
      return TRUE;

    if (fstat(fd, &info) < 0)
      return TRUE;
    else
      stream->memory.size = info.st_size;

    stream->memory.addr = mmap(NULL, stream->memory.size, PROT_READ, MAP_static, fd, (off_t)0);
    close(fd);
    if (stream->memory.addr == MAP_FAILED)
      return TRUE;
  }
  else
  */
  {
    if (stream->memory.addr == NULL)
      return TRUE;
  }

  stream->memory.pos = 0;

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


static int stream_read(STREAM *stream, char *buffer, long len)
{
  CHECK_enter();
  
  if (setjmp(CHECK_jump) == 0)
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


static int stream_write(STREAM *stream, char *buffer, long len)
{
  CHECK_enter();
  
  if (setjmp(CHECK_jump) == 0)
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


static int stream_seek(STREAM *stream, long long pos, int whence)
{
  long new_pos;

  switch(whence)
  {
    case SEEK_SET:
      new_pos = (long)pos;
      break;

    case SEEK_CUR:
      new_pos = stream->memory.pos + (long)pos;
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


static int stream_tell(STREAM *stream, long *pos)
{
  *pos = stream->memory.pos;
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


static int stream_lof(STREAM *stream, long long *len)
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
