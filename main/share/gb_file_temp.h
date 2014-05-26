/***************************************************************************

  gb_file_temp.h

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

#define __GB_FILE_C

#include "config.h"

#include "gb_common.h"
#include "gb_error.h"
#include "gb_limit.h"
#include "gb_component.h"

#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <dirent.h>

#ifdef PROJECT_EXEC

#if defined(OS_FREEBSD) || defined(OS_OPENBSD)
#include <sys/mount.h>
#else
#include <sys/statfs.h>
#endif

#include <sys/types.h>
#include <pwd.h>
#include <grp.h>

#endif

#include "gb_file.h"

#ifdef PROJECT_EXEC

//FILE_STAT FILE_stat_info = { 0 };

static DIR *file_dir = NULL;
static char *file_pattern = NULL;
static char *file_path = NULL;
static bool file_dir_arch = FALSE;
static int file_attr;
static char *file_rdir_path = NULL;

#endif

static char file_buffer[PATH_MAX + 16];
static int file_buffer_length;

#ifdef _DIRENT_HAVE_D_TYPE
#undef _DIRENT_HAVE_D_TYPE
#endif

#ifdef _DIRENT_HAVE_D_TYPE
static bool _last_is_dir;
#endif

#ifdef PROJECT_EXEC

typedef
	struct _path {
		struct _path *next;
		char *path;
		}
	FILE_PATH;

typedef
	struct {
		unsigned int mask;
		unsigned char test[4];
		unsigned int mode[4];
	}
	FILE_MODE_DECODE;

static void push_path(void **list, const char *path)
{
	FILE_PATH *slot;

	ALLOC(&slot, sizeof(FILE_PATH));
	slot->path = STRING_new_zero(path);

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
	FREE(&slot);

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


char *FILE_make_temp(int *len, const char *pattern)
{
	static int count = 0;

	if (len)
	{
		if (pattern)
			*len = snprintf(file_buffer, sizeof(file_buffer), FILE_TEMP_PATTERN, (int)getuid(), (int)getpid(), pattern);
		else
		{
			count++;
			*len = snprintf(file_buffer, sizeof(file_buffer), FILE_TEMP_FILE, (int)getuid(), (int)getpid(), count);
		}
	}
	else
		snprintf(file_buffer, sizeof(file_buffer), FILE_TEMP_DIR, (int)getuid(), (int)getpid());

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

void FILE_remove_temp_file(void)
{
	FILE_recursive_dir(FILE_make_temp(NULL, NULL), NULL, remove_temp_file, 0, FALSE);
	rmdir(FILE_make_temp(NULL, NULL));
}

void FILE_remove_temp_file_pid(pid_t pid)
{
	snprintf(file_buffer, sizeof(file_buffer), FILE_TEMP_DIR, (int)getuid(), (int)pid);
	FILE_recursive_dir(file_buffer, NULL, remove_temp_file, 0, FALSE);
	rmdir(FILE_make_temp(NULL, NULL));
}

void FILE_init(void)
{
	struct stat info;
	
	FILE_remove_temp_file();
	
	snprintf(file_buffer, sizeof(file_buffer), FILE_TEMP_PREFIX, (int)getuid());
	(void)mkdir(file_buffer, S_IRWXU);
	
	if (lstat(file_buffer, &info) == 0 && S_ISDIR(info.st_mode) && chown(file_buffer, getuid(), getgid()) == 0 && chmod(file_buffer, S_IRWXU) == 0)
	{
		snprintf(file_buffer, sizeof(file_buffer), FILE_TEMP_DIR, (int)getuid(), (int)getpid());
		(void)mkdir(file_buffer, S_IRWXU);
		if (lstat(file_buffer, &info) == 0 && S_ISDIR(info.st_mode) && chown(file_buffer, getuid(), getgid()) == 0 && chmod(file_buffer, S_IRWXU) == 0)
			return;
	}

	ERROR_fatal("cannot initialize interpreter temporary directory. Do you try to hijack Gambas?");
}

void FILE_exit(void)
{
	FILE_remove_temp_file();
	STRING_free(&file_rdir_path);
	dir_exit();
}

#endif

static char *stradd(char *d, const char *s)
{
	for(;;)
	{
		if ((*d = *s) == 0)
			break;
		d++;
		s++;
	}
	
	return d;
}

/*#define stradd(d, s) \
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
})*/

const char *FILE_cat(const char *path, ...)
{
	char *p;
	va_list args;
	int len;
	bool end_slash = FALSE;
	bool add_slash = FALSE;

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
			if ((p + len) > &file_buffer[PATH_MAX])
				THROW(E_TOOLONG);

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

	va_end(args);
	
	file_buffer_length = p - file_buffer;
	return file_buffer;
}


char *FILE_buffer(void)
{
	return file_buffer;
}


int FILE_buffer_length(void)
{
	if (file_buffer_length < 0)
		file_buffer_length = strlen(file_buffer);

	return file_buffer_length;
}

static void init_file_buffer(const char *path)
{
	int len;
	
	if (path == file_buffer)
		return;
	
	len = strlen(path);
	
	if (len > PATH_MAX)
		THROW(E_TOOLONG);
	
	strcpy(file_buffer, path);
	file_buffer_length = len;
}

//#define INIT_FILE_BUFFER(_path) (init_file_buffer(), _path = file_buffer)

const char *FILE_get_dir(const char *path)
{
	char *p;

	if (path == NULL || path[0] == 0)
		return NULL;

	if (path[0] == '/' && path[1] == 0)
		return "/";

	init_file_buffer(path);

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


const char *FILE_get_name(const char *path)
{
	const char *p;

	p = rindex(path, '/');
	if (p)
		return &p[1];
	else
		return path;
}


const char *FILE_get_ext(const char *path)
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


const char *FILE_set_ext(const char *path, const char *ext)
{
	char *p;

	init_file_buffer(path);

	p = (char *)FILE_get_ext(file_buffer);

	if (!ext)
	{
		if (p > file_buffer && p[-1] == '.')
			p[-1] = 0;
		else
			*p = 0;
		return file_buffer;
	}

	if (&p[strlen(ext)] >= &file_buffer[PATH_MAX])
		THROW(E_TOOLONG);

	if (p == file_buffer || p[-1] != '.')
		*p++ = '.';

	if (*ext == '.')
		ext++;

	strcpy(p, ext);

	file_buffer_length = -1;
	return file_buffer;
}


const char *FILE_get_basename(const char *path)
{
	char *p;

	path = FILE_get_name(path);

	init_file_buffer(path);
	
	p = rindex(file_buffer, '.');
	if (p)
		*p = 0;

	file_buffer_length = -1;
	return file_buffer;
}

bool FILE_is_dir(const char *path)
{
	struct stat buf;

#ifdef PROJECT_EXEC

	if (FILE_is_relative(path))
	{
		/*if (!EXEC_arch)
		{
			chdir(PROJECT_path);
			if (lstat(path, &buf) == 0)
				goto __OK;
		}*/

		return ARCHIVE_is_dir(NULL, path);
	}

#endif

	if (stat(path, &buf))
		return FALSE;

/*#ifdef PROJECT_EXEC
__OK:
#endif*/

	return (S_ISDIR(buf.st_mode));
}


#ifdef PROJECT_EXEC

bool FILE_exist_real(const char *path)
{
	struct stat buf;

	if (chdir(PROJECT_path)) return FALSE;
	return (stat(path, &buf) == 0);
}

void FILE_stat(const char *path, FILE_STAT *info, bool follow)
{
	struct stat buf;
	int ret;

	//fprintf(stderr, "FILE_stat: %s\n", path);

	if (FILE_is_relative(path))
	{
		/*if (!EXEC_arch)
		{
			chdir(PROJECT_path);
			if (lstat(path, &buf) == 0)
				goto _OK;
		}*/

		ARCHIVE_stat(NULL, path, info);
		return;
	}

	if (follow)
		ret = stat(path, &buf);
	else
		ret = lstat(path, &buf);
		
	if (ret)
		THROW_SYSTEM(errno, path);

//_OK:

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

char *FILE_mode_to_string(mode_t mode)
{
	char *str = file_buffer;
	
  str[0] = mode & S_IRUSR ? 'r' : '-';
  str[1] = mode & S_IWUSR ? 'w' : '-';
  str[2] = (mode & S_ISUID
            ? (mode & S_IXUSR ? 's' : 'S')
            : (mode & S_IXUSR ? 'x' : '-'));
  str[3] = mode & S_IRGRP ? 'r' : '-';
  str[4] = mode & S_IWGRP ? 'w' : '-';
  str[5] = (mode & S_ISGID
            ? (mode & S_IXGRP ? 's' : 'S')
            : (mode & S_IXGRP ? 'x' : '-'));
  str[6] = mode & S_IROTH ? 'r' : '-';
  str[7] = mode & S_IWOTH ? 'w' : '-';
  str[8] = (mode & S_ISVTX
            ? (mode & S_IXOTH ? 't' : 'T')
            : (mode & S_IXOTH ? 'x' : '-'));
	str[9] = 0;
	
	file_buffer_length = 9;
	
	return str;
}

mode_t FILE_mode_from_string(mode_t mode, const char *str)
{
	static FILE_MODE_DECODE decode[] = {
		{ S_IRUSR, { '-', 'r' }, { 0, S_IRUSR } },
		{ S_IWUSR, { '-', 'w' }, { 0, S_IWUSR } },
		{ S_IXUSR | S_ISUID, { '-', 'x', 'S', 's' }, { 0, S_IXUSR, S_ISUID, S_IXUSR | S_ISUID } },
		{ S_IRGRP, { '-', 'r' }, { 0, S_IRGRP } },
		{ S_IWGRP, { '-', 'w' }, { 0, S_IWGRP } },
		{ S_IXGRP | S_ISGID, { '-', 'x', 'S', 's' }, { 0, S_IXGRP, S_ISGID, S_IXGRP | S_ISGID } },
		{ S_IROTH, { '-', 'r' }, { 0, S_IROTH } },
		{ S_IWOTH, { '-', 'w' }, { 0, S_IWOTH } },
		{ S_IXOTH | S_ISVTX, { '-', 'x', 'T', 't' }, { 0, S_IXOTH, S_ISVTX, S_IXOTH | S_ISVTX } },
		{ 0 }
	};
	
	unsigned char c, test;
	FILE_MODE_DECODE *d = decode;
	int i;
	
	while (d->mask)
	{
		c = *str++;
		if (!c)
			break;
		
		for (i = 0; i < 3; i++)
		{
			test = d->test[i];
			if (!test)
				break;
			if (c == test)
			{
				mode = (mode & ~(d->mask)) | d->mode[i];
				break;
			}
		}
		
		d++;
	}
	
	return mode;
}


void FILE_chmod(const char *path, mode_t mode)
{
	if (FILE_is_relative(path))
		THROW(E_ACCESS);

	if (chmod(path, mode))
		THROW_SYSTEM(errno, path);
}


void FILE_chown(const char *path, const char *user)
{
	struct passwd *pwd;
	uid_t uid;
	
	if (FILE_is_relative(path))
		THROW(E_ACCESS);

	if (!strcmp(user, "root"))
		uid = (uid_t)0;
	else
	{
		errno = 0;
		pwd = getpwnam(user);
		if (errno)
			THROW_SYSTEM(errno, path);
		if (!pwd)
			THROW(E_USER);
		uid = pwd->pw_uid;
	}
	
	if (chown(path, uid, (gid_t)-1))
		THROW_SYSTEM(errno, path);
}


void FILE_chgrp(const char *path, const char *group)
{
	struct group *grp;
	gid_t gid;
	
	if (FILE_is_relative(path))
		THROW(E_ACCESS);

	if (!strcmp(group, "root"))
		gid = (gid_t)0;
	else
	{
		errno = 0;
		grp = getgrnam(group);
		if (errno)
			THROW_SYSTEM(errno, path);
		if (!grp)
			THROW(E_USER);
		gid = grp->gr_gid;
	}
	
	if (chown(path, (uid_t)-1, gid))
		THROW_SYSTEM(errno, path);
}


void FILE_dir_first(const char *path, const char *pattern, int attr)
{
	dir_exit();

	if (!path || *path == 0)
		path = ".";

	if (attr == (GB_STAT_FILE | GB_STAT_DIRECTORY))
		attr = 0;
	file_attr = attr;

	if (FILE_is_relative(path))
	{
		/*if (!EXEC_arch)
		{
			chdir(PROJECT_path);
			if (lstat(path, &buf) == 0)
				goto _OK;
		}*/

		file_dir_arch = TRUE;
		ARCHIVE_dir_first(NULL, path, pattern, attr);
		return;
	}

	file_dir_arch = FALSE;
	file_dir = opendir(path);
	if (file_dir == NULL)
		THROW_SYSTEM(errno, path);

	file_pattern = STRING_new_zero(pattern);
	file_path = STRING_new_zero(path);
}


bool FILE_dir_next(char **path, int *len)
{
	struct dirent *entry;
	int len_entry;
	bool ret;
	char *name;
	#ifdef _DIRENT_HAVE_D_TYPE
	#else
	struct stat info;
	char *p = file_buffer;
	#endif

	if (file_dir_arch)
	{
		ret = ARCHIVE_dir_next(path, len, file_attr);
		if (ret)
			file_dir_arch = FALSE;
		return ret;
	}

	if (file_dir == NULL)
		return TRUE;

	#ifdef _DIRENT_HAVE_D_TYPE
	#else
	if (file_attr)
	{
		init_file_buffer(file_path);
		p += file_buffer_length;
		
		if (p[-1] != '/' && (file_buffer[1] || file_buffer[0] != '/'))
			*p++ = '/';
	}
	#endif

	for(;;)
	{
		entry = readdir(file_dir);
		if (entry == NULL)
		{
			dir_exit();
			return TRUE;
		}
		
		name = entry->d_name;
		
		if (name[0] == '.' && (name[1] == 0 || (name[1] == '.' && name[2] == 0)))
			continue;

		len_entry = strlen(name);
		if ((len_entry + file_buffer_length) > PATH_MAX)
			continue;
			
		if (file_attr)
		{
			#ifdef _DIRENT_HAVE_D_TYPE
			if ((file_attr == GB_STAT_DIRECTORY) ^ (entry->d_type == DT_DIR))
				continue;
			#else
			strcpy(p, name);
			if (stat(file_buffer, &info))
			{
				if (file_attr == GB_STAT_DIRECTORY)
					continue;
			}
			else if ((file_attr == GB_STAT_DIRECTORY) ^ (S_ISDIR(info.st_mode) != 0))
				continue;
			#endif
		}

		if (file_pattern == NULL)
			break;

		if (REGEXP_match(file_pattern, STRING_length(file_pattern), name, len_entry))
			break;
	}

	*path = name;
	*len = len_entry;
	#ifdef _DIRENT_HAVE_D_TYPE
	_last_is_dir = entry->d_type == DT_DIR;
	#endif

	return FALSE;
}

//#undef _DIRENT_HAVE_D_TYPE

void FILE_recursive_dir(const char *dir, void (*found)(const char *), void (*afterfound)(const char *), int attr, bool follow)
{
	void *list = NULL;
	void *dir_list = NULL;
	char *file;
	int len;
	char *path;
	//struct stat buf;
	#ifdef _DIRENT_HAVE_D_TYPE
	#else
	FILE_STAT info;
	#endif
	char *temp;
	bool is_dir;

	if (!dir || *dir == 0)
		dir = ".";
	else if (!FILE_is_dir(dir))
		return;

	STRING_free(&file_rdir_path);
	file_rdir_path = STRING_new_zero(dir);
	
	FILE_dir_first(dir, NULL, attr != GB_STAT_DIRECTORY ? 0 : GB_STAT_DIRECTORY);
	while (!FILE_dir_next(&file, &len))
	{
		temp = STRING_new_temp(file, len);
		path = (char *)FILE_cat(file_rdir_path, temp, NULL);
		#ifdef _DIRENT_HAVE_D_TYPE
		is_dir = _last_is_dir;
		#else
		if (follow)
			is_dir = FILE_is_dir(path);
		else
		{
			FILE_stat(path, &info, FALSE);
			is_dir = info.type == GB_STAT_DIRECTORY;
		}
		#endif
		
		if (is_dir)
			push_path(&dir_list, path);
		else
			push_path(&list, path);
	}

	while (dir_list)
	{
		path = pop_path(&dir_list);
		//fprintf(stderr, "%s\n", path);

		TRY
		{
			if (found && (!attr || (attr & GB_STAT_DIRECTORY))) (*found)(path);
			FILE_recursive_dir(path, found, afterfound, attr, follow);
			if (afterfound && (!attr || (attr & GB_STAT_DIRECTORY))) (*afterfound)(path);
		}
		CATCH
		{
			//ERROR_print_at(stdout);
		}
		END_TRY

		STRING_free((char **)&path);
	}

	while (list)
	{
		path = pop_path(&list);
		//fprintf(stderr, "%s\n", path);

		TRY
		{
			if (found) (*found)(path);
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


void FILE_rmdir(const char *path)
{
	if (FILE_is_relative(path))
		THROW(E_ACCESS);

	if (rmdir(path) != 0)
	{
		if (errno == ENOTEMPTY || errno == EEXIST)
			THROW(E_NEMPTY);
		else
			THROW_SYSTEM(errno, path);
	}
}


void FILE_mkdir(const char *path)
{
	if (FILE_is_relative(path))
		THROW(E_ACCESS);

	if (mkdir(path, S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH) != 0)
		THROW_SYSTEM(errno, path);
}


void FILE_make_path_dir(const char *path)
{
	int i;
	char c;

	if (FILE_is_relative(path))
		return;

	init_file_buffer(path);

	for (i = 1;; i++)
	{
		c = file_buffer[i];
		if (c == 0)
			break;
		if (c == '/')
		{
			file_buffer[i] = 0;
			(void)mkdir(file_buffer, S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH);
			file_buffer[i] = c;
		}
		c++;
	}
}


void FILE_copy(const char *src, const char *dst)
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

	ALLOC(&buf, MAX_IO);

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

		FREE(&buf);
	}
	CATCH
	{
		if (stream_src.type)
			STREAM_close(&stream_src);
		if (stream_dst.type)
			STREAM_close(&stream_dst);
		FREE(&buf);

		PROPAGATE();
	}
	END_TRY
}


bool FILE_access(const char *path, int mode)
{
	if (FILE_is_relative(path))
	{
		if (mode & (W_OK | X_OK))
			return FALSE;

		/*if (!EXEC_arch)
		{
			chdir(PROJECT_path);
			if (access(path, mode) == 0)
				return TRUE;
		}*/

		return ARCHIVE_exist(NULL, path);
	}

	return (access(path, mode) == 0);
}


bool FILE_exist_follow(const char *path, bool follow)
{
	struct stat buf;

	if (FILE_is_relative(path))
		return ARCHIVE_exist(NULL, path);

	return follow ? (stat(path, &buf) == 0) : (lstat(path, &buf) == 0);
}


void FILE_link(const char *src, const char *dst)
{
	/* src can be relative */
	if (FILE_is_relative(dst))
		THROW(E_ACCESS);

	if (FILE_exist(dst))
		THROW(E_EXIST, dst);

	if (symlink(src, dst) != 0)
		THROW_SYSTEM(errno, dst);
}

int64_t FILE_free(const char *path)
{
	struct statfs info;

	if (FILE_is_relative(path))
		return 0;

	statfs(path, &info);
	return (int64_t)(getuid() == 0 ? info.f_bfree : info.f_bavail) * info.f_bsize;
}

#else

bool FILE_exist(const char *path)
{
	return (access(path, F_OK) == 0);
}

time_t FILE_get_time(const char *path)
{
	struct stat info;

	if (stat(path, &info) == 0)
		return info.st_mtime;
	else
		return (time_t)-1L;
}

bool FILE_copy(const char *src, const char *dst)
{
	int src_fd;
	int dst_fd;
	ssize_t len;
	char *buf = NULL;
	int save_errno;
	struct stat info;

	fprintf(stderr, "FILE_copy: %s -> %s\n", src, dst);
	
	if (stat(src, &info))
		return TRUE;
	
	src_fd = open(src, O_RDONLY);
	if (src_fd < 0)
	{
		fprintf(stderr, "open src failed\n");
		return TRUE;
	}
	
	dst_fd = creat(dst, info.st_mode);
	if (dst_fd < 0)
	{
		fprintf(stderr, "open dst failed\n");
		save_errno = errno;
		close(src_fd);
		errno = save_errno;
		return TRUE;
	}
	
	ALLOC(&buf, MAX_IO);

	for(;;)
	{
		len = read(src_fd, buf, MAX_IO);
		if (len == 0)
			break;
		if (len < 0 && errno == EINTR)
			continue;
		if (write(dst_fd, buf, len) < 0)
		{
			save_errno = errno;
			close(src_fd);
			close(dst_fd);
			unlink(dst);
			errno = save_errno;
			IFREE(buf);
			return TRUE;
		}
	}
	
	close(src_fd);
	close(dst_fd);
	IFREE(buf);
	
	return FALSE;
}

#endif

const char *FILE_getcwd(const char *subdir)
{
	if (getcwd(file_buffer, PATH_MAX) == NULL)
		return NULL;

	file_buffer_length = strlen(file_buffer);

	if (subdir != NULL)
		return FILE_cat(file_buffer, subdir, NULL);
	else
		return file_buffer;
}


const char *FILE_readlink(const char *link)
{
	int len = readlink(link, file_buffer, PATH_MAX);

	if (len < 0)
		return NULL;

	file_buffer[len] = 0;
	file_buffer_length = len;
	return file_buffer;

}

const char *FILE_find_gambas(void)
{
	const char *path;

	path = getenv("GB_PATH");

	if (!path || !*path)
	{
		if (FILE_exist(GAMBAS_LINK_PATH))
		{
			path = FILE_readlink(GAMBAS_LINK_PATH);
			if (!path)
				path = GAMBAS_LINK_PATH;
		}
		else
		{
			path = GAMBAS_PATH "/gbx" GAMBAS_VERSION_STRING;
		}
	}

	return path;
}

void FILE_chdir(const char *path)
{
	#ifdef PROJECT_EXEC
	if (chdir(path))
		THROW_SYSTEM(errno, path);
	#else
	if (chdir(path))
		THROW("Cannot change current directory to '&1': &2", path, strerror(errno));
	#endif
}

void FILE_unlink(const char *path)
{
	#ifdef PROJECT_EXEC
	if (FILE_is_relative(path))
		THROW(E_ACCESS);

	if (unlink(path) != 0)
		THROW_SYSTEM(errno, path);
	#else
	if (unlink(path) != 0 && errno != ENOENT)
		THROW("Cannot remove file '&1': &2", path, strerror(errno));
	#endif
}


void FILE_rename(const char *src, const char *dst)
{
	#ifdef PROJECT_EXEC
	if (FILE_is_relative(src) || FILE_is_relative(dst))
		THROW(E_ACCESS);

	if (FILE_exist(dst))
		THROW(E_EXIST, dst);

	if (rename(src, dst) != 0)
		THROW_SYSTEM(errno, dst);
	#else
	if (rename(src, dst) != 0)
		THROW("Cannot rename file '&1' to '&2': &3", src, dst, strerror(errno));
	#endif
}

