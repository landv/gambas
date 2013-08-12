/***************************************************************************

  gbx_signal.c

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

#define __GBX_SIGNAL_C

#include "gb_error.h"
#include "gb_array.h"
#include "gbx_api.h"
#include "gbx_signal.h"

//#define DEBUG_ME 1

static SIGNAL_HANDLER *_handlers = NULL;
static int _pipe[2] = { -1, -1 };
static volatile int _count = 0;
static int _raising_callback = 0;

void SIGNAL_install(SIGNAL_HANDLER *handler, int signum, void (*callback)(int, siginfo_t *, void *))
{
	struct sigaction action;
	
	#ifdef DEBUG_ME
	fprintf(stderr, "SIGNAL_install: %d %p\n", signum, callback);
	#endif
	
	handler->signum = signum;
	
	action.sa_flags = SA_SIGINFO;
	sigemptyset(&action.sa_mask);
	action.sa_sigaction = callback;

	if (sigaction(signum, NULL, &handler->old_action) != 0 || sigaction(signum, &action, NULL) != 0)
		ERROR_panic("Cannot install signal handler: %s", strerror(errno));
}

void SIGNAL_uninstall(SIGNAL_HANDLER *handler, int signum)
{
	#ifdef DEBUG_ME
	fprintf(stderr, "SIGNAL_uninstall: %d\n", signum);
	#endif
	
	if (sigaction(signum, &handler->old_action, NULL) != 0)
		ERROR_panic("Cannot uninstall signal handler");

	while (handler->callbacks)
		SIGNAL_unregister(handler->signum, handler->callbacks);
}

void SIGNAL_previous(SIGNAL_HANDLER *handler, int signum, siginfo_t *info, void *context)
{
	if (handler->old_action.sa_handler != SIG_DFL && handler->old_action.sa_handler != SIG_IGN)
	{
		if (handler->old_action.sa_flags & SA_SIGINFO)
		{
			//fprintf(stderr, "Calling old action %p\n", _old_SIGCHLD_action.sa_sigaction);
			(*handler->old_action.sa_sigaction)(signum, info, context);
		}
		else
		{
			//fprintf(stderr, "Calling old handler %p\n", _old_SIGCHLD_action.sa_handler);
			(*handler->old_action.sa_handler)(signum);
		}
	}
}

static SIGNAL_HANDLER *find_handler(int signum)
{
	int i;
	
	for (i = 0; i < ARRAY_count(_handlers); i++)
	{
		if (_handlers[i].signum == signum)
			return &_handlers[i];
	}
	
	return NULL;
}

static SIGNAL_HANDLER *add_handler(void)
{
	if (!_handlers)
		ARRAY_create_inc(&_handlers, 1);
	
	return ARRAY_add_void(&_handlers);
}

static void handle_signal(int signum, siginfo_t *info, void *context)
{
	char buffer;
	int save_errno;

	save_errno = errno;
	
	if (_count)
	{
		buffer = signum;
		for(;;)
		{
			if (write(_pipe[1], &buffer, 1) == 1)
				break;
			
			if (errno != EINTR)
				ERROR_panic("Cannot write signal #%d into signal pipe: %s", signum, strerror(errno));
		}
	}
	
	SIGNAL_previous(find_handler(signum), signum, info, context);
	
	errno = save_errno;
}

static bool _must_purge_callbacks = FALSE;
static int _purge_signum;
static SIGNAL_HANDLER *_purge_handler;

static void purge_callbacks(void)
{
	SIGNAL_CALLBACK *cb, *next_cb;

	_raising_callback--;
	if (_raising_callback)
		return;
	
	#ifdef DEBUG_ME
	fprintf(stderr, ">> purge_callbacks\n");
	#endif

	while (_must_purge_callbacks)
	{
		_must_purge_callbacks = FALSE;

		cb = _purge_handler->callbacks;
		while (cb)
		{
			#ifdef DEBUG_ME
			fprintf(stderr, "purge_callbacks: cb = %p\n", cb);
			#endif
			next_cb = cb->next;

			if (!cb->callback)
				SIGNAL_unregister(_purge_signum, cb);
			
			cb = next_cb;
		}
	}

	#ifdef DEBUG_ME
	fprintf(stderr, "<< purge_callbacks\n");
	#endif
}

void SIGNAL_raise_callbacks(int fd, int type, void *data)
{
	SIGNAL_HANDLER *handler;
	SIGNAL_CALLBACK *cb;
	char signum;

	/*old = signal(SIGCHLD, signal_child);*/

	if (read(fd, &signum, 1) != 1)
		return;
	
	handler = find_handler(signum);
	if (!handler)
		return;
	
	#ifdef DEBUG_ME
	fprintf(stderr, ">> SIGNAL_raise_callbacks (%d)\n", _raising_callback);
	#endif

	_raising_callback++;
	_purge_signum = signum;
	_purge_handler = handler;
	
	ON_ERROR(purge_callbacks)
	{
		cb = handler->callbacks;
		while (cb)
		{
			#ifdef DEBUG_ME
			fprintf(stderr, "SIGNAL_raise_callbacks: cb = %p cb->callback = %p\n", cb, cb->callback);
			#endif
			if (cb->callback)
				(*cb->callback)((int)signum, cb->data);
			
			cb = cb->next;
		}
	}
	END_ERROR
	
	#ifdef DEBUG_ME
	fprintf(stderr, "SIGNAL_raise_callbacks: purge_callbacks\n");
	#endif
	purge_callbacks();

	#ifdef DEBUG_ME
	fprintf(stderr, "<< SIGNAL_raise_callbacks (%d)\n", _raising_callback);
	#endif
}

SIGNAL_CALLBACK *SIGNAL_register(int signum, void (*callback)(int, intptr_t), intptr_t data)
{
	SIGNAL_HANDLER *handler;
	SIGNAL_CALLBACK *cb;
	
	if (!_count)
	{
		if (pipe(_pipe) != 0)
			ERROR_panic("Cannot create signal handler pipes: %s", strerror(errno));

		fcntl(_pipe[0], F_SETFD, FD_CLOEXEC);
		fcntl(_pipe[1], F_SETFD, FD_CLOEXEC);
		// Allows to read the signal pipe without blocking
		fcntl(_pipe[0], F_SETFL, fcntl(_pipe[0], F_GETFL) | O_NONBLOCK);

		GB_Watch(_pipe[0], GB_WATCH_READ, (void *)SIGNAL_raise_callbacks, 0);
	}
	
	_count++;
	
	handler = find_handler(signum);
	if (!handler)
	{
		handler = add_handler();
		SIGNAL_install(handler, signum, handle_signal);
	}
	
	ALLOC(&cb, sizeof(SIGNAL_CALLBACK));
	
	cb->prev = NULL;
	cb->next = handler->callbacks;
	cb->callback = callback;
	cb->data = data;
	
	if (cb->next)
		cb->next->prev = cb;
	handler->callbacks = cb;

	#ifdef DEBUG_ME
	fprintf(stderr, "SIGNAL_register: %d -> %p (%p)\n", signum, cb, cb->callback);
	#endif
	
	#ifdef DEBUG_ME
	fprintf(stderr, "handler->callbacks %p:", handler);
	SIGNAL_CALLBACK *save = cb;
	cb = handler->callbacks;
	while (cb)
	{
		fprintf(stderr, " -> %p (%p)", cb, cb->callback);
		cb = cb->next;
	}
	fprintf(stderr, "\n");
	cb = save;
	#endif

	return cb;
}

void SIGNAL_unregister(int signum, SIGNAL_CALLBACK *cb)
{
	SIGNAL_HANDLER *handler = find_handler(signum);
	
	if (!handler)
		return;
	
	if (_raising_callback)
	{
		#ifdef DEBUG_ME
		fprintf(stderr, "SIGNAL_unregister: disable %d %p (%p)\n", signum, cb, cb->callback);
		#endif
		cb->callback = NULL;
		_must_purge_callbacks = TRUE;
		return;
	}
	
	#ifdef DEBUG_ME
	fprintf(stderr, "SIGNAL_unregister: remove %d %p (%p)\n", signum, cb, cb->callback);
	#endif
	
	if (cb->prev)
		cb->prev->next = cb->next;
	
	if (cb->next)
		cb->next->prev = cb->prev;
	
	if (cb == handler->callbacks)
		handler->callbacks = cb->next;
	
	IFREE(cb);
	
	_count--;
	
	if (_count == 0)
	{
		GB_Watch(_pipe[0], GB_WATCH_NONE, NULL, 0);
		close(_pipe[0]);
		close(_pipe[1]);
		_pipe[0] = -1;
		_pipe[1] = -1;
	}
	
	#ifdef DEBUG_ME
	fprintf(stderr, "handler->callbacks %p:", handler);
	cb = handler->callbacks;
	while (cb)
	{
		fprintf(stderr, " -> %p (%p)", cb, cb->callback);
		cb = cb->next;
	}
	fprintf(stderr, "\n");
	#endif
}

void SIGNAL_exit(void)
{
	int i;
	SIGNAL_HANDLER *handler;
	
	if (_handlers)
	{
		for (i = 0; i < ARRAY_count(_handlers); i++)
		{
			handler = &_handlers[i];
			SIGNAL_uninstall(handler, handler->signum);
		}
		
		ARRAY_delete(&_handlers);
	}
}

int SIGNAL_get_fd(void)
{
	return _pipe[0];
}
