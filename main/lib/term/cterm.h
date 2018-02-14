/***************************************************************************

  cterm.h

  (c) 2000-2017 Benoît Minisini <g4mba5@gmail.com>

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

#ifndef __CTERM_H
#define __CTERM_H

#include "gambas.h"
#include <termios.h>
#include <unistd.h>

#ifndef __CTERM_C
extern GB_DESC TermDesc[];
extern GB_DESC StreamTermDesc[];
extern GB_DESC TerminalSettingsDesc[];
#endif

typedef
	struct {
		GB_BASE ob;
		struct termios settings;
	}
	CTERMINALSETTINGS;

#endif
