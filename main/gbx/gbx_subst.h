/***************************************************************************

  subst.h

  string substitution routines

  (c) 2000-2007 Benoit Minisini <gambas@users.sourceforge.net>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 1, or (at your option)
  any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

***************************************************************************/

#ifndef __GBX_SUBST_H
#define __GBX_SUBST_H

typedef
  void (*SUBST_FUNC)(int, char **, long *);

PUBLIC void SUBST_init(void);
PUBLIC void SUBST_add(const char *src, long len);
PUBLIC void SUBST_add_char(unsigned char c);
PUBLIC void SUBST_exit(void);
PUBLIC char *SUBST_buffer(void);


#endif
