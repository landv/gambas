/***************************************************************************

  stream.c

  The stream management routines

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

#define __GBX_STREAM_C

#include "gb_common.h"

#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <errno.h>
#include <sys/ioctl.h>

#include "gb_common_buffer.h"
#include "gb_common_swap.h"
#include "gb_common_check.h"
#include "gb_error.h"
#include "gbx_value.h"
#include "gb_limit.h"
#include "gbx_exec.h"
#include "gbx_project.h"
#include "gb_file.h"
#include "gambas.h"
#include "gbx_regexp.h"
#include "gbx_api.h"
#include "gbx_string.h"

#include "gbx_stream.h"

#if 0
static long do_nothing(void)
{
  return 0;
}

PUBLIC STREAM_CLASS STREAM_null =
{
  (void *)do_nothing,
  (void *)do_nothing,
  (void *)do_nothing,
  (void *)do_nothing,
  (void *)do_nothing,
  (void *)do_nothing,
  (void *)do_nothing,
  (void *)do_nothing,
  (void *)do_nothing,
  (void *)do_nothing
};
#endif

PUBLIC long STREAM_eff_read;

PUBLIC bool STREAM_in_archive(const char *path)
{
	ARCHIVE *arch;

  if (FILE_is_relative(path))
  {
  	ARCHIVE_get_current(&arch);
    if (arch->name || EXEC_arch) // || !FILE_exist_real(path)) - Why that test ?
    	return TRUE;
	}

  return FALSE;
}

static int STREAM_get_readable(int fd, long *len)
{
	int ret;
  off_t off;
  off_t end;

//_IOCTL:

	ret = ioctl(fd, FIONREAD, len);
	if (ret >= 0)
	{
		//fprintf(stderr, "STREAM_get_readable: ioctl: %d\n", *len);
		return 0;
	}

//_LSEEK:

	off = lseek(fd, 0, SEEK_CUR);
	if (off < 0) return (-1);

	end = lseek(fd, 0, SEEK_END);
	if (end < 0) return (-1);

	*len = (int)(end - off);

	off = lseek(fd, off, SEEK_SET);
	if (off < 0) return (-1);

	//fprintf(stderr, "STREAM_get_readable: lseek: %d\n", *len);
	return 0;
}


PUBLIC void STREAM_open(STREAM *stream, const char *path, int mode)
{
  if (FILE_is_relative(path))
  {
  	ARCHIVE *arch;

  	ARCHIVE_get_current(&arch);
    if (arch->name || EXEC_arch) // || !FILE_exist_real(path)) - Why that test ?
    {
      stream->type = &STREAM_arch;
      goto _OPEN;
    }

    if ((mode & ST_ACCESS) != ST_READ || mode & ST_PIPE)
      THROW(E_ACCESS);
  }

	if (mode & ST_PIPE)
	{
    stream->type = &STREAM_pipe;
	}
	else
	{
  	if (mode & ST_DIRECT)
	    stream->type = &STREAM_direct;
	  else
    	stream->type = &STREAM_buffer;
	}

_OPEN:

  stream->common.mode = mode;
  stream->common.swap = FALSE;
  stream->common.eol = 0;

  /*if (((mode & ST_BIG) && !EXEC_big_endian)
      || ((mode & ST_LITTLE) && EXEC_big_endian))
    stream->common.flag.swap = TRUE;*/

  if ((*(stream->type->open))(stream, path, mode, NULL))
  {
    stream->type = NULL;
    THROW_SYSTEM(errno, path);
  }

  //stream->common.flag.closed = FALSE;
}


PUBLIC void STREAM_close(STREAM *stream)
{
  int fd;

  if (!stream->type)
    return;

  fd = STREAM_handle(stream);
  if (fd >= 0)
    GB_Watch(fd, GB_WATCH_NONE, NULL, 0);

  if (!(*(stream->type->close))(stream))
    stream->type = NULL;
}


PUBLIC void STREAM_flush(STREAM *stream)
{
  (*(stream->type->flush))(stream);
}


PUBLIC void STREAM_read(STREAM *stream, void *addr, long len)
{
  STREAM_eff_read = 0;

  if ((*(stream->type->read))(stream, addr, len))
  {
    switch(errno)
    {
      case 0:
      case EAGAIN:
        THROW(E_EOF);
      case EIO:
        THROW(E_READ);
      default:
        THROW_SYSTEM(errno, NULL);
    }
  }
}


PUBLIC void STREAM_read_max(STREAM *stream, void *addr, long len)
{
  STREAM_eff_read = 0;

  if ((*(stream->type->read))(stream, addr, len))
  {
    switch(errno)
    {
      case 0:
      case EAGAIN:
        break;
      case EIO:
        break; //THROW(E_READ);
      default:
        THROW_SYSTEM(errno, NULL);
    }
  }
}


PUBLIC void STREAM_write(STREAM *stream, void *addr, long len)
{
  if ((*(stream->type->write))(stream, addr, len))
  {
    switch(errno)
    {
      case EIO:
        THROW(E_WRITE);
      default:
        THROW_SYSTEM(errno, NULL);
    }
  }
}

PUBLIC void STREAM_write_eol(STREAM *stream)
{
	switch(stream->common.eol)
	{
		case ST_EOL_UNIX: STREAM_write(stream, "\n", 1); break;
		case ST_EOL_WINDOWS: STREAM_write(stream, "\r\n", 2); break;
		case ST_EOL_MAC: STREAM_write(stream, "\r", 1); break;
	}
}


PUBLIC long long STREAM_tell(STREAM *stream)
{
  long long pos;

  if (stream->type->tell(stream, &pos))
    /*THROW(E_SEEK, ERROR_get());*/
    THROW_SYSTEM(errno, NULL);

  return pos;
}


PUBLIC void STREAM_seek(STREAM *stream, long long pos, int whence)
{
  if (stream->type->seek(stream, pos, whence))
  {
    switch(errno)
    {
      case EINVAL:
        THROW(E_ARG);
      default:
        THROW_SYSTEM(errno, NULL);
    }
  }
}


static void add_string(char **addr, long *len_str, const char *src, int len)
{
  STRING_extend(addr, *len_str + len);
  memcpy(&((*addr)[*len_str]), src, len);

  *len_str += len;
}


static void input(STREAM *stream, char **addr, boolean line)
{
  int len = 0;
  long len_str = 0;
  unsigned char c, lc = 0;
  void *test;
  bool eol;

  *addr = NULL;

	if (!line)
		test = &&__TEST_INPUT;
	else
	{
		switch(stream->common.eol)
		{
			case 1: test = &&__TEST_WINDOWS; break;
			case 2: test = &&__TEST_MAC; break;
			default: test = &&__TEST_UNIX; break;
		}
	}

  for(;;)
  {
    STREAM_read(stream, &c, 1);

		goto *test;

	__TEST_INPUT:

		eol = (c <= ' ');
		goto __TEST_OK;

	__TEST_UNIX:

		eol = (c == '\n');
		if (lc == '\r')
			len--;
		lc = c;
		goto __TEST_OK;

	__TEST_MAC:

		eol = (c == '\r');
		goto __TEST_OK;

	__TEST_WINDOWS:

		eol = (lc == '\r') && (c == '\n');
		if (eol)
			len--;
		lc = c;
		goto __TEST_OK;

	__TEST_OK:

		if (eol)
			break;

    COMMON_buffer[len++] = c;
    if (len >= COMMON_BUF_MAX)
    {
      add_string(addr, &len_str, COMMON_buffer, len);
      len = 0;
    }

    if (STREAM_eof(stream))
      break;
  }

  if (len > 0)
    add_string(addr, &len_str, COMMON_buffer, len);

  STRING_extend_end(addr);
}


PUBLIC void STREAM_line_input(STREAM *stream, char **addr)
{
  input(stream, addr, TRUE);
}


PUBLIC void STREAM_input(STREAM *stream, char **addr)
{
  input(stream, addr, FALSE);
}


PUBLIC void STREAM_read_type(STREAM *stream, TYPE type, VALUE *value, long len)
{
  unsigned char data[4];

  value->type = type;

  switch (type)
  {
    case T_BOOLEAN:

      STREAM_read(stream, data, 1);
      value->_integer.value = (*data != 0) ? (-1) : 0;
      break;

    case T_BYTE:

      STREAM_read(stream, data, 1);
      value->_integer.value = *data;
      break;

    case T_SHORT:

      STREAM_read(stream, data, sizeof(short));
      if (stream->common.swap)
        SWAP_short((short *)data);
      value->_integer.value = (long)*((short *)data);
      break;

    case T_INTEGER:

      STREAM_read(stream, &value->_integer.value, sizeof(long));
      if (stream->common.swap)
        SWAP_long(&value->_integer.value);
      break;

    case T_LONG:

      STREAM_read(stream, &value->_long.value, sizeof(long long));
      if (stream->common.swap)
        SWAP_double((double *)&value->_long.value);
      break;

    case T_SINGLE:

      STREAM_read(stream, data, sizeof(float));
      if (stream->common.swap)
        SWAP_long((long *)data);
      value->_float.value = (double)*((float *)data);
      break;

    case T_FLOAT:

      STREAM_read(stream, &value->_float.value, sizeof(double));
      if (stream->common.swap)
        SWAP_double(&value->_float.value);
      break;

    case T_DATE:

      STREAM_read(stream, &value->_date.date, sizeof(long));
      STREAM_read(stream, &value->_date.time, sizeof(long));
      if (stream->common.swap)
      {
        SWAP_long(&value->_date.date);
        SWAP_long(&value->_date.time);
      }
      break;

    case T_STRING:
    case T_CSTRING:

      if (len == 0)
      {
        if (stream->type == &STREAM_memory)
        {
        	if (CHECK_strlen(stream->memory.addr + stream->memory.pos, &len))
            THROW(E_READ);
        }
        else
        {
          STREAM_read(stream, data, 1);

          switch (*data >> 6)
          {
            case 0:
            case 1:
              len = *data;
              break;

            case 2:
              STREAM_read(stream, &data[1], 1);
              *data &= 0x3F;

              if (!EXEC_big_endian)
                SWAP_short((short *)data);

              len = *((short *)data);
              break;

            case 3:
              STREAM_read(stream, &data[1], 3);
              *data &= 0x3F;

              if (!EXEC_big_endian)
                SWAP_long((long *)data);

              len = *((long *)data);
              break;
          }
        }
      }

      STRING_new_temp_value(value, NULL, labs(len));
      if (len > 0)
        STREAM_read(stream, value->_string.addr, len);
      else
      {
        len = (-len);
        STREAM_read_max(stream, value->_string.addr, len);
        if (STREAM_eff_read < len)
        {
          if (STREAM_eff_read == 0)
            value->type = T_NULL;
          else
          {
            STRING_extend(&value->_string.addr, STREAM_eff_read);
            value->_string.len = STREAM_eff_read;
          }
        }
      }

      break;

    default:

      THROW(E_TYPE, "Standard type", TYPE_get_name(type));
  }
}


PUBLIC void STREAM_write_type(STREAM *stream, TYPE type, VALUE *value, long len)
{
  unsigned char data[8];
  bool write_zero;

  value->type = type;

  switch (type)
  {
    case T_BOOLEAN:

      *data = value->_integer.value ? 0xFF : 0;
      STREAM_write(stream, data, 1);
      break;

    case T_BYTE:

      *data = (unsigned char)value->_integer.value;
      STREAM_write(stream, data, 1);
      break;

    case T_SHORT:

      *((short *)data) = (short)value->_integer.value;
      if (stream->common.swap)
        SWAP_short((short *)data);
      STREAM_write(stream, data, sizeof(short));
      break;

    case T_INTEGER:

      *((long *)data) = value->_integer.value;
      if (stream->common.swap)
        SWAP_long((long *)data);
      STREAM_write(stream, data, sizeof(long));
      break;

    case T_LONG:

      *((long long *)data) = value->_long.value;
      if (stream->common.swap)
        SWAP_double((double *)data);
      STREAM_write(stream, data, sizeof(long long));
      break;

    case T_SINGLE:

      *((float *)data) = (float)value->_float.value;
      if (stream->common.swap)
        SWAP_long((long *)data);
      STREAM_write(stream, data, sizeof(float));
      break;

    case T_FLOAT:

      *((double *)data) = value->_float.value;
      if (stream->common.swap)
        SWAP_double((double *)data);
      STREAM_write(stream, data, sizeof(double));
      break;

    case T_DATE:

      *((long *)data) = value->_date.date;
      if (stream->common.swap)
        SWAP_long((long *)data);
      STREAM_write(stream, data, sizeof(long));

      *((long *)data) = value->_date.time;
      if (stream->common.swap)
        SWAP_long((long *)data);
      STREAM_write(stream, data, sizeof(long));

      break;

    case T_STRING:
    case T_CSTRING:

      write_zero = (stream->type == &STREAM_memory) && len == 0;

      if (len == 0)
      {
        len = value->_string.len;

        if (stream->type != &STREAM_memory)
        {
          if (len < 0x80)
          {
            *data = (unsigned char)len;
            STREAM_write(stream, data, 1);
          }
          else if (len < 0x4000)
          {
            *((short *)data) = (short)len | 0x8000;
            if (!EXEC_big_endian)
              SWAP_short((short *)data);
            STREAM_write(stream, data, 2);
          }
          else
          {
            *((long *)data) = len | 0xC0000000;
            if (!EXEC_big_endian)
              SWAP_long((long *)data);
            STREAM_write(stream, data, 4);
          }
        }
      }

      STREAM_write(stream, value->_string.addr + value->_string.start, Min(len, value->_string.len));

      *data = 0;

      while (len > value->_string.len)
      {
        STREAM_write(stream, data, 1);
        len--;
      }

      if (write_zero)
        STREAM_write(stream, data, 1);

      break;

    default:

      THROW(E_TYPE, "Standard type", TYPE_get_name(type));
  }
}


PUBLIC void STREAM_load(const char *path, char **buffer, long *rlen)
{
  STREAM stream;
  long long len;

  STREAM_open(&stream, path, ST_READ);
  STREAM_lof(&stream, &len);

  if (len >> 31)
    THROW(E_MEMORY);

  *rlen = len;

  ALLOC(buffer, *rlen, "STREAM_load");

  STREAM_read(&stream, *buffer, *rlen);
  STREAM_close(&stream);
}


PUBLIC bool STREAM_map(const char *path, char **buffer, long *rlen)
{
  STREAM stream;

  STREAM_open(&stream, path, ST_READ);
  if (stream.type != &STREAM_arch)
  {
    STREAM_close(&stream);
    STREAM_load(path, buffer, rlen);
    return TRUE;
  }

  //fprintf(stderr, "use mmap: %s\n", path);

  *buffer = (char *)stream.arch.arch->arch->addr + stream.arch.start;
  *rlen = stream.arch.size;
  return FALSE;
}


PUBLIC int STREAM_handle(STREAM *stream)
{
  if (stream->type->handle)
    return (*stream->type->handle)(stream);
  else
    return (-1);
}


PUBLIC void STREAM_lock(STREAM *stream, bool unlock)
{
  long long pos;
  int fd;

  fd = STREAM_handle(stream);
  if (fd < 0)
    THROW(E_LOCK);

  pos = lseek(fd, 0, SEEK_CUR);
  if (pos < 0)
    goto __ERROR;

  if (lseek(fd, 0, SEEK_SET) < 0)
    goto __ERROR;

  if (lockf(fd, unlock ? F_ULOCK : F_TLOCK, 0))
  {
    if (errno == EAGAIN)
      THROW(E_LOCK);
    else
      goto __ERROR;
  }

  if (lseek(fd, pos, SEEK_SET) < 0)
    goto __ERROR;

  return;

__ERROR:
  THROW_SYSTEM(errno, NULL);
}



PUBLIC void STREAM_lof(STREAM *stream, long long *len)
{
  int fd;
  long ilen;

	if (stream->type->lof)
	{
  	(*(stream->type->lof))(stream, len);
  	return;
	}

	fd = STREAM_handle(stream);
	if ((fd >= 0) && (STREAM_get_readable(fd, &ilen) == 0))
		*len = ilen;
}

PUBLIC bool STREAM_eof(STREAM *stream)
{
  int fd;
  long ilen;

	if (stream->type->eof)
  	return ((*(stream->type->eof))(stream));

	fd = STREAM_handle(stream);
  if (fd < 0 || STREAM_get_readable(fd, &ilen))
    return TRUE;

  return (ilen == 0);
}


PUBLIC int STREAM_read_direct(int fd, char *buffer, long len)
{
  ssize_t eff_read;
  ssize_t len_read;

  while (len > 0)
  {
    len_read = Min(len, MAX_IO);
    eff_read = read(fd, buffer, len_read);

    if (eff_read > 0)
    {
      STREAM_eff_read += eff_read;
      len -= eff_read;
      buffer += eff_read;
    }

    if (eff_read < len_read)
    {
      if (eff_read == 0)
        errno = 0;
      if (eff_read <= 0 && errno != EINTR)
        return TRUE;
    }

  }

  return FALSE;
}


PUBLIC int STREAM_write_direct(int fd, char *buffer, long len)
{
  ssize_t eff_write;
  ssize_t len_write;

  while (len > 0)
  {
    len_write = Min(len, MAX_IO);
    eff_write = write(fd, buffer, len_write);

    if (eff_write < len_write)
    {
      if (eff_write <= 0 && errno != EINTR)
        return TRUE;
    }

    len -= eff_write;
    buffer += eff_write;
  }

  return FALSE;
}



