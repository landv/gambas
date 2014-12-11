/***************************************************************************

  gbx_stream_process.c

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

#include "gbx_c_process.h"

#include "gbx_stream.h"


#define FDR ((CPROCESS *)stream->process.process)->out
#define FDW ((CPROCESS *)stream->process.process)->in


static int stream_open(STREAM *stream, const char *path, int mode, CPROCESS *process)
{
	stream->process.process = process;
	STREAM_blocking(stream, FALSE);
  return FALSE;
}


static int stream_close(STREAM *stream)
{
  if (FDW >= 0)
  {
    close(FDW);
    FDW = -1;
    return TRUE;
  }

  return FALSE;
}


static int stream_read(STREAM *stream, char *buffer, int len)
{
	return STREAM_read_direct(FDR, buffer, len);
}

#define stream_getchar NULL

static int stream_write(STREAM *stream, char *buffer, int len)
{
	return STREAM_write_direct(FDW, buffer, len);
}


static int stream_seek(STREAM *stream, int64_t pos, int whence)
{
  if (FDR < 0)
    return TRUE;

  return (lseek(FDR, pos, whence) < 0);
}


static int stream_tell(STREAM *stream, int64_t *pos)
{
  if (FDR < 0)
    return TRUE;

  *pos = lseek(FDR, 0, SEEK_CUR);
  return (*pos < 0);
}


static int stream_flush(STREAM *stream)
{
  return FALSE;
}


/*static int stream_eof(STREAM *stream)
{
  int ilen;

  if (STREAM_get_readable(FD, &ilen))
    return TRUE;

  return (ilen == 0);
}*/
#define stream_eof NULL


/*static int stream_lof(STREAM *stream, int64_t *len)
{
  *len = 0; //stream->direct.size;
  return FALSE;
}*/
#define stream_lof NULL


static int stream_handle(STREAM *stream)
{
  return FDR;
}


DECLARE_STREAM(STREAM_process);

