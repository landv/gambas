/***************************************************************************

  CDebug.c

  The Debug class

  (c) 2000-2007 Benoit Minisini <gambas@freesurf.fr>

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

#define __CDEBUG_C

#include <fcntl.h>
#include <errno.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "main.h"
#include "gb_limit.h"
#include "CDebug.h"

/*#define DEBUG*/


DECLARE_EVENT(EVENT_Read);

static int _started = FALSE;
static int _fdr;
static int _fdw;
static CDEBUG *_debug_object = NULL;

#define BUFFER_SIZE 4096
static char *_buffer = NULL;
static int _buffer_left;

static void callback_read(int fd, int type, long param)
{
  int n, i, p;

  for(;;)
  {
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
      break;
    
    p = 0;
    for (i = 0; i < n; i++)
    {
      if (_buffer[i] == '\n')
      {
        /*fprintf(stderr, "CDEBUG_read: <<< %.*s >>>\n", i - p, &_buffer[p]);*/
        GB.Raise(_debug_object, EVENT_Read, 1, GB_T_STRING, i <= p ? NULL : &_buffer[p], i - p);
        if (!_buffer)
          return;
        p = i + 1;
      }
    }
    
    if (p == 0 && n >= BUFFER_SIZE)
    {
      GB.Raise(_debug_object, EVENT_Read, 1, GB_T_STRING, _buffer, BUFFER_SIZE);
      if (!_buffer)
        return;
      _buffer_left = 0;
    }
    else
    {
      _buffer_left = n - p;
      if (p && n > p)
        memmove(_buffer, &_buffer[p], _buffer_left);
    }
  }  
}

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

  char path[MAX_PATH];
  
  signal(SIGPIPE, SIG_IGN);
  
  input_fifo(path);
  unlink(path);
  if (mkfifo(path, 0600))  
  {
    GB.Error("Cannot create input fifo");
    return;
  }
  
  output_fifo(path);
  unlink(path);
  if (mkfifo(path, 0600))  
  {
    GB.Error("Cannot create output fifo");
    return;
  }
  
END_METHOD
 

BEGIN_METHOD_VOID(CDEBUG_start) 
  
  char path[MAX_PATH];
  
  if (_started)
    return;
  
  _fdw = open(output_fifo(path), O_WRONLY);
  _fdr = open(input_fifo(path), O_RDONLY | O_NONBLOCK);
  
  GB.New(POINTER(&_debug_object), GB.FindClass("Debug"), "Debug", NULL);
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
  
  _started = FALSE;

END_METHOD


BEGIN_METHOD_VOID(CDEBUG_end)

  char path[MAX_PATH];
  
  CALL_METHOD_VOID(CDEBUG_stop);
  
  unlink(input_fifo(path));
  unlink(output_fifo(path));
 
  signal(SIGPIPE, SIG_DFL);
  
END_METHOD


BEGIN_METHOD(CDEBUG_write, GB_STRING data)

  /*fprintf(stderr, "CDEBUG_write: %.*s\n", LENGTH(data), STRING(data));
  if (STRING(data) && *STRING(data) == 'T')
    fprintf(stderr, "T ?\n");*/
  
  if (STRING(data))
    write(_fdw, STRING(data), LENGTH(data));
  write(_fdw, "\n", 1);

END_METHOD



GB_DESC CDebugDesc[] =
{
  GB_DECLARE("Debug", sizeof(CDEBUG)),
  GB_NOT_CREATABLE(),

  GB_STATIC_METHOD("_exit", NULL, CDEBUG_end, NULL),
  
  GB_STATIC_METHOD("Begin", NULL, CDEBUG_begin, NULL),
  GB_STATIC_METHOD("End", NULL, CDEBUG_end, NULL),
  GB_STATIC_METHOD("Start", NULL, CDEBUG_start, NULL),
  GB_STATIC_METHOD("Stop", NULL, CDEBUG_stop, NULL),

  GB_STATIC_METHOD("Write", NULL, CDEBUG_write, "(Data)s"),
  
  GB_EVENT("Read", NULL, "(Data)s", &EVENT_Read),

  GB_END_DECLARE
};

