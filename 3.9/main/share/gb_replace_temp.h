/***************************************************************************

	gb_replace_temp.h

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

#include "gb_replace.h"

#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <termios.h>

#ifndef HAVE_SETENV

int setenv(const char *name, const char *value, int overwrite)
{
	char *env;
	int env_size; 

	if (!name || *name == 0)
		return (-1);

	if (overwrite == 0)
	{
		if (getenv(name))
			return 0;
	}

	env_size = strlen(name) + strlen(value) + 2;
	env = malloc(env_size);
	if (!env)
		return (-1);

	strcpy(env, name);
	strcat(env, "=");
	strcat(env, value);
	putenv(env);

	return 0;
}

#endif

#ifndef HAVE_UNSETENV

extern char **environ;

void unsetenv(const char *name)
{
	size_t len;
	char **ep;

	if (name == NULL || *name == '\0' || strchr (name, '=') != NULL)
		return;

	len = strlen(name);

	ep = environ;
	while (*ep != NULL)
	{
		if (!strncmp (*ep, name, len) && (*ep)[len] == '=')
		{
			char **dp = ep;

			do
				dp[0] = dp[1];
			while (*dp++);
		}
		else
			++ep;
	}
}

#endif

#ifndef HAVE_GETDOMAINNAME

int getdomainname(char *name, size_t len)
{
#if defined(HAVE_SYSINFO) && defined(SI_SRPC_DOMAIN)
	sysinfo(SI_SRPC_DOMAIN, name, len);
#else
	*name = 0;
	return 0;
#endif
}

#endif

#ifndef HAVE_GETPT

int getpt(void)
{
#ifdef OS_FREEBSD
	return posix_openpt(O_RDWR | O_NOCTTY);
#else
	return -1;
#endif
}

#endif

#ifndef HAVE_CFMAKERAW

void cfmakeraw(struct termios *termios_p)
{
	termios_p->c_iflag &= ~(IGNBRK|BRKINT|PARMRK|ISTRIP|INLCR|IGNCR|ICRNL|IXON);
	termios_p->c_oflag &= ~OPOST;
	termios_p->c_lflag &= ~(ECHO|ECHONL|ICANON|ISIG|IEXTEN);
	termios_p->c_cflag &= ~(CSIZE|PARENB);
	termios_p->c_cflag |= CS8;
}

#endif
