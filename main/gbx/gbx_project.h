/***************************************************************************

  gbx_project.h

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

#ifndef __GBX_PROJECT_H
#define __GBX_PROJECT_H

#include "gbx_class.h"

typedef
  struct {
    char *command;
    void (*func)();
    }
  PROJECT_COMMAND;

typedef
	void (*PROJECT_COMPONENT_CALLBACK)(char *);
	
#ifndef __GBX_PROJECT_C
EXTERN char *PROJECT_path;
EXTERN char *PROJECT_exec_path;
EXTERN char *PROJECT_name;
EXTERN char *PROJECT_title;
EXTERN char *PROJECT_version;
EXTERN const char *PROJECT_startup;
EXTERN CLASS *PROJECT_class;
EXTERN int PROJECT_argc;
EXTERN char **PROJECT_argv;
EXTERN char *PROJECT_oldcwd;
EXTERN char *PROJECT_user_home;
EXTERN bool PROJECT_run_httpd;
#endif

void PROJECT_init(const char *file);
void PROJECT_load(void);
void PROJECT_load_finish(void);
void PROJECT_exit(void);
char *PROJECT_get_home(void);
void PROJECT_analyze_startup(char *addr, int len, PROJECT_COMPONENT_CALLBACK cb);

#endif
