/***************************************************************************

  CDebug.c

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

#define __CDEBUG_C

#include <fcntl.h>
#include <errno.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>

#include "main.h"
#include "gb_limit.h"
#include "gb.debug.h"
#include "CDebug.h"

/*#define DEBUG*/


DECLARE_EVENT(EVENT_Read);

static int _started = FALSE;
static int _fdr = -1;
static int _fdw = -1;
static CDEBUG *_debug_object = NULL;

#define BUFFER_SIZE DEBUG_OUTPUT_MAX_SIZE
static char *_buffer = NULL;
static int _buffer_left;

static void callback_read(int fd, int type, intptr_t param)
{
  int n, i, p;

	for(;;)
  {
		fcntl(_fdr, F_SETFL, fcntl(_fdr, F_GETFL) | O_NONBLOCK);

    if (_buffer_left)
    {
      n = read(_fdr,  &_buffer[_buffer_left], BUFFER_SIZE - _buffer_left);
      if (n < 0)
        n = 0;
      
       n += _buffer_left;
      _buffer_left = 0;
    }
    else
      n = read(_fdr, _buffer, BUFFER_SIZE);

    if (n <= 0)
		{
			//usleep(10000); // the callback is called again and again even if there is nothing to read, why?
      break;
		}
    
    p = 0;

    for (i = 0; i < n; i++)
    {
      if (_buffer[i] == '\n')
      {
        /*fprintf(stderr, "CDEBUG_read: <<< %.*s >>>\n", i - p, &_buffer[p]);*/
        GB.Raise(_debug_object, EVENT_Read, 1, GB_T_STRING, i <= p ? NULL : &_buffer[p], i - p);
        if (!_buffer)
          break;
        p = i + 1;
      }
    }

    if (!_buffer)
			break;
    
    if (p == 0 && n >= BUFFER_SIZE)
    {
      GB.Raise(_debug_object, EVENT_Read, 1, GB_T_STRING, _buffer, BUFFER_SIZE);
      if (!_buffer)
        break;
      _buffer_left = 0;
    }
    else
    {
      _buffer_left = n - p;
      if (p && n > p)
        memmove(_buffer, &_buffer[p], _buffer_left);
    }
  }  
  
	fcntl(_fdr, F_SETFL, fcntl(_fdr, F_GETFL) & ~O_NONBLOCK);
}

// static void callback_read(int fd, int type, intptr_t param)
// {
// 	char *buffer = NULL;
// 	char tmp[256];
// 	int n;
//   
//   for(;;)
//   {
// 		n = read(_fdr, tmp, 256);
// 		if (n <= 0)
// 			break;
// 		GB.AddString(&buffer, tmp, n);
//   }
//   
// 	GB.Raise(_debug_object, EVENT_Read, 1, GB_T_STRING, buffer, GB.StringLength(buffer));  
// }

static char *input_fifo(char *path)
{
  sprintf(path, "/tmp/gambas.%d/%d.in", getuid(), getpid());
  return path;
}

static char *output_fifo(char *path)
{
  sprintf(path, "/tmp/gambas.%d/%d.out", getuid(), getpid());
  return path;
}

BEGIN_METHOD_VOID(CDEBUG_begin)

  char path[PATH_MAX];
	char name[16];
  
  signal(SIGPIPE, SIG_IGN);
  
  input_fifo(path);
  unlink(path);
  if (mkfifo(path, 0600))  
  {
    GB.Error("Cannot create input fifo in /tmp: &1", strerror(errno));
    return;
  }
  
  output_fifo(path);
  unlink(path);
  if (mkfifo(path, 0600))  
  {
    GB.Error("Cannot create output fifo in /tmp: &1", strerror(errno));
    return;
  }
	
	sprintf(name, "%d", getpid());
	GB.ReturnNewZeroString(name);
  
END_METHOD
 

BEGIN_METHOD_VOID(CDEBUG_start) 
  
  char path[DEBUG_FIFO_PATH_MAX];
  int i;
  
  if (_started)
    return;
  
  for (i = 0; i < 25; i++)
  {
  	_fdw = open(output_fifo(path), O_WRONLY | O_NONBLOCK);
  	if (_fdw >= 0)
  		break;
		usleep(20000);
	}
	
	if (_fdw < 0)
	{
		GB.Error("Unable to open fifo");
		return;
	}
	
  _fdr = open(input_fifo(path), O_RDONLY | O_NONBLOCK);
	fcntl(_fdr, F_SETFL, fcntl(_fdr, F_GETFL) & ~O_NONBLOCK);

  _debug_object = GB.New(GB.FindClass("Debug"), "Debug", NULL);
  GB.Ref(_debug_object);
  
  GB.Alloc(POINTER(&_buffer), BUFFER_SIZE);
  _buffer_left = 0;
  
  GB.Watch(_fdr, GB_WATCH_READ, (void *)callback_read, 0);
  
  _started = TRUE;

END_METHOD


BEGIN_METHOD_VOID(CDEBUG_stop) 
  
  if (!_started)
    return;

  GB.Watch(_fdr, GB_WATCH_NONE, (void *)callback_read, 0);
  GB.Free(POINTER(&_buffer));

  GB.Unref(POINTER(&_debug_object));

  close(_fdw);
  close(_fdr);
  
  _fdw = _fdr = -1;
  _started = FALSE;

END_METHOD


BEGIN_METHOD_VOID(CDEBUG_end)

  char path[DEBUG_FIFO_PATH_MAX];
  
  CALL_METHOD_VOID(CDEBUG_stop);
  
  unlink(input_fifo(path));
  unlink(output_fifo(path));
 
  signal(SIGPIPE, SIG_DFL);
  
END_METHOD


BEGIN_METHOD(CDEBUG_write, GB_STRING data)

	const char *data = STRING(data);
	int len = LENGTH(data);

  if (_fdw < 0)
  	return;
  	
  if (data && len > 0)
	{
    if (write(_fdw, data, len) != len)
			goto __ERROR;
	}
  if (write(_fdw, "\n", 1) != 1)
		goto __ERROR;
	
	return;
	
__ERROR:

	fprintf(stderr, "gb.debug: warning: unable to send data to the debugger: %s\n", strerror(errno));

END_METHOD


BEGIN_METHOD(Debug_GetSignal, GB_INTEGER signal)

	GB.ReturnNewZeroString(strsignal(VARG(signal)));

END_METHOD


GB_DESC CDebugDesc[] =
{
  GB_DECLARE("Debug", sizeof(CDEBUG)),
  GB_NOT_CREATABLE(),

  GB_STATIC_METHOD("_exit", NULL, CDEBUG_end, NULL),
  
  GB_STATIC_METHOD("Begin", "s", CDEBUG_begin, NULL),
  GB_STATIC_METHOD("End", NULL, CDEBUG_end, NULL),
  GB_STATIC_METHOD("Start", NULL, CDEBUG_start, NULL),
  GB_STATIC_METHOD("Stop", NULL, CDEBUG_stop, NULL),

  GB_STATIC_METHOD("GetSignal", "s", Debug_GetSignal, "(Signal)i"),

  GB_STATIC_METHOD("Write", NULL, CDEBUG_write, "(Data)s"),
	
  GB_EVENT("Read", NULL, "(Data)s", &EVENT_Read),

  GB_END_DECLARE
};

