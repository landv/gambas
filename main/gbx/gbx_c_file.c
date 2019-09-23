/***************************************************************************

  gbx_c_file.c

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

#define __GBX_C_FILE_C

#include "gbx_info.h"

#ifndef GBX_INFO

#include <stdio.h>
#include <pwd.h>
#include <grp.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <termios.h>

#include "gb_common.h"
#include "gb_list.h"
#include "gb_file.h"

#include "gbx_api.h"
#include "gambas.h"
#include "gbx_class.h"
#include "gbx_stream.h"
#include "gbx_exec.h"
#include "gbx_project.h"
#include "gbx_string.h"
#include "gbx_date.h"
#include "gbx_watch.h"
#include "gbx_signal.h"

#include "gbx_c_file.h"

#define STREAM_FD STREAM_handle(CSTREAM_stream(THIS_STREAM))

CFILE *CFILE_in, *CFILE_out, *CFILE_err;

DECLARE_EVENT(EVENT_Read);
DECLARE_EVENT(EVENT_Write);
DECLARE_EVENT(EVENT_Resize);

static GB_FUNCTION _read_func;

static char _buffer[16];

static bool _term_init = FALSE;
static ushort _term_width = 0;
static ushort _term_height = 0;
static SIGNAL_CALLBACK *_SIGWINCH_callback = NULL;
static GB_FUNCTION _term_resize_func;


static void callback_read(int fd, int type, CSTREAM *stream)
{
	if (!STREAM_read_ahead(CSTREAM_stream(stream)))
		GB_Raise(stream, EVENT_Read, 0);
	else
		WATCH_little_sleep();
}

static void callback_write(int fd, int type, CSTREAM *stream)
{
	GB_Raise(stream, EVENT_Write, 0);
}

static void cb_term_resize(int signum, intptr_t data)
{
	_term_width = _term_height = 0;
	if (CFILE_in)
		GB_Raise(CFILE_in, EVENT_Resize, 0);
}

static void watch_stream(CSTREAM *_object, int mode, bool on)
{
	STREAM *stream = &THIS_STREAM->stream;
	int fd = STREAM_handle(stream);
	
	if (mode & STO_READ)
		GB_Watch(fd, GB_WATCH_READ, (void *)(on ? callback_read : NULL), (intptr_t)THIS);

	if (mode & STO_WRITE)
		GB_Watch(fd, GB_WATCH_WRITE, (void *)(on ? callback_write : NULL), (intptr_t)THIS);
}

CFILE *CFILE_create(STREAM *stream, int mode)
{
	CFILE *file = OBJECT_new(CLASS_File, NULL, NULL);
	OBJECT_UNREF_KEEP(file);

	if (stream)
	{
		*CSTREAM_stream(file) = *stream;
		//file->watch_fd = -1;

		if (mode & STO_WATCH)
		{
			watch_stream(&file->ob, mode, TRUE);
			OBJECT_attach((OBJECT *)file, OP ? (OBJECT *)OP : (OBJECT *)CP, "File");
		}
	}

	return file;
}

static CFILE *create_default_stream(FILE *file, int mode)
{
	CFILE *ob;
	STREAM stream;
	bool tty = isatty(fileno(file));
	
	CLEAR(&stream);
	stream.type = &STREAM_buffer;
	stream.common.available_now = !tty;
	stream.common.no_read_ahead = tty;
	stream.common.standard = TRUE;
	stream.buffer.file = file;
	STREAM_check_blocking(&stream);
	ob = CFILE_create(&stream, mode);
	OBJECT_REF(ob);
	return ob;
}

void CFILE_init(void)
{
	CFILE_in = create_default_stream(stdin, STO_READ);
	CFILE_out = create_default_stream(stdout, STO_WRITE);
	CFILE_err = create_default_stream(stderr, STO_WRITE);
}

void CFILE_exit(void)
{
	OBJECT_UNREF(CFILE_in);
	OBJECT_UNREF(CFILE_out);
	OBJECT_UNREF(CFILE_err);
	
	if (_term_init)
		SIGNAL_unregister(SIGWINCH, _SIGWINCH_callback);
}

void CFILE_init_watch(void)
{
	bool has_term_func = GB_GetFunction(&_term_resize_func, PROJECT_class, "Application_Resize", "", "") == 0;
	bool has_read_func = GB_GetFunction(&_read_func, PROJECT_class, "Application_Read", "", "") == 0;
	
	if (has_term_func || has_read_func)
		OBJECT_attach((OBJECT *)CFILE_in, (OBJECT *)PROJECT_class, "Application");
	
	if (has_read_func)
	{
		//fprintf(stderr, "watch stdin\n");
		//CFILE_in->watch_fd = STDIN_FILENO;
		GB_Watch(STDIN_FILENO, GB_WATCH_READ, (void *)callback_read, (intptr_t)CFILE_in);
	}
}

BEGIN_METHOD_VOID(File_free)

	STREAM_close(&THIS->ob.stream);

END_METHOD


BEGIN_PROPERTY(File_In)

	GB_ReturnObject(CFILE_in);

END_PROPERTY


BEGIN_PROPERTY(File_Out)

	GB_ReturnObject(CFILE_out);

END_PROPERTY


BEGIN_PROPERTY(File_Err)

	GB_ReturnObject(CFILE_err);

END_PROPERTY


BEGIN_METHOD_VOID(Stat_free)

	STRING_unref(&THIS_STAT->path);

END_METHOD


BEGIN_PROPERTY(Stat_Type)

	GB_ReturnInt(THIS_STAT->info.type);

END_PROPERTY


BEGIN_PROPERTY(Stat_Path)

	GB_ReturnString(THIS_STAT->path);

END_PROPERTY


BEGIN_PROPERTY(Stat_Link)

	if (THIS_STAT->info.type == GB_STAT_LINK)
		GB_ReturnNewZeroString(FILE_readlink(THIS_STAT->path));
	else
		GB_ReturnVoidString();

END_PROPERTY


BEGIN_PROPERTY(Stat_Mode)

	GB_ReturnInt(THIS_STAT->info.mode);

END_PROPERTY


BEGIN_PROPERTY(Stat_Hidden)

	GB_ReturnBoolean(THIS_STAT->info.hidden);

END_PROPERTY


BEGIN_PROPERTY(Stat_Size)

	GB_ReturnLong(THIS_STAT->info.size);

END_PROPERTY


BEGIN_PROPERTY(Stat_LastAccess)

	VALUE date;

	DATE_from_time(THIS_STAT->info.atime, 0, &date);

	GB_ReturnDate((GB_DATE *)&date);

END_PROPERTY


BEGIN_PROPERTY(Stat_LastChange)

	VALUE date;

	DATE_from_time(THIS_STAT->info.ctime, 0, &date);

	GB_ReturnDate((GB_DATE *)&date);

END_PROPERTY


BEGIN_PROPERTY(Stat_Time)

	VALUE date;

	DATE_from_time(THIS_STAT->info.mtime, 0, &date);

	GB_ReturnDate((GB_DATE *)&date);

END_PROPERTY


static char *get_file_user(CFILE *_object)
{
	struct passwd *pwd;
	uid_t uid = (uid_t)THIS_STAT->info.uid;

	if (uid == 0)
		return "root";
	else
	{
		pwd = getpwuid(uid);
		if (!pwd)
		{
			snprintf(_buffer, sizeof(_buffer), "%d", (int)uid);
			return _buffer;
		}
		else
			return pwd->pw_name;
	}
}

BEGIN_PROPERTY(Stat_User)

	GB_ReturnNewZeroString(get_file_user(THIS));

END_PROPERTY


static char *get_file_group(CFILE *_object)
{
	struct group *grp;
	gid_t gid = (gid_t)THIS_STAT->info.gid;

	if (gid == 0)
		return "root";
	else
	{
		grp = getgrgid(gid);
		if (!grp)
		{
			snprintf(_buffer, sizeof(_buffer), "%d", (int)gid);
			return _buffer;
		}
		else
			return grp->gr_name;
	}
}

BEGIN_PROPERTY(Stat_Group)

	GB_ReturnNewZeroString(get_file_group(THIS));

END_PROPERTY


BEGIN_PROPERTY(Stat_SetUID)

	GB_ReturnBoolean(THIS_STAT->info.mode & S_ISUID);

END_PROPERTY


BEGIN_PROPERTY(Stat_SetGID)

	GB_ReturnBoolean(THIS_STAT->info.mode & S_ISGID);

END_PROPERTY


BEGIN_PROPERTY(Stat_Sticky)

	GB_ReturnBoolean(THIS_STAT->info.mode & S_ISVTX);

END_PROPERTY


BEGIN_PROPERTY(Stat_Auth)

	char *auth = FILE_mode_to_string(THIS_STAT->info.mode);

	GB_ReturnNewString(auth, FILE_buffer_length());

END_PROPERTY

#if 0
BEGIN_METHOD(CFILE_access, GB_INTEGER who; GB_INTEGER access)

	bool ret;
	int access = VARGOPT(access, GB_STAT_READ);
	int who = VARG(who);
	int mode = THIS_STAT->info.mode;

	if ((access & GB_STAT_READ) == 0)
		mode &= ~(S_IRUSR | S_IRGRP | S_IROTH);
	if ((access & GB_STAT_WRITE) == 0)
		mode &= ~(S_IWUSR | S_IWGRP | S_IWOTH);
	if ((access & GB_STAT_EXEC) == 0)
		mode &= ~(S_IXUSR | S_IXGRP | S_IXOTH);

	switch(who)
	{
		case GB_STAT_USER: ret = mode & S_IRWXU; break;
		case GB_STAT_GROUP: ret = mode & S_IRWXG; break;
		case GB_STAT_OTHER: ret = mode & S_IRWXO; break;
		default: ret = FALSE; break;
	}

	GB_ReturnBoolean(ret);

END_METHOD
#endif


static void return_perm(CSTAT *_object, int rf, int wf, int xf)
{
	char perm[4];
	char *p;
	int mode = THIS_STAT->info.mode;

	p = perm;

	if (mode & rf) *p++ = 'r';
	if (mode & wf) *p++ = 'w';
	if (mode & xf) *p++ = 'x';

	*p = 0;

	GB_ReturnNewZeroString(perm);
}


BEGIN_PROPERTY(CFILE_perm_user)

	return_perm(THIS_STAT, S_IRUSR, S_IWUSR, S_IXUSR);

END_PROPERTY

BEGIN_PROPERTY(CFILE_perm_group)

	return_perm(THIS_STAT, S_IRGRP, S_IWGRP, S_IXGRP);

END_PROPERTY

BEGIN_PROPERTY(CFILE_perm_other)

	return_perm(THIS_STAT, S_IROTH, S_IWOTH, S_IXOTH);

END_PROPERTY

BEGIN_METHOD(CFILE_perm_get, GB_STRING user)

	char *who;
	char *user = GB_ToZeroString(ARG(user));

	who = get_file_user(THIS);
	if (strcmp(user, who) == 0)
	{
		return_perm(THIS_STAT, S_IRUSR, S_IWUSR, S_IXUSR);
		return;
	}

	who = get_file_group(THIS);
	if (strlen(user) > 2 && user[0] == '*' && user[1] == '.' && strcmp(&user[2], who) == 0)
	{
		return_perm(THIS_STAT, S_IRGRP, S_IWGRP, S_IXGRP);
		return;
	}

	return_perm(THIS_STAT, S_IROTH, S_IWOTH, S_IXOTH);

END_METHOD


/*---- File path functions --------------------------------------------------*/

static char *_dir;
static char *_basename;
static char *_ext;

static void split_path(char *path)
{
	char *p;

	p = rindex(path, '/');
	if (p)
	{
		if (p == path)
		{
			_dir = "/";
			_basename = path + 1;
		}
		else
		{
			*p = 0;
			_dir = path;
			_basename = p + 1;
		}
	}
	else
	{
		_dir = "";
		_basename = path;
	}

	p = rindex(_basename, '.');
	if (p)
	{
		*p = 0;
		_ext = p + 1;
	}
	else
		_ext = "";
}

static void return_path(void)
{
	char *tmp = NULL;
	int len = strlen(_dir);
	char *test;
	
	if (len) 
	{
		tmp = STRING_add(tmp, _dir, len);
		test = _basename ? _basename : _ext;
		if (tmp[len - 1] != '/' && *test != '/')
			tmp = STRING_add_char(tmp, '/');
	}
	
	if (_basename && *_basename)
		tmp = STRING_add(tmp, _basename, 0);
	
	if (*_ext)
	{
		if (*_ext != '.')
			tmp = STRING_add_char(tmp, '.');
	
		tmp = STRING_add(tmp, _ext, 0);
	}

	STRING_extend_end(tmp);

	GB_ReturnString(tmp);
}

BEGIN_METHOD(File_Dir, GB_STRING path)

	split_path(GB_ToZeroString(ARG(path)));
	GB_ReturnNewZeroString(_dir);

END_METHOD


BEGIN_METHOD(File_SetDir, GB_STRING path; GB_STRING new_dir)

	split_path(GB_ToZeroString(ARG(path)));
	_dir = GB_ToZeroString(ARG(new_dir));
	return_path();

END_METHOD


BEGIN_METHOD(File_Name, GB_STRING path)

	char *path;

	//if (LENGTH(path) && STRING(path)[LENGTH(path) - 1] == '/')
	//  LENGTH(path)--;

	path = GB_ToZeroString(ARG(path));
	GB_ReturnNewZeroString(FILE_get_name(path));

END_METHOD


BEGIN_METHOD(File_SetName, GB_STRING path; GB_STRING new_name)

	char *path;

	if (LENGTH(path) && STRING(path)[LENGTH(path) - 1] == '/')
		LENGTH(path)--;

	path = GB_ToZeroString(ARG(path));

	split_path(path);
	_basename = GB_ToZeroString(ARG(new_name));
	_ext = "";
	return_path();

END_METHOD


BEGIN_METHOD(File_Ext, GB_STRING path)

	split_path(GB_ToZeroString(ARG(path)));
	GB_ReturnNewZeroString(_ext);

END_METHOD


BEGIN_METHOD(File_SetExt, GB_STRING path; GB_STRING new_ext)

	split_path(GB_ToZeroString(ARG(path)));
	_ext = GB_ToZeroString(ARG(new_ext));
	return_path();

END_METHOD


BEGIN_METHOD(File_BaseName, GB_STRING path)

	split_path(GB_ToZeroString(ARG(path)));
	GB_ReturnNewZeroString(_basename);

END_METHOD


BEGIN_METHOD(File_SetBaseName, GB_STRING path; GB_STRING new_basename)

	split_path(GB_ToZeroString(ARG(path)));
	_basename = GB_ToZeroString(ARG(new_basename));
	return_path();

END_METHOD

static void error_CFILE_load_save(STREAM *stream)
{
	STREAM_close(stream);
}

BEGIN_METHOD(File_Load, GB_STRING path)

	STREAM stream;
	int64_t len;
	int rlen;
	char *str = NULL;

	STREAM_open(&stream, STRING_conv_file_name(STRING(path), LENGTH(path)), STO_READ);
	
	ON_ERROR_1(error_CFILE_load_save, &stream)
	{
		STREAM_lof(&stream, &len);
		if (len >> 31)
			THROW(E_MEMORY);
		
		if (len == 0)
		{
			char buffer[256];
			
			str = NULL;
			
			for(;;)
			{
				len = STREAM_read_max(&stream, buffer, sizeof(buffer));
				if (len) str = STRING_add(str, buffer, len);
				if (len < sizeof(buffer))
					break;
			}
		}
		else
		{
			rlen = len;

			str = STRING_new(NULL, rlen);
			rlen = STREAM_read_max(&stream, str, rlen);
			str = STRING_extend(str, rlen);
		}
		
		STREAM_close(&stream);

		STRING_free_later(str);
		GB_ReturnString(str);
	}
	END_ERROR

END_METHOD

BEGIN_METHOD(File_Save, GB_STRING path; GB_STRING data)

	STREAM stream;

	STREAM_open(&stream, STRING_conv_file_name(STRING(path), LENGTH(path)), STO_CREATE);
	
	ON_ERROR_1(error_CFILE_load_save, &stream)
	{
		STREAM_write(&stream, STRING(data), LENGTH(data));
		STREAM_close(&stream);
	}
	END_ERROR

END_METHOD

BEGIN_METHOD(File_IsRelative, GB_STRING path)

	char *path = STRING(path);
	int len = LENGTH(path);
	
	if (len <= 0)
	{
		GB_ReturnBoolean(FALSE);
		return;
	}
	
	GB_ReturnBoolean(FILE_is_relative(path));

END_METHOD

BEGIN_METHOD(File_IsHidden, GB_STRING path)

	char *path = STRING(path);
	int len = LENGTH(path);
	int i;
	char c;
	
	for (i = 0; i < len; i++)
	{
		if (path[i] != '.')
			continue;
		
		if (i > 0 && path[i - 1] != '/')
			continue;
		
		if (i == (len - 1))
			continue;
		
		c = path[i + 1];
		if (c == '/')
			continue;
		
		if (c == '.')
		{
			if (i == (len - 2))
				continue;
		
			if (path[i + 2] == '/')
				continue;
		}
		
		GB_ReturnBoolean(TRUE);
		return;
	}
	
	GB_ReturnBoolean(FALSE);

END_METHOD

BEGIN_METHOD(File_RealPath, GB_STRING path)

	char *path = STRING_conv_file_name(STRING(path), LENGTH(path));
	char *rpath = NULL;
	
	if (!FILE_is_relative(path))
	{
		rpath = realpath(path, NULL);
		path = rpath;
	}
	
	GB_ReturnNewZeroString(path);
	if (rpath) free(rpath);

END_METHOD

//---------------------------------------------------------------------------

BEGIN_PROPERTY(Stream_Handle)

	GB_ReturnInteger(STREAM_FD);

END_PROPERTY


BEGIN_PROPERTY(Stream_ByteOrder)

	bool endian = EXEC_big_endian;

	if (READ_PROPERTY)
	{
		if (CSTREAM_stream(THIS_STREAM)->common.swap)
			endian = !endian;

		GB_ReturnInteger(endian ? 1 : 0);
	}
	else
	{
		bool val = VPROP(GB_INTEGER);
		CSTREAM_stream(THIS_STREAM)->common.swap = endian ^ val;
	}

END_PROPERTY

BEGIN_PROPERTY(Stream_EndOfLine)

	if (READ_PROPERTY)
		GB_ReturnInteger(CSTREAM_stream(THIS_STREAM)->common.eol);
	else
	{
		int eol = VPROP(GB_INTEGER);

		if (eol >= 0 && eol <= 2)
			CSTREAM_stream(THIS_STREAM)->common.eol = eol;
	}

END_PROPERTY

BEGIN_METHOD_VOID(Stream_Close)

	STREAM_close(CSTREAM_stream(THIS_STREAM));

END_METHOD

BEGIN_PROPERTY(Stream_EndOfFile)

	GB_ReturnBoolean(CSTREAM_stream(THIS_STREAM)->common.eof);
	
END_PROPERTY

BEGIN_PROPERTY(Stream_Blocking)

	if (READ_PROPERTY)
		GB_ReturnBoolean(STREAM_is_blocking(CSTREAM_stream(THIS_STREAM)));
	else
		STREAM_blocking(CSTREAM_stream(THIS_STREAM), VPROP(GB_BOOLEAN));

END_PROPERTY

BEGIN_PROPERTY(Stream_Tag)

	if (READ_PROPERTY)
		GB_ReturnVariant(&THIS_STREAM->tag);
	else
		GB_StoreVariant(PROP(GB_VARIANT), POINTER(&(THIS_STREAM->tag)));

END_METHOD

BEGIN_METHOD_VOID(Stream_free)

	STREAM_close(CSTREAM_stream(THIS_STREAM));
	GB_StoreVariant(NULL, POINTER(&(THIS_STREAM->tag)));

END_METHOD

BEGIN_PROPERTY(Stream_Eof)

	GB_ReturnBoolean(STREAM_eof(CSTREAM_stream(THIS_STREAM)));

END_PROPERTY

BEGIN_METHOD(Stream_ReadLine, GB_STRING escape)

	char *escape;
	char *str;
	
	if (MISSING(escape))
		escape = NULL;
	else
	{
		escape = GB_ToZeroString(ARG(escape));
		if (!*escape)
			escape = NULL;
	}

	str = STREAM_line_input(CSTREAM_stream(THIS_STREAM), escape);
	STRING_free_later(str);
	GB_ReturnString(str);

END_METHOD

BEGIN_METHOD_VOID(Stream_Begin)

	STREAM_begin(CSTREAM_stream(THIS_STREAM));

END_METHOD

BEGIN_METHOD_VOID(Stream_End)

	STREAM_end(CSTREAM_stream(THIS_STREAM));

END_METHOD

BEGIN_METHOD_VOID(Stream_Cancel)

	STREAM_cancel(CSTREAM_stream(THIS_STREAM));

END_METHOD

BEGIN_PROPERTY(Stream_IsTerm)

	GB_ReturnBoolean(isatty(STREAM_FD));

END_PROPERTY

BEGIN_METHOD(Stream_Watch, GB_INTEGER mode; GB_BOOLEAN on)

	int mode = VARG(mode);
	
	if (mode == R_OK)
		mode = STO_READ;
	else if (mode == W_OK)
		mode = STO_WRITE;
	else
	{
		GB_Error("Unknown watch");
		return;
	}
	
	watch_stream(THIS_STREAM, mode, VARG(on));

END_METHOD

BEGIN_METHOD_VOID(StreamLines_next)

	char *str;

	if (STREAM_eof(CSTREAM_stream(THIS_STREAM)))
		GB_StopEnum();
	else
	{
		str = STREAM_line_input(CSTREAM_stream(THIS_STREAM), NULL);
		STRING_free_later(str);
		GB_ReturnString(str);
	}

END_METHOD

//---------------------------------------------------------------------------

static void init_term_size(void *_object)
{
	struct winsize winSize;
	
	if (_term_width == 0 || _term_height == 0)
	{
		if (ioctl(STREAM_FD, TIOCGWINSZ, (char *)&winSize))
			THROW_SYSTEM(errno, NULL);
		
		_term_width = winSize.ws_col;
		_term_height = winSize.ws_row;
	}
	
	if (!_term_init)
	{
		_SIGWINCH_callback = SIGNAL_register(SIGWINCH, cb_term_resize, 0);
		_term_init = TRUE;
	}
}

BEGIN_PROPERTY(StreamTerm_Name)
	
	GB_ReturnNewZeroString(ttyname(STREAM_FD));

END_PROPERTY

BEGIN_METHOD(StreamTerm_Resize, GB_INTEGER width; GB_INTEGER height)

	struct winsize winSize = { 0 };
	
	winSize.ws_row = (unsigned short)Max(1, Min(65535, VARG(height)));
  winSize.ws_col = (unsigned short)Max(1, Min(65535, VARG(width)));
  
	if (ioctl(STREAM_FD, TIOCSWINSZ, (char *)&winSize))
		THROW_SYSTEM(errno, NULL);

END_METHOD

static void handle_term_property(void *_object, void *_param, bool iflag, int flag)
{
	struct termios ttmode;
	
	if (tcgetattr(STREAM_FD, &ttmode))
		THROW_SYSTEM(errno, "");
	
	if (READ_PROPERTY)
		GB_ReturnBoolean(((iflag ? ttmode.c_iflag : ttmode.c_lflag) & flag) == flag);
	else
	{
		if (VPROP(GB_BOOLEAN))
		{
			if (iflag)
				ttmode.c_iflag |= flag;
			else
				ttmode.c_lflag |= flag;
		}
		else
		{
			if (iflag)
				ttmode.c_iflag &= ~flag;
			else
				ttmode.c_lflag &= ~flag;
		}

		if (tcsetattr(STREAM_FD, TCSANOW, &ttmode))
		THROW_SYSTEM(errno, "");
	}
}

BEGIN_PROPERTY(StreamTerm_Echo)

	handle_term_property(_object, _param, FALSE, ECHO);

END_PROPERTY

BEGIN_PROPERTY(StreamTerm_FlowControl)

	handle_term_property(_object, _param, TRUE, IXON | IXOFF);

END_PROPERTY

BEGIN_PROPERTY(StreamTerm_Width)

	init_term_size(_object);
	GB_ReturnInteger(_term_width);

END_PROPERTY

BEGIN_PROPERTY(StreamTerm_Height)

	init_term_size(_object);
	GB_ReturnInteger(_term_height);

END_PROPERTY

#endif

//---------------------------------------------------------------------------

GB_DESC StreamLinesDesc[] = 
{
	GB_DECLARE_VIRTUAL(".Stream.Lines"),
	
	GB_METHOD("_next", "s", StreamLines_next, NULL),
	
	GB_END_DECLARE
};

GB_DESC StreamTermDesc[] = 
{
	GB_DECLARE_VIRTUAL(".Stream.Term"),
	
	GB_PROPERTY_READ("Name", "s", StreamTerm_Name),
	GB_PROPERTY("Echo", "b", StreamTerm_Echo),
	GB_PROPERTY("FlowControl", "b", StreamTerm_FlowControl),
	GB_PROPERTY_READ("Width", "i", StreamTerm_Width),
	GB_PROPERTY_READ("W", "i", StreamTerm_Width),
	GB_PROPERTY_READ("Height", "i", StreamTerm_Height),
	GB_PROPERTY_READ("H", "i", StreamTerm_Height),
	GB_METHOD("Resize", NULL, StreamTerm_Resize, "(Width)i(Height)i"),
	
	GB_END_DECLARE
};

GB_DESC StreamDesc[] =
{
	GB_DECLARE("Stream", sizeof(CSTREAM)),
	GB_NOT_CREATABLE(),

	GB_METHOD("_free", NULL, Stream_free, NULL),

	GB_PROPERTY("ByteOrder", "i", Stream_ByteOrder),
	GB_PROPERTY_READ("Handle", "i", Stream_Handle),
	GB_PROPERTY("EndOfLine", "i", Stream_EndOfLine),
	GB_METHOD("Close", NULL, Stream_Close, NULL),
	GB_PROPERTY_READ("EndOfFile", "b", Stream_EndOfFile),
	GB_PROPERTY("Blocking", "b", Stream_Blocking),
	GB_PROPERTY("Tag", "v", Stream_Tag),
	GB_METHOD("ReadLine", "s", Stream_ReadLine, "[(Escape)s]"),
	GB_PROPERTY_READ("IsTerm", "b", Stream_IsTerm),
	
	GB_PROPERTY_SELF("Lines", ".Stream.Lines"),
	GB_PROPERTY_SELF("Term", ".Stream.Term"),
	
	GB_METHOD("Begin", NULL, Stream_Begin, NULL),
	GB_METHOD("Send", NULL, Stream_End, NULL),
	GB_METHOD("Drop", NULL, Stream_Cancel, NULL),
	
	GB_METHOD("Watch", NULL, Stream_Watch, "(Mode)i(Watch)b"),
	
	GB_PROPERTY_READ("Eof", "b", Stream_Eof),

	GB_END_DECLARE
};


GB_DESC StatPermDesc[] =
{
	GB_DECLARE_VIRTUAL(".Stat.Perm"),

	GB_METHOD("_get", "s", CFILE_perm_get, "(UserOrGroup)s"),
	GB_PROPERTY_READ("User", "s", CFILE_perm_user),
	GB_PROPERTY_READ("Group", "s", CFILE_perm_group),
	GB_PROPERTY_READ("Other", "s", CFILE_perm_other),

	GB_END_DECLARE
};


GB_DESC StatDesc[] =
{
	GB_DECLARE("Stat", sizeof(CSTAT)),
	GB_NOT_CREATABLE(),

	GB_METHOD("_free", NULL, Stat_free, NULL),

	GB_PROPERTY_READ("Type", "i", Stat_Type),
	GB_PROPERTY_READ("Mode", "i", Stat_Mode),
	GB_PROPERTY_READ("Hidden", "b", Stat_Hidden),
	GB_PROPERTY_READ("Size", "l", Stat_Size),
	GB_PROPERTY_READ("Time", "d", Stat_Time),
	GB_PROPERTY_READ("LastAccess", "d", Stat_LastAccess),
	GB_PROPERTY_READ("LastModified", "d", Stat_Time),
	GB_PROPERTY_READ("LastChange", "d", Stat_LastChange),
	GB_PROPERTY_READ("User", "s", Stat_User),
	GB_PROPERTY_READ("Group", "s", Stat_Group),
	GB_PROPERTY_SELF("Perm", ".Stat.Perm"),
	GB_PROPERTY_READ("SetGID", "b", Stat_SetGID),
	GB_PROPERTY_READ("SetUID", "b", Stat_SetUID),
	GB_PROPERTY_READ("Sticky", "b", Stat_Sticky),
	GB_PROPERTY_READ("Path", "s", Stat_Path),
	GB_PROPERTY_READ("Link", "s", Stat_Link),
	GB_PROPERTY_READ("Auth", "s", Stat_Auth),

	GB_END_DECLARE
};


GB_DESC FileDesc[] =
{
	GB_DECLARE("File", sizeof(CFILE)),
	GB_INHERITS("Stream"),
	GB_NOT_CREATABLE(),

	GB_METHOD("_free", NULL, File_free, NULL),

//  GB_STATIC_PROPERTY_READ("Separator", "s", CFILE_separator),

	GB_STATIC_PROPERTY_READ("In", "File", File_In),
	GB_STATIC_PROPERTY_READ("Out", "File", File_Out),
	GB_STATIC_PROPERTY_READ("Err", "File", File_Err),

	GB_STATIC_METHOD("Dir", "s", File_Dir, "(Path)s"),
	GB_STATIC_METHOD("Name", "s", File_Name, "(Path)s"),
	GB_STATIC_METHOD("Ext", "s", File_Ext, "(Path)s"),
	GB_STATIC_METHOD("BaseName", "s", File_BaseName, "(Path)s"),

	GB_STATIC_METHOD("SetDir", "s", File_SetDir, "(Path)s(NewDir)s"),
	GB_STATIC_METHOD("SetName", "s", File_SetName, "(Path)s(NewName)s"),
	GB_STATIC_METHOD("SetExt", "s", File_SetExt, "(Path)s(NewExt)s"),
	GB_STATIC_METHOD("SetBaseName", "s", File_SetBaseName, "(Path)s(NewBaseName)s"),

	GB_STATIC_METHOD("IsRelative", "b", File_IsRelative, "(Path)s"),
	GB_STATIC_METHOD("IsHidden", "b", File_IsHidden, "(Path)s"),

	GB_STATIC_METHOD("Load", "s", File_Load, "(FileName)s"),
	GB_STATIC_METHOD("Save", NULL, File_Save, "(FileName)s(Data)s"),

	GB_STATIC_METHOD("RealPath", "s", File_RealPath, "(Path)s"),
	
	GB_EVENT("Read", NULL, NULL, &EVENT_Read),
	GB_EVENT("Write", NULL, NULL, &EVENT_Write),
	GB_EVENT("Resize", NULL, NULL, &EVENT_Resize),

	GB_END_DECLARE
};
