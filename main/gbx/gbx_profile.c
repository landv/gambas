/***************************************************************************

  gbx_profile.c

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

#define __GBX_PROFILE_C

#include <sys/time.h>

#include "gb_common.h"
#include "gbx_class.h"
#include "gbx_exec.h"
#include "gbx_signal.h"
#include "gbx_debug.h"
#include "gbx_stream.h"
#include "gbx_profile.h"

typedef
	struct {
		CLASS *cp;
		ushort count;
		ushort fp;
		#ifndef OS_64BITS
		int _reserved;
		#endif
	}
	PACKED
	PROFILE_SLOT;

static SIGNAL_HANDLER _SIGVTALRM_handler;
static int _fd;
static int _count = 0;

static PROFILE_SLOT _last = { 0 };
static FUNCTION *_last_fp = NULL;


#define BUFFER_MAX 256
static PROFILE_SLOT _buffer[BUFFER_MAX];
static int _buffer_count = 0;
static struct timeval _last_time;

static void flush_buffer()
{
	if (_buffer_count > 0)
	{
		STREAM_write_direct(_fd, (char *)_buffer, sizeof(PROFILE_SLOT) * _buffer_count);
		_buffer_count = 0;
	}
}

static void add_buffer()
{
	_buffer[_buffer_count++] = _last;
	if (_buffer_count >= BUFFER_MAX)
		flush_buffer();
}

static void signal_profile(int sig, siginfo_t *info, void *context)
{
	/*char buf[8];
	
	_count++;
	
	sprintf(buf, "%d\n", _count);
	write(fileno(stderr), buf, strlen(buf));*/
	
	struct timeval tv;
	int diff;

	gettimeofday(&tv, NULL);
	diff = (tv.tv_sec - _last_time.tv_sec) * 1000 + (tv.tv_usec - _last_time.tv_usec) / 1000;
	if (diff <= 0)
		return;
	
	_last_time.tv_sec = tv.tv_sec;
	_last_time.tv_usec = tv.tv_usec / 1000 * 1000;

	_count += diff;
	
	if (FP == _last_fp)
	{
		if (((int)_last.count + diff) <= 65535)
		{
			_last.count += diff;
			return;
		}
	}
	
	if (_last.count)
		add_buffer();
	
	if (CP && FP)
	{
		_last.cp = CP;
		_last.fp = FP - CP->load->func;
	}
	else
	{
		_last.cp = NULL;
		_last.fp = 0;
	}
	
	_last.count = diff;
	_last_fp = FP;
}

void PROFILE_init(void)
{
	struct itimerval timer;
	char path[PATH_MAX + 1];
	
	if (!EXEC_profile)
		return;
	
	sprintf(path, "%d.gambas.prof.tmp", getpid());
	_fd = open(path, O_CREAT | O_RDWR, 0666);
	if (_fd < 0)
		ERROR_panic("Cannot create profile file '&1': &2", path, strerror(errno));
	
	unlink(path);
	
	SIGNAL_install(&_SIGVTALRM_handler, SIGPROF, signal_profile);
	
	gettimeofday(&_last_time, NULL);
	
	timer.it_interval.tv_sec = 0;
	timer.it_interval.tv_usec = 1000;
	timer.it_value.tv_sec = 0;
	timer.it_value.tv_usec = 1000;
	setitimer(ITIMER_PROF, &timer, NULL);
}

void PROFILE_exit(void)
{
	struct itimerval timer = { { 0 } };
	PROFILE_SLOT slot;
	CLASS *cp;
	FUNCTION *fp;
	int ret;
	
	if (!EXEC_profile)
		return;
	
	setitimer(ITIMER_PROF, &timer, NULL);
	SIGNAL_uninstall(&_SIGVTALRM_handler);
	
	add_buffer();
	flush_buffer();
	
	lseek(_fd, SEEK_SET, 0);
	
	for(;;)
	{
		ret = read(_fd, &slot, sizeof(PROFILE_SLOT));
		if (ret != sizeof(PROFILE_SLOT))
		{
			if (errno == EINTR)
				continue;
			if (ret == 0)
				break;
			ERROR_panic("Cannot read profile temporary file: %s", strerror(errno));
		}
		
		cp = slot.cp;
		if (cp)
		{
			fp = &cp->load->func[slot.fp];
			fprintf(stderr, "  % 5d %s.%s\n", slot.count, cp->name, fp->debug ? fp->debug->name : "?");
		}
		else
			fprintf(stderr, "  % 5d ?\n", slot.count);
	}
	
	close(_fd);
	
	fprintf(stderr, "% 7d *\n", _count);
}

