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
	
	fclose(_file);
}

void PROFILE_add(void *cp, void *fp, void *pc)
{
	ushort line;
	
	line = 0;
	DEBUG_calc_line_from_position(cp, fp, pc, &line);
	fprintf(_file, "%d %" PRId64 "\n", line, get_time());
}

void PROFILE_begin(void *cp, void *fp)
{
	const char *where = cp ? DEBUG_get_position(cp, fp, NULL) : ".System.EventLoop";
	fprintf(_file, "+%s %" PRId64 "\n", where, get_time());
}

void PROFILE_end(void *cp, void *fp)
{
	const char *where = cp ? DEBUG_get_position(cp, fp, NULL) : ".System.EventLoop";
	fprintf(_file, "-%s %" PRId64 "\n", where, get_time());
}
