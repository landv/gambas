/***************************************************************************

  profile.c

  (c) 2000-2012 Benoît Minisini <gambas@users.sourceforge.net>

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

static bool _init = FALSE;
static FILE *_file;
static int _last_line = 0;
static bool _new_line = TRUE;

static uint64_t get_time(void)
{
	static uint64_t last = 0;
	
	struct timeval time;
	uint64_t t;

	gettimeofday(&time, NULL);
	t = (uint64_t)time.tv_sec * 1000000 + (uint64_t)time.tv_usec - last;
	last += t;
	return t;
}

void PROFILE_init(void)
{
	char path[PATH_MAX + 1];
	
	if (_init)
		return;
	
	sprintf(path, ".prof.%d", getpid());

	//fprintf(stderr, "gb.profile: start profiling: %s\n", path);
	
	_file = fopen(path, "w");
	if (!_file)
	{
		fprintf(stderr, "gb.profile: cannot create profile file '%s': %s\n", path, strerror(errno));
		abort();
	}
	
	_init = TRUE;
	get_time();
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
	const char *where = cp ? DEBUG_get_position(cp, fp, NULL) : ".System.EventLoop";
	
	if (!_new_line)
		fputc('\n', _file);
	fprintf(_file, " >%s %" PRId64 "\n", where, time);
	_last_line = 0;
	_new_line = TRUE;
	
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
		add_line(ffp->debug->line + ffp->debug->nline, time);
	}

	//where = cp ? DEBUG_get_position(cp, fp, NULL) : ".System.EventLoop";

	if (!_new_line)
		fputc('\n', _file);
	fprintf(_file, " <\n");
	_last_line = 0;
	_new_line = TRUE;
}
