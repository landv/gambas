/***************************************************************************

  project.h

  Project loader

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

#ifndef __GBX_PROJECT_H
#define __GBX_PROJECT_H

#include "gbx_class.h"

typedef
  struct {
    char *command;
    void (*func)();
    }
  PROJECT_COMMAND;

#ifndef __GBX_PROJECT_C
EXTERN char *PROJECT_path;
EXTERN char *PROJECT_exec_path;
EXTERN char *PROJECT_name;
EXTERN char *PROJECT_title;
EXTERN char *PROJECT_version;
EXTERN char *PROJECT_startup;
EXTERN CLASS *PROJECT_class;
EXTERN int PROJECT_argc;
EXTERN char **PROJECT_argv;
EXTERN char *PROJECT_oldcwd;
EXTERN char *PROJECT_user_home;
#endif

PUBLIC void PROJECT_init(const char *file);
PUBLIC void PROJECT_load(void);
PUBLIC void PROJECT_exit(void);

#endif
