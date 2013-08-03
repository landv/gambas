/***************************************************************************

	csignal.c

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

// The "-1" signal is used for ignoring signal numbers

#ifndef SIGPWR
#define SIGPWR -1
#endif

#ifdef OS_CYGWIN
#define SIGIOT -1
#endif


#define NUM_SIGNALS 32

enum {
	SH_DEFAULT = 0,
	SH_IGNORE = 1,
	SH_CATCH = 2
};

typedef
	struct SIGNAL_HANDLER {
		GB_SIGNAL_CALLBACK *handler;
		struct sigaction action;
		char state;
	}
	SIGNAL_HANDLER;

static SIGNAL_HANDLER _signals[NUM_SIGNALS] = { { 0 } };
static int _signal = -1;
static GB_FUNCTION _application_signal_func;
static bool _init_signal = FALSE;

static void init_signal(void)
{
	if (GB.GetFunction(&_application_signal_func, (void *)GB.Application.StartupClass(), "Application_Signal", "i", ""))
	{
		GB.Error("No Application_Signal event handler defined in startup class");
		return;
	}

	_init_signal = TRUE;
}

static void catch_signal(int num, intptr_t data)
{
	GB.Push(1, GB_T_INTEGER, num);
	GB.Call(&_application_signal_func, 1, TRUE);
}

static void handle_signal(int num, char state)
{
	struct sigaction action;
	SIGNAL_HANDLER *sh;
	
	if (num < 0)
		return;
	
	sh = &_signals[num];
	
	if (sh->state == state)
		return;
	
	if (sh->state == SH_IGNORE)
	{
		if (sigaction(num, &sh->action, NULL))
		{
			GB.Error("Unable to reset signal handler");
			return;
		}
	}
	else if (sh->state == SH_CATCH)
	{
		if (sh->handler)
		{
			GB.Signal.Unregister(num, sh->handler);
			sh->handler = NULL;
		}
	}
	
	if (state == SH_IGNORE)
	{
		action.sa_handler = SIG_IGN;
		sigemptyset(&action.sa_mask);
		action.sa_flags = 0;
		
		if (sigaction(num, &action, &sh->action))
		{
			GB.Error("Unable to modify signal handler");
			return;
		}
	}
	else if (state == SH_CATCH)
	{
		if (!_init_signal)
			init_signal();
		
		sh->handler = GB.Signal.Register(num, catch_signal, 0);
	}
	
	sh->state = state;
}

static void exit_signal(void)
{
	int i;
	
	for (i = 0; i < NUM_SIGNALS; i++)
		handle_signal(i, SH_DEFAULT);
}


BEGIN_METHOD_VOID(Signal_Reset)

	handle_signal(_signal, SH_DEFAULT);

END_METHOD

BEGIN_METHOD_VOID(Signal_Ignore)

	handle_signal(_signal, SH_IGNORE);

END_METHOD

BEGIN_METHOD_VOID(Signal_Catch)

	handle_signal(_signal, SH_CATCH);

END_METHOD

BEGIN_METHOD(Signal_get, GB_INTEGER num)

	int num = VARG(num);

	if (num < -1 || num >= NUM_SIGNALS)
	{
		GB.Error("Bad signal number");
		return;
	}
	
	_signal = num;
	RETURN_SELF();

END_METHOD

BEGIN_METHOD_VOID(Signal_exit)

	exit_signal();

END_METHOD


GB_DESC CSignalHandlerDesc[] =
{
	GB_DECLARE_VIRTUAL(".SignalHandler"),
	
	GB_STATIC_METHOD("Reset", NULL, Signal_Reset, NULL),
	GB_STATIC_METHOD("Ignore", NULL, Signal_Ignore, NULL),
	GB_STATIC_METHOD("Catch", NULL, Signal_Catch, NULL),
	
	GB_END_DECLARE
};

GB_DESC CSignalDesc[] =
{
	GB_DECLARE_VIRTUAL("Signal"),

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

