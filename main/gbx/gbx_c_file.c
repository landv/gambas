/***************************************************************************

  CFile.c

  The native File class

  (c) 2000-2007 Benoit Minisini <gambas@users.sourceforge.net>

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

#define __GBX_C_FILE_C

#include "gbx_info.h"

#ifndef GBX_INFO

#include <stdio.h>
#include <pwd.h>
#include <grp.h>
#include <sys/stat.h>

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

#include "gbx_c_file.h"


CFILE CFILE_in, CFILE_out, CFILE_err;

DECLARE_EVENT(EVENT_Read);
DECLARE_EVENT(EVENT_Write);

static GB_FUNCTION read_func;

static char _buffer[16];

static void init(CFILE *_object, FILE *file)
{
	CLEAR(THIS);

  THIS->ob.ob.class = CLASS_File;
  THIS->ob.ob.ref = 1;

  THIS->ob.stream.type = &STREAM_buffer;
  THIS->ob.stream.buffer.file = file;
  THIS->ob.stream.buffer.size = 0;
  THIS->ob.stream.common.buffer = NULL;
}

static void callback_read(int fd, int type, CFILE *file)
{
  //fprintf(stderr, "callback_read\n");
  GB_Raise(file, EVENT_Read, 0);
}

static void callback_write(int fd, int type, CFILE *file)
{
  GB_Raise(file, EVENT_Write, 0);
}


void CFILE_init(void)
{
  init(&CFILE_in, stdin);
  init(&CFILE_out, stdout);
  init(&CFILE_err, stderr);
}

void CFILE_exit(void)
{
	STREAM_release(&CFILE_in.ob.stream);
	STREAM_release(&CFILE_out.ob.stream);
	STREAM_release(&CFILE_err.ob.stream);
}

void CFILE_init_watch(void)
{
  if (GB_GetFunction(&read_func, PROJECT_class, "Application_Read", "", "") == 0)
  {
    //fprintf(stderr, "watch stdin\n");
    GB_Attach(&CFILE_in, PROJECT_class, "Application");
    GB_Watch(STDIN_FILENO, GB_WATCH_READ, (void *)callback_read, (intptr_t)&CFILE_in);
  }
}


CFILE *CFILE_create(STREAM *stream, int mode)
{
  int fd;
  CFILE *file;

  OBJECT_new((void **)(void *)&file, CLASS_File, NULL, NULL);
  OBJECT_UNREF_KEEP(&file, "CFILE_new");

  if (stream)
  {
    *CSTREAM_stream(file) = *stream;
    file->watch_fd = -1;

    if (mode & ST_WATCH)
    {
      fd = STREAM_handle(&file->ob.stream);
      file->watch_fd = fd;

      if (mode & ST_READ)
        GB_Watch(fd, GB_WATCH_READ, (void *)callback_read, (intptr_t)file);

      if (mode & ST_WRITE)
        GB_Watch(fd, GB_WATCH_WRITE, (void *)callback_write, (intptr_t)file);

      OBJECT_attach((OBJECT *)file, OP ? (OBJECT *)OP : (OBJECT *)CP, "File");
    }
  }

  return file;
}


BEGIN_METHOD_VOID(CFILE_free)

  STREAM_close(&THIS->ob.stream);
  if (THIS->watch_fd >= 0)
    GB_Watch(THIS->watch_fd, GB_WATCH_NONE, NULL, 0);

END_METHOD


BEGIN_PROPERTY(CFILE_get_in)

  GB_ReturnObject(&CFILE_in);

END_PROPERTY


BEGIN_PROPERTY(CFILE_get_out)

  GB_ReturnObject(&CFILE_out);

END_PROPERTY


BEGIN_PROPERTY(CFILE_get_err)

  GB_ReturnObject(&CFILE_err);

END_PROPERTY


BEGIN_METHOD_VOID(CSTAT_free)

	STRING_unref(&THIS_STAT->path);

END_METHOD


BEGIN_PROPERTY(CFILE_type)

  GB_ReturnInt(THIS_STAT->info.type);

END_PROPERTY


BEGIN_PROPERTY(CSTAT_path)

	GB_ReturnString(THIS_STAT->path);

END_PROPERTY


BEGIN_PROPERTY(CSTAT_link)

	if (THIS_STAT->info.type == GB_STAT_LINK)
		GB_ReturnNewZeroString(FILE_readlink(THIS_STAT->path));
	else
		GB_ReturnNull();

END_PROPERTY


BEGIN_PROPERTY(CFILE_mode)

  GB_ReturnInt(THIS_STAT->info.mode);

END_PROPERTY


BEGIN_PROPERTY(CFILE_hidden)

  GB_ReturnBoolean(THIS_STAT->info.hidden);

END_PROPERTY


BEGIN_PROPERTY(CFILE_size)

  GB_ReturnLong(THIS_STAT->info.size);

END_PROPERTY


BEGIN_PROPERTY(CFILE_atime)

  GB_DATE date;

  DATE_from_time(THIS_STAT->info.atime, 0, (VALUE *)&date);

  GB_ReturnDate(&date);

END_PROPERTY


BEGIN_PROPERTY(CFILE_ctime)

  GB_DATE date;

  DATE_from_time(THIS_STAT->info.ctime, 0, (VALUE *)&date);

  GB_ReturnDate(&date);

END_PROPERTY


BEGIN_PROPERTY(CFILE_mtime)

  GB_DATE date;

  DATE_from_time(THIS_STAT->info.mtime, 0, (VALUE *)&date);

  GB_ReturnDate(&date);

END_PROPERTY


static char *get_file_user(CFILE *_object)
{
  struct passwd *pwd;
  uid_t uid = THIS_STAT->info.uid;

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

BEGIN_PROPERTY(CFILE_user)

  GB_ReturnNewZeroString(get_file_user(THIS));

END_PROPERTY


static char *get_file_group(CFILE *_object)
{
  struct group *grp;
  gid_t gid = THIS_STAT->info.gid;

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

BEGIN_PROPERTY(CFILE_group)

  GB_ReturnNewZeroString(get_file_group(THIS));

END_PROPERTY


BEGIN_PROPERTY(CFILE_setuid)

  GB_ReturnBoolean(THIS_STAT->info.mode & S_ISUID);

END_PROPERTY


BEGIN_PROPERTY(CFILE_setgid)

  GB_ReturnBoolean(THIS_STAT->info.mode & S_ISGID);

END_PROPERTY


BEGIN_PROPERTY(CFILE_sticky)

  GB_ReturnBoolean(THIS_STAT->info.mode & S_ISVTX);

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
    return_perm(THIS_STAT, S_IRUSR, S_IWUSR, S_IWUSR);
    return;
  }

  who = get_file_group(THIS);
  if (strlen(user) > 2 && user[0] == '*' && user[1] == '.' && strcmp(&user[2], who) == 0)
  {
    return_perm(THIS_STAT, S_IRGRP, S_IWGRP, S_IWGRP);
    return;
  }

  return_perm(THIS_STAT, S_IROTH, S_IWOTH, S_IWOTH);

END_METHOD


/*---- File path functions --------------------------------------------------*/

static char *_dir;
static char *_basename;
static char *_ext;
static char *_name = NULL;

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
  int len;

  len = strlen(_dir);

  STRING_add(&tmp, _dir, len);
  if (_name && *_name)
  {
    _basename = _name;
    _ext = "";
    _name = NULL;
  }

  if (*_basename || *_ext)
  {
    if (*_dir && _dir[len - 1] != '/' && *_basename && *_basename != '/')
      STRING_add(&tmp, "/", 1);
    STRING_add(&tmp, _basename, 0);
    if (*_ext && *_ext != '.')
      STRING_add(&tmp, ".", 1);
    STRING_add(&tmp, _ext, 0);
  }

  STRING_extend_end(&tmp);

  GB_ReturnString(tmp);
}

BEGIN_METHOD(CFILE_dir, GB_STRING path)

  split_path(GB_ToZeroString(ARG(path)));
  GB_ReturnNewZeroString(_dir);

END_METHOD


BEGIN_METHOD(CFILE_set_dir, GB_STRING path; GB_STRING new_dir)

  split_path(GB_ToZeroString(ARG(path)));
  _dir = GB_ToZeroString(ARG(new_dir));
  return_path();

END_METHOD


BEGIN_METHOD(CFILE_name, GB_STRING path)

  char *path;

  if (LENGTH(path) && STRING(path)[LENGTH(path) - 1] == '/')
    LENGTH(path)--;

  path = GB_ToZeroString(ARG(path));
  GB_ReturnNewString(FILE_get_name(path), -1);

END_METHOD


BEGIN_METHOD(CFILE_set_name, GB_STRING path; GB_STRING new_name)

  char *path;

  if (LENGTH(path) && STRING(path)[LENGTH(path) - 1] == '/')
    LENGTH(path)--;

  path = GB_ToZeroString(ARG(path));

  split_path(path);
  _name = GB_ToZeroString(ARG(new_name));
  return_path();

END_METHOD


BEGIN_METHOD(CFILE_ext, GB_STRING path)

  split_path(GB_ToZeroString(ARG(path)));
	GB_ReturnNewZeroString(_ext);

END_METHOD


BEGIN_METHOD(CFILE_set_ext, GB_STRING path; GB_STRING new_ext)

  split_path(GB_ToZeroString(ARG(path)));
  _ext = GB_ToZeroString(ARG(new_ext));
  return_path();

END_METHOD


BEGIN_METHOD(CFILE_basename, GB_STRING path)

  split_path(GB_ToZeroString(ARG(path)));
	GB_ReturnNewZeroString(_basename);

END_METHOD


BEGIN_METHOD(CFILE_set_basename, GB_STRING path; GB_STRING new_basename)

  split_path(GB_ToZeroString(ARG(path)));
  _basename = GB_ToZeroString(ARG(new_basename));
  return_path();

END_METHOD


BEGIN_METHOD(CFILE_load, GB_STRING path)

  STREAM stream;
  int64_t len;
  int rlen;
  char *str;
  boolean opened = FALSE;
  //ERROR_INFO save; We suppose it is useless

  TRY
  {
    STREAM_open(&stream, STRING_conv_file_name(STRING(path), LENGTH(path)), ST_READ);
    opened = TRUE;

    STREAM_lof(&stream, &len);
    if (len >> 31)
      THROW(E_MEMORY);
    rlen = len;

    STRING_new_temp(&str, NULL, rlen);

    STREAM_read(&stream, str, rlen);
    STREAM_close(&stream);

    GB_ReturnString(str);
  }
  CATCH
  {
  	//ERROR_save(&save);
    if (opened)
      STREAM_close(&stream);
		//ERROR_restore(&save);
    PROPAGATE();
  }
  END_TRY

END_METHOD


BEGIN_METHOD(CFILE_save, GB_STRING path; GB_STRING data)

  STREAM stream;
  boolean opened = FALSE;

  TRY
  {
    STREAM_open(&stream, STRING_conv_file_name(STRING(path), LENGTH(path)), ST_CREATE);
    opened = TRUE;
    STREAM_write(&stream, STRING(data), LENGTH(data));
    STREAM_close(&stream);
  }
  CATCH
  {
    if (opened)
      STREAM_close(&stream);
    PROPAGATE();
  }
  END_TRY

END_METHOD


BEGIN_PROPERTY(CSTREAM_id)

  GB_ReturnInteger(STREAM_handle(CSTREAM_stream(THIS_STREAM)));

END_PROPERTY


BEGIN_PROPERTY(CSTREAM_byte_order)

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

BEGIN_PROPERTY(CSTREAM_eol)

	if (READ_PROPERTY)
		GB_ReturnInteger(CSTREAM_stream(THIS_STREAM)->common.eol);
	else
	{
		int eol = VPROP(GB_INTEGER);

		if (eol >= 0 && eol <= 2)
    	CSTREAM_stream(THIS_STREAM)->common.eol = eol;
	}

END_PROPERTY

BEGIN_METHOD_VOID(CSTREAM_close)

	STREAM_close(CSTREAM_stream(THIS_STREAM));

END_METHOD

BEGIN_PROPERTY(CSTREAM_eof)

  GB_ReturnBoolean(CSTREAM_stream(THIS_STREAM)->common.eof);
  
END_PROPERTY
		
#endif

GB_DESC NATIVE_Stream[] =
{
  GB_DECLARE("Stream", sizeof(CSTREAM)),
  GB_NOT_CREATABLE(),

  GB_PROPERTY("ByteOrder", "i", CSTREAM_byte_order),
  GB_PROPERTY_READ("Handle", "i", CSTREAM_id),
  GB_PROPERTY_READ("Id", "i", CSTREAM_id),
  GB_PROPERTY("EndOfLine", "i", CSTREAM_eol),
  GB_METHOD("Close", NULL, CSTREAM_close, NULL),
  GB_PROPERTY_READ("EndOfFile", "b", CSTREAM_eof),

  GB_END_DECLARE
};


GB_DESC NATIVE_FilePerm[] =
{
  GB_DECLARE(".FilePerm", 0),
  GB_VIRTUAL_CLASS(),

  GB_METHOD("_get", "s", CFILE_perm_get, "(UserOrGroup)s"),
  GB_PROPERTY_READ("User", "s", CFILE_perm_user),
  GB_PROPERTY_READ("Group", "s", CFILE_perm_group),
  GB_PROPERTY_READ("Other", "s", CFILE_perm_other),

  GB_END_DECLARE
};


GB_DESC NATIVE_Stat[] =
{
  GB_DECLARE("Stat", sizeof(CSTAT)),
  GB_NOT_CREATABLE(),

	GB_METHOD("_free", NULL, CSTAT_free, NULL),

  GB_PROPERTY_READ("Type", "i", CFILE_type),
  GB_PROPERTY_READ("Mode", "i", CFILE_mode),
  GB_PROPERTY_READ("Hidden", "b", CFILE_hidden),
  GB_PROPERTY_READ("Size", "l", CFILE_size),
  GB_PROPERTY_READ("Time", "d", CFILE_mtime),
  GB_PROPERTY_READ("LastAccess", "d", CFILE_atime),
  GB_PROPERTY_READ("LastModified", "d", CFILE_mtime),
  GB_PROPERTY_READ("LastChange", "d", CFILE_ctime),
  GB_PROPERTY_READ("User", "s", CFILE_user),
  GB_PROPERTY_READ("Group", "s", CFILE_group),
  GB_PROPERTY_SELF("Perm", ".FilePerm"),
  GB_PROPERTY_READ("SetGID", "b", CFILE_setgid),
  GB_PROPERTY_READ("SetUID", "b", CFILE_setuid),
  GB_PROPERTY_READ("Sticky", "b", CFILE_sticky),
  GB_PROPERTY_READ("Path", "s", CSTAT_path),
  GB_PROPERTY_READ("Link", "s", CSTAT_link),

  GB_END_DECLARE
};


GB_DESC NATIVE_File[] =
{
  GB_DECLARE("File", sizeof(CFILE)),
  GB_INHERITS("Stream"),
  GB_NOT_CREATABLE(),

  GB_METHOD("_free", NULL, CFILE_free, NULL),

//  GB_STATIC_PROPERTY_READ("Separator", "s", CFILE_separator),

  GB_STATIC_PROPERTY_READ("In", "File", CFILE_get_in),
  GB_STATIC_PROPERTY_READ("Out", "File", CFILE_get_out),
  GB_STATIC_PROPERTY_READ("Err", "File", CFILE_get_err),

  GB_STATIC_METHOD("Dir", "s", CFILE_dir, "(Path)s"),
  GB_STATIC_METHOD("Name", "s", CFILE_name, "(Path)s"),
  GB_STATIC_METHOD("Ext", "s", CFILE_ext, "(Path)s"),
  GB_STATIC_METHOD("BaseName", "s", CFILE_basename, "(Path)s"),

  GB_STATIC_METHOD("SetDir", "s", CFILE_set_dir, "(Path)s(NewDir)s"),
  GB_STATIC_METHOD("SetName", "s", CFILE_set_name, "(Path)s(NewName)s"),
  GB_STATIC_METHOD("SetExt", "s", CFILE_set_ext, "(Path)s(NewExt)s"),
  GB_STATIC_METHOD("SetBaseName", "s", CFILE_set_basename, "(Path)s(NewBaseName)s"),

  GB_STATIC_METHOD("Load", "s", CFILE_load, "(FileName)s"),
  GB_STATIC_METHOD("Save", NULL, CFILE_save, "(FileName)s(Data)s"),

  GB_EVENT("Read", NULL, NULL, &EVENT_Read),
  GB_EVENT("Write", NULL, NULL, &EVENT_Write),

  GB_END_DECLARE
};
