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
#include "gbx_signal.h"

void SIGNAL_install(SIGNAL_HANDLER *handler, int signum, void (*callback)(int, siginfo_t *, void *))
{
	struct sigaction action;
	
	action.sa_flags = SA_SIGINFO;
	sigemptyset(&action.sa_mask);
	action.sa_sigaction = callback;

	if (sigaction(signum, NULL, &handler->old_action) != 0 || sigaction(signum, &action, NULL) != 0)
		ERROR_panic("Cannot install signal handler: %s", strerror(errno));
	
	handler->signum = signum;
}

void SIGNAL_uninstall(SIGNAL_HANDLER *handler)
{
	if (sigaction(handler->signum, &handler->old_action, NULL) != 0)
		ERROR_panic("Cannot uninstall signal handler");
}

void SIGNAL_previous(SIGNAL_HANDLER *handler, int sig, siginfo_t *info, void *context)
{
	if (handler->old_action.sa_handler != SIG_DFL && handler->old_action.sa_handler != SIG_IGN)
	{
		if (handler->old_action.sa_flags & SA_SIGINFO)
		{
			//fprintf(stderr, "Calling old action %p\n", _old_SIGCHLD_action.sa_sigaction);
			(*handler->old_action.sa_sigaction)(sig, info, context);
		}
		else
		{
			//fprintf(stderr, "Calling old handler %p\n", _old_SIGCHLD_action.sa_handler);
			(*handler->old_action.sa_handler)(sig);
		}
	}
}

