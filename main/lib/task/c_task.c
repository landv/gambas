/***************************************************************************

  c_task.c

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

#define __C_TASK_C

#include <sys/wait.h>
#include <errno.h>

#include "c_task.h"

//DECLARE_EVENT(EVENT_Read);
//DECLARE_EVENT(EVENT_Error);
DECLARE_EVENT(EVENT_Kill);

static GB_SIGNAL_CALLBACK *_signal_handler = NULL;
static CTASK *_task_list = NULL;
static int _task_count = 0;

//-------------------------------------------------------------------------

static void stop_task(CTASK *_object);

static void callback_child(int signum, intptr_t data)
{
	CTASK *_object, *next;
	int status;
	
	_object = _task_list;
	while (_object)
	{
		next = THIS->list.next;
		if (wait4(THIS->pid, &status, WNOHANG, NULL) == THIS->pid)
		{
			stop_task(THIS);
		}
		_object = next;
	}
}

static void init_task(void)
{
	_task_count++;
	
	if (_task_count > 1)
		return;
	
	_signal_handler = GB.Signal.Register(SIGCHLD, callback_child, 0);
}

static void exit_task(void)
{
	_task_count--;
	
	if (_task_count > 0)
		return;
	
	GB.Signal.Unregister(SIGCHLD, _signal_handler);
	_signal_handler = NULL;
}

static bool start_task(CTASK *_object)
{
	pid_t pid;
	sigset_t sig, old;
	GB_FUNCTION func;

	init_task();

	GB.List.Add(&_task_list, THIS, &THIS->list);
	//process->running = TRUE;

	/*{
		if ((mode & PM_WRITE) && pipe(fdin) != 0)
			THROW_SYSTEM(errno, NULL);

		if ((mode & PM_READ) && (pipe(fdout) != 0 || pipe(fderr) != 0))
			THROW_SYSTEM(errno, NULL);
	}*/

	// Block SIGCHLD

	sigemptyset(&sig);

	sigaddset(&sig, SIGCHLD);
	sigprocmask(SIG_BLOCK, &sig, &old);

	pid = fork();
	if (pid == (-1))
	{
		stop_task(THIS);
		sigprocmask(SIG_SETMASK, &old, &sig);
		GB.Error("Cannot run task: &1", strerror(errno));
		return TRUE;
	}

	if (pid)
	{
		THIS->pid = pid;

		#ifdef DEBUG_ME
		fprintf(stderr, "fork: pid = %d\n", pid);
		#endif

		/*if (mode & PM_WRITE)
		{
			if (mode & PM_TERM)
			{
				process->in = fd_master;
			}
			else
			{
				close(fdin[0]);
				process->in = fdin[1];
			}
		}

		if (mode & PM_READ)
		{
			if (mode & PM_TERM)
			{
				process->out = fd_master;
				process->err = -1;
			}
			else
			{
				close(fdout[1]);
				close(fderr[1]);

				process->out = fdout[0];
				process->err = fderr[0];
			}

			GB_Watch(process->out, GB_WATCH_READ, (void *)callback_write, (intptr_t)process);
			if (process->err >= 0)
			{
				fcntl(process->err, F_SETFL, fcntl(process->err, F_GETFL) | O_NONBLOCK);
				GB_Watch(process->err, GB_WATCH_READ, (void *)callback_error, (intptr_t)process);
			}
		}

		if ((mode & PM_SHELL) == 0)
		{
			FREE(&argv, "run_process");
		}*/

		sigprocmask(SIG_SETMASK, &old, &sig);
	}
	else // child task
	{
		//bool stdin_isatty = isatty(STDIN_FILENO);
		
		THIS->pid = getpid();
		
		GB.System.HasForked();
		
		sigprocmask(SIG_SETMASK, &old, &sig);
		
		/*{
			if (mode & PM_WRITE)
			{
				close(fdin[1]);

				if (dup2(fdin[0], STDIN_FILENO) == -1)
					abort_child(CHILD_CANNOT_PLUG_INPUT);
			}

			if (mode & PM_READ)
			{
				close(fdout[0]);
				close(fderr[0]);

				if ((dup2(fdout[1], STDOUT_FILENO) == -1)
						|| (dup2(fderr[1], STDERR_FILENO) == -1))
					abort_child(CHILD_CANNOT_PLUG_OUTPUT);
			}
		}*/

		GB.GetFunction(&func, THIS, "Main", "", NULL);
		GB.Call(&func, 0, TRUE);
		exit(0);
	}
	
	return FALSE;
}


static void stop_task(CTASK *_object)
{
	GB.List.Remove(&_task_list, THIS, &THIS->list);

	GB.Raise(THIS, EVENT_Kill, 0);

	THIS->stopped = TRUE;
	
	GB.Unref(POINTER(&_object));
	
	exit_task();
}

//-------------------------------------------------------------------------

BEGIN_METHOD_VOID(Task_new)

	GB_FUNCTION func;
	
	if (GB.GetFunction(&func, THIS, "Main", "", NULL))
		return;
	
	GB.Ref(THIS);
	GB.Post((GB_CALLBACK)start_task, (intptr_t)THIS);
	
END_METHOD


BEGIN_PROPERTY(Task_Handle)

	GB.ReturnInteger(THIS->pid);

END_PROPERTY


BEGIN_METHOD_VOID(Task_Stop)

	kill(THIS->pid, SIGKILL);

END_METHOD

BEGIN_METHOD_VOID(Task_Wait)

	for(;;)
	{
		GB.Wait(0);
		if (THIS->stopped)
			break;
		sleep(10);
	}

END_METHOD

//-------------------------------------------------------------------------

GB_DESC TaskDesc[] =
{
	GB_DECLARE("Task", sizeof(CTASK)), GB_NOT_CREATABLE(),

	GB_METHOD("_new", NULL, Task_new, NULL),
	//GB_METHOD("_free", NULL, Task_free, NULL),

	GB_PROPERTY_READ("Handle", "i", Task_Handle),

	GB_METHOD("Stop", NULL, Task_Stop, NULL),
	GB_METHOD("Wait", NULL, Task_Wait, NULL),

	//GB_EVENT("Read", NULL, "(Data)s", &EVENT_Read),
	//GB_EVENT("Error", NULL, "(Error)s", &EVENT_Error),
	GB_EVENT("Kill", NULL, NULL, &EVENT_Kill),

	GB_END_DECLARE
};
