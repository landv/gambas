/***************************************************************************

  gbx_stream.c

  (c) 2000-2009 Benoît Minisini <gambas@users.sourceforge.net>

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
#include "gbx_watch.h"

#include "gbx_stream.h"

int STREAM_eff_read;

#if DEBUG_STREAM
static unsigned char _tag = 0;
static int _nopen = 0;
#endif

#if DEBUG_STREAM
void STREAM_exit(void)
{
	if (_nopen)
		fprintf(stderr, "WARNING: %d streams yet opened\n", _nopen);
}
#endif

bool STREAM_in_archive(const char *path)
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

static int STREAM_get_readable(STREAM *stream, int *len)
{
	int fd;
	off_t off;
	off_t end;

	fd = STREAM_handle(stream);
	
//_IOCTL:

	#ifdef FIONREAD

	if (!stream->common.no_fionread)
	{
		if (ioctl(fd, FIONREAD, len) >= 0)
			return 0;
		
		stream->common.no_fionread = TRUE;
	}
	
	#endif

//_LSEEK:

	if (!stream->common.no_lseek)
	{
		off = lseek(fd, 0, SEEK_CUR);
		if (off >= 0)
		{
			end = lseek(fd, 0, SEEK_END);
			if (end >= 0)
			{
				*len = (int)(end - off);
	
				off = lseek(fd, off, SEEK_SET);
				if (off >= 0)
					return 0;
			}
		}
		
		stream->common.no_lseek = TRUE;
	}

	//fprintf(stderr, "STREAM_get_readable: lseek: %d\n", *len);
	return (-1);
}

static int default_eof(STREAM *stream)
{
	int fd;
	int ilen;

	fd = STREAM_handle(stream);
	if (fd < 0 || STREAM_get_readable(stream, &ilen))
		return TRUE;

	return (ilen == 0);
}

// STREAM_open *MUST* initialize completely the stream structure

void STREAM_open(STREAM *stream, const char *path, int mode)
{
	if (mode & ST_PIPE)
	{
		stream->type = &STREAM_pipe;
	}
	else if (mode & ST_MEMORY)
	{
		stream->type = &STREAM_memory;
	}
	else
	{
		if (FILE_is_relative(path))
		{
			ARCHIVE *arch = NULL;

			if (strncmp(path, "../", 3))
				ARCHIVE_get_current(&arch);
			else if (!EXEC_arch)
				path += 3;
			
			if ((arch && arch->name) || EXEC_arch) // || !FILE_exist_real(path)) - Why that test ?
			{
				stream->type = &STREAM_arch;
				goto _OPEN;
			}

			if ((mode & ST_ACCESS) != ST_READ || mode & ST_PIPE)
				THROW(E_ACCESS);
		}

		if (mode & ST_DIRECT)
			stream->type = &STREAM_direct;
		else
			stream->type = &STREAM_buffer;
	}

_OPEN:

	stream->common.mode = mode;
	stream->common.swap = FALSE;
	stream->common.eol = 0;
	stream->common.eof = FALSE;
	stream->common.buffer = NULL;
	stream->common.buffer_pos = 0;
	stream->common.buffer_len = 0;
	stream->common.no_fionread = FALSE;
	stream->common.no_lseek = FALSE;
	stream->common.standard = FALSE;
	stream->common.blocking = TRUE;

	if ((*(stream->type->open))(stream, path, mode, NULL))
	{
		stream->type = NULL;
		THROW_SYSTEM(errno, path);
	}
	
	#if DEBUG_STREAM
	_tag++;
	stream->common.tag = _tag;
	_nopen++;
	fprintf(stderr, "Open %p [%d] (%d)\n", stream, _tag, _nopen);
	#endif
}


void STREAM_release(STREAM *stream)
{
	if (stream->common.buffer)
	{
		#if DEBUG_STREAM
		fprintf(stderr, "Stream %p [%d]: Free buffer\n", stream, stream->common.tag);
		#endif
		FREE(&stream->common.buffer, "STREAM_close");
		stream->common.buffer_pos = 0;
		stream->common.buffer_len = 0;
	}
}

void STREAM_close(STREAM *stream)
{
	int fd;

	STREAM_release(stream);

	if (!stream->type)
		return;

	fd = STREAM_handle(stream);
	if (fd >= 0)
		GB_Watch(fd, GB_WATCH_NONE, NULL, 0);

	if (stream->common.standard || !(*(stream->type->close))(stream))
		stream->type = NULL;
		
	#if DEBUG_STREAM
	_nopen--;
	fprintf(stderr, "Close %p [%d] (%d)\n", stream, stream->common.tag, _nopen);
	#endif
}


void STREAM_flush(STREAM *stream)
{
	if (!stream->type)
		THROW(E_CLOSED);
		
	(*(stream->type->flush))(stream);
}

static void read_buffer(STREAM *stream, void **addr, int *len)
{
	int l = stream->common.buffer_len - stream->common.buffer_pos;
	if (l > *len)
		l = *len;
	if (l > 0)
	{
		memcpy(*addr, stream->common.buffer + stream->common.buffer_pos, l);
		*addr = (char *)*addr + l;
		*len -= l;
		STREAM_eff_read += l;
		stream->common.buffer_pos += l;
	}
}

void STREAM_read(STREAM *stream, void *addr, int len)
{
	STREAM_eff_read = 0;

	if (!stream->type)
		THROW(E_CLOSED);
	
	if (stream->common.buffer)
		read_buffer(stream, &addr, &len);
	
	while ((*(stream->type->read))(stream, addr, len))
	{
		switch(errno)
		{
			case EINTR:
				break;
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

char STREAM_getchar(STREAM *stream)
{
	char c = 0;
	bool ret;
	
	if (!stream->type)
		THROW(E_CLOSED);
	
	if (stream->common.buffer && stream->common.buffer_pos < stream->common.buffer_len)
		return stream->common.buffer[stream->common.buffer_pos++];
	
	for(;;)
	{
		if (stream->type->getchar)
			ret = (*(stream->type->getchar))(stream, &c);
		else
			ret = (*(stream->type->read))(stream, &c, 1);
			
		if (!ret)
			break;
		
		switch(errno)
		{
			case EINTR:
				continue;
			case 0:
			case EAGAIN:
				THROW(E_EOF);
			case EIO:
				THROW(E_READ);
			default:
				THROW_SYSTEM(errno, NULL);
		}
	}
	
	return c;
}


int STREAM_read_max(STREAM *stream, void *addr, int len)
{
	bool err;
	int flags, handle;

	if (!stream->type)
		THROW(E_CLOSED);
		
	STREAM_eff_read = 0;
	
	if (stream->common.buffer)
		read_buffer(stream, &addr, &len);
	
	if (len > 0)
	{
		for(;;)
		{
			if (stream->common.available_now)
			{
				err = (*(stream->type->read))(stream, addr, len);
			}
			else
			{
				handle = STREAM_handle(stream);
				flags = fcntl(handle, F_GETFL);
				fcntl(handle, F_SETFL, flags | O_NONBLOCK);
				
				err = (*(stream->type->read))(stream, addr, len);
				
				fcntl(handle, F_SETFL, flags);
			}
		
			if (!err)
				break;
				
			switch(errno)
			{
				case EINTR:
					continue;
				case 0:
				case EAGAIN:
					return -1;
				case EIO:
					return -1; //THROW(E_READ);
				default:
					THROW_SYSTEM(errno, NULL);
			}
		}
	}
	
	return STREAM_eff_read;
}


void STREAM_write(STREAM *stream, void *addr, int len)
{
	if (!stream->type)
		THROW(E_CLOSED);
		
	while ((*(stream->type->write))(stream, addr, len))
	{
		switch(errno)
		{
			case EINTR:
				break;
			case EIO:
				THROW(E_WRITE);
			default:
				THROW_SYSTEM(errno, NULL);
		}
	}
}

void STREAM_write_eol(STREAM *stream)
{
	if (!stream->type)
		THROW(E_CLOSED);
		
	switch(stream->common.eol)
	{
		case ST_EOL_UNIX: STREAM_write(stream, "\n", 1); break;
		case ST_EOL_WINDOWS: STREAM_write(stream, "\r\n", 2); break;
		case ST_EOL_MAC: STREAM_write(stream, "\r", 1); break;
	}
}


int64_t STREAM_tell(STREAM *stream)
{
	int64_t pos;

	if (!stream->type)
		THROW(E_CLOSED);
		
	if (stream->type->tell(stream, &pos))
		/*THROW(E_SEEK, ERROR_get());*/
		THROW_SYSTEM(errno, NULL);

	return pos;
}


void STREAM_seek(STREAM *stream, int64_t pos, int whence)
{
	if (!stream->type)
		THROW(E_CLOSED);
		
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

static void fill_buffer(STREAM *stream, char *addr)
{
	bool err;
	int flags, fd;

	STREAM_eff_read = 0;
	fd = STREAM_handle(stream);
	
	for(;;)
	{
		if (stream->common.available_now)
			err = (*(stream->type->read))(stream, addr, STREAM_BUFFER_SIZE);
		else
		{
			//printf("WATCH_process(%d, -1)\n", fd); fflush(stdout);
			WATCH_process(fd, -1);
			flags = fcntl(fd, F_GETFL);
			if ((flags & O_NONBLOCK) == 0)
				fcntl(fd, F_SETFL, flags | O_NONBLOCK);
			
			//printf("read\n"); fflush(stdout);
			err = (*(stream->type->read))(stream, addr, STREAM_BUFFER_SIZE);
			
			if ((flags & O_NONBLOCK) == 0)
				fcntl(fd, F_SETFL, flags);
		}
	
		if (!err)
			break;
			
		switch(errno)
		{
			case EINTR:
				continue;
			case 0:
			case EAGAIN:
			case EINPROGRESS:
				return;
			case EIO:
				return; //THROW(E_READ);
			default:
				THROW_SYSTEM(errno, NULL);
		}
	}
}


static void input(STREAM *stream, char **addr, bool line)
{
	int len = 0;
	int start;
	unsigned char c, lc = 0;
	void *test;
	bool eol;
	char *buffer;
	short buffer_len;
	short buffer_pos;
	
	*addr = NULL;

	stream->common.eof = FALSE;
	
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

	if (!STREAM_is_blocking(stream) && STREAM_eof(stream))
	{
		//printf("EOF!\n"); fflush(stdout);
		THROW(E_EOF);
	}
		
	buffer = stream->common.buffer;
	buffer_len = stream->common.buffer_len;
	buffer_pos = stream->common.buffer_pos;
		
	if (!buffer)
	{
		#if DEBUG_STREAM
		fprintf(stderr, "Stream %p [%d]: Alloc buffer\n", stream, stream->common.tag);
		#endif
		ALLOC(&buffer, STREAM_BUFFER_SIZE, "input");
		buffer_pos = 0;
		buffer_len = 0;
	}

	/*eof_func = stream->type->eof;
	if (!eof_func)
		eof_func = default_eof;*/

	start = buffer_pos;
	
	for(;;)
	{
		if (buffer_pos == buffer_len)
		{
			if (len)
			{
				//add_string(addr, &len_str, stream->common.buffer + start, len);
				STRING_add(addr, buffer + start, len);
				len = 0;
			}
			
			stream->common.buffer = buffer;
			stream->common.buffer_pos = buffer_pos;
			stream->common.buffer_len = buffer_len;
			
			//printf("fill_buffer: (is_file = %d)\n", stream->common.is_file); fflush(stdout);
			
			fill_buffer(stream, buffer);
			//STREAM_read_max(stream, buffer, STREAM_BUFFER_SIZE);
			
			buffer_pos = 0;
			buffer_len = STREAM_eff_read;
			
			#if 0
			printf("-> %d: ", buffer_len);
			{
				int i;
				for (i = 0; i < buffer_len; i++)
				{
					printf("%02X ", buffer[i]);
				}
				putchar('\n');
			}
			#endif
			
			if (!buffer_len)
			{
				if (stream->common.available_now)
				{
					stream->common.eof = TRUE;
					break;
				}
			}
			
			start = 0;
		}
		
		c = buffer[buffer_pos++]; //STREAM_getchar(stream);

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

		len++;
	}

	if (len > 0)
		//add_string(addr, &len_str, stream->common.buffer + start, len);
		STRING_add(addr, buffer + start, len);

	STRING_extend_end(addr);
	
	stream->common.buffer = buffer;
	stream->common.buffer_pos = buffer_pos;
	stream->common.buffer_len = buffer_len;
}


void STREAM_line_input(STREAM *stream, char **addr)
{
	input(stream, addr, TRUE);
}


void STREAM_input(STREAM *stream, char **addr)
{
	input(stream, addr, FALSE);
}


void STREAM_read_type(STREAM *stream, TYPE type, VALUE *value, int len)
{
	union
	{
		unsigned char _byte;
		short _short;
		int _int;
		float _single;
		unsigned char _data[4];
	}
	buffer;

	if (type == T_VARIANT)
	{
		STREAM_read(stream, &buffer._byte, 1);
		type = buffer._byte;
	}
	
	value->type = type;

	switch (type)
	{
		case T_BOOLEAN:

			STREAM_read(stream, &buffer._byte, 1);
			value->_integer.value = (buffer._byte != 0) ? (-1) : 0;
			break;

		case T_BYTE:

			STREAM_read(stream, &buffer._byte, 1);
			value->_integer.value = buffer._byte;
			break;

		case T_SHORT:

			STREAM_read(stream, &buffer._short, sizeof(short));
			if (stream->common.swap)
				SWAP_short(&buffer._short);
			value->_integer.value = buffer._short;
			break;

		case T_INTEGER:

			STREAM_read(stream, &value->_integer.value, sizeof(int));
			if (stream->common.swap)
				SWAP_int(&value->_integer.value);
			break;

		case T_LONG:

			STREAM_read(stream, &value->_long.value, sizeof(int64_t));
			if (stream->common.swap)
				SWAP_int64(&value->_long.value);
			break;

		case T_SINGLE:

			STREAM_read(stream, &buffer._single, sizeof(float));
			if (stream->common.swap)
				SWAP_float(&buffer._single);
			value->_float.value = buffer._single;
			break;

		case T_FLOAT:

			STREAM_read(stream, &value->_float.value, sizeof(double));
			if (stream->common.swap)
				SWAP_double(&value->_float.value);
			break;

		case T_DATE:

			STREAM_read(stream, &value->_date.date, sizeof(int));
			STREAM_read(stream, &value->_date.time, sizeof(int));
			if (stream->common.swap)
			{
				SWAP_int(&value->_date.date);
				SWAP_int(&value->_date.time);
			}
			break;

		case T_CSTRING:
			value->type = T_STRING;
			// continue
		
		case T_STRING:

			if (len == 0)
			{
				if (stream->type == &STREAM_memory)
				{
					size_t slen;
					if (CHECK_strlen(stream->memory.addr + stream->memory.pos, &slen))
						THROW(E_READ);
					len = (int)slen;
				}
				else
				{
					STREAM_read(stream, buffer._data, 1);

					switch (buffer._data[0] >> 6)
					{
						case 0:
						case 1:
							len = buffer._data[0];
							break;

						case 2:
							STREAM_read(stream, &buffer._data[1], 1);
							buffer._data[0] &= 0x3F;

							if (!EXEC_big_endian)
								SWAP_short(&buffer._short);

							len = buffer._short;
							break;

						case 3:
							STREAM_read(stream, &buffer._data[1], 3);
							buffer._data[0] &= 0x3F;

							if (!EXEC_big_endian)
								SWAP_int(&buffer._int);

							len = buffer._int;
							break;
					}
				}
			}

			if (len > 0)
			{
				STRING_new_temp_value(value, NULL, labs(len));
				STREAM_read(stream, value->_string.addr, len);
			}
			else
			{
				len = (-len);
				
				value->type = T_STRING;
				STRING_new(&value->_string.addr, NULL, len);
				value->_string.start = 0;
				value->_string.len = len;
				
				STREAM_read_max(stream, value->_string.addr, len);
				
				if (STREAM_eff_read == 0)
				{
					value->type = T_NULL;
					STRING_free(&value->_string.addr);
				}
				else
				{
					if (STREAM_eff_read < len)
					{
						STRING_extend(&value->_string.addr, STREAM_eff_read);
						value->_string.len = STREAM_eff_read;
					}
					STRING_extend_end(&value->_string.addr);
				}
			}

			break;

		default:

			THROW(E_TYPE, "Standard type", TYPE_get_name(type));
	}
}


void STREAM_write_type(STREAM *stream, TYPE type, VALUE *value, int len)
{
	union
	{
		unsigned char _byte;
		short _short;
		int _int;
		int64_t _long;
		float _single;
		double _float;
		unsigned char _data[8];
	}
	buffer;
	
	bool write_zero;

	if (type == T_VARIANT)
	{
		VARIANT_undo(value);
		type = value->type;
		buffer._byte = (unsigned char)type;
		STREAM_write(stream, &buffer._byte, 1);
	}

	//value->type = type;

	switch (type)
	{
		case T_BOOLEAN:

			buffer._byte = value->_integer.value ? 0xFF : 0;
			STREAM_write(stream, &buffer._byte, 1);
			break;

		case T_BYTE:

			buffer._byte = (unsigned char)value->_integer.value;
			STREAM_write(stream, &buffer._byte, 1);
			break;

		case T_SHORT:

			buffer._short = (short)value->_integer.value;
			if (stream->common.swap)
				SWAP_short(&buffer._short);
			STREAM_write(stream, &buffer._short, sizeof(short));
			break;

		case T_INTEGER:

			buffer._int = value->_integer.value;
			if (stream->common.swap)
				SWAP_int(&buffer._int);
			STREAM_write(stream, &buffer._int, sizeof(int));
			break;

		case T_LONG:

			buffer._long = value->_long.value;
			if (stream->common.swap)
				SWAP_int64(&buffer._long);
			STREAM_write(stream, &buffer._long, sizeof(int64_t));
			break;

		case T_SINGLE:

			buffer._single = (float)value->_float.value;
			if (stream->common.swap)
				SWAP_float(&buffer._single);
			STREAM_write(stream, &buffer._single, sizeof(float));
			break;

		case T_FLOAT:

			buffer._float = value->_float.value;
			if (stream->common.swap)
				SWAP_double(&buffer._float);
			STREAM_write(stream, &buffer._float, sizeof(double));
			break;

		case T_DATE:

			buffer._int = value->_date.date;
			if (stream->common.swap)
				SWAP_int(&buffer._int);
			STREAM_write(stream, &buffer._int, sizeof(int));

			buffer._int = value->_date.time;
			if (stream->common.swap)
				SWAP_int(&buffer._int);
			STREAM_write(stream, &buffer._int, sizeof(int));

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
            buffer._byte = (unsigned char)len;
            STREAM_write(stream, &buffer._byte, 1);
          }
          else if (len < 0x4000)
          {
            buffer._short = (short)len | 0x8000;
            if (!EXEC_big_endian)
              SWAP_short(&buffer._short);
            STREAM_write(stream, &buffer._short, sizeof(short));
          }
          else
          {
            buffer._int = len | 0xC0000000;
            if (!EXEC_big_endian)
              SWAP_int(&buffer._int);
            STREAM_write(stream, &buffer._int, sizeof(int));
          }
				}
			}

			STREAM_write(stream, value->_string.addr + value->_string.start, Min(len, value->_string.len));

			buffer._byte = 0;

			while (len > value->_string.len)
			{
				STREAM_write(stream, &buffer._byte, 1);
				len--;
			}

			if (write_zero)
				STREAM_write(stream, &buffer._byte, 1);

			break;
			
		case T_NULL:
		
			buffer._byte = 0;
			STREAM_write(stream, &buffer._byte, 1);
			break;

		/*case T_OBJECT:
		
			class = OBJECT_class(value->_object.object);
			if (class == CLASS_Array || CLASS_inherits(class, CLASS_Array))
			{
				CARRAY_stream_write((CARRAY *)value->_object.object, stream);
				break;
			}
			else if (class == CLASS_Collection || CLASS_inherits(class, CLASS_Collection))
			{
				CCOLLECTION_stream_write((CCOLLECTION *)value->_object.object, stream);
				break;
			}*/			 

		default:

			THROW(E_TYPE, "Standard type", TYPE_get_name(type));
	}
}


void STREAM_load(const char *path, char **buffer, int *rlen)
{
	STREAM stream;
	int64_t len;

	STREAM_open(&stream, path, ST_READ);
	STREAM_lof(&stream, &len);

	if (len >> 31)
		THROW(E_MEMORY);

	*rlen = len;

	ALLOC(buffer, *rlen, "STREAM_load");

	STREAM_read(&stream, *buffer, *rlen);
	STREAM_close(&stream);
}


bool STREAM_map(const char *path, char **paddr, int *plen)
{
	STREAM stream;
	int fd;
  struct stat info;
	void *addr;
	size_t len;
	bool ret = TRUE;

	STREAM_open(&stream, path, ST_READ + ST_DIRECT);
	
	if (stream.type == &STREAM_arch)
	{
		*paddr = (char *)stream.arch.arch->arch->addr + stream.arch.start;
		*plen = stream.arch.size;
		ret = FALSE;
		goto __RETURN;
	}
	
	fd = STREAM_handle(&stream);
	if (fd < 0)
		goto __RETURN;
	
  if (fstat(fd, &info) < 0)
    goto __RETURN;

  len = info.st_size;
  addr = mmap(NULL, len, PROT_READ, MAP_PRIVATE, fd, 0);
  if (addr == MAP_FAILED)
    goto __RETURN;
	
	*paddr = addr;
	*plen = len;
	ret = FALSE;
	
__RETURN:

	STREAM_close(&stream);
	return ret;
}

void STREAM_unmap(char *addr, int len)
{
  if (addr && len > 0 && ARCHIVE_check_addr(addr))
  {
  	munmap(addr, len);
	}
}


int STREAM_handle(STREAM *stream)
{
	if (!stream->type)
		THROW(E_CLOSED);
		
	if (stream->type->handle)
		return (*stream->type->handle)(stream);
	else
		return (-1);
}


void STREAM_lock(STREAM *stream)
{
	int64_t pos;
	int fd;

	if (!stream->type)
		THROW(E_CLOSED);
		
	fd = STREAM_handle(stream);
	if (fd < 0)
		THROW(E_LOCK);

	pos = lseek(fd, 0, SEEK_CUR);
	if (pos < 0)
		goto __ERROR;

	if (lseek(fd, 0, SEEK_SET) < 0)
		goto __ERROR;

	#ifdef F_TLOCK

		if (lockf(fd, F_TLOCK, 0))
		{
			if (errno == EAGAIN)
				THROW(E_LOCK);
			else
				goto __ERROR;
		}
		
	#else
	
		fprintf(stderr, "locking is not implemented\n");
	
	#endif

	if (lseek(fd, pos, SEEK_SET) < 0)
		goto __ERROR;

	return;

__ERROR:
	THROW_SYSTEM(errno, NULL);
}



void STREAM_lof(STREAM *stream, int64_t *len)
{
	int fd;
	int ilen;

	if (!stream->type)
		THROW(E_CLOSED);
		
	if (stream->type->lof)
	{
		if (!(*(stream->type->lof))(stream, len))
			return;
	}

	fd = STREAM_handle(stream);
	if ((fd >= 0) && (STREAM_get_readable(stream, &ilen) == 0))
		*len = ilen;
		
	if (stream->common.buffer)
		*len += stream->common.buffer_len - stream->common.buffer_pos;
}

bool STREAM_eof(STREAM *stream)
{
	if (!stream->type)
		THROW(E_CLOSED);
		
	if (stream->common.buffer && stream->common.buffer_pos < stream->common.buffer_len)
		return FALSE;

	if (stream->type->eof)
		return ((*(stream->type->eof))(stream));
	else
		return default_eof(stream);
}


int STREAM_read_direct(int fd, char *buffer, int len)
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

int STREAM_write_direct(int fd, char *buffer, int len)
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


void STREAM_blocking(STREAM *stream, bool block)
{
	int fd = STREAM_handle(stream);
	
	if (fd < 0)
		return;
	
	stream->common.blocking = block;
	
	if (block)
		fcntl(fd, F_SETFL, fcntl(fd, F_GETFL) & ~O_NONBLOCK);
	else
		fcntl(fd, F_SETFL, fcntl(fd, F_GETFL) | O_NONBLOCK);
}

bool STREAM_is_blocking(STREAM *stream)
{
	return stream->common.blocking;
	/*int fd = STREAM_handle(stream);
	
	if (fd < 0)
		return TRUE;
		
	return (fcntl(fd, F_GETFL) & O_NONBLOCK) == 0;*/
}
