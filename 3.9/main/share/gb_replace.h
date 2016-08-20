/***************************************************************************

  gb_replace.h

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

#ifndef __GB_REPLACE_H
#define __GB_REPLACE_H

#include <config.h>
#include <gb_common.h>
#include <termios.h>

#ifndef HAVE_SETENV
int setenv(const char *name, const char *value, int overwrite);
#endif

#ifndef HAVE_UNSETENV
void unsetenv(const char *name);
#endif

#ifndef HAVE_GETDOMAINNAME
int getdomainname(char *name, size_t len);
#endif

#ifndef HAVE_GETPT
int getpt(void);
#endif

#ifndef HAVE_CFMAKERAW
void cfmakeraw(struct termios *termios_p);
#endif

#endif
