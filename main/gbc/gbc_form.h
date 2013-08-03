/***************************************************************************

  gbc_form.h

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

#ifndef __GBC_FORM_H
#define __GBC_FORM_H

enum {
	FORM_NORMAL = 0,
	FORM_WEBPAGE = 1
};

typedef
	struct {
		char *ext;
		int type;
	}
	FORM_FAMILY;

typedef
  struct {
    char *name;
    int len;
    }
  FORM_PARENT;

#define FORM_FIRST_LINE 100000
#define FORM_FIRST_LINE_STRING "100000"
	
void FORM_do(char *source, bool ctrl_public);
char *FORM_get_file_family(const char *file, const FORM_FAMILY **family);
void FORM_print_len(const char *buffer, int len);
void FORM_print(const char *buffer);
void FORM_print_char(char c);

void FORM_webpage(char *source);

#endif
