/***************************************************************************

  gbx_stream_pipe.c

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
#include "gb_error.h"
#include "gbx_value.h"
#include "gb_limit.h"

#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>

#include "gbx_stream.h"


#define FD (stream->direct.fd)


static int stream_open(STREAM *stream, const char *path, int mode)
{
  int fd;
  int fmode;

	if (mkfifo(path, 0666) != 0)
	{
		if (errno != EEXIST)
			return TRUE;
	}

  fmode = 0;

  switch (mode & ST_MODE)
  {
    case ST_READ: fmode |= O_RDONLY; break;
    case ST_WRITE: fmode |= O_WRONLY; break;
    case ST_READ_WRITE: fmode |= O_RDWR; break;
    default: fmode |= O_RDONLY;
  }

  fd = open(path, fmode);
  if (fd < 0)
    return TRUE;

  stream->direct.size = 0;

  FD = fd;
  return FALSE;
}


static int stream_close(STREAM *stream)
{
  if (close(FD) < 0)
    return TRUE;

  FD = -1;
  return FALSE;
}


static int stream_read(STREAM *stream, char *buffer, int len)
{
	return STREAM_read_direct(FD, buffer, len);
}

#define stream_getchar NULL


static int stream_write(STREAM *stream, char *buffer, int len)
{
	return STREAM_write_direct(FD, buffer, len);
}


static int stream_seek(STREAM *stream, int64_t pos, int whence)
{
  return TRUE;
}


static int stream_tell(STREAM *stream, int64_t *pos)
{
	return TRUE;
}


static int stream_flush(STREAM *stream)
{
  return FALSE;
}


#define stream_eof NULL

#define stream_lof NULL


static int stream_handle(STREAM *stream)
{
  return FD;
}


DECLARE_STREAM(STREAM_pipe);

