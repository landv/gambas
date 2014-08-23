/***************************************************************************

  profile.c

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

#define __PROFILE_C

#include <sys/time.h>
#include <stdio.h>

#include "gambas.h"
#include "debug.h"
#include "main.h"

// Maximum profile file is 512 Mb by default

#define MAX_PROFILE_SIZE (512 << 20)

static bool _init = FALSE;
static FILE *_file;
static int _last_line = 0;
static bool _new_line = TRUE;
static int _count = 0;
static size_t _max_profile_size = MAX_PROFILE_SIZE;
//static long _ticks_per_sec;

static uint64_t get_time(void)
{
	static uint64_t last = 0;
	
	struct timeval time;
	//struct tms time;
	uint64_t t;

	gettimeofday(&time, NULL);
	t = (uint64_t)time.tv_sec * 1000000 + (uint64_t)time.tv_usec - last;

	//times(&time);	
	//t = (uint64_t)(time.tms_utime + time.tms_stime) * 1000000 / _ticks_per_sec - last;
	
	last += t;
	
	return t;
}

void PROFILE_init(const char *path)
{
	char *env;
	size_t max;
	char buffer[PATH_MAX + 1];
	
	if (_init)
		return;
	
	if (!path)
	{
		sprintf(buffer, ".%d.prof", getpid());
		path = buffer;
	}

	//fprintf(stderr, "gb.profile: start profiling: %s\n", path);
	
	_file = fopen(path, "w");
	if (!_file)
	{
		fprintf(stderr, "gb.profile: cannot create profile file '%s': %s\n", path, strerror(errno));
		abort();
	}
	
	fprintf(_file, "[1]\n");
	
	//_ticks_per_sec = sysconf(_SC_CLK_TCK);
	//fprintf(stderr, "_ticks_per_sec = %ld\n", _ticks_per_sec);
	
	env = getenv("GB_PROFILE_MAX");
	if (env)
	{
		max = atoi(env);
		if (max > 0)
		{
			if (max < 128)
				max = 128;
			else if (max > 4096)
				max = 4096;
			
			_max_profile_size = max << 20;
		}
	}
	
	_init = TRUE;
	get_time();
}

void PROFILE_cancel(void)
{
	if (_init)
	{
		close(fileno(_file));
		_init = FALSE;
	}
}

void PROFILE_exit(void)
{
	if (!_init)
		return;
	
	//fprintf(stderr, "gb.profile: stop profiling\n");
	
	if (!_new_line)
		fputc('\n', _file);

	fclose(_file);
}

static void check_size()
{
	_count = 0;
	if (ftell(_file) > _max_profile_size)
	{
		fprintf(stderr, "gb.profile: maximum profile size reached\n");
		PROFILE_exit();
		abort();
	}
}


#define CODE(n) (n + '9' + 1)

static void add_line(ushort line, uint64_t time)
{
	int n;
	char buf[32], num[16];
	char *p;
	
	n = line - _last_line;
	p = buf;
	
	if (n >= -9 && n <= 9)
		*p++ = CODE(0 + n + 9);
	else if (n >= -99 && n <= 99)
	{
		*p++ = n > 0 ? CODE(19) : CODE(20);
		*p++ = CODE(0) + abs(n) - 10;
	}
	else
	{
		*p++ = n > 0 ? CODE(21) : CODE(22);
		n = sprintf(num, "%d", abs(n));
		*p++ = CODE(n);
		strcpy(p, num);
		p += n;
	}
	
	if (time <= 9)
		*p++ = CODE(time);
	else
	{
		n = sprintf(num, "%" PRIu64, time);
		*p++ = CODE(10 + n - 2);
		strcpy(p, num);
		p += n;
	}
	
	*p = 0;
	
	fputs(buf, _file);
	
	_last_line = line;
	_new_line = FALSE;
	_count++;
	if ((_count & 0xFFFFF) == 0)
		check_size();
}

void PROFILE_add(void *cp, void *fp, void *pc)
{
	uint64_t time = get_time();
	ushort line;
	
	line = 0;
	DEBUG_calc_line_from_position(cp, fp, pc, &line);
	
	add_line(line, time);
}

void PROFILE_begin(void *cp, void *fp)
{
	uint64_t time = get_time();
	const char *where = cp ? DEBUG_get_profile_position(cp, fp, NULL) : "0";
	
	if (!_new_line)
		fputc('\n', _file);
	fprintf(_file, "(%s %" PRId64 "\n", where, time);
	_last_line = 0;
	_new_line = TRUE;
	
	_count++;
	if ((_count & 0xFFFFF) == 0)
		check_size();
	/*if (cp && fp)
	{
		FUNCTION *ffp = (FUNCTION *)fp;
		add_line(ffp->debug->line, time);
	}*/
}

void PROFILE_end(void *cp, void *fp)
{
	uint64_t time = get_time();
	//const char *where;
	
	if (cp && fp)
	{
		FUNCTION *ffp = (FUNCTION *)fp;
		if (ffp->debug)
			add_line(ffp->debug->line + ffp->debug->nline, time);
	}

	//where = cp ? DEBUG_get_position(cp, fp, NULL) : ".System.EventLoop";

	if (!_new_line)
		fputc('\n', _file);
	fprintf(_file, ")\n");
	_last_line = 0;
	_new_line = TRUE;
}
