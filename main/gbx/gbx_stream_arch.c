/***************************************************************************

	gbx_stream_arch.c

	(c) 2000-2017 Benoît Minisini <g4mba5@gmail.com>

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

#include "gbx_archive.h"
#include "gbx_stream.h"


static int stream_open(STREAM *stream, const char *path, int mode)
{
	ARCHIVE_FIND find;

	if (ARCHIVE_get(NULL, &path, &find))
	{
		errno = ENOENT;
		return TRUE;
	}

	if (find.pos < 0)
	{
		errno = EISDIR;
		return TRUE;
	}

	if ((mode & ST_ACCESS) != ST_READ)
	{
		errno = EACCES;
		return TRUE;
	}

	stream->common.available_now = TRUE;
	stream->arch.arch = find.arch;
	stream->arch.size = find.len;
	stream->arch.start = find.pos;
	stream->arch.pos = 0;

	return FALSE;
}


static int stream_close(STREAM *stream)
{
	return FALSE;
}


static int stream_read(STREAM *stream, char *buffer, int len)
{
	int max;

	max = stream->arch.size - stream->arch.pos;

	if (len > max)
	{
		len = max;
		errno = 0;
	}

	ARCHIVE_read(stream->arch.arch, stream->arch.start + stream->arch.pos, buffer, len);
	stream->arch.pos += len;
	return len;
}


static int stream_write(STREAM *stream, char *buffer, int len)
{
	return -1;
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
			new_pos = stream->arch.pos + pos;
			break;

		case SEEK_END:
			new_pos = stream->arch.size - pos;
			break;

		default:
			return TRUE;
	}

	if (new_pos < 0 || new_pos > stream->arch.size)
		return TRUE;

	stream->arch.pos = (int)new_pos;
	return FALSE;
}

static int stream_tell(STREAM *stream, int64_t *pos)
{
	*pos = stream->arch.pos;
	return FALSE;
}


static int stream_flush(STREAM *stream)
{
	return FALSE;
}


static int stream_eof(STREAM *stream)
{
	return (stream->arch.pos >= stream->arch.size);
}


static int stream_lof(STREAM *stream, int64_t *len)
{
	*len = stream->arch.size;
	return FALSE;
}

static int stream_handle(STREAM *stream)
{
	return -1;
}


DECLARE_STREAM(STREAM_arch);

