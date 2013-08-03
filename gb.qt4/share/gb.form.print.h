/***************************************************************************

  gb.form.print.h

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

#ifndef __GB_FORM_PRINT_H
#define __GB_FORM_PRINT_H

enum {
	GB_PRINT_PORTRAIT,
  GB_PRINT_LANDSCAPE
  //GB_PRINT_REVERSE_PORTRAIT,
  //GB_PRINT_REVERSE_LANDSCAPE
};

enum {
	GB_PRINT_CUSTOM,
	GB_PRINT_A3,
	GB_PRINT_A4,
	GB_PRINT_A5,
	GB_PRINT_B5,
	GB_PRINT_LETTER,
	GB_PRINT_EXECUTIVE,
	GB_PRINT_LEGAL
};

enum {
	GB_PRINT_SIMPLEX,
	GB_PRINT_DUPLEX_HORIZONTAL,
	GB_PRINT_DUPLEX_VERTICAL
};



#endif


