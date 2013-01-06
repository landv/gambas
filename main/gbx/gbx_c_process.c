/***************************************************************************

  gbx_c_process.c

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

#define __GBX_C_PROCESS_C

#include "gbx_info.h"

#ifndef GBX_INFO

#include "gb_common.h"
#include "gb_common_buffer.h"

#include <unistd.h>
#include <signal.h>
#include <fcntl.h>
#include <termios.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <sys/resource.h>

#ifdef OS_OPENBSD
/* granpt(), unlockpt() and ptsname() unavailable under OpenBSD
	and are replaced with openpty() implementation because of security
	issues */
#include <util.h>
#endif

#include "gb_replace.h"
#include "gb_limit.h"
#include "gb_array.h"
#include "gbx_api.h"
#include "gambas.h"
#include "gbx_stream.h"
#include "gbx_exec.h"
#include "gbx_class.h"
#include "gbx_watch.h"
#include "gbx_project.h"
#include "gbx_c_array.h"
#include "gbx_local.h"
#include "gbx_signal.h"

#include "gbx_c_process.h"

//#define DEBUG_ME

char *CPROCESS_shell = NULL;

extern char **environ;

DECLARE_EVENT(EVENT_Read);
DECLARE_EVENT(EVENT_Error);
DECLARE_EVENT(EVENT_Kill);

static CPROCESS *RunningProcessList = NULL;
//static int _pipe_child[2];
static SIGNAL_CALLBACK *_SIGCHLD_callback;
static bool _init = FALSE;

static int _last_status = 0;

static int _last_child_error = 0;
static int _last_child_error_errno = 0;

static void init_child(void);
static void exit_child(void);

enum {
	CHILD_NO_ERROR,
	CHILD_CANNOT_OPEN_TTY,
	CHILD_CANNOT_INIT_TTY,
	CHILD_CANNOT_PLUG_INPUT,
	CHILD_CANNOT_PLUG_OUTPUT,
	CHILD_CANNOT_EXEC,
};
	
static const char *const _child_error[] = {
	NULL,
	"cannot open slave pseudo-terminal: ",
	"cannot initialize pseudo-terminal: ",
	"cannot plug standard input: ",
	"cannot plug standard output and standard error: ",
	"cannot exec program: "
};

//-------------------------------------------------------------------------

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

static void callback_write(int fd, int type, CPROCESS *process)
{
	#ifdef DEBUG_ME
	fprintf(stderr, "callback_write: %d %p\n", fd, process);
	#endif
	
	if (process->to_string)
	{
		int n = read(fd, COMMON_buffer, 256);
		if (n > 0)
			process->result = STRING_add(process->result, COMMON_buffer, n);
	}
	else if (!STREAM_is_closed(CSTREAM_stream(process)) && !STREAM_eof(CSTREAM_stream(process))) //process->running &&
		GB_Raise(process, EVENT_Read, 0);
	else
		close_fd(&process->out);
}


static int callback_error(int fd, int type, CPROCESS *process)
{
	char buffer[256];
	int n;

	#ifdef DEBUG_ME
	fprintf(stderr, "callback_error: %d %p\n", fd, process);
	#endif

	n = read(fd, buffer, sizeof(buffer));

	if (n <= 0) /* || !process->running)*/
	{
		/*close(process->err);*/
		return TRUE;
	}

	/*printf("callback_error: (%d) %.*s\n", n, n, buffer);*/

	GB_Raise(process, EVENT_Error, 1, GB_T_STRING, buffer, n);

	return FALSE;

	/*fprintf(stderr, "<< Write\n"); fflush(stderr);*/
}


static void update_stream(CPROCESS *process)
{
	STREAM *stream = &process->ob.stream;

	stream->type = &STREAM_process;
	(*stream->type->open)(stream, NULL, 0, process);
}


static void init_process(CPROCESS *process)
{
	process->watch = GB_WATCH_NONE;
	process->in = process->out = process->err = -1;
	update_stream(process);
}

static void exit_process(CPROCESS *_object)
{
	#ifdef DEBUG_ME
	fprintf(stderr, "exit_process: %p\n", _object);
	#endif

	if (THIS->in >= 0)
	{
		if (THIS->in != THIS->out)
			close(THIS->in);
		THIS->in = -1;
	}

	close_fd(&THIS->out);
	close_fd(&THIS->err);

	STREAM_close(&THIS->ob.stream);
}

static void prepare_child_error(CPROCESS *_object)
{
	int fd;
	char path[PATH_MAX];
	
	snprintf(path, sizeof(path), FILE_TEMP_DIR "/%d.child", (int)getuid(), (int)getpid(), (int)THIS->pid);
	
	#ifdef DEBUG_ME
	fprintf(stderr, "prepare_child_error: %p: %s\n", _object, path);
	#endif

	if (_last_child_error == 0)
	{
		fd = open(path, O_RDONLY);
		if (fd >= 0)
		{
			if (read(fd, &_last_child_error, sizeof(int)) != sizeof(int)
					|| read(fd, &_last_child_error_errno, sizeof(int)) != sizeof(int))
			{
				_last_child_error = -1;
				_last_child_error_errno = 0;
			}
			
			close(fd);
		}

		#ifdef DEBUG_ME
		fprintf(stderr, "prepare_child_error: error = %d errno = %d\n", _last_child_error, _last_child_error_errno);
		#endif
	}
	
	unlink(path);
}

static void throw_last_child_error()
{
	int child_error, child_errno;
	
	if (_last_child_error == 0)
		return;
	
	child_error = _last_child_error;
	child_errno = _last_child_error_errno;
	
	_last_child_error = 0;
	
	#ifdef DEBUG_ME
	fprintf(stderr, "throw_last_child_error: %d %d\n", child_error, child_errno);
	#endif
	
	if (child_error < 0)
		THROW(E_CHILD, "unknown error", "");
	else
		THROW(E_CHILD, _child_error[child_error], strerror(child_errno));
}

static void stop_process_after(CPROCESS *_object)
{
	STREAM *stream;
	int64_t len, len2;

	#ifdef DEBUG_ME
	fprintf(stderr, "stop_process_after: %p\n", _object);
	#endif

	if (WIFEXITED(THIS->status) && WEXITSTATUS(THIS->status) == 255)
	{
		prepare_child_error(THIS);
		exit_process(THIS);
		OBJECT_detach((OBJECT *)THIS);
	}

	/* Vidage du tampon d'erreur */
	if (THIS->err >= 0)
		while (callback_error(THIS->err, 0, THIS) == 0);

	/* Vidage du tampon de sortie */
	if (THIS->out >= 0)
	{
		stream = CSTREAM_stream(THIS);
		while (!STREAM_eof(stream))
		{
			STREAM_lof(stream, &len);
			callback_write(THIS->out, 0, THIS);
			if (STREAM_is_closed(stream))
				break;
			STREAM_lof(stream, &len2);
			if (len == len2)
				break;
		}
	}

	exit_process(THIS);

	GB_Raise(THIS, EVENT_Kill, 0);

	OBJECT_detach((OBJECT *)THIS);

	/*printf("** stop_process_after\n");*/
	//GB_Unref((void **)&_object); /* Ref du post */
}


static void stop_process(CPROCESS *process)
{
	if (!process->running)
		return;

	#ifdef DEBUG_ME
	fprintf(stderr, "stop_process: %p\n", process);
	#endif

	/* Remove from running process list */

	if (process->prev)
		process->prev->next = process->next;

	if (process->next)
		process->next->prev = process->prev;

	if (process == RunningProcessList)
		RunningProcessList = process->next;

	process->running = FALSE;

	/*printf("** stop_process\n");*/

	stop_process_after(process);

	OBJECT_UNREF(process, "stop_process"); /* Le processus ne tourne plus */

	if (!RunningProcessList)
		exit_child();
}

static void abort_child(int error)
{
	int fd;
	int save_errno;
	char path[PATH_MAX];
	
	fflush(stdout);
	fflush(stderr);
	
	save_errno = errno;
	
	#ifdef DEBUG_ME
	fprintf(stderr, "abort_child: %d %d\n", error, save_errno);
	#endif
	
	snprintf(path, sizeof(path), FILE_TEMP_DIR "/%d.child", (int)getuid(), (int)getppid(), (int)getpid());
	
	fd = open(path, O_CREAT | O_WRONLY, 0600);
	if (fd >= 0)
	{
		write(fd, &error, sizeof(int)) == sizeof(int)
		&& write(fd, &save_errno, sizeof(int)) == sizeof(int);
		close(fd);
	}
	
	exit(255);
}

static void init_child_tty(int fd)
{
	struct termios terminal = { 0 };
	tcgetattr(fd, &terminal);
	
	terminal.c_iflag |= ICRNL | IXON | IXOFF;
	#ifdef IUTF8
	if (LOCAL_is_UTF8) 
		terminal.c_iflag |= IUTF8;
	#endif
	
	terminal.c_oflag |= OPOST;
	terminal.c_oflag &= ~ONLCR;
	
	terminal.c_lflag |= ISIG | ICANON | IEXTEN; // | ECHO;
	terminal.c_lflag &= ~ECHO;
	
	#ifdef DEBUG_ME
	fprintf(stderr, "init_child_tty: %s\n", isatty(fd) ? ttyname(fd) : "not a tty!");
	#endif
	
	if (tcsetattr(fd, TCSANOW, &terminal))
	{
		#ifdef DEBUG_ME
		int save_errno = errno;
		fprintf(stderr, "init_child_tty: errno = %d\n", errno);
		errno = save_errno;
		#endif
		abort_child(CHILD_CANNOT_INIT_TTY);
	}
}

static void run_process(CPROCESS *process, int mode, void *cmd, CARRAY *env)
{
	static const char *shell[] = { "/bin/sh", "-c", NULL, NULL };

	int fdin[2], fdout[2], fderr[2];
	pid_t pid;
	char **argv;
	CARRAY *array;
	int i, n;
	sigset_t sig, old;

	/* for terminal */
	int fd_master = -1;
	int fd_slave;
	char *slave = NULL;
	//struct termios termios_stdin;
	//struct termios termios_check;
	struct termios termios_master;

	init_child();

	if (mode & PM_SHELL)
	{
		#ifdef DEBUG_ME
		fprintf(stderr, "run_process %p: %s\n", process, (char *)cmd);
		#endif

		argv = (char **)shell;

		if (CPROCESS_shell)
			argv[0] = CPROCESS_shell;
		
		argv[2] = (char *)cmd;

		if (argv[2] == NULL || *argv[2] == 0)
			return;
		
		process->process_group = TRUE;
	}
	else
	{
		array = (CARRAY *)cmd;
		n = ARRAY_count(array->data);

		if (n == 0)
			return;

		ALLOC(&argv, sizeof(*argv) * (n + 1), "run_process");
		memcpy(argv, array->data, sizeof(*argv) * n);
		argv[n] = NULL;

		for (i = 0; i < n; i++)
		{
			if (!argv[i])
				argv[i] = "";
		}

		#ifdef DEBUG_ME
		{
			int i;

			fprintf(stderr, "run_process %p: ", process);
			for (i = 0; i < n; i++)
				fprintf(stderr, "%s ", argv[i]);
			fprintf(stderr, "\n");
		}
		#endif
	}

	// Adding to the running process list

	OBJECT_REF(process, "run_process");

	if (RunningProcessList)
		RunningProcessList->prev = process;

	process->next = RunningProcessList;
	process->prev = NULL;

	RunningProcessList = process;

	process->running = TRUE;

	if (mode & PM_STRING)
	{
		process->to_string = TRUE;
		process->result = NULL;
		mode |= PM_READ;
	}

	if (mode & PM_TERM)
	{
		#ifdef OS_OPENBSD
		if (openpty(&fd_master, &fd_slave, NULL, NULL, NULL)<0)
			THROW_SYSTEM(errno, NULL);
		#else
		fd_master = posix_openpt(O_RDWR | O_NOCTTY);
		if (fd_master < 0)
			THROW_SYSTEM(errno, NULL);

		grantpt(fd_master);
		unlockpt(fd_master);
		#endif
		slave = ptsname(fd_master);
		#ifdef DEBUG_ME
		fprintf(stderr, "run_process: slave = %s\n", slave);
		#endif
	}
	else
	{
		/* Create pipes */

		if ((mode & PM_WRITE) && pipe(fdin) != 0)
			THROW_SYSTEM(errno, NULL);

		if ((mode & PM_READ) && (pipe(fdout) != 0 || pipe(fderr) != 0))
			THROW_SYSTEM(errno, NULL);
	}

	/* Block SIGCHLD */

	sigemptyset(&sig);

	sigaddset(&sig, SIGCHLD);
	sigprocmask(SIG_BLOCK, &sig, &old);

	pid = fork();
	if (pid == (-1))
	{
		stop_process(process);
		sigprocmask(SIG_SETMASK, &old, &sig);
		THROW_SYSTEM(errno, NULL);
	}

	if (pid)
	{
		process->pid = pid;

		#ifdef DEBUG_ME
		fprintf(stderr, "fork: pid = %d\n", pid);
		#endif

		if (mode & PM_TERM)
		{
			if (tcgetattr(fd_master, &termios_master))
				THROW_SYSTEM(errno, NULL);
				
			cfmakeraw(&termios_master);
			//termios_master.c_lflag &= ~ECHO;

			if (tcsetattr(fd_master, TCSANOW, &termios_master))
				THROW_SYSTEM(errno, NULL);
		}

		if (mode & PM_WRITE)
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
		}

		sigprocmask(SIG_SETMASK, &old, &sig);
	}
	else /* child */
	{
		//bool stdin_isatty = isatty(STDIN_FILENO);
		
		sigprocmask(SIG_SETMASK, &old, &sig);
		
		if (mode & PM_SHELL)
			setpgid(0, 0);

		if (mode & PM_TERM)
		{
			close(fd_master);
			setsid();
			fd_slave = open(slave, O_RDWR);
			if (fd_slave < 0)
				abort_child(CHILD_CANNOT_OPEN_TTY);
			
			#ifdef DEBUG_ME
			fprintf(stderr, "run_process (child): slave = %s isatty = %d\n", slave, isatty(fd_slave));
			#endif

			if (mode & PM_WRITE)
			{
				if (dup2(fd_slave, STDIN_FILENO) == -1)
					abort_child(CHILD_CANNOT_PLUG_INPUT);
			}

			if (mode & PM_READ)
			{
				if ((dup2(fd_slave, STDOUT_FILENO) == -1)
						|| (dup2(fd_slave, STDERR_FILENO) == -1))
					abort_child(CHILD_CANNOT_PLUG_OUTPUT);
			}

			// Strange Linux behaviour ?
			// Terminal initialization must be done on STDIN_FILENO after using dup2().
			// If it is done on fd_slave, before using dup2(), it sometimes fails with no error.
		
			if (mode & PM_WRITE)
				init_child_tty(STDIN_FILENO);
			else if (mode & PM_READ)
				init_child_tty(STDOUT_FILENO);
			
			/*puts("---------------------------------");
			if (stdin_isatty) puts("STDIN is a tty");*/
			/*tcgetattr(STDIN_FILENO, &termios_check);
			puts(termios_check.c_lflag & ISIG ? "+ISIG" : "-ISIG");
			//tcsetattr(STDIN_FILENO, TCSADRAIN, &termios_check);
			system("stty icanon");
			system("stty -a");
			puts("---------------------------------");*/
		}
		else
		{
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
		}

		// Return to the parent working directory
		FILE_chdir(PROJECT_oldcwd);
		
		if (env)
		{
			char *str;
			n = ARRAY_count(env->data);
			for (i = 0; i < n; i++)
			{
				str = ((char **)env->data)[i];
				if (putenv(str))
					ERROR_warning("cannot set environment string: %s", str);
			}
		}
		
		execvp(argv[0], (char **)argv);
		//execve(argv[0], (char **)argv, environ);
		abort_child(CHILD_CANNOT_EXEC);
	}

	update_stream(process);
}


static void callback_child(int signum, intptr_t data)
{
	int status;
	CPROCESS *process, *next;
	//int buffer;

#if 0
	for(;;)
	{
		if (read(fd, (char *)&buffer, 1) == 1)
			break;
		if (errno != EINTR)
			ERROR_panic("Cannot read from SIGCHLD pipe: %s", strerror(errno));
	}
#endif
	#ifdef DEBUG_ME
	fprintf(stderr, "<< callback_child\n");
	#endif

	for (process = RunningProcessList; process; )
	{
		next = process->next;

		if (wait4(process->pid, &status, WNOHANG, NULL) == process->pid)
		{
			process->status = status;
			_last_status = status;

			#ifdef DEBUG_ME
			fprintf(stderr, "Process %d has returned %d\n", process->pid, status);
			#endif

			/*printf("** signal_child\n");*/

			stop_process(process);
			//stop_process_after(process);
			//break; // one exit at a time // why ?
		}

		process = next;
	}

	throw_last_child_error();
	
	#ifdef DEBUG_ME
	fprintf(stderr, ">> callback_child\n");
	#endif
}

#if 0
static void signal_child(int sig, siginfo_t *info, void *context)
{
	#ifdef DEBUG_ME
	fprintf(stderr, "SIGCHLD: _pipe_child[0] = %d\n", _pipe_child[0]);
	#endif
	char buffer;
	int save_errno;

	save_errno = errno;
			
	if (_init)
	{
		//fprintf(stderr, "SIGCHLD\n");
		
		buffer = 42;
		for(;;)
		{
			if (write(_pipe_child[1], &buffer, 1) == 1)
				break;
			
			if (errno != EINTR)
				ERROR_panic("Cannot write into SIGCHLD pipe: %s", strerror(errno));
		}
	}
	
	SIGNAL_previous(&_SIGCHLD_handler, sig, info, context);
	
	errno = save_errno;
}
#endif

static void init_child(void)
{
	if (_init)
		return;

	#ifdef DEBUG_ME
	fprintf(stderr, "init_child()\n");
	#endif

	_SIGCHLD_callback = SIGNAL_register(SIGCHLD, callback_child, 0);
	
#if 0
	if (pipe(_pipe_child) != 0)
		ERROR_panic("Cannot create SIGCHLD pipes: %s", strerror(errno));

	fcntl(_pipe_child[0], F_SETFD, FD_CLOEXEC);
	fcntl(_pipe_child[1], F_SETFD, FD_CLOEXEC);

	#ifdef DEBUG_ME
	fprintf(stderr, "_pipe_child[0] = %d\n", _pipe_child[0]);
	#endif

	SIGNAL_install(&_SIGCHLD_handler, SIGCHLD, signal_child);
	
	GB_Watch(_pipe_child[0], GB_WATCH_READ, (void *)callback_child, 0);
#endif
	_init = TRUE;
}

static void exit_child(void)
{
	if (!_init)
		return;

	#ifdef DEBUG_ME
	fprintf(stderr, "exit_child()\n");
	#endif

	SIGNAL_unregister(SIGCHLD, _SIGCHLD_callback);

#if 0
	GB_Watch(_pipe_child[0], GB_WATCH_NONE, NULL, 0);
	close(_pipe_child[0]);
	close(_pipe_child[1]);
	
	SIGNAL_uninstall(&_SIGCHLD_handler, SIGCHLD);
#endif
	_init = FALSE;
}


CPROCESS *CPROCESS_create(int mode, void *cmd, char *name, CARRAY *env)
{
	CPROCESS *process;

	/*printf("** CPROCESS_create <<<< \n");*/

	//if (!name || !*name)
	//	name = "Process";

	process = OBJECT_new(CLASS_Process, name, OP  ? (OBJECT *)OP : (OBJECT *)CP);

	init_process(process);
	run_process(process, mode, cmd, env);

	OBJECT_UNREF_KEEP(process, "CPROCESS_create");

	if (!name || !*name)
		STREAM_blocking(CSTREAM_stream(process), TRUE);
	
	/*printf("** CPROCESS_create >>>> \n");*/

	return process;
}

static void error_CPROCESS_wait_for(CPROCESS *process)
{
	OBJECT_UNREF(process, "CPROCESS_wait_for");
}

void CPROCESS_wait_for(CPROCESS *process, int timeout)
{
	int ret;
	int sigfd;

	#ifdef DEBUG_ME
	printf("Waiting for %d\n", process->pid);
	#endif

	OBJECT_REF(process, "CPROCESS_wait_for");
	
	sigfd = SIGNAL_get_fd();
	
	ON_ERROR_1(error_CPROCESS_wait_for, process)
	{
		while (process->running)
		{
			ret = WATCH_process(sigfd, process->out, timeout);
			if (ret & WP_OUTPUT)
				callback_write(process->out, GB_WATCH_READ, process);
			if (ret & WP_END)
				SIGNAL_raise_callbacks(sigfd, GB_WATCH_READ, 0);
			if (ret & WP_TIMEOUT)
				break;
			if (ret == 0)
				usleep(1000);
		}
	}
	END_ERROR
	
	OBJECT_UNREF(process, "CPROCESS_wait_for");

	#if 0
	{
		sigsuspend(&old);
		if (!process->running)
			break;

		#ifdef DEBUG_ME
		fprintf(stderr, "Waiting: %d\n", process->running);
		#endif

		sleep(10);
	}
	#endif

	/*if (have_sigchld)
		sigaddset(&old, SIGCHLD);
	sigprocmask(SIG_SETMASK, &old, NULL);*/

	#ifdef DEBUG_ME
	printf("Got it !\n");
	#endif
}



BEGIN_METHOD_VOID(Process_exit)

	//fprintf(stderr, "Process_exit\n");

	while (RunningProcessList)
		stop_process(RunningProcessList);

	exit_child();
	
	STRING_free(&CPROCESS_shell);

END_METHOD


BEGIN_METHOD_VOID(Process_new)

	init_process(THIS);

END_METHOD


BEGIN_METHOD_VOID(Process_free)

	#ifdef DEBUG_ME
	printf("Process_free %p\n", THIS);
	#endif

	exit_process(THIS);

END_METHOD


BEGIN_PROPERTY(Process_Id)

	GB_ReturnInt(THIS->pid);

END_PROPERTY


BEGIN_METHOD_VOID(Process_Kill)

	if (!THIS->running)
		return;

	if (THIS->process_group)
		kill(-getpgid(THIS->pid), SIGKILL);
	else
		kill(THIS->pid, SIGKILL);
	
	//CPROCESS_wait_for(THIS);

END_METHOD

BEGIN_METHOD_VOID(Process_Signal)

	if (!THIS->running)
		return;

	/*
	printf("Send SIGUSR1 to process %d\n", THIS->pid);
	fflush(NULL);*/
	kill(THIS->pid, SIGUSR1);

END_METHOD


#if 0
BEGIN_METHOD(CPROCESS_send, GB_STRING text)

	if (!THIS->running || THIS->in < 0)
		return;

	STREAM_write(&THIS->ob.stream, STRING(text), LENGTH(text));

END_METHOD
#endif

BEGIN_PROPERTY(Process_State)

	if (THIS->running)
		GB_ReturnInteger(1);
	else
	{
		if (WIFEXITED(THIS->status))
			GB_ReturnInteger(0);
		else
			GB_ReturnInteger(2);
	}

END_PROPERTY


BEGIN_PROPERTY(Process_Value)

	int status;

	if (THIS->running)
	{
		GB_ReturnInteger(0);
		return;
	}

	status = THIS->status;

	if (WIFEXITED(status))
		GB_ReturnInteger(WEXITSTATUS(status));
	else if (WIFSIGNALED(status))
		GB_ReturnInteger(WTERMSIG(status));
	else
		GB_ReturnInteger(-1);

END_PROPERTY

BEGIN_PROPERTY(Process_LastState)

	if (WIFEXITED(_last_status))
		GB_ReturnInteger(0);
	else
		GB_ReturnInteger(2);

END_PROPERTY


BEGIN_PROPERTY(Process_LastValue)

	int status;

	status = _last_status;

	if (WIFEXITED(status))
		GB_ReturnInteger(WEXITSTATUS(status));
	else if (WIFSIGNALED(status))
		GB_ReturnInteger(WTERMSIG(status));
	else
		GB_ReturnInteger(-1);

END_PROPERTY

BEGIN_METHOD(Process_Wait, GB_FLOAT timeout)

	CPROCESS_wait_for(THIS, (int)(VARGOPT(timeout, 0.0) * 1000));

END_METHOD

#endif

GB_DESC NATIVE_Process[] =
{
	GB_DECLARE("Process", sizeof(CPROCESS)), GB_NOT_CREATABLE(),
	GB_INHERITS("Stream"),

	GB_CONSTANT("Stopped", "i", 0),
	GB_CONSTANT("Running", "i", 1),
	GB_CONSTANT("Crashed", "i", 2),
	GB_CONSTANT("Signaled", "i", 2),

	GB_STATIC_PROPERTY_READ("LastState", "i", Process_LastState),
	GB_STATIC_PROPERTY_READ("LastValue", "i", Process_LastValue),
	
	GB_PROPERTY_READ("Id", "i", Process_Id),
	GB_PROPERTY_READ("Handle", "i", Process_Id),
	GB_PROPERTY_READ("State", "i", Process_State),
	GB_PROPERTY_READ("Value", "i", Process_Value),

	GB_STATIC_METHOD("_exit", NULL, Process_exit, NULL),
	GB_METHOD("_new", NULL, Process_new, NULL),
	GB_METHOD("_free", NULL, Process_free, NULL),

	GB_METHOD("Kill", NULL, Process_Kill, NULL),
	GB_METHOD("Signal", NULL, Process_Signal, NULL),
	GB_METHOD("Wait", NULL, Process_Wait, "[(Timeout)f]"),

	GB_EVENT("Read", NULL, NULL, &EVENT_Read),
	GB_EVENT("Error", NULL, "(Error)s", &EVENT_Error),
	GB_EVENT("Kill", NULL, NULL, &EVENT_Kill),

	GB_END_DECLARE
};

