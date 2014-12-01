/***************************************************************************

  gbx_stream_direct.c

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
#include "gbx_number.h"

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
	struct stat info;
	int fmode, omode;
	VALUE val;
	
	if (mode & ST_CREATE)
		fmode = O_CREAT | O_TRUNC; // | O_EXCL;
	else if (mode & ST_APPEND)
		fmode = O_APPEND | O_CREAT;
	else
		fmode = 0;

	switch (mode & ST_MODE)
	{
		case ST_READ: fmode |= O_RDONLY; break;
		case ST_WRITE: fmode |= O_WRONLY; break;
		case ST_READ_WRITE: fmode |= O_RDWR; break;
		default: fmode |= O_RDONLY;
	}

	if (path[0] == '.' && isdigit(path[1]))
	{
		if ((mode & ST_CREATE) || (mode & ST_APPEND))
			THROW(E_ACCESS);
		
		if (NUMBER_from_string(NB_READ_INTEGER, &path[1], strlen(path) - 1, &val) || val._integer.value < 0)
		{
			errno = ENOENT;
			return TRUE;
		}
		
		fd = val._integer.value;
		omode = fcntl(fd, F_GETFL, NULL);
		if (omode < 0)
			return TRUE;
		
		if (((mode & ST_MODE) == ST_READ && (omode & O_ACCMODE) == O_WRONLY)
			  || ((mode & ST_MODE) == ST_WRITE && (omode & O_ACCMODE) == O_RDONLY)
			  || ((mode & ST_MODE) == ST_READ_WRITE && (omode & O_ACCMODE) != O_RDWR))
			THROW(E_ACCESS);
		
		stream->direct.watch = TRUE;
	}
	else
	{
		stream->direct.watch = FALSE;
		
		fd = open(path, fmode, 0666);
		if (fd < 0)
			return TRUE;

		if (fstat(fd, &info) < 0)
		{
			close(fd);
			return TRUE;
		}

		if (S_ISDIR(info.st_mode))
		{
			close(fd);
			errno = EISDIR;
			return TRUE;
		}

		if (!S_ISREG(info.st_mode))
		{
			stream->common.available_now = FALSE;
			fcntl(fd, F_SETFL, O_NONBLOCK);
		}
		else
			stream->common.available_now = TRUE;
	}

	FD = fd;
	return FALSE;
}


static int stream_close(STREAM *stream)
{
	if (!stream->direct.watch)
	{
		if (close(FD) < 0)
			return TRUE;
	}

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
	return (lseek(FD, pos, whence) < 0);
}


static int stream_tell(STREAM *stream, int64_t *pos)
{
	*pos = lseek(FD, 0, SEEK_CUR);
	return (*pos < 0);
}


static int stream_flush(STREAM *stream)
{
	return FALSE;
}


#define stream_eof NULL


static int stream_lof(STREAM *stream, int64_t *len)
{
	struct stat info;
	
	if (!stream->common.available_now)
		return TRUE;
		
	if (fstat(FD, &info) < 0)
		return TRUE;
	
	*len = info.st_size;
	return FALSE;
}


static int stream_handle(STREAM *stream)
{
	return FD;
}


DECLARE_STREAM(STREAM_direct);

