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

#ifndef __GBX_SUBST_C
EXTERN char *SUBST_buffer;
EXTERN char SUBST_temp[];
EXTERN int SUBST_ntemp;
#endif

#define SUBST_TEMP_SIZE 256

typedef
  void (*SUBST_FUNC)(int, char **, int *);

typedef
  void (*SUBST_ADD_FUNC)(int);

void SUBST_init(int len, int inc);
void SUBST_init_ext(int len, int inc);
void SUBST_add(const char *src, int len);
void SUBST_exit(void);
void SUBST_dump_temp(void);

#define SUBST_add_char(_c) \
{ \
  if (SUBST_ntemp == SUBST_TEMP_SIZE) \
  	SUBST_dump_temp(); \
  SUBST_temp[SUBST_ntemp++]= (_c); \
}

#define SUBST_init() SUBST_init_ext(0, 0)
#define SUBST_init_max(_max) SUBST_init_ext((_max), 0)

#endif
