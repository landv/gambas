/***************************************************************************

  gb_file_share.h

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

#ifndef __GB_FILE_H
#define __GB_FILE_H

#include <sys/time.h>

#include "gb_common.h"

#ifdef PROJECT_EXEC

#include "gambas.h"

typedef
	GB_FILE_STAT FILE_STAT;

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

#define FILE_TEMP_PREFIX    "/tmp/gambas.%d"
#define FILE_TEMP_DIR       FILE_TEMP_PREFIX "/%d"
#define FILE_TEMP_FILE      FILE_TEMP_DIR "/%d.tmp"
#define FILE_TEMP_PATTERN   FILE_TEMP_DIR "/%s.tmp"

#endif

#ifndef GBX_INFO

const char *FILE_cat(const char *path, ...);
char *FILE_buffer(void);
int FILE_buffer_length(void);
const char *FILE_get_dir(const char *path);
const char *FILE_get_name(const char *path);
const char *FILE_get_ext(const char *path);
const char *FILE_get_basename(const char *path);
/*PUBLIC const char *FILE_get(const char *path);*/
const char *FILE_set_ext(const char *path, const char *ext);

const char *FILE_getcwd(const char *subdir);
#define FILE_get_current_dir() FILE_getcwd(NULL)
void FILE_chdir(const char *path);

const char *FILE_readlink(const char *link);
bool FILE_is_dir(const char *path);

const char *FILE_find_gambas(void);

void FILE_rename_ext(const char *src, const char *dst, bool unlink);
#define FILE_rename(_src, _dst) FILE_rename_ext(_src, _dst, FALSE)
#define FILE_rename_unlink(_src, _dst) FILE_rename_ext(_src, _dst, TRUE)

void FILE_unlink(const char *path);

char *FILE_get_home(void);
void FILE_exit(void);

#ifdef PROJECT_EXEC

void FILE_init(void);
void FILE_remove_temp_file(void);
void FILE_remove_temp_file_pid(pid_t pid);

bool FILE_exist_follow(const char *path, bool follow);
#define FILE_exist(_path) FILE_exist_follow(_path, FALSE)
bool FILE_exist_real(const char *path);

void FILE_stat(const char *path, FILE_STAT *info, bool follow);
void FILE_dir_first(const char *path, const char *pattern, int attr);
bool FILE_dir_next(char **path, int *len);

void FILE_rmdir(const char *path);
void FILE_mkdir(const char *path);
void FILE_copy(const char *src, const char *dst);

bool FILE_access(const char *path, int mode);
void FILE_link(const char *src, const char *dst);

char *FILE_make_temp(int *len, const char *pattern);

void FILE_recursive_dir(const char *dir, void (*found)(const char *), void (*afterfound)(const char *), int attr, bool follow);

void FILE_make_path_dir(const char *path);

int64_t FILE_free(const char *path);

char *FILE_mode_to_string(mode_t mode);
mode_t FILE_mode_from_string(mode_t mode, const char *str);

void FILE_chmod(const char *path, mode_t mode);
void FILE_chown(const char *path, const char *user);
void FILE_chgrp(const char *path, const char *group);

#else

bool FILE_exist(const char *path);
time_t FILE_get_time(const char *path);
bool FILE_copy(const char *src, const char *dst);

#endif

#define FILE_is_absolute(_path) (*(_path) == '/' || * (_path) == '~')
#define FILE_is_relative(_path) (!FILE_is_absolute(_path))

#endif

#endif
