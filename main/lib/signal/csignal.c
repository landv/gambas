/***************************************************************************

  csignal.c

  (c) 2000-2009 Benoît Minisini <gambas@users.sourceforge.net>

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
  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

***************************************************************************/

#define __CSIGNAL_C

#include <fcntl.h>
#include <errno.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "main.h"
#include "csignal.h"

#ifndef SIGPOLL
#define SIGPOLL SIGIO
#endif

#ifndef SIGPWR
#define SIGPWR -1
#endif

// The -1 signal is used for ignored signal numbers

/*#define DEBUG*/

#define SIGNAL_MAX 31

typedef
	struct SIGNAL_HANDLER {
		struct SIGNAL_HANDLER *next;
		struct SIGNAL_HANDLER *prev;
		int num;
		bool catch;
		struct sigaction action;
	}
	SIGNAL_HANDLER;

static int _signal = -1;
static SIGNAL_HANDLER *_handlers = NULL;
static bool _init_signal = FALSE;
static int _num_signal_watch = 0;
static int _pipe_signal[2];
static GB_FUNCTION _application_signal_func;

static void callback_signal(int fd, int type, void *data)
{
	char num;

	fprintf(stderr, "callback_signal\n");
	if (read(fd, &num, 1) != 1)
		return;

	GB.Push(1, GB_T_INTEGER, num);
	GB.Call(&_application_signal_func, 1, TRUE);
}

static void init_signal(void)
{
	_num_signal_watch++;
	if (_num_signal_watch > 1)
		return;

	if (GB.GetFunction(&_application_signal_func, (void *)GB.Application.StartupClass(), "Application_Signal", "i", ""))
	{
		GB.Error("No Application_Signal event handler defined in startup class");
		return;
	}

	if (pipe(_pipe_signal) != 0)
	{
		GB.Error("Cannot create signal handler pipes");
		return;
	}

	fcntl(_pipe_signal[0], F_SETFD, FD_CLOEXEC);
	fcntl(_pipe_signal[1], F_SETFD, FD_CLOEXEC);

	GB.Watch(_pipe_signal[0], GB_WATCH_READ, (void *)callback_signal, 0);
	_init_signal = TRUE;
}

static void exit_signal(void)
{
	if (_num_signal_watch <= 0)
		return;
	_num_signal_watch--;
	if (_num_signal_watch)
		return;

	GB.Watch(_pipe_signal[0], GB_WATCH_NONE, NULL, 0);
	close(_pipe_signal[0]);
	close(_pipe_signal[1]);
	
	_init_signal = FALSE;
}

static SIGNAL_HANDLER *find_handler(int num)
{
	SIGNAL_HANDLER *sh;
	
	sh = _handlers;
	while (sh)
	{
		if (sh->num == num)
			return sh;
		sh = sh->next;
	}
	
	return NULL;
}

static void remove_handler(SIGNAL_HANDLER *handler)
{
	if (handler->prev)
		handler->prev->next = handler->next;
	if (handler->next)
		handler->next->prev = handler->prev;
	if (handler == _handlers)
		_handlers = handler->next;
	
	if (handler->catch)
		exit_signal();
	
	GB.Free(POINTER(&handler));
}

static void add_handler(int num, struct sigaction *action)
{
	SIGNAL_HANDLER *sh;
	
	sh = find_handler(num);
	if (sh)
	{
		sigaction(num, action, NULL);
		return;
	}
	
	GB.Alloc(POINTER(&sh), sizeof(SIGNAL_HANDLER));
	
	sh->num = num;
	sh->next = _handlers;
	sh->prev = NULL;
	_handlers = sh;
	
	sh->catch = (action->sa_flags & SA_SIGINFO) != 0;
	if (sh->catch)
		init_signal();
	
	if (sigaction(num, action, &sh->action))
	{
		GB.Error("Unable to modify signal handler");
		exit_signal();
		return;
	}
}

static void handle_signal(int num, struct siginfo *info, void *context)
{
	char cnum = (char)num;
	if (write(_pipe_signal[1], &cnum, 1) != 1)
		return;
}

BEGIN_METHOD_VOID(Signal_Reset)

	SIGNAL_HANDLER *sh;
	
	if (_signal < 0)
		return;

	sh = find_handler(_signal);
	
	if (!sh)
		return;
	
	if (sigaction(_signal, &sh->action, NULL))
	{
		GB.Error("Unable to reset signal handler");
		return;
	}
	
	remove_handler(sh);

END_METHOD

BEGIN_METHOD_VOID(Signal_Ignore)

	struct sigaction action;

	if (_signal < 0)
		return;
	
	action.sa_handler = SIG_IGN;
	sigemptyset(&action.sa_mask);
	action.sa_flags = 0;
	
	add_handler(_signal, &action);

END_METHOD

BEGIN_METHOD_VOID(Signal_Catch)

	struct sigaction action;

	if (_signal < 0)
		return;
	
	action.sa_sigaction = handle_signal;
	sigemptyset(&action.sa_mask);
	action.sa_flags = SA_SIGINFO;
	
	add_handler(_signal, &action);

END_METHOD

BEGIN_METHOD(Signal_get, GB_INTEGER num)

	int num = VARG(num);

	if (num < -1 || num > SIGNAL_MAX)
	{
		GB.Error("Bad signal number");
		return;
	}
	
	_signal = num;
	RETURN_SELF();

END_METHOD

BEGIN_METHOD_VOID(Signal_exit)

	exit_signal();

	while (_handlers)
		remove_handler(_handlers);

END_METHOD


GB_DESC CSignalHandlerDesc[] =
{
  GB_DECLARE(".SignalHandler", 0), GB_VIRTUAL_CLASS(),
  
  GB_STATIC_METHOD("Reset", NULL, Signal_Reset, NULL),
  GB_STATIC_METHOD("Ignore", NULL, Signal_Ignore, NULL),
  GB_STATIC_METHOD("Catch", NULL, Signal_Catch, NULL),
  
  GB_END_DECLARE
};

GB_DESC CSignalDesc[] =
{
  GB_DECLARE("Signal", 0),
  GB_VIRTUAL_CLASS(),

	GB_CONSTANT("SIGHUP", "i", SIGHUP),
	GB_CONSTANT("SIGINT", "i", SIGINT),
	GB_CONSTANT("SIGQUIT", "i", SIGQUIT),
	GB_CONSTANT("SIGILL", "i", SIGILL),
	GB_CONSTANT("SIGTRAP", "i", SIGTRAP),
	GB_CONSTANT("SIGABRT", "i", SIGABRT),
	GB_CONSTANT("SIGIOT", "i", SIGIOT),
	GB_CONSTANT("SIGBUS", "i", SIGBUS),
	GB_CONSTANT("SIGFPE", "i", SIGFPE),
	GB_CONSTANT("SIGKILL", "i", SIGKILL),
	GB_CONSTANT("SIGUSR1", "i", SIGUSR1),
	GB_CONSTANT("SIGSEGV", "i", SIGSEGV),
	GB_CONSTANT("SIGUSR2", "i", SIGUSR2),
	GB_CONSTANT("SIGPIPE", "i", SIGPIPE),
	GB_CONSTANT("SIGALRM", "i", SIGALRM),
	GB_CONSTANT("SIGTERM", "i", SIGTERM),
	//GB_CONSTANT("SIGSTKFLT", "i", SIGSTKFLT),
	//GB_CONSTANT("SIGCLD", "i", SIGCLD),
	GB_CONSTANT("SIGCHLD", "i", SIGCHLD),
	GB_CONSTANT("SIGCONT", "i", SIGCONT),
	GB_CONSTANT("SIGSTOP", "i", SIGSTOP),
	GB_CONSTANT("SIGTSTP", "i", SIGTSTP),
	GB_CONSTANT("SIGTTIN", "i", SIGTTIN),
	GB_CONSTANT("SIGTTOU", "i", SIGTTOU),
	GB_CONSTANT("SIGURG", "i", SIGURG),
	GB_CONSTANT("SIGXCPU", "i", SIGXCPU),
	GB_CONSTANT("SIGXFSZ", "i", SIGXFSZ),
	GB_CONSTANT("SIGVTALRM", "i", SIGVTALRM),
	GB_CONSTANT("SIGPROF", "i", SIGPROF),
	GB_CONSTANT("SIGWINCH", "i", SIGWINCH),
	GB_CONSTANT("SIGPOLL", "i", SIGPOLL),
	GB_CONSTANT("SIGIO", "i", SIGIO),
	GB_CONSTANT("SIGPWR", "i", SIGPWR),
	GB_CONSTANT("SIGSYS", "i", SIGSYS),

	GB_STATIC_METHOD("_exit", NULL, Signal_exit, NULL),
  
  GB_STATIC_METHOD("_get", ".SignalHandler", Signal_get, "(Signal)i"),

  GB_END_DECLARE
};

