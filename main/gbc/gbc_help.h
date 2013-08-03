/***************************************************************************

  gbc_help.h

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

#ifndef __GBC_HELP_H
#define __GBC_HELP_H

#include <stdio.h>

#ifndef __GBC_HELP_C
#endif

void HELP_add_at_current_line(const char *help);
void HELP_search_and_print(FILE *file, int line);
void HELP_search_and_print_for_class(FILE *file);

#define HELP_is_help_comment(_ptr) ((_ptr)[0] == '\'' && ((_ptr)[1] == ' ' || ((_ptr)[1] == '\'' && (_ptr)[2] == ' ')))
#define HELP_is_for_class(_help) ((_help)[0] == '\'' && (_help)[1] == '\'')
#define HELP_is_void_line(_help) ((_help)[0] == '\n' && (_help)[1] == 0)

#endif
