/***************************************************************************

  gbx_signal.c

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

#define __GBX_SIGNAL_C

#include "gb_error.h"
#include "gb_array.h"
#include "gbx_api.h"
#include "gbx_signal.h"

static SIGNAL_HANDLER *_handlers = NULL;
static int _pipe[2];
static int _count = 0;

void SIGNAL_install(SIGNAL_HANDLER *handler, int signum, void (*callback)(int, siginfo_t *, void *))
{
	struct sigaction action;
	
	fprintf(stderr, "SIGNAL_install: %d %p\n", signum, callback);

	handler->signum = signum;
	
	action.sa_flags = SA_SIGINFO;
	sigemptyset(&action.sa_mask);
	action.sa_sigaction = callback;

	if (sigaction(signum, NULL, &handler->old_action) != 0 || sigaction(signum, &action, NULL) != 0)
		ERROR_panic("Cannot install signal handler: %s", strerror(errno));
}

void SIGNAL_uninstall(SIGNAL_HANDLER *handler, int signum)
{
	fprintf(stderr, "SIGNAL_uninstall: %d\n", signum);

	while (handler->callbacks)
		SIGNAL_unregister(handler->signum, handler->callbacks);
	
	if (sigaction(signum, &handler->old_action, NULL) != 0)
		ERROR_panic("Cannot uninstall signal handler");
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
			
	buffer = signum;
	for(;;)
	{
		if (write(_pipe[1], &buffer, 1) == 1)
			break;
		
		if (errno != EINTR)
			ERROR_panic("Cannot write into signal pipe: %s", strerror(errno));
	}
	
	SIGNAL_previous(find_handler(signum), signum, info, context);
	
	errno = save_errno;
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
	cb = handler->callbacks;
	while (cb)
	{
		(*cb->callback)((int)signum, cb->data);
		cb = cb->next;
	}
}

SIGNAL_CALLBACK *SIGNAL_register(int signum, void (*callback)(int, intptr_t), intptr_t data)
{
	SIGNAL_HANDLER *handler = find_handler(signum);
	SIGNAL_CALLBACK *cb;
	
	if (!_count)
	{
		if (pipe(_pipe) != 0)
			ERROR_panic("Cannot create signal handler pipes: %s", strerror(errno));

		fcntl(_pipe[0], F_SETFD, FD_CLOEXEC);
		fcntl(_pipe[1], F_SETFD, FD_CLOEXEC);

		GB_Watch(_pipe[0], GB_WATCH_READ, (void *)SIGNAL_raise_callbacks, 0);
	}
	
	_count++;
	
	if (!handler)
	{
		handler = add_handler();
		SIGNAL_install(handler, signum, handle_signal);
	}
	
	ALLOC(&cb, sizeof(SIGNAL_CALLBACK), "SIGNAL_register_callback");
	
	cb->prev = NULL;
	cb->next = handler->callbacks;
	handler->callbacks = cb;
	
	cb->callback = callback;
	cb->data = data;

	fprintf(stderr, "SIGNAL_register: %d -> %p\n", signum, cb);
	
	return cb;
}

void SIGNAL_unregister(int signum, SIGNAL_CALLBACK *cb)
{
	SIGNAL_HANDLER *handler = find_handler(signum);
	
	if (!handler)
		return;
	
	if (cb->prev)
		cb->prev->next = cb->next;
	
	if (cb->next)
		cb->next->prev = cb->prev;
	
	if (cb == handler->callbacks)
		handler->callbacks = cb->next;
	
	fprintf(stderr, "SIGNAL_unregister: %d %p\n", signum, cb);
	
	FREE(&cb, "SIGNAL_unregister_callback");
	
	_count--;
	
	if (_count == 0)
	{
		GB_Watch(_pipe[0], GB_WATCH_NONE, NULL, 0);
		close(_pipe[0]);
		close(_pipe[1]);
	}
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
