/***************************************************************************

  file.h

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

#ifndef __GB_FILE_H
#define __GB_FILE_H

#include <sys/time.h>

#include "gb_common.h"

#ifdef PROJECT_EXEC

#ifndef GBX_INFO

typedef
  struct {
    short type;
    short mode;
    int64_t size;
    int atime;
    int mtime;
    int ctime;
    uid_t uid;
    gid_t gid;
    bool hidden;
		char _reserved[3];
    }
  PACKED
  FILE_STAT;

#endif

/* Constants for Stat() function */

#define GB_STAT_FILE        1
#define GB_STAT_DIRECTORY   2
#define GB_STAT_DEVICE      3
#define GB_STAT_PIPE        4
#define GB_STAT_SOCKET      5
#define GB_STAT_LINK        6

#define GB_STAT_READ        R_OK
#define GB_STAT_WRITE       W_OK
#define GB_STAT_EXEC        X_OK

#define GB_STAT_USER        0
#define GB_STAT_GROUP       1
#define GB_STAT_OTHER       2

#define FILE_TEMP_PREFIX "/tmp/gambas.%d"
#define FILE_TEMP_DIR "/tmp/gambas.%d/%d"
#define FILE_TEMP_FILE "/tmp/gambas.%d/%d/%d.tmp"
#define FILE_TEMP_PATTERN "/tmp/gambas.%d/%d/%s.tmp"

#endif

#ifndef GBX_INFO

PUBLIC const char *FILE_cat(const char *path, ...);
PUBLIC char *FILE_buffer(void);
PUBLIC int FILE_buffer_length(void);
PUBLIC const char *FILE_get_dir(const char *path);
PUBLIC const char *FILE_get_name(const char *path);
PUBLIC const char *FILE_get_ext(const char *path);
PUBLIC const char *FILE_get_basename(const char *path);
/*PUBLIC const char *FILE_get(const char *path);*/
PUBLIC const char *FILE_set_ext(const char *path, const char *ext);

PUBLIC const char *FILE_getcwd(const char *subdir);
#define FILE_get_current_dir() FILE_getcwd(NULL)

PUBLIC const char *FILE_readlink(const char *link);
PUBLIC bool FILE_is_dir(const char *path);
#define FILE_isdir FILE_is_dir

PUBLIC const char *FILE_find_gambas(void);

PUBLIC bool FILE_exist(const char *path);

#ifdef PROJECT_EXEC

PUBLIC void FILE_init(void);
PUBLIC void FILE_remove_temp_file(void);
PUBLIC void FILE_exit(void);

PUBLIC bool FILE_exist_real(const char *path);

PUBLIC void FILE_stat(const char *path, FILE_STAT *info, bool follow);
PUBLIC void FILE_dir_first(const char *path, const char *pattern, int attr);
PUBLIC bool FILE_dir_next(char **path, int *len);

PUBLIC void FILE_unlink(const char *path);
PUBLIC void FILE_rmdir(const char *path);
PUBLIC void FILE_mkdir(const char *path);
PUBLIC void FILE_rename(const char *src, const char *dst);
PUBLIC void FILE_copy(const char *src, const char *dst);

PUBLIC bool FILE_access(const char *path, int mode);
PUBLIC void FILE_link(const char *src, const char *dst);

PUBLIC char *FILE_make_temp(int *len, char *pattern);

PUBLIC void FILE_recursive_dir(const char *dir, void (*found)(const char *), void (*afterfound)(const char *), int attr);

PUBLIC void FILE_make_path_dir(const char *path);

PUBLIC int64_t FILE_free(const char *path);

#else

PUBLIC time_t FILE_get_time(const char *path);

#endif

#define FILE_is_absolute(_path) (*(_path) == '/')
#define FILE_is_relative(_path) (*(_path) != '/')

#endif

#endif
