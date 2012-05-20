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
static uint64_t _start;

static uint64_t get_time(void)
{
	struct timeval time;
	gettimeofday(&time, NULL);
	return (uint64_t)time.tv_sec * 1000000 + (uint64_t)time.tv_usec;
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
	_start = get_time();
}

void PROFILE_exit(void)
{
	if (!_init)
		return;
	
	//fprintf(stderr, "gb.profile: stop profiling\n");
	
	fclose(_file);
}

void PROFILE_add(void *cp, void *fp, void *pc)
{
	static void *_last_fp = NULL;
	static void *_last_pc = NULL;
	static struct timeval _last_time;
	
	struct timeval time;
	ushort line;
	
	gettimeofday(&time, NULL);
	
	if (fp == _last_fp)
	{
		line = 0;
		DEBUG_calc_line_from_position(cp, fp, _last_pc, &line);
		//fprintf(_file, "%d %d %s\n", line, (int)((time.tv_sec - _last_time.tv_sec) * 1000000 + time.tv_usec - _last_time.tv_usec), DEBUG_get_position(cp, fp, pc));
		fprintf(_file, "%d %d\n", line, (int)((time.tv_sec - _last_time.tv_sec) * 1000000 + time.tv_usec - _last_time.tv_usec));
	}
	
	_last_fp = fp;
	_last_pc = pc;
	_last_time = time;
}

void PROFILE_begin(void *cp, void *fp)
{
	fprintf(_file, "+%s %" PRId64 "\n", DEBUG_get_position(cp, fp, NULL), get_time() - _start);
}

void PROFILE_end(void *cp, void *fp)
{
	struct timeval time;
	gettimeofday(&time, NULL);
	
	fprintf(_file, "-%s %" PRId64 "\n", DEBUG_get_position(cp, fp, NULL), get_time() - _start);
}
