/***************************************************************************

  stream_buffer.c

  The buffered stream management routines

  (c) 2000-2005 Benoît Minisini <gambas@users.sourceforge.net>

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
#include "gb_error.h"
#include "gbx_value.h"
#include "gb_limit.h"

#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>

#include "gbx_exec.h"
#include "gbx_stream.h"


#define FD (stream->buffer.file)


static int stream_open(STREAM *stream, const char *path, int mode)
{
  FILE *file;
  char *fmode;
  struct stat info;

  if (mode & ST_CREATE)
    fmode = "w+";
  else if (mode & ST_APPEND)
    fmode = "a+";
  else if (mode & ST_WRITE)
    fmode = "r+";
  else
    fmode = "r";

  file = fopen(path, fmode);
  if (file == NULL)
    return TRUE;

  if (fstat(fileno(file), &info) < 0)
    return TRUE;

  if (S_ISDIR(info.st_mode))
  {
    errno = EISDIR;
    return TRUE;
  }

  stream->buffer.size = info.st_size;
  stream->buffer.is_term = isatty(fileno(file));

  FD = file;
  return FALSE;
}


static int stream_close(STREAM *stream)
{
  if (FD == NULL)
    return FALSE;

  if (fclose(FD) < 0)
    return TRUE;

  FD = NULL;
  return FALSE;
}


static int stream_read(STREAM *stream, char *buffer, long len)
{
  size_t eff_read;
  size_t len_read;

  while (len > 0)
  {
    len_read = Min(len, MAX_IO);
    eff_read = fread(buffer, 1, len_read, FD);

    if (eff_read > 0)
    {
      STREAM_eff_read += eff_read;
      len -= eff_read;
      buffer += eff_read;
    }

    if (eff_read < len_read)
    {
      if (feof(FD))
      {
        errno = 0;
        return TRUE;
      }
      if (ferror(FD))
        return TRUE;
    }
  }

  return FALSE;
}


static int stream_flush(STREAM *stream)
{
  return (fflush(FD) != 0);
}


static int stream_write(STREAM *stream, char *buffer, long len)
{
  size_t eff_write;
  size_t len_write;

  while (len > 0)
  {
    len_write = Min(len, MAX_IO);
    eff_write = fwrite(buffer, 1, len_write, FD);

    if (eff_write < len_write)
    {
      if (ferror(FD))
        return TRUE;
    }

    len -= eff_write;
    buffer += eff_write;
  }

  if (EXEC_debug)
    return stream_flush(stream);
  else
    return FALSE;
}


static int stream_seek(STREAM *stream, long long pos, int whence)
{
  return (fseek(FD, (off_t)pos, whence) != 0);
}


static int stream_tell(STREAM *stream, long long *pos)
{
  *pos = (long long)ftell(FD);
  return (*pos < 0);
}


static int stream_eof(STREAM *stream)
{
  unsigned char c;

  fread(&c, 1, 1, FD);
  if (feof(FD))
    return TRUE;

  ungetc(c, FD);
  return FALSE;
}


static int stream_lof(STREAM *stream, long long *len)
{
  *len = stream->buffer.size;
  return FALSE;
}


static int stream_handle(STREAM *stream)
{
  return fileno(FD);
}



DECLARE_STREAM(STREAM_buffer);
