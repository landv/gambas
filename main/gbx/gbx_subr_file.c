/***************************************************************************

  subr_file.c

  The file and input/output subroutines

  (c) 2000-2006 Benoît Minisini <gambas@users.sourceforge.net>

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

#include "gb_common.h"
#include "gb_common_buffer.h"

#include "gbx_subr.h"
#include "gb_file.h"
#include "gbx_stream.h"
#include "gbx_archive.h"
#include "gbx_api.h"
#include "gbx_local.h"
#include "gbx_regexp.h"
#include "gbx_list.h"
#include "gbx_string.h"
#include "gbx_c_file.h"
#include "gbx_print.h"

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
static long _len_pattern;
static long _ignore;

static STREAM *_stream;


static void push_stream(void **list, CSTREAM *stream)
{
  CSTREAM_NODE *slot;

  ALLOC(&slot, sizeof(CSTREAM_NODE), "push_stream");
  slot->stream = stream;
  //OBJECT_REF(stream, "push_stream");

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
  *list = ((CSTREAM_NODE *)*list)->next;
  FREE(&slot, "pop_stream");

  return stream;
}

static STREAM *get_default(long val)
{
  static STREAM_MEMORY memory_stream;
  STREAM *stream;

  switch(val)
  {
    case 0:
      if (_default_in)
        stream = CSTREAM_stream(((CSTREAM_NODE *)_default_in)->stream);
      else
        stream = CSTREAM_stream(&CFILE_in);
      break;
    case 1:
      if (_default_out)
        stream = CSTREAM_stream(((CSTREAM_NODE *)_default_out)->stream);
      else
        stream = CSTREAM_stream(&CFILE_out);
      break;
    case 2:
    	if (_default_err)
        stream = CSTREAM_stream(((CSTREAM_NODE *)_default_err)->stream);
			else
      	stream = CSTREAM_stream(&CFILE_err);
      break;
    default:
      memory_stream.common.type = &STREAM_memory;
      memory_stream.addr = (void *)val;
      memory_stream.pos = 0;
      stream = (STREAM *)&memory_stream;
      break;
  }

  return stream;
}

static STREAM *get_stream(VALUE *value, boolean can_default)
{
  STREAM *stream;

  if (TYPE_is_integer(value->type) && can_default)
    stream = get_default(value->_integer.value);
  else
  {
    if (VALUE_is_null(value))
      THROW(E_NULL);

    /*if (OBJECT_class(value->_object.object) != CLASS_Process)*/
    VALUE_conv(value, (TYPE)CLASS_Stream);

    stream = CSTREAM_stream(value->_object.object);
  }

  if (STREAM_is_closed(stream))
    THROW(E_CLOSED);

  return stream;
}


static char *get_path(VALUE *param)
{
  char *name;
  long len;

  SUBR_get_string_len(param, &name, &len);

  return STRING_conv_file_name(name, len);
}

PUBLIC void SUBR_open(void)
{
  CFILE *file;
  STREAM stream;
  int mode;

  SUBR_ENTER_PARAM(2);

  SUBR_check_integer(&PARAM[1]);
  mode = PARAM[1]._integer.value;

  STREAM_open(&stream, get_path(PARAM), mode);

  file = CFILE_create(&stream, mode);

  OBJECT_put(RETURN, file);

  SUBR_LEAVE();
}


PUBLIC void SUBR_close(void)
{
  SUBR_ENTER_PARAM(1);

  STREAM_close(get_stream(PARAM, FALSE));

  SUBR_LEAVE_VOID();
}


PUBLIC void SUBR_flush(void)
{
  SUBR_ENTER_PARAM(1);

  STREAM_flush(get_stream(PARAM, TRUE));

  SUBR_LEAVE_VOID();
}


static void print_it(char *addr, long len)
{
  STREAM_write(_stream, addr, len);
}

PUBLIC void SUBR_print(void)
{
  int i;
  char *addr;
  long len;

  SUBR_ENTER();

  if (NPARAM < 1)
    THROW(E_NEPARAM);

  _stream = get_stream(PARAM, TRUE);

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


PUBLIC void SUBR_linput(void)
{
  STREAM *stream;
  char *addr;

  SUBR_ENTER_PARAM(1);

  stream = get_stream(PARAM, TRUE);

  STREAM_line_input(stream, &addr);

  RETURN->type = T_STRING;
  RETURN->_string.addr = addr;
  RETURN->_string.start = 0;
  RETURN->_string.len = STRING_length(addr);

  SUBR_LEAVE();
}


PUBLIC void SUBR_input(void)
{
  static STREAM *stream = NULL;
  char *addr;

  SUBR_ENTER();

	if (NPARAM == 1)
  	stream = get_stream(PARAM, TRUE);

	if (stream)
	{
  	STREAM_input(stream, &addr);

		VALUE_from_string(RETURN, addr, STRING_length(addr));

		if (RETURN->type == T_NULL)
		{
			RETURN->type = T_STRING;
			RETURN->_string.addr = addr;
			RETURN->_string.start = 0;
			RETURN->_string.len = STRING_length(addr);
		}
	}
	else
		RETURN->type = T_NULL;

  SUBR_LEAVE();
}


PUBLIC void SUBR_eof(void)
{
  STREAM *stream;

  SUBR_ENTER();

  if (NPARAM == 1)
    stream = get_stream(PARAM, FALSE);
  else
    stream = get_default(0);

  //fprintf(stderr, "Eof(stream) = %d\n", STREAM_eof(stream));

  RETURN->type = T_BOOLEAN;
  RETURN->_boolean.value = STREAM_eof(stream) ? -1 : 0;

  SUBR_LEAVE();
}


PUBLIC void SUBR_lof(void)
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


PUBLIC void SUBR_seek(void)
{
  STREAM *stream;
  long long pos;
  long long len;
  long whence = SEEK_SET;

  SUBR_ENTER();

  stream = get_stream(PARAM, FALSE);

  if (NPARAM >= 2)
  {
    VALUE_conv(&PARAM[1], T_LONG);
    pos = PARAM[1]._long.value;

    if (NPARAM == 3)
    {
      VALUE_conv(&PARAM[2], T_INTEGER);
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


PUBLIC void SUBR_read(void)
{
  STREAM *stream;
  long len;
  bool do_not = FALSE;

  SUBR_ENTER();

  stream = get_stream(PARAM, TRUE);

  if (NPARAM == 3)
  {
    VALUE_conv(&PARAM[2], T_INTEGER);
    len = PARAM[2]._integer.value;
    if (len == 0)
      do_not = TRUE;
  }
  else
    len = 0;

  if (do_not)
    RETURN->type = T_NULL;
  else
    STREAM_read_type(stream, PARAM[1].type, RETURN, len);

  SUBR_LEAVE();
}


PUBLIC void SUBR_write(void)
{
  STREAM *stream;
  long len;

  SUBR_ENTER();

  stream = get_stream(PARAM, TRUE);

  if (NPARAM == 3)
  {
    VALUE_conv(&PARAM[2], T_INTEGER);
    len = PARAM[2]._integer.value;
  }
  else
    len = 0;

  STREAM_write_type(stream, PARAM[1].type, &PARAM[1], len);

  SUBR_LEAVE_VOID();
}


PUBLIC void SUBR_stat(void)
{
  const char *path;
  CSTAT *cstat;
  FILE_STAT info;
  bool follow = FALSE;

  SUBR_ENTER();

  path = get_path(PARAM);

	if (NPARAM == 2)
	{
		VALUE_conv(&PARAM[1], T_BOOLEAN);
		follow = PARAM[1]._boolean.value;
	}
	
  FILE_stat(path, &info, follow);

  OBJECT_new((void **)&cstat, CLASS_Stat, NULL, NULL);
  OBJECT_UNREF_KEEP((void **)&cstat, "SUBR_stat");
  cstat->info = info;
  STRING_new(&cstat->path, path, 0);

  OBJECT_put(RETURN, cstat);

  SUBR_LEAVE();
}


PUBLIC void SUBR_exist(void)
{
  bool exist;
  const char *path;

  SUBR_ENTER_PARAM(1);

  path = get_path(PARAM);

  exist = FILE_exist(path);

  RETURN->type = T_BOOLEAN;
  RETURN->_integer.value = exist ? -1 : 0;

  SUBR_LEAVE();
}


PUBLIC void SUBR_dir()
{
  GB_ARRAY array;
  const char *path;
  char *pattern;
  long len_pattern;
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
     // TODO: If STRING_conv fails, there are memory leaks.
      STRING_conv(&str, pattern, len_pattern, LOCAL_encoding, "UTF-8", TRUE);
      STRING_ref(str);
    }
    else
      STRING_new(&str, pattern, len_pattern);

    *((char **)GB_ArrayAdd(array)) = str;
  }

  RETURN->_object.class = OBJECT_class(array);
  RETURN->_object.object = array;

  SUBR_LEAVE();
}


static void found_file(const char *path)
{
  char *str;
  long len;

  path += _ignore;
  len = strlen(path);

  if (_pattern && !REGEXP_match(_pattern, _len_pattern, path, len))
    return;

  if (!LOCAL_is_UTF8)
  {
    STRING_conv(&str, path, len, LOCAL_encoding, "UTF-8", TRUE);
    STRING_ref(str);
  }
  else
    STRING_new(&str, path, len);

  *((char **)GB_ArrayAdd(_result)) = str;
}

PUBLIC void SUBR_rdir()
{
  const char *path;
  int attr = 0;

  SUBR_ENTER();

  path = get_path(PARAM);

  if (NPARAM >= 2)
  {
    SUBR_get_string_len(&PARAM[1], &_pattern, &_len_pattern);
    if (NPARAM == 3)
      attr = SUBR_get_integer(&PARAM[2]);
  }
  else
    _pattern = NULL;

  GB_ArrayNew(&_result, T_STRING, 0);

	if (!path || *path == 0)
		path = ".";
  _ignore = strlen(path);
  if (_ignore > 0 && path[_ignore - 1] != '/')
    _ignore++;

  FILE_recursive_dir(path, found_file, NULL, attr);

  RETURN->_object.class = OBJECT_class(_result);
  RETURN->_object.object = _result;

  SUBR_LEAVE();
}


PUBLIC void SUBR_kill(void)
{
  SUBR_ENTER_PARAM(1);

  FILE_unlink(get_path(PARAM));

  SUBR_LEAVE_VOID();
}


PUBLIC void SUBR_mkdir(void)
{
  SUBR_ENTER_PARAM(1);

  FILE_mkdir(get_path(PARAM));

  SUBR_LEAVE_VOID();
}


PUBLIC void SUBR_rmdir(void)
{
  SUBR_ENTER_PARAM(1);

  FILE_rmdir(get_path(PARAM));

  SUBR_LEAVE_VOID();
}


PUBLIC void SUBR_rename(void)
{
  SUBR_ENTER_PARAM(2);

  FILE_rename(get_path(&PARAM[0]), get_path(&PARAM[1]));

  SUBR_LEAVE_VOID();
}


PUBLIC void SUBR_temp(void)
{
  char *temp;
  long len;

  SUBR_ENTER();

  if (NPARAM == 0)
    temp = FILE_make_temp(&len, NULL);
  else
    temp = FILE_make_temp(&len, SUBR_get_string(PARAM));

  STRING_new_temp_value(RETURN, temp, len);

  SUBR_LEAVE();
}


PUBLIC void SUBR_isdir(void)
{
  bool isdir;
  const char *path;

  SUBR_ENTER_PARAM(1);

  path = get_path(PARAM);

  isdir = FILE_isdir(path);

  RETURN->type = T_BOOLEAN;
  RETURN->_integer.value = isdir ? -1 : 0;

  SUBR_LEAVE();
}


PUBLIC void SUBR_copy(void)
{
  SUBR_ENTER_PARAM(2);

  FILE_copy(get_path(&PARAM[0]), get_path(&PARAM[1]));

  SUBR_LEAVE_VOID();
}


PUBLIC void SUBR_access(void)
{
  int access;

  SUBR_ENTER();

  if (NPARAM == 1)
    access = R_OK;
  else
  {
    VALUE_conv(&PARAM[1], T_INTEGER);
    access = PARAM[1]._integer.value;
  }

  RETURN->type = T_BOOLEAN;
  RETURN->_integer.value = FILE_access(get_path(PARAM), access) ? -1 : 0;

  SUBR_LEAVE();
}



PUBLIC void SUBR_link(void)
{
  SUBR_ENTER_PARAM(2);

  /* Parameters are NOT inverted ANYMORE! */
  FILE_link(get_path(&PARAM[0]), get_path(&PARAM[1]));

  SUBR_LEAVE_VOID();
}


PUBLIC void SUBR_lock(void)
{
  SUBR_ENTER_PARAM(1);

	if (EXEC_code & 0x1F)
	{
	  STREAM_close(get_stream(PARAM, FALSE));
	  SUBR_LEAVE_VOID();
	}
	else
	{
		STREAM stream;
		CFILE *file;
		const char *path = get_path(PARAM);
		
		if (FILE_is_relative(path))
			THROW(E_BADPATH);

	  STREAM_open(&stream, path, ST_WRITE | ST_CREATE | ST_DIRECT);
	  STREAM_lock(&stream);
	  
  	file = CFILE_create(&stream, ST_WRITE | ST_CREATE | ST_DIRECT);
  	OBJECT_put(RETURN, file);
  	SUBR_LEAVE();
	}
}


PUBLIC void SUBR_inp_out(void)
{
  CSTREAM *stream;
  void **where;

  SUBR_ENTER_PARAM(1);

	switch(EXEC_code & 0x1F)
	{
		case 0: where = &_default_in; break;
		case 1: where = &_default_out; break;
		default: where = &_default_err; break;
  }

  if (VALUE_is_null(PARAM))
  {
    stream = pop_stream(where);
    if (stream)
      OBJECT_UNREF(&stream, "SUBR_inp_out");
    return;
  }

  VALUE_conv(PARAM, (TYPE)CLASS_Stream);

  stream = PARAM->_object.object;
  OBJECT_REF(stream, "SUBR_inp_out");

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
    OBJECT_UNREF(&stream, "free_list");
  }
}

PUBLIC void SUBR_exit_inp_out(void)
{
  free_list((void **)&_default_in);
  free_list((void **)&_default_out);
  free_list((void **)&_default_err);
}


PUBLIC void SUBR_dfree(void)
{
  SUBR_ENTER_PARAM(1);;

  RETURN->type = T_LONG;
  RETURN->_long.value = FILE_free(get_path(PARAM));

  SUBR_LEAVE();
}

PUBLIC void SUBR_debug(void)
{
  const int NPARAM = 0;
  STREAM *stream = get_default(2);
  const char *s = DEBUG_get_current_position();
  
  STREAM_write(stream, (void *)s, strlen(s));
  STREAM_write(stream, ": ", 2);
  
  SUBR_LEAVE();
}
