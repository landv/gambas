/***************************************************************************

	gbx_subr_file.c

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

#include "gb_common.h"
#include "gb_common_buffer.h"

#include "gbx_subr.h"
#include "gb_file.h"
#include "gb_list.h"
#include "gbx_stream.h"
#include "gbx_archive.h"
#include "gbx_api.h"
#include "gbx_local.h"
#include "gbx_regexp.h"
#include "gbx_string.h"
#include "gbx_c_file.h"
#include "gbx_math.h"

typedef
	struct _stream {
		struct _stream *next;
		CSTREAM *stream;
		}
	CSTREAM_NODE;

static void *_default_in = NULL;
static void *_default_out = NULL;
static void *_default_err = NULL;

static GB_ARRAY _result;
static char *_pattern;
static int _len_pattern;
static int _ignore;

static STREAM *_stream;


static void push_stream(void **list, CSTREAM *stream)
{
	CSTREAM_NODE *slot;

	ALLOC(&slot, sizeof(CSTREAM_NODE));
	slot->stream = stream;
	//OBJECT_REF(stream);

	slot->next = *list;
	*list = slot;
}


static CSTREAM *pop_stream(void **list)
{
	CSTREAM *stream;
	CSTREAM_NODE *slot;

	if (!*list)
		return NULL;

	stream = ((CSTREAM_NODE *)*list)->stream;
	slot = *list;
	*list = slot->next;
	FREE(&slot);

	return stream;
}

static STREAM *get_default(intptr_t val)
{
	STREAM *stream;

	switch(val)
	{
		case 0:
			if (_default_in)
				stream = CSTREAM_stream(((CSTREAM_NODE *)_default_in)->stream);
			else
				stream = CSTREAM_stream(CFILE_in);
			break;
		case 1:
			if (_default_out)
				stream = CSTREAM_stream(((CSTREAM_NODE *)_default_out)->stream);
			else
				stream = CSTREAM_stream(CFILE_out);
			break;
		case 2:
			if (_default_err)
				stream = CSTREAM_stream(((CSTREAM_NODE *)_default_err)->stream);
			else
				stream = CSTREAM_stream(CFILE_err);
			break;
		default:
			stream = NULL;
	}

	return stream;
}

#define _get_stream(_value, _can_default) \
	STREAM *stream; \
	\
	VARIANT_undo(_value); \
	\
	if ((_can_default) && TYPE_is_integer((_value)->type) && (_value)->_integer.value >= 0 && (_value)->_integer.value <= 2) \
		stream = get_default((intptr_t)(_value)->_integer.value); \
	else \
	{ \
		if (TYPE_is_object((_value)->type) && (_value)->_object.object && OBJECT_class((_value)->_object.object)->is_stream) \
			stream = CSTREAM_stream((_value)->_object.object); \
		else \
		{ \
			if (VALUE_is_null(_value)) \
				THROW(E_NULL); \
			\
			VALUE_conv_object((_value), (TYPE)CLASS_Stream); \
			stream = NULL; \
		} \
	}

#define get_stream(_value, _can_default) \
({ \
	_get_stream(_value, _can_default); \
	\
	if (STREAM_is_closed(stream)) \
		THROW(E_CLOSED); \
	\
	stream; \
})

#define get_stream_for_writing(_value, _can_default) \
({ \
	_get_stream(_value, _can_default); \
	\
	if (STREAM_is_closed_for_writing(stream)) \
		THROW(E_CLOSED); \
	\
	stream; \
})

static char *get_path(VALUE *param)
{
	char *name;
	int len;

	SUBR_get_string_len(param, &name, &len);

	return STRING_conv_file_name(name, len);
}

void SUBR_open(ushort code)
{
	CFILE *file;
	STREAM stream;
	int mode;
	void *addr;

	SUBR_ENTER_PARAM(2);

	SUBR_check_integer(&PARAM[1]);
	mode = PARAM[1]._integer.value;

	if (code & 0x3F)
	{
		if (TYPE_is_pointer(PARAM->type))
			addr = (void *)PARAM->_pointer.value;
		else
			THROW(E_TYPE, "Pointer", TYPE_get_name(PARAM->type));
		
		STREAM_open(&stream, (char *)addr, mode | ST_MEMORY);
	}
	else if (mode & ST_STRING)
	{
		char *str;

		STREAM_open(&stream, NULL, mode);

		if (!VALUE_is_null(PARAM))
		{
			str = SUBR_get_string(PARAM);
			
			if (mode & ST_WRITE)
			{
				stream.string.buffer = STRING_new(str, STRING_length(str));
			}
			else
			{
				stream.string.buffer = str;
				STRING_ref(str);
			}
			stream.string.size = STRING_length(str);
		}
	}
	else
	{
		STREAM_open(&stream, get_path(PARAM), mode);
	}

	file = CFILE_create(&stream, mode);

	OBJECT_put(RETURN, file);

	SUBR_LEAVE();
}


void SUBR_close(void)
{
	STREAM *stream;

	SUBR_ENTER_PARAM(1);

	stream = get_stream(PARAM, FALSE);

	if (stream->type == &STREAM_string)
	{
		char *buffer = stream->string.buffer;

		RETURN->type = T_STRING;
		RETURN->_string.addr = buffer;
		RETURN->_string.start = 0;
		RETURN->_string.len = stream->string.size;

		STRING_ref(buffer);
		STREAM_close(stream);
		STRING_free_later(buffer);

		//fprintf(stderr, "buffer ref = %d\n", STRING_from_ptr(buffer)->ref);

		SUBR_LEAVE();
	}
	else
	{
		STREAM_close(stream);
		SUBR_LEAVE_VOID();
	}
}


void SUBR_flush(void)
{
	SUBR_ENTER_PARAM(1);

	STREAM_flush(get_stream(PARAM, TRUE));

	SUBR_LEAVE_VOID();
}


/*static void print_it(char *addr, long len)
{
	STREAM_write(_stream, addr, len);
}*/

void SUBR_print(ushort code)
{
	int i;
	char *addr;
	int len;

	SUBR_ENTER();

	if (NPARAM < 1)
		THROW(E_NEPARAM);

	_stream = get_stream_for_writing(PARAM, TRUE);

	//PRINT_init(print_it, FALSE);

	for (i = 1; i < NPARAM; i++)
	{
		PARAM++;
		//PRINT_value(PARAM);
		VALUE_to_string(PARAM, &addr, &len);
		if (len == 1 && *addr == '\n')
			STREAM_write_eol(_stream);
		else
			STREAM_write(_stream, addr, len);
	}

	SUBR_LEAVE_VOID();
}


void SUBR_linput(void)
{
	STREAM *stream;
	char *addr;

	stream = get_stream(&SP[-1], TRUE);

	addr = STREAM_line_input(stream, NULL);

	SP--;
	if (!TYPE_is_integer(SP->type))
		RELEASE_OBJECT(SP);
	
	SP->type = T_STRING;
	SP->_string.addr = addr;
	SP->_string.start = 0;
	SP->_string.len = STRING_length(addr);
	
	SP++;
}


void SUBR_input(ushort code)
{
	static STREAM *stream = NULL;
	char *addr;

	SUBR_ENTER();

	if (NPARAM == 1)
		stream = get_stream(PARAM, TRUE);

	if (stream)
		addr = STREAM_input(stream);
	else
		addr = NULL;
		
	if (NPARAM == 1)
	{
		SP--;
		if (!TYPE_is_integer(SP->type))
			RELEASE_OBJECT(SP);
	}
	
	if (addr)
	{
		//VALUE_from_string(SP, addr, STRING_length(addr));
		SP->type = T_STRING;
		SP->_string.addr = addr;
		SP->_string.start = 0;
		SP->_string.len = STRING_length(addr);
	}
	else
		SP->type = T_NULL;
		
	SP++;
}


void SUBR_eof(ushort code)
{
	STREAM *stream;
	bool eof;

	SUBR_ENTER();

	if (NPARAM == 1)
	{
		stream = get_stream(PARAM, FALSE);
		eof = STREAM_eof(stream);
		RELEASE_OBJECT(PARAM);
		SP--;
	}
	else
	{
		stream = get_default(0);
		eof = STREAM_eof(stream);
	}

	SP->type = T_BOOLEAN;
	SP->_boolean.value = (-eof);
	SP++;
}


void SUBR_lof(ushort code)
{
	STREAM *stream;

	SUBR_ENTER();

	if (NPARAM == 1)
		stream = get_stream(PARAM, FALSE);
	else
		stream = get_default(0);

	RETURN->type = T_LONG;
	STREAM_lof(stream, &(RETURN->_long.value));

	SUBR_LEAVE();
}


void SUBR_seek(ushort code)
{
	STREAM *stream;
	int64_t pos;
	int64_t len;
	int whence = SEEK_SET;

	SUBR_ENTER();

	stream = get_stream(PARAM, FALSE);

	if (NPARAM >= 2)
	{
		VALUE_conv(&PARAM[1], T_LONG);
		pos = PARAM[1]._long.value;

		if (NPARAM == 3)
		{
			VALUE_conv_integer(&PARAM[2]);
			whence = PARAM[2]._integer.value;
			if (whence != SEEK_SET && whence != SEEK_CUR && whence != SEEK_END)
				THROW(E_ARG);
		}
		else
		{
			if (pos < 0)
			{
				STREAM_lof(stream, &len);
				pos += len;
			}
		}

		STREAM_seek(stream, pos, (int)whence);
		RETURN->type = T_VOID;
	}
	else
	{
		RETURN->type = T_LONG;
		RETURN->_long.value = STREAM_tell(stream);
	}

	SUBR_LEAVE();
}

void SUBR_read(ushort code)
{
	STREAM *stream;
	int len = 0;

	SUBR_ENTER_PARAM(2);

	stream = get_stream(PARAM, TRUE);
	
	if (code & 0x3F)
	{
		VALUE_conv_integer(&PARAM[1]);
		len = PARAM[1]._integer.value;
		
		if (len == 0)
		{
			RETURN->type = T_NULL;
		}
		else if (len > 0)
		{
			STRING_new_temp_value(RETURN, NULL, len);
			STREAM_read(stream, RETURN->_string.addr, len);
		}
		else
		{
			len = (-len);
			
			RETURN->type = T_STRING;
			RETURN->_string.addr = STRING_new(NULL, len);
			RETURN->_string.start = 0;
			RETURN->_string.len = len;
			
			STREAM_read_max(stream, RETURN->_string.addr, len);
			
			if (STREAM_eff_read == 0)
			{
				RETURN->type = T_NULL;
				STRING_free(&RETURN->_string.addr);
			}
			else
			{
				if (STREAM_eff_read < len)
				{
					RETURN->_string.addr = STRING_extend(RETURN->_string.addr, STREAM_eff_read);
					RETURN->_string.len = STREAM_eff_read;
				}
				STRING_extend_end(RETURN->_string.addr);
			}
		}
	}
	else
	{
		TYPE type = SUBR_get_type(&PARAM[1]);
		STREAM_read_type(stream, type, RETURN);
	}

	SUBR_LEAVE();
}


void SUBR_write(ushort code)
{
	STREAM *stream;

	SUBR_ENTER_PARAM(3);

	stream = get_stream_for_writing(PARAM, TRUE);

	if (code & 0x3F)
	{
		char *str;
		int len;
		int lenw;
		
		VALUE_conv_integer(&PARAM[2]);
		lenw = PARAM[2]._integer.value;
		
		if (TYPE_is_pointer(PARAM[1].type))
		{
			if (lenw < 0)
				lenw = 0;
			len = lenw;
			str = (char *)PARAM[1]._pointer.value;
			fprintf(stderr, "%p\n", str);
		}
		else
		{
			SUBR_get_string_len(&PARAM[1], &str, &len);
			if (lenw < 0)
				lenw = len;
		}
		
		if (lenw > 0)
		{
			STREAM_write(stream, str, Min(len, lenw));
			if (lenw > len)
				STREAM_write_zeros(stream, lenw - len);
		}
	}
	else
	{
		TYPE type;
		type = SUBR_get_type(&PARAM[2]);
		VALUE_conv(&PARAM[1], type);
		STREAM_write_type(stream, type, &PARAM[1]);
	}
	
	SUBR_LEAVE_VOID();
}


void SUBR_stat(ushort code)
{
	const char *path;
	CSTAT *cstat;
	FILE_STAT info;
	bool follow = FALSE;

	SUBR_ENTER();

	path = get_path(PARAM);

	if (NPARAM == 2)
		follow = SUBR_get_boolean(&PARAM[1]);
	
	FILE_stat(path, &info, follow);

	cstat = OBJECT_new(CLASS_Stat, NULL, NULL);
	OBJECT_UNREF_KEEP(cstat);
	cstat->info = info;
	cstat->path = STRING_new_zero(path);

	RETURN->_object.class = CLASS_Stat;
	RETURN->_object.object = cstat;

	SUBR_LEAVE();
}


void SUBR_exist(ushort code)
{
	bool exist;
	const char *path;
	bool follow = FALSE;

	SUBR_ENTER();
	
	if (!NPARAM)
	{
		NPARAM++;
		PARAM--;
	}

	path = get_path(PARAM);

	if (NPARAM == 2)
		follow = SUBR_get_boolean(&PARAM[1]);

	exist = FILE_exist_follow(path, follow);

	RETURN->type = T_BOOLEAN;
	RETURN->_integer.value = exist ? -1 : 0;

	SUBR_LEAVE();
}


void SUBR_dir(ushort code)
{
	GB_ARRAY array;
	const char *path;
	char *pattern;
	int len_pattern;
	char *str;
	int attr = 0;

	SUBR_ENTER();

	path = get_path(PARAM);

	if (NPARAM >= 2)
	{
		pattern = SUBR_get_string(&PARAM[1]);
		if (NPARAM == 3)
			attr = SUBR_get_integer(&PARAM[2]);
	}
	else
		pattern = NULL;

	FILE_dir_first(path, pattern, attr);

	GB_ArrayNew(&array, T_STRING, 0);

	while (!FILE_dir_next(&pattern, &len_pattern))
	{
		if (!LOCAL_is_UTF8)
		{
			if (STRING_conv(&str, pattern, len_pattern, LOCAL_encoding, SC_UTF8, FALSE))
				str = STRING_new(pattern, len_pattern);
			else
				STRING_ref(str);
		}
		else
			str = STRING_new(pattern, len_pattern);

		*((char **)GB_ArrayAdd(array)) = str;
	}

	RETURN->_object.class = OBJECT_class(array);
	RETURN->_object.object = array;

	SUBR_LEAVE();
}


static void found_file(const char *path)
{
	char *str;
	int len;

	path += _ignore;
	len = strlen(path);

	if (_pattern && !REGEXP_match(_pattern, _len_pattern, path, len))
		return;

	if (!LOCAL_is_UTF8)
	{
		if (STRING_conv(&str, path, len, LOCAL_encoding, SC_UTF8, FALSE))
			str = STRING_new(path, len);
		else
			STRING_ref(str);
	}
	else
		str = STRING_new(path, len);

	*((char **)GB_ArrayAdd(_result)) = str;
}

void SUBR_rdir(ushort code)
{
	const char *path;
	int attr = 0;
	bool follow = FALSE;

	SUBR_ENTER();

	path = get_path(PARAM);

	if (NPARAM >= 2)
	{
		SUBR_get_string_len(&PARAM[1], &_pattern, &_len_pattern);
		if (NPARAM >= 3)
		{
			attr = SUBR_get_integer(&PARAM[2]);
			if (NPARAM == 4)
				follow = SUBR_get_boolean(&PARAM[3]);
		}
	}
	else
		_pattern = NULL;

	GB_ArrayNew(&_result, T_STRING, 0);

	if (!path || *path == 0)
		path = ".";
	_ignore = strlen(path);
	if (_ignore > 0 && path[_ignore - 1] != '/')
		_ignore++;

	FILE_recursive_dir(path, found_file, NULL, attr, follow);

	RETURN->_object.class = OBJECT_class(_result);
	RETURN->_object.object = _result;

	SUBR_LEAVE();
}


void SUBR_kill(ushort code)
{
	SUBR_ENTER_PARAM(1);

	switch(code & 0xFF)
	{
		case 0:
			FILE_unlink(get_path(PARAM));
			break;
			
		case 1:
			FILE_mkdir(get_path(PARAM));
			break;
			
		case 2:
			FILE_rmdir(get_path(PARAM));
			break;

		default:
			THROW_ILLEGAL();
	}

	SUBR_LEAVE_VOID();
}


void SUBR_mkdir(ushort code)
{
	SUBR_ENTER_PARAM(1);
	
	switch (code & 0xFF)
	{
		case 0: // Deprecated Mkdir
			SUBR_kill(1);
			return;
			
		case 1: // Even
			VALUE_conv(PARAM, T_LONG);
			PARAM->type = GB_T_BOOLEAN;
			PARAM->_boolean.value = (PARAM->_long.value & 1) == 0 ? -1 : 0;
			break;

		case 2: // Odd
			VALUE_conv(PARAM, T_LONG);
			PARAM->type = GB_T_BOOLEAN;
			PARAM->_boolean.value = (PARAM->_long.value & 1) != 0 ? -1 : 0;
			break;
		
		default:
			THROW_ILLEGAL();
	}
}


void SUBR_rmdir(ushort code)
{
	SUBR_ENTER();
	int min, max;

	switch (code & 0x3F)
	{
		case 0: // Deprecated RmDir
			SUBR_kill(2);
			return;

		default: // Rand

			if (NPARAM == 1)
			{
				VALUE_conv_integer(PARAM);
				min = 0;
				max = PARAM->_integer.value;
			}
			else
			{
				VALUE_conv_integer(PARAM);
				VALUE_conv_integer(&PARAM[1]);
				min = PARAM->_integer.value;
				max = PARAM[1]._integer.value;
				if (max < min)
				{
					int temp = max;
					max = min;
					min = temp;
				}
			}

			RETURN->type = T_INTEGER;
			RETURN->_integer.value = min + (int)(rnd() * (max + 1 - min));
			break;
	}

	SUBR_LEAVE();
}


void SUBR_move(ushort code)
{
	char *path;
	char *auth;
	FILE_STAT info;
	
	SUBR_ENTER_PARAM(2);

	path = get_path(&PARAM[0]);
	
	switch (code & 0xFF)
	{
		case 0: // Move
			
			FILE_rename(path, get_path(&PARAM[1]));
			break;
			
		case 1: // Copy
			
			FILE_copy(path, get_path(&PARAM[1]));
			break;
			
		case 2: // Link
			
			/* Parameters are NOT inverted ANYMORE! */
			FILE_link(path, get_path(&PARAM[1]));
			break;
			
		case 3: // Chmod
			
			auth = SUBR_get_string(&PARAM[1]);
			FILE_stat(path, &info, TRUE);
			FILE_chmod(path, FILE_mode_from_string(info.mode, auth));
			break;
			
		case 4: // Chown
			
			auth = SUBR_get_string(&PARAM[1]);
			FILE_chown(path, auth);
			break;
			
		case 5: // Chgrp
			
			auth = SUBR_get_string(&PARAM[1]);
			FILE_chgrp(path, auth);
			break;
			
		default:
			THROW_ILLEGAL();
	}

	SUBR_LEAVE_VOID();
}


void SUBR_link(ushort code)
{
	SUBR_ENTER_PARAM(1);
	
	switch (code & 0xFF)
	{
		case 0: // Deprecated Link
			SUBR_move(2);
			return;
			
		case 1: // IsNan
			VALUE_conv(PARAM, T_FLOAT);
			PARAM->type = GB_T_BOOLEAN;
			PARAM->_boolean.value = isnan(PARAM->_float.value) ? -1 : 0;
			break;

		case 2: // IsInf
			VALUE_conv(PARAM, T_FLOAT);
			PARAM->type = GB_T_INTEGER;
			PARAM->_integer.value = isinf(PARAM->_float.value);
			break;
		
		default:
			THROW_ILLEGAL();
	}
}


void SUBR_temp(ushort code)
{
	char *temp;
	int len;

	SUBR_ENTER();

	if (NPARAM == 0)
		temp = FILE_make_temp(&len, NULL);
	else
		temp = FILE_make_temp(&len, SUBR_get_string(PARAM));

	STRING_new_temp_value(RETURN, temp, len);

	SUBR_LEAVE();
}


void SUBR_isdir(void)
{
	bool isdir;
	const char *path;

	SUBR_ENTER_PARAM(1);

	path = get_path(PARAM);

	isdir = FILE_is_dir(path);

	RETURN->type = T_BOOLEAN;
	RETURN->_integer.value = isdir ? -1 : 0;

	SUBR_LEAVE();
}


void SUBR_access(ushort code)
{
	int access;

	SUBR_ENTER();

	if (NPARAM == 1)
		access = R_OK;
	else
	{
		VALUE_conv_integer(&PARAM[1]);
		access = PARAM[1]._integer.value;
	}

	RETURN->type = T_BOOLEAN;
	RETURN->_integer.value = FILE_access(get_path(PARAM), access) ? -1 : 0;

	SUBR_LEAVE();
}


void SUBR_lock(ushort code)
{
	code &= 0x1F;

	if (code == 1)
	{
		SUBR_ENTER_PARAM(1);
		STREAM_close(get_stream(PARAM, FALSE));
		SUBR_LEAVE_VOID();
	}
	else
	{
		SUBR_ENTER_PARAM((code == 2) ? 2 : 1);
		STREAM stream;
		CFILE *file;
		const char *path = get_path(PARAM);
		double wait, timer;

		if (FILE_is_relative(path))
			THROW(E_BADPATH);

		if (code == 2)
		{
			DATE_timer(&wait, FALSE);
			wait += SUBR_get_float(&PARAM[1]);
		}

		STREAM_open(&stream, path, ST_WRITE | ST_CREATE | ST_DIRECT);

		for(;;)
		{
			if (!STREAM_lock(&stream))
				break;

			if (code == 2)
			{
				DATE_timer(&timer, FALSE);
				if (timer < wait)
				{
					usleep(10000);
					continue;
				}
			}

			STREAM_close(&stream);
			THROW(E_LOCK);
		}

		file = CFILE_create(&stream, ST_WRITE | ST_CREATE | ST_DIRECT);
		OBJECT_put(RETURN, file);
		SUBR_LEAVE();
	}
}


void SUBR_inp_out(ushort code)
{
	CSTREAM *stream;
	void **where;

	SUBR_ENTER_PARAM(1);

	switch(code & 0x1F)
	{
		case 0: where = &_default_in; break;
		case 1: where = &_default_out; break;
		default: where = &_default_err; break;
	}

	if (VALUE_is_null(PARAM))
	{
		stream = pop_stream(where);
		if (stream)
			OBJECT_UNREF(stream);
		return;
	}

	VALUE_conv_object(PARAM, (TYPE)CLASS_Stream);

	stream = PARAM->_object.object;
	OBJECT_REF(stream);

	push_stream(where, stream);

	SUBR_LEAVE_VOID();
}

static void free_list(void **list)
{
	CSTREAM *stream;

	for(;;)
	{
		stream = pop_stream(list);
		if (!stream)
			return;
		OBJECT_UNREF(stream);
	}
}

void SUBR_exit_inp_out(void)
{
	free_list((void **)&_default_in);
	free_list((void **)&_default_out);
	free_list((void **)&_default_err);
}


void SUBR_dfree(void)
{
	SUBR_ENTER_PARAM(1);

	RETURN->type = T_LONG;
	RETURN->_long.value = FILE_free(get_path(PARAM));

	SUBR_LEAVE();
}

void SUBR_debug(void)
{
	const int NPARAM = 0;
	STREAM *stream = get_default(2);
	const char *s = DEBUG_get_current_position();
	
	STREAM_write(stream, (void *)s, strlen(s));
	STREAM_write(stream, ": ", 2);
	
	SUBR_LEAVE();
}

