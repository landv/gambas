/***************************************************************************

  gbx_stream.c

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
#include "gbx_c_array.h"
#include "gbx_c_collection.h"
#include "gbx_struct.h"
#include "gbx_stream.h"

int STREAM_eff_read;

static STREAM _temp_stream = { 0 };
static STREAM *_temp_save = NULL;
static int _temp_level;

#if DEBUG_STREAM
static unsigned char _tag = 0;
static int _nopen = 0;
#endif

void STREAM_exit(void)
{
#if DEBUG_STREAM
	if (_nopen)
		ERROR_warning("%d streams yet opened", _nopen);
#endif
	STREAM_close(&_temp_stream);
}

static void wait_for_fd_ready_to_read(fd)
{
	if (fd >= 0)
		WATCH_process(fd, -1, 0);
}

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

int STREAM_get_readable(STREAM *stream, int *len)
{
	int fd;
	off_t off;
	off_t end;

	fd = STREAM_handle(stream);
	if (fd < 0)
		return TRUE;
	
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
	if (fd < 0)
		return TRUE;
	
	if (STREAM_is_blocking(stream) && !stream->common.available_now)
		wait_for_fd_ready_to_read(STREAM_handle(stream));
		
	if (STREAM_get_readable(stream, &ilen))
		return TRUE;

	return (ilen == 0);
}

// STREAM_open *MUST* initialize completely the stream structure

void STREAM_open(STREAM *stream, const char *path, int mode)
{
	STREAM_CLASS *sclass;
	int fd;
	
	stream->type = NULL;
	
	if (mode & ST_PIPE)
	{
		sclass = &STREAM_pipe;
	}
	else if (mode & ST_MEMORY)
	{
		sclass = &STREAM_memory;
	}
	else if (mode & ST_STRING)
	{
		sclass = &STREAM_string;
	}
	else
	{
		// ".99" is used for opening a file descriptor in direct mode
		
		if (FILE_is_relative(path) && !((mode & ST_DIRECT) && path[0] == '.' && isdigit(path[1])))
		{
			ARCHIVE *arch = NULL;
			
			if (strncmp(path, "../", 3))
				ARCHIVE_get_current(&arch);
			else if (!EXEC_arch)
				path += 3;
			
			if ((arch && arch->name) || EXEC_arch) // || !FILE_exist_real(path)) - Why that test ?
			{
				sclass = &STREAM_arch;
				goto _OPEN;
			}

			if ((mode & ST_ACCESS) != ST_READ || mode & ST_PIPE)
				THROW(E_ACCESS);
		}

		if (mode & ST_DIRECT)
			sclass = &STREAM_direct;
		else
			sclass = &STREAM_buffer;
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
	stream->common.available_now = FALSE;
	stream->common.redirected = FALSE;
	stream->common.redirect = NULL;

	if ((*(sclass->open))(stream, path, mode, NULL))
		THROW_SYSTEM(errno, path);
	
	stream->type = sclass;

	fd = STREAM_handle(stream);
	if (fd >= 0)
		fcntl(fd, F_SETFD, fcntl(fd, F_GETFD) | O_CLOEXEC);
	
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
		FREE(&stream->common.buffer);
		stream->common.buffer_pos = 0;
		stream->common.buffer_len = 0;
	}
	
	STREAM_cancel(stream);
}

static void stop_watching(STREAM *stream, int mode)
{
	int fd = STREAM_handle(stream);
	if (fd >= 0)
		GB_Watch(fd, mode, NULL, 0);
}

void STREAM_close(STREAM *stream)
{
	STREAM_release(stream);

	if (!stream->type)
		return;

	stop_watching(stream, GB_WATCH_NONE);

	if (stream->common.standard || !(*(stream->type->close))(stream))
		stream->type = NULL;
		
	#if DEBUG_STREAM
	_nopen--;
	fprintf(stderr, "Close %p [%d] (%d)\n", stream, stream->common.tag, _nopen);
	#endif
}


void STREAM_flush(STREAM *stream)
{
	STREAM_end(stream);
	
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

	if (STREAM_is_closed(stream))
		THROW(E_CLOSED);
	
	if (len <= 0)
		return;
	
	if (stream->common.buffer)
		read_buffer(stream, &addr, &len);
	
	while ((*(stream->type->read))(stream, addr, len))
	{
		if (errno == EINTR)
			continue;

		stop_watching(stream, GB_WATCH_READ);

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

#if 0
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
		
		if (errno == EINTR)
			continue;

		stop_watching(stream, GB_WATCH_READ);

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
	
	return c;
}
#endif


int STREAM_read_max(STREAM *stream, void *addr, int len)
{
	bool err;
	int flags, handle;
	int save_errno;

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
				if ((flags & O_NONBLOCK) == 0)
				{
					wait_for_fd_ready_to_read(handle);
					fcntl(handle, F_SETFL, flags | O_NONBLOCK);
				}
				
				errno = 0;
				err = (*(stream->type->read))(stream, addr, len);
				save_errno = errno;
				
				if ((flags & O_NONBLOCK) == 0)
					fcntl(handle, F_SETFL, flags);
				
				errno = save_errno;
			}
		
			if (!err)
				break;

			if (errno == EINTR)
				continue;

			if (errno == 0)
				stop_watching(stream, GB_WATCH_READ);

			switch(errno)
			{
				case 0:
				case EAGAIN:
					return STREAM_eff_read;
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
	if (STREAM_is_closed_for_writing(stream))
		THROW(E_CLOSED);
	
	if (len <= 0)
		return;
	
	if (stream->common.redirected)
		stream = stream->common.redirect;
	
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

void STREAM_write_zeros(STREAM *stream, int len)
{
	static const char buffer[32] = { 0 };
	int lenw;
	
	while (len > 0)
	{
		lenw = Min(len, sizeof(buffer));
		STREAM_write(stream, (void *)buffer, lenw);
		len -= lenw;
	}
}

void STREAM_write_eol(STREAM *stream)
{
	if (STREAM_is_closed_for_writing(stream))
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

	if (STREAM_is_closed(stream))
		THROW(E_CLOSED);
		
	if (stream->type->tell(stream, &pos))
		/*THROW(E_SEEK, ERROR_get());*/
		THROW_SYSTEM(errno, NULL);

	return pos;
}


void STREAM_seek(STREAM *stream, int64_t pos, int whence)
{
	if (STREAM_is_closed(stream))
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
			wait_for_fd_ready_to_read(fd);

			flags = fcntl(fd, F_GETFL);
			if ((flags & O_NONBLOCK) == 0)
				fcntl(fd, F_SETFL, flags | O_NONBLOCK);
			
			err = (*(stream->type->read))(stream, addr, STREAM_BUFFER_SIZE);
			
			if ((flags & O_NONBLOCK) == 0)
			{
				int save_errno = errno;
				fcntl(fd, F_SETFL, flags);
				errno = save_errno;
			}
		}
	
		if (!err)
			break;

		if (errno == EINTR)
			continue;

		//if (STREAM_eff_read == 0 && stream->common.available_now)
		//	WATCH_need_little_sleep = TRUE;

		switch(errno)
		{
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

bool STREAM_read_ahead(STREAM *stream)
{
	if (stream->common.buffer && stream->common.buffer_pos < stream->common.buffer_len)
		return FALSE;

	if (!stream->common.buffer)
		ALLOC(&stream->common.buffer, STREAM_BUFFER_SIZE);

	fill_buffer(stream, stream->common.buffer);

	stream->common.buffer_pos = 0;
	stream->common.buffer_len = STREAM_eff_read;

	//fprintf(stderr, "STREAM_read_ahead: %d\n", STREAM_eff_read);

	if (STREAM_eff_read == 0)
	{
		stream->common.eof = TRUE;
		return TRUE;
	}
	else
	{
		stream->common.eof = FALSE;
		return FALSE;
	}
}


static char *input(STREAM *stream, bool line, char *escape)
{
	int len = 0;
	int start;
	unsigned char c, lc = 0;
	void *test, *test_org;
	bool eol;
	char *buffer;
	int buffer_len;
	int buffer_pos;
	char *addr;
	
	addr = NULL;
	
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
	
	test_org = test;
	
	stream->common.eof = FALSE;
	
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
		ALLOC(&buffer, STREAM_BUFFER_SIZE);
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
				addr = STRING_add(addr, buffer + start, len);
				len = 0;
			}
			
			stream->common.buffer = buffer;
			stream->common.buffer_pos = buffer_pos;
			stream->common.buffer_len = buffer_len;
			
			fill_buffer(stream, buffer);
			
			buffer_pos = 0;
			buffer_len = STREAM_eff_read;
			
			if (!buffer_len)
			{
				stream->common.eof = TRUE;
				break;
			}
			
			start = 0;
		}
		
		c = buffer[buffer_pos++]; //STREAM_getchar(stream);

		if (escape && c == *escape)
		{
			if (test == &&__TEST_CONT)
				test = test_org;
			else
				test = &&__TEST_CONT;
		}

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
		
	__TEST_CONT:
		
		len++;
	}

	if (len > 0)
		addr = STRING_add(addr, buffer + start, len);
	else if (len < 0)
		addr = STRING_extend(addr, STRING_length(addr) + len);

	stream->common.buffer = buffer;
	stream->common.buffer_pos = buffer_pos;
	stream->common.buffer_len = buffer_len;
	
	return addr;
}


char *STREAM_line_input(STREAM *stream, char *escape)
{
	return input(stream, TRUE, escape);
}


char *STREAM_input(STREAM *stream)
{
	return input(stream, FALSE, NULL);
}


static int read_length(STREAM *stream)
{
	union
	{
		unsigned char _data[4];
		short _short;
		int _int;
	}
	buffer;
	
	int len = 0;

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
	
	return len;
}

static STREAM *enter_temp_stream(STREAM *stream)
{
	if (stream != &_temp_stream)
	{
		_temp_save = stream;
		_temp_level = 0;
		if (&_temp_stream.type)
			STREAM_close(&_temp_stream);
		STREAM_open(&_temp_stream, NULL, ST_STRING | ST_WRITE);
	}

	_temp_level++;
	
	return &_temp_stream;
}

static STREAM *leave_temp_stream(void)
{
	_temp_level--;
	if (_temp_level > 0)
		return &_temp_stream;
	
	STREAM_write(_temp_save, _temp_stream.string.buffer, STRING_length(_temp_stream.string.buffer));
	STREAM_close(&_temp_stream);
	return _temp_save;
}

static void read_structure(STREAM *stream, CLASS *class, char *base);

static void read_value_ctype(STREAM *stream, CLASS *class, CTYPE ctype, void *addr)
{
	TYPE type;
	VALUE temp;
	
	type = (TYPE)ctype.id;
	if (type == T_OBJECT && ctype.value >= 0)
	{
		class = class->load->class_ref[ctype.value];
		if (CLASS_is_struct(class))
		{
			CSTRUCT *structure = (CSTRUCT *)OBJECT_create(class, NULL, NULL, 0);
			OBJECT_REF(structure);
			read_structure(stream, class, (char *)structure + sizeof(CSTRUCT));
			*((void **)addr) = structure;
			return;
		}
	}
	else if (type == TC_STRUCT)
	{
		class = class->load->class_ref[ctype.value];
		read_structure(stream, class, addr);
		return;
	}
	
	STREAM_read_type(stream, type, &temp);
	VALUE_class_write(class, &temp, addr, ctype);
}

static void read_structure(STREAM *stream, CLASS *class, char *base)
{
	int i, n;
	CLASS_DESC *desc;
	char *addr;
	CTYPE ctype;
		
	for (n = 0; n < class->n_desc; n++)
	{
		desc = class->table[n].desc;
		ctype = desc->variable.ctype;
		addr = base + desc->variable.offset;
					
		if (ctype.id == TC_STRUCT)
		{
			read_structure(stream, class->load->class_ref[ctype.value], addr);
		}
		else if (ctype.id == TC_ARRAY)
		{
			CLASS_ARRAY *adesc = class->load->array[ctype.value];
			int size = CLASS_sizeof_ctype(class, adesc->ctype);
			
			for (i = 0; i < CARRAY_get_static_count(adesc); i++)
			{
				read_value_ctype(stream, desc->variable.class, adesc->ctype, addr);
				addr += size;
			}
		}
		else
		{
			read_value_ctype(stream, desc->variable.class, ctype, addr);
		}
	}
}

void STREAM_read_type(STREAM *stream, TYPE type, VALUE *value)
{
	bool variant;
	int len;
	
	union
	{
		unsigned char _byte;
		short _short;
		int _int;
		float _single;
		unsigned char _data[4];
	}
	buffer;

	variant = (type == T_VARIANT);
	
	if (variant || TYPE_is_object(type))
	{
		if (TYPE_is_pure_object(type) && CLASS_is_struct((CLASS *)type))
		{
			CLASS *class = (CLASS *)type;
			CSTRUCT *structure = (CSTRUCT *)OBJECT_create(class, NULL, NULL, 0);
			
			read_structure(stream, class, (char *)structure + sizeof(CSTRUCT));
				
			value->_object.class = class;
			value->_object.object = structure;
			
			return;
		}
		
		STREAM_read(stream, &buffer._byte, 1);
		
		if (buffer._byte == 0)
		{
			value->type = T_VARIANT;
			value->_variant.vtype = T_NULL;
			return;
		}
		
		if (buffer._byte == 'A')
		{
			CARRAY *array;
			int size, i;
			VALUE temp;
			void *data;
			
			STREAM_read(stream, &buffer._byte, 1);
			type = (TYPE)buffer._byte;
			
			size = read_length(stream);
			
			GB_ArrayNew((GB_ARRAY *)&array, type, size);
			for (i = 0; i < size; i++)
			{
				data = CARRAY_get_data(array, i);
				STREAM_read_type(stream, type, &temp);
				VALUE_write(&temp, data, type);
			}
			
			value->type = T_VARIANT;
			value->_variant.vtype = (TYPE)OBJECT_class(array);
			value->_variant.value._object = array;
			
			return;
		}
		
		if (buffer._byte == 'c' || buffer._byte == 'C')
		{
			GB_COLLECTION col;
			int size, i;
			VALUE temp;
			char *key;
			char tkey[32];
			int len;
			
			size = read_length(stream);
			
			GB_CollectionNew(&col, buffer._byte == 'c');
			for (i = 0; i < size; i++)
			{
				len = read_length(stream);
				if (len < sizeof(tkey))
					key = tkey;
				else
					key = STRING_new(NULL, len);
				STREAM_read(stream, key, len);
				STREAM_read_type(stream, T_VARIANT, &temp);
				GB_CollectionSet(col, key, len, (GB_VARIANT *)&temp);
				if (len >= 32)
					STRING_free_real(key);
			}
			
			value->type = T_VARIANT;
			value->_variant.vtype = (TYPE)OBJECT_class(col);
			value->_variant.value._object = col;
			
			return;
		}
		
		if (variant)
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

		case T_POINTER:

			STREAM_read(stream, &value->_pointer.value, sizeof(void *));
			if (stream->common.swap)
				SWAP_pointer(&value->_pointer.value);
			break;

		case T_SINGLE:

			STREAM_read(stream, &buffer._single, sizeof(float));
			if (stream->common.swap)
				SWAP_float(&buffer._single);
			value->_single.value = buffer._single;
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

			if (stream->type == &STREAM_memory)
			{
				ssize_t slen;
				if (CHECK_strlen(stream->memory.addr + stream->memory.pos, &slen))
					THROW(E_READ);
				len = (int)slen;
			}
			else
				len = read_length(stream);

			if (len > 0)
			{
				STRING_new_temp_value(value, NULL, labs(len));
				STREAM_read(stream, value->_string.addr, len);
			}
			else
			{
				STRING_void_value(value);
			}

			break;

		default:

			THROW(E_TYPE, "Standard type", TYPE_get_name(type));
	}
	
	if (variant)
		VALUE_convert_variant(value);
}

static void write_length(STREAM *stream, int len)
{
	union
	{
		unsigned char _byte;
		short _short;
		int _int;
	}
	buffer;

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

void STREAM_write_type(STREAM *stream, TYPE type, VALUE *value)
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
		void *_pointer;
	}
	buffer;
	
	if (VALUE_is_null(value))
	{
		buffer._byte = 0;
		STREAM_write(stream, &buffer._byte, 1);
		return;
	}
	
	if (type == T_VARIANT)
	{
		VARIANT_undo(value);
		type = value->type;
		if (!TYPE_is_object(type))
		{
			buffer._byte = (unsigned char)type;
			STREAM_write(stream, &buffer._byte, 1);
		}
	}

	if (TYPE_is_object(type))
		type = T_OBJECT;

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

		case T_POINTER:

			buffer._pointer = value->_pointer.value;
			if (stream->common.swap)
				SWAP_pointer(&buffer._pointer);
			STREAM_write(stream, &buffer._pointer, sizeof(void *));
			break;

		case T_SINGLE:

			buffer._single = value->_single.value;
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

			if (stream->type == &STREAM_memory)
			{
				STREAM_write(stream, value->_string.addr + value->_string.start, value->_string.len);
				buffer._byte = 0;
				STREAM_write(stream, &buffer._byte, 1);
			}
			else
			{
				write_length(stream, value->_string.len);
				STREAM_write(stream, value->_string.addr + value->_string.start, value->_string.len);
			}
			break;
			
		case T_OBJECT:
		{
			CLASS *class = OBJECT_class(value->_object.object);
			void *structure;
			
			if (class->quick_array == CQA_ARRAY || class->is_array_of_struct)
			{
				CARRAY *array = (CARRAY *)value->_object.object;
				VALUE temp;
				void *data;
				int i;
				
				if (OBJECT_is_locked((OBJECT *)array))
					THROW(E_SERIAL);
				OBJECT_lock((OBJECT *)array, TRUE);
				
				if (!array->ref)
				{
					buffer._byte = 'A';
					STREAM_write(stream, &buffer._byte, 1);
					
					buffer._byte = (unsigned char)Min(T_OBJECT, array->type);
					STREAM_write(stream, &buffer._byte, 1);
					
					write_length(stream, array->count);
				}
				
				if (class->is_array_of_struct)
				{
					for (i = 0; i < array->count; i++)
					{
						data = CARRAY_get_data(array, i);
						structure = CSTRUCT_create_static(array, (CLASS *)array->type, data);
						temp._object.class = OBJECT_class(structure);
						temp._object.object = structure;
						OBJECT_REF(structure);
						STREAM_write_type(stream, T_OBJECT, &temp);
						OBJECT_UNREF(structure);
					}
				}
				else
				{
					for (i = 0; i < array->count; i++)
					{
						data = CARRAY_get_data(array, i);
						VALUE_read(&temp, data, array->type);
						STREAM_write_type(stream, array->type, &temp);
					}
				}
				
				OBJECT_lock((OBJECT *)array, FALSE);
				break;
			}
			else if (class->quick_array == CQA_COLLECTION)
			{
				CCOLLECTION *col = (CCOLLECTION *)value->_object.object;
				GB_COLLECTION_ITER iter;
				char *key = NULL;
				int len;
				VALUE temp;
				
				if (OBJECT_is_locked((OBJECT *)col))
					THROW(E_SERIAL);
				OBJECT_lock((OBJECT *)col, TRUE);
				
				buffer._byte = col->mode ? 'c' : 'C';
				STREAM_write(stream, &buffer._byte, 1);
				
				write_length(stream, CCOLLECTION_get_count(col));
				
				GB_CollectionEnum(col, &iter, (GB_VARIANT *)&temp, NULL, NULL);
				while (!GB_CollectionEnum(col, &iter, (GB_VARIANT *)&temp, &key, &len))
				{
					write_length(stream, len);
					STREAM_write(stream, key, len);
					STREAM_write_type(stream, T_VARIANT, &temp);
				}
				
				OBJECT_lock((OBJECT *)col, FALSE);
				break;
			}
			else if (class->is_struct)
			{
				CSTRUCT *structure = (CSTRUCT *)value->_object.object;
				int i;
				CLASS_DESC *desc;
				VALUE temp;
				char *addr;
				
				stream = enter_temp_stream(stream);
				
				if (OBJECT_is_locked((OBJECT *)structure))
					THROW(E_SERIAL);
				OBJECT_lock((OBJECT *)structure, TRUE);
				
				for (i = 0; i < class->n_desc; i++)
				{
					desc = class->table[i].desc;

					if (structure->ref)
						addr = (char *)((CSTATICSTRUCT *)structure)->addr + desc->variable.offset;
					else
						addr = (char *)structure + sizeof(CSTRUCT) + desc->variable.offset;
					
					VALUE_class_read(desc->variable.class, &temp, (void *)addr, desc->variable.ctype, (void *)structure);
					BORROW(&temp);
					STREAM_write_type(stream, temp.type, &temp);
					RELEASE(&temp);
				}
				
				stream = leave_temp_stream();
				
				OBJECT_lock((OBJECT *)structure, FALSE);
				break;
			}
			// continue;
		}

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

	ALLOC(buffer, *rlen);

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
	if (STREAM_is_closed(stream))
		THROW(E_CLOSED);
		
	if (stream->type->handle)
		return (*stream->type->handle)(stream);
	else
		return (-1);
}


bool STREAM_lock(STREAM *stream)
{
	int64_t pos;
	int fd;

	if (STREAM_is_closed(stream))
		THROW(E_CLOSED);
		
	fd = STREAM_handle(stream);
	if (fd < 0)
		return TRUE;

	pos = lseek(fd, 0, SEEK_CUR);
	if (pos < 0)
		goto __ERROR;

	if (lseek(fd, 0, SEEK_SET) < 0)
		goto __ERROR;

	#ifdef F_TLOCK

		if (lockf(fd, F_TLOCK, 0))
		{
			if (errno == EAGAIN)
				return TRUE;
			else
				goto __ERROR;
		}
		
	#else
	
		ERROR_warning("locking is not implemented");
	
	#endif

	if (lseek(fd, pos, SEEK_SET) < 0)
		goto __ERROR;

	return FALSE;

__ERROR:
	THROW_SYSTEM(errno, NULL);
	return TRUE;
}



void STREAM_lof(STREAM *stream, int64_t *len)
{
	int fd;
	int ilen;

	if (STREAM_is_closed(stream))
		THROW(E_CLOSED);

	*len = 0;

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
	if (STREAM_is_closed(stream))
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

void STREAM_check_blocking(STREAM *stream)
{
	int fd = STREAM_handle(stream);
	
	stream->common.blocking = (fd < 0) ? TRUE : ((fcntl(fd, F_GETFL) & O_NONBLOCK) == 0);
}

void STREAM_cancel(STREAM *stream)
{
	if (!stream->common.redirect)
		return;

	STREAM_close(stream->common.redirect);
	FREE(&stream->common.redirect);
	stream->common.redirected = FALSE;
}

void STREAM_begin(STREAM *stream)
{
	STREAM_cancel(stream);
	
	if (!stream->common.redirect)
	{
		ALLOC_ZERO(&stream->common.redirect, sizeof(STREAM));
		STREAM_open(stream->common.redirect, NULL, ST_STRING | ST_WRITE);
	}

	stream->common.redirected = TRUE;
}

void STREAM_end(STREAM *stream)
{
	if (!stream->common.redirect)
		return;
	
	stream->common.redirected = FALSE;
	STREAM_write(stream, stream->common.redirect->string.buffer, STRING_length(stream->common.redirect->string.buffer));
	STREAM_cancel(stream);
}

