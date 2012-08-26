/***************************************************************************

  gbx_c_task.c

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

#define __GBX_C_TASK_C

#include "gbx_info.h"

#ifndef GBX_INFO

#include <sys/wait.h>
#include <sys/ioctl.h>
#include <errno.h>

#include "gb_common.h"
#include "gb_list.h"
#include "gb_file.h"
#include "gb_error.h"
#include "gbx_api.h"
#include "gbx_exec.h"
#include "gbx_string.h"
#include "gbx_signal.h"
#include "gbx_event.h"

#include "gbx_c_task.h"

DECLARE_EVENT(EVENT_Read);
DECLARE_EVENT(EVENT_Error);
DECLARE_EVENT(EVENT_Kill);

enum {
	CHILD_OK = 0,
	CHILD_ERROR = 1,
	CHILD_STDOUT = 2,
	CHILD_STDERR = 3,
	CHILD_RETURN = 4
};

static SIGNAL_CALLBACK *_signal_handler = NULL;
static CTASK *_task_list = NULL;
static int _task_count = 0;

//-------------------------------------------------------------------------

static void stop_task(CTASK *_object);

static void has_forked(void)
{
	FILE_init();
	EXEC_debug = FALSE;
	EXEC_task = TRUE;
}

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
			THIS->status = status;
			stop_task(THIS);
		}
		_object = next;
	}
}

static int get_readable(int fd)
{
	int len;
	
	if (ioctl(fd, FIONREAD, &len) < 0 || len <= 0)
		return 0;
	else
		return len;
}

static void callback_write(int fd, int type, CTASK *_object)
{
	int len;
	char *data;
	char *p;
	int n;
	
	//fprintf(stderr, "callback_write: %d %p\n", fd, THIS);
	
	len = get_readable(fd);
	
	data = STRING_new(NULL, len);
	p = data;
	
	while (len > 0)
	{
		n = read(fd, p, len);
		if (n < 0)
		{
			if (errno == EINTR)
				continue;
			else
				break;
		}
		len -= n;
		p += n;
	}
	
	GB_Raise(THIS, EVENT_Read, 1, GB_T_STRING, data, STRING_length(data) - len);
	
	STRING_free(&data);
}


static int callback_error(int fd, int type, CTASK *_object)
{
	char buffer[256];
	int n;

	//fprintf(stderr, "callback_error: %d %p\n", fd, THIS);

	n = read(fd, buffer, sizeof(buffer));

	if (n <= 0)
		return TRUE;

	GB_Raise(THIS, EVENT_Error, 1, GB_T_STRING, buffer, n);
	return FALSE;
}

static bool create_return_directory(void)
{
	static bool mkdir_done = FALSE;

	char buf[PATH_MAX];
	
	if (mkdir_done)
		return FALSE;

	sprintf(buf, RETURN_DIR_PATTERN, getuid(), getpid());
	
	if (mkdir(buf, S_IRWXU) != 0)
	{
		GB_Error("Cannot create task return directory");
		return TRUE;
	}
	
	mkdir_done = TRUE;
	return FALSE;
}

static void init_task(void)
{
	_task_count++;
	
	if (_task_count > 1)
		return;
	
	_signal_handler = SIGNAL_register(SIGCHLD, callback_child, 0);
}

static void exit_task(void)
{
	_task_count--;
	
	if (_task_count > 0)
		return;
	
	SIGNAL_unregister(SIGCHLD, _signal_handler);
	_signal_handler = NULL;
}

static void prepare_task(CTASK *_object)
{
	THIS->fd_out = -1;
	THIS->fd_err = -1;
}

static bool start_task(CTASK *_object)
{
	const char *err = NULL;
	pid_t pid;
	sigset_t sig, old;
	GB_FUNCTION func;
	int fd_out[2], fd_err[2];
	bool has_read, has_error;
	GB_VALUE *ret;
	char buf[PATH_MAX];
	FILE *f;

	init_task();

	LIST_insert(&_task_list, THIS, &THIS->list);

	// Create pipes
	
	has_read = GB_CanRaise(THIS, EVENT_Read);
	has_error = GB_CanRaise(THIS, EVENT_Error);
	
	if (has_read && pipe(fd_out) != 0)
		goto __ERROR;

	if (has_error && pipe(fd_err) != 0)
		goto __ERROR;

	// Block SIGCHLD

	sigemptyset(&sig);

	sigaddset(&sig, SIGCHLD);
	sigprocmask(SIG_BLOCK, &sig, &old);

	pid = fork();
	if (pid == (-1))
	{
		stop_task(THIS);
		sigprocmask(SIG_SETMASK, &old, &sig);
		goto __ERROR;
	}

	if (pid)
	{
		THIS->pid = pid;

		if (has_read)
		{
			close(fd_out[1]);
			THIS->fd_out = fd_out[0];

			GB_Watch(THIS->fd_out, GB_WATCH_READ, (void *)callback_write, (intptr_t)THIS);
		}

		if (has_error)
		{
			close(fd_err[1]);
			THIS->fd_err = fd_err[0];

			fcntl(THIS->fd_err, F_SETFL, fcntl(THIS->fd_err, F_GETFL) | O_NONBLOCK);
			GB_Watch(THIS->fd_err, GB_WATCH_READ, (void *)callback_error, (intptr_t)THIS);
		}

		sigprocmask(SIG_SETMASK, &old, &sig);
	}
	else // child task
	{
		THIS->child = TRUE;
		THIS->pid = getpid();
		
		has_forked();
		
		sigprocmask(SIG_SETMASK, &old, &sig);
		
		if (has_read)
		{
			close(fd_out[0]);

			if (dup2(fd_out[1], STDOUT_FILENO) == -1)
				exit(CHILD_STDOUT);
			
			setlinebuf(stdout);
		}

		if (has_error)
		{
			close(fd_err[0]);

			if (dup2(fd_err[1], STDERR_FILENO) == -1)
				exit(CHILD_STDERR);
			
			setlinebuf(stderr);
		}

		GB_GetFunction(&func, THIS, "Main", "", NULL);
		
		TRY
		{
			ret = GB_Call(&func, 0, FALSE);
			if (ret->type != GB_T_VOID)
			{
				sprintf(buf, RETURN_FILE_PATTERN, getuid(), getppid(), getpid());
				//fprintf(stderr, "serialize to: %s\n", buf);
				GB_ReturnConvVariant();
				if (GB_Serialize(buf, ret))
				{
					//fprintf(stderr, "gb.task: serialization has failed\n");
					exit(CHILD_RETURN);
				}
			}
		}
		CATCH
		{
			if (ERROR_current->info.code && ERROR_current->info.code != E_ABORT)
			{
				sprintf(buf, RETURN_FILE_PATTERN, getuid(), getppid(), getpid());
				f = fopen(buf, "w+");
				if (f)
				{
					ERROR_print_at(f, FALSE, FALSE);
					fclose(f);
				}
				
				exit(CHILD_ERROR);
			}
		}
		END_TRY
		
		exit(CHILD_OK);
	}
	
	return FALSE;

__ERROR:

	// TODO: as the routine is posted, nobody will see the error!
	if (!err)
		err = strerror(errno);
	fprintf(stderr, "gb.task: cannot run task: %s\n", err);
	GB_Error("Cannot run task: &1", err);
	
	return TRUE;
}

static void close_fd(int *pfd)
{
	int fd = *pfd;
	
	if (fd >= 0)
	{
		GB_Watch(fd, GB_WATCH_NONE, NULL, 0);
		close(fd);
		*pfd = -1;
	}
}

static void stop_task(CTASK *_object)
{
	int len;
	
	THIS->stopped = TRUE;
	
	// Flush standard error
	if (THIS->fd_err >= 0)
		while (callback_error(THIS->fd_err, 0, THIS) == 0);

	// Flush standard output
	if (THIS->fd_out >= 0)
	{
		for(;;)
		{
			len = get_readable(THIS->fd_out);
			if (len <= 0)
				break;
			
			THIS->something_read = FALSE;
			callback_write(THIS->fd_out, 0, THIS);
			if (!THIS->something_read)
				break;
		}
	}

	LIST_remove(&_task_list, THIS, &THIS->list);

	GB_Raise(THIS, EVENT_Kill, 0);

	close_fd(&THIS->fd_out);
	close_fd(&THIS->fd_err);
	
	OBJECT_UNREF(_object, "stop_task");
	
	exit_task();
}

//-------------------------------------------------------------------------

BEGIN_METHOD_VOID(Task_new)

	GB_FUNCTION func;
	
	if (create_return_directory())
		return;
	
	if (GB_GetFunction(&func, THIS, "Main", "", NULL))
		return;
	
	prepare_task(THIS);
	
	OBJECT_REF(THIS, "Task_new");
	EVENT_post((GB_CALLBACK)start_task, (intptr_t)THIS);
	
END_METHOD


BEGIN_PROPERTY(Task_Handle)

	GB_ReturnInteger(THIS->pid);

END_PROPERTY


BEGIN_METHOD_VOID(Task_Stop)

	kill(THIS->pid, SIGKILL);

END_METHOD

BEGIN_METHOD_VOID(Task_Wait)

	for(;;)
	{
		GB_Wait(0);
		if (THIS->stopped)
			break;
		sleep(10);
	}

END_METHOD

BEGIN_PROPERTY(Task_Value)

	char path[PATH_MAX];
	GB_VALUE ret;
	FILE_STAT info;
	char *err = NULL;
	int fd;
	
	if (!THIS->child && THIS->stopped)
	{
		sprintf(path, RETURN_FILE_PATTERN, getuid(), getpid(), THIS->pid);
		
		if (WIFEXITED(THIS->status))
		{
			switch (WEXITSTATUS(THIS->status))
			{
				case CHILD_OK:
					
					if (!GB_UnSerialize(path, &ret))
					{
						unlink(path);
						GB_ReturnVariant(&ret._variant.value);
						return;
					}
					else
						unlink(path);
					
					break;
					
				case CHILD_STDOUT:
					
					GB_Error("Unable to redirect task standard output");
					return;
					
				case CHILD_STDERR:
					
					GB_Error("Unable to redirect task error output");
					return;
					
				case CHILD_RETURN:
					
					GB_Error("Unable to serialize task return value");
					return;
					
				case CHILD_ERROR:
					
					FILE_stat(path, &info, FALSE);
					
					err = STRING_new_temp(NULL, info.size);
					
					fd = open(path, O_RDONLY);
					if (fd < 0) goto __ERROR;
					
					STREAM_read_direct(fd, err, info.size);
					close(fd);
					
					GB_Error("Task has failed: &1", err);
					return;
			}
		}
		else if (WIFSIGNALED(THIS->status))
		{
			GB_Error("Task has aborted: &1", strsignal(WTERMSIG(THIS->status)));
			return;
		}
	}
	
__ERROR:
	
	GB_ReturnNull();
	GB_ReturnConvVariant();
	
END_PROPERTY


//-------------------------------------------------------------------------

#endif

GB_DESC TaskDesc[] =
{
	GB_DECLARE("Task", sizeof(CTASK)), GB_NOT_CREATABLE(),
	
	GB_METHOD("_new", NULL, Task_new, NULL),
	//GB_METHOD("_free", NULL, Task_free, NULL),

	GB_PROPERTY_READ("Handle", "i", Task_Handle),
	GB_PROPERTY_READ("Value", "v", Task_Value),

	GB_METHOD("Stop", NULL, Task_Stop, NULL),
	GB_METHOD("Wait", NULL, Task_Wait, NULL),

	GB_EVENT("Read", NULL, "(Data)s", &EVENT_Read),
	GB_EVENT("Error", NULL, "(Data)s", &EVENT_Error),
	GB_EVENT("Kill", NULL, NULL, &EVENT_Kill),

	GB_END_DECLARE
};
