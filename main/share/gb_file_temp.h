/***************************************************************************

  gb_file_temp.h

  The file management routines

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

#define __GB_FILE_C

#include "config.h"

#include "gb_common.h"
#include "gb_error.h"
#include "gb_limit.h"
#include "gb_component.h"

#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>

#include <sys/param.h>
#include <sys/stat.h>


#include <dirent.h>

#ifdef PROJECT_EXEC

#if defined(OS_FREEBSD) || defined(OS_OPENBSD)
#include <sys/mount.h>
#else
#include <sys/statfs.h>
#endif

#endif

#include "gb_file.h"

#ifdef PROJECT_EXEC

//PUBLIC FILE_STAT FILE_stat_info = { 0 };

static DIR *file_dir = NULL;
static char *file_pattern = NULL;
static char *file_path = NULL;
static bool file_dir_arch = FALSE;
static int file_attr;
static char *file_rdir_path = NULL;

#endif

static char file_buffer[PATH_MAX + 16];
static int file_buffer_length;

#ifdef PROJECT_EXEC

typedef
  struct _path {
    struct _path *next;
    char *path;
    }
  FILE_PATH;


static void push_path(void **list, const char *path)
{
  FILE_PATH *slot;

  ALLOC(&slot, sizeof(FILE_PATH), "push_path");
  STRING_new(&slot->path, path, 0);

  slot->next = *list;
  *list = slot;

  //printf("push_path: %s\n", path);
}


static char *pop_path(void **list)
{
  char *path;
  FILE_PATH *slot;

  if (!*list)
    return NULL;

  path = ((FILE_PATH *)*list)->path;
  slot = *list;
  *list = ((FILE_PATH *)*list)->next;
  FREE(&slot, "pop_path");

  //printf("pop_path: %s\n", path);
  return path;
}


static void dir_exit(void)
{
  if (file_dir != NULL)
  {
    closedir(file_dir);
    file_dir = NULL;
  }

  STRING_free(&file_pattern);
  STRING_free(&file_path);
}


PUBLIC char *FILE_make_temp(int *len, char *pattern)
{
  static int count = 0;

  if (len)
  {
    if (pattern)
      *len = snprintf(file_buffer, sizeof(file_buffer), FILE_TEMP_PATTERN, getuid(), getpid(), pattern);
     else
    {
      count++;
      *len = snprintf(file_buffer, sizeof(file_buffer), FILE_TEMP_FILE, getuid(), getpid(), count);
    }
  }
  else
    snprintf(file_buffer, sizeof(file_buffer), FILE_TEMP_DIR, getuid(), getpid());

  return file_buffer;
}

static void remove_temp_file(const char *path)
{
  if (FILE_is_dir(path))
  {
	  //fprintf(stderr, "rmdir: %s\n", path);
    rmdir(path);
	}
  else
  {
	  //fprintf(stderr, "unlink: %s\n", path);
    unlink(path);
	}
}

PUBLIC void FILE_remove_temp_file(void)
{
  FILE_recursive_dir(FILE_make_temp(NULL, NULL), NULL, remove_temp_file, 0);
  rmdir(FILE_make_temp(NULL, NULL));
}

PUBLIC void FILE_init(void)
{
	FILE_remove_temp_file();
  
  snprintf(file_buffer, sizeof(file_buffer), FILE_TEMP_PREFIX, getuid());
  mkdir(file_buffer, S_IRWXU);
  snprintf(file_buffer, sizeof(file_buffer), FILE_TEMP_DIR, getuid(), getpid());
  mkdir(file_buffer, S_IRWXU);
}

PUBLIC void FILE_exit(void)
{
	FILE_remove_temp_file();
  STRING_free(&file_rdir_path);
  dir_exit();
}

#endif

#define stradd(d, s) \
({ \
  char *_d = (d); \
  const char *_s = (s); \
  \
  for(;;) \
  { \
    if ((*_d = *_s) == 0) \
      break; \
    \
    _d++; \
    _s++; \
  } \
  \
  _d; \
})

PUBLIC const char *FILE_cat(const char *path, ...)
{
  char *p;
  va_list args;
  int len;
  boolean end_slash = FALSE;
  boolean add_slash = FALSE;

  va_start(args, path);

  p = file_buffer;

  if (path != file_buffer)
    *p = 0;

  for(;;)
  {
    if (*path == '/' && p != file_buffer)
      path++;

    len = strlen(path);
    if (add_slash)
      len++;

    if (len > 0)
    {
      if ((p + len) > &file_buffer[MAX_PATH])
        return NULL;

      if (p != path)
      {
        if (add_slash)
          p = stradd(p, "/");

        p = stradd(p, path);
      }
      else
        p += len;

      end_slash = (p[-1] == '/');
    }

    path = va_arg(args, char *);
    if (path == NULL)
      break;

    add_slash = ((!end_slash) && (*path != 0) && (*path != '/'));
  }

  file_buffer_length = p - file_buffer;
  return file_buffer;
}


PUBLIC char *FILE_buffer(void)
{
  return file_buffer;
}


PUBLIC int FILE_buffer_length(void)
{
  if (file_buffer_length < 0)
    file_buffer_length = strlen(file_buffer);

  return file_buffer_length;
}

PUBLIC const char *FILE_get_dir(const char *path)
{
  char *p;

  if (path == NULL || path[0] == 0)
    return NULL;

  if (path[0] == '/' && path[1] == 0)
    return "/";

  if (file_buffer != path)
    strcpy(file_buffer, path);

  p = rindex(file_buffer, '/');

  if (p == NULL)
    *file_buffer = 0;
  else
  {
    *p = 0;

    if (file_buffer[0] == 0 && path[0] == '/')
      strcpy(file_buffer, "/");
  }

  file_buffer_length = -1;
  return file_buffer;
}


PUBLIC const char *FILE_get_name(const char *path)
{
  const char *p;

  p = rindex(path, '/');
  if (p)
    return &p[1];
  else
    return path;
}


PUBLIC const char *FILE_get_ext(const char *path)
{
  const char *p;

  p = rindex(path, '/');
  if (p)
    path = &p[1];

  p = rindex(path, '.');
  if (p == NULL)
    return &path[strlen(path)];
  else
    return p + 1;
}


PUBLIC const char *FILE_set_ext(const char *path, const char *ext)
{
  char *p;

  if (path != file_buffer)
  {
    strcpy(file_buffer, path);
    path = file_buffer;
  }

  p = (char *)FILE_get_ext(path);

  if (!ext)
  {
    if (p > file_buffer && p[-1] == '.')
      p[-1] = 0;
    else
      *p = 0;
    return path;
  }

  if (&p[strlen(ext)] >= &file_buffer[MAX_PATH])
    return path;

  if (p == path || p[-1] != '.')
    *p++ = '.';

  if (*ext == '.')
    ext++;

  strcpy(p, ext);

  file_buffer_length = -1;
  return path;
}


PUBLIC const char *FILE_get_basename(const char *path)
{
  char *p;

  path = FILE_get_name(path);

  if (file_buffer != path)
    strcpy(file_buffer, path);

  p = rindex(file_buffer, '.');
  if (p)
    *p = 0;

  file_buffer_length = -1;

  return file_buffer;
}

PUBLIC bool FILE_is_dir(const char *path)
{
  struct stat buf;

#ifdef PROJECT_EXEC

  if (FILE_is_relative(path))
  {
  	if (!EXEC_arch)
  	{
      chdir(PROJECT_path);
      if (lstat(path, &buf) == 0)
        goto __OK;
  	}

    return ARCHIVE_is_dir(NULL, path);
	}

#endif

  if (stat(path, &buf))
    return FALSE;

#ifdef PROJECT_EXEC
__OK:
#endif

  return (S_ISDIR(buf.st_mode));
}


#ifdef PROJECT_EXEC

PUBLIC bool FILE_exist_real(const char *path)
{
  struct stat buf;

  chdir(PROJECT_path);
  return (stat(path, &buf) == 0);
}

PUBLIC void FILE_stat(const char *path, FILE_STAT *info, bool follow)
{
  struct stat buf;
  int ret;

  if (FILE_is_relative(path))
  {
    if (!EXEC_arch)
    {
      chdir(PROJECT_path);
      if (lstat(path, &buf) == 0)
        goto _OK;
    }

    ARCHIVE_stat(NULL, path, info);
    return;
  }

	if (follow)
		ret = stat(path, &buf);
	else
		ret = lstat(path, &buf);
		
  if (ret)
    THROW_SYSTEM(errno, path);

_OK:

  if (S_ISREG(buf.st_mode))
    info->type = GB_STAT_FILE;
  else if (S_ISDIR(buf.st_mode))
    info->type = GB_STAT_DIRECTORY;
  else if (S_ISCHR(buf.st_mode) || S_ISBLK(buf.st_mode))
    info->type = GB_STAT_DEVICE;
  else if (S_ISFIFO(buf.st_mode))
    info->type = GB_STAT_PIPE;
  else if (S_ISSOCK(buf.st_mode))
    info->type = GB_STAT_SOCKET;
  else if (S_ISLNK(buf.st_mode))
    info->type = GB_STAT_LINK;

  info->mode = buf.st_mode & 07777;
  info->size = buf.st_size;
  info->atime = (int)buf.st_atime;
  info->mtime = (int)buf.st_mtime;
  info->ctime = (int)buf.st_ctime;
  info->hidden = (*FILE_get_name(path) == '.');
  info->uid = buf.st_uid;
  info->gid = buf.st_gid;
}


PUBLIC void FILE_dir_first(const char *path, const char *pattern, int attr)
{
  struct stat buf;

  dir_exit();

  if (!path || *path == 0)
    path = ".";

  if (attr == (GB_STAT_FILE | GB_STAT_DIRECTORY))
    attr = 0;
  file_attr = attr;

  if (FILE_is_relative(path))
  {
    if (!EXEC_arch)
    {
      chdir(PROJECT_path);
      if (lstat(path, &buf) == 0)
        goto _OK;
    }

    file_dir_arch = TRUE;
    ARCHIVE_dir_first(NULL, path, pattern);
    return;
  }

_OK:

  file_dir = opendir(path);
  if (file_dir == NULL)
    THROW_SYSTEM(errno, path);

  STRING_new(&file_pattern, pattern, 0);
  STRING_new(&file_path, path, 0);
}


PUBLIC bool FILE_dir_next(char **path, int *len)
{
  struct dirent *entry;
  int len_entry;
  bool ret;
  struct stat info;
  char *p = file_buffer;

  if (file_dir_arch)
  {
    ret = ARCHIVE_dir_next(path, len, file_attr);
    if (ret)
      file_dir_arch = FALSE;
    return ret;
  }

  if (file_dir == NULL)
    return TRUE;

  if (file_attr)
  {
    strcpy(p, file_path);
    p += strlen(file_path);
    if (p[-1] != '/' && (file_buffer[1] || file_buffer[0] != '/'))
      *p++ = '/';
  }

  for(;;)
  {
    entry = readdir(file_dir);
    if (entry == NULL)
    {
      dir_exit();
      return TRUE;
    }

    if ((strcmp(entry->d_name, ".") == 0) || (strcmp(entry->d_name, "..") == 0))
      continue;

    if (file_attr)
    {
      strcpy(p, entry->d_name);
      stat(file_buffer, &info);
      if ((file_attr == GB_STAT_DIRECTORY) ^ (S_ISDIR(info.st_mode) != 0))
        continue;
    }

    len_entry = strlen(entry->d_name);

    if (file_pattern == NULL)
      break;

    if (REGEXP_match(file_pattern, STRING_length(file_pattern), entry->d_name, len_entry))
      break;
  }

  *path = entry->d_name;
  *len = len_entry;

  return FALSE;
}


PUBLIC void FILE_recursive_dir(const char *dir, void (*found)(const char *), void (*afterfound)(const char *), int attr)
{
  void *list = NULL;
  char *file;
  int len;
  char *path;
  //struct stat buf;
  FILE_STAT info;
  char *temp;

  if (!FILE_is_dir(dir))
    return;

  if (!dir || *dir == 0)
  	dir = ".";

  STRING_free(&file_rdir_path);
  STRING_new(&file_rdir_path, dir, 0);

	FILE_dir_first(dir, NULL, attr);
	while (!FILE_dir_next(&file, &len))
	{
		STRING_new_temp(&temp, file, len);
		push_path(&list, FILE_cat(file_rdir_path, temp, NULL));
	}

  while (list)
  {
    path = pop_path(&list);
		//fprintf(stderr, "%s\n", path);

		TRY
		{
			FILE_stat(path, &info, FALSE);

			if (found) (*found)(path);

			if (info.type == GB_STAT_DIRECTORY)
				FILE_recursive_dir(path, found, afterfound, attr);

			if (afterfound) (*afterfound)(path);
		}
		CATCH
		{
			//ERROR_print_at(stdout);
		}
		END_TRY

    STRING_free((char **)&path);
  }
}


PUBLIC void FILE_unlink(const char *path)
{
  if (FILE_is_relative(path))
    THROW(E_ACCESS);

  if (unlink(path) != 0)
    THROW_SYSTEM(errno, path);
}


PUBLIC void FILE_rmdir(const char *path)
{
  if (FILE_is_relative(path))
    THROW(E_ACCESS);

  if (rmdir(path) != 0)
    THROW_SYSTEM(errno, path);
}


PUBLIC void FILE_mkdir(const char *path)
{
  if (FILE_is_relative(path))
    THROW(E_ACCESS);

  if (mkdir(path, S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH) != 0)
    THROW_SYSTEM(errno, path);
}


PUBLIC void FILE_make_path_dir(const char *path)
{
  int i;
  char c;

  if (FILE_is_relative(path))
    return;

  if (path != file_buffer)
    strcpy(file_buffer, path);

  for (i = 1;; i++)
  {
    c = file_buffer[i];
    if (c == 0)
      break;
    if (c == '/')
    {
      file_buffer[i] = 0;
      mkdir(file_buffer, S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH);
      file_buffer[i] = c;
    }
    c++;
  }
}


PUBLIC void FILE_rename(const char *src, const char *dst)
{
  if (FILE_is_relative(src) || FILE_is_relative(dst))
    THROW(E_ACCESS);

  if (FILE_exist(dst))
    THROW(E_EXIST, dst);

  if (rename(src, dst) != 0)
    THROW_SYSTEM(errno, dst);
}

PUBLIC void FILE_copy(const char *src, const char *dst)
{
  STREAM stream_src;
  STREAM stream_dst;
  int64_t len;
  int64_t n;
  char *buf = NULL;

  CLEAR(&stream_src);
  CLEAR(&stream_dst);

  if (FILE_exist(dst))
    THROW(E_EXIST, dst);

  ALLOC(&buf, MAX_IO, "FILE_copy");

  TRY
  {
    STREAM_open(&stream_src, src, ST_READ);
    STREAM_open(&stream_dst, dst, ST_CREATE);

    STREAM_lof(&stream_src, &len);

    while (len)
    {
      n = len > MAX_IO ? MAX_IO : len;
      STREAM_read(&stream_src, buf, n);
      STREAM_write(&stream_dst, buf, n);
      len -= n;
    }

    STREAM_close(&stream_src);
    STREAM_close(&stream_dst);

    FREE(&buf, "FILE_copy");
  }
  CATCH
  {
    if (stream_src.type)
      STREAM_close(&stream_src);
    if (stream_dst.type)
      STREAM_close(&stream_dst);
    FREE(&buf, "FILE_copy");

    PROPAGATE();
  }
  END_TRY
}


PUBLIC bool FILE_access(const char *path, int mode)
{
  if (FILE_is_relative(path))
  {
    if (mode & (W_OK | X_OK))
      return FALSE;

    if (!EXEC_arch)
    {
      chdir(PROJECT_path);
      if (access(path, mode) == 0)
        return TRUE;
    }

    return ARCHIVE_exist(NULL, path);
  }

  return (access(path, mode) == 0);
}


PUBLIC bool FILE_exist(const char *path)
{
	struct stat buf;

  if (FILE_is_relative(path))
  {
    if (!EXEC_arch)
    {
      chdir(PROJECT_path);
      if (lstat(path, &buf) == 0)
        return TRUE;
    }

    return ARCHIVE_exist(NULL, path);
  }

  return lstat(path, &buf) == 0;
}


PUBLIC void FILE_link(const char *src, const char *dst)
{
  /* src can be relative */
  if (FILE_is_relative(dst))
    THROW(E_ACCESS);

  if (FILE_exist(dst))
    THROW(E_EXIST, dst);

  if (symlink(src, dst) != 0)
    THROW_SYSTEM(errno, dst);
}

PUBLIC int64_t FILE_free(const char *path)
{
  struct statfs info;

  if (FILE_is_relative(path))
    return 0;

  statfs(path, &info);
  return (int64_t)(getuid() == 0 ? info.f_bfree : info.f_bavail) * info.f_bsize;
}

#else

PUBLIC bool FILE_exist(const char *path)
{
  return (access(path, F_OK) == 0);
}

PUBLIC time_t FILE_get_time(const char *path)
{
  struct stat info;

  if (stat(path, &info) == 0)
    return info.st_mtime;
  else
    return (time_t)-1L;
}

#endif

PUBLIC const char *FILE_getcwd(const char *subdir)
{
  if (getcwd(file_buffer, PATH_MAX) == NULL)
    return NULL;

  file_buffer_length = strlen(file_buffer);

  if (subdir != NULL)
    return FILE_cat(file_buffer, subdir, NULL);
  else
    return file_buffer;
}


PUBLIC const char *FILE_readlink(const char *link)
{
  int len = readlink(link, file_buffer, MAX_PATH);

  if (len < 0)
    return NULL;

  file_buffer[len] = 0;
  file_buffer_length = len;
  return file_buffer;

}

PUBLIC const char *FILE_get_gambas_dir(void)
{
  const char *path;
	const char *dir;
	
	dir = getenv("GB_DIR");
	if (dir)
		return dir;

  if (FILE_exist(GAMBAS_LINK_PATH))
  {
    path = FILE_readlink(GAMBAS_LINK_PATH);
    if (!path)
      path = GAMBAS_LINK_PATH;
		return FILE_get_dir(FILE_get_dir(path));
  }
  else
  {
		return FILE_get_dir(GAMBAS_PATH);
  }

  return path;
}

