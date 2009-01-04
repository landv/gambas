/***************************************************************************

  compile.h

  Compiler initialization, reserved keywords table and subroutines table.

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

#ifndef _COMPILE_H
#define _COMPILE_H

#include <sys/stat.h>

#include "gb_array.h"
#include "gb_buffer.h"
#include "gb_table.h"
#include "gb_alloc.h"

#include "gb_limit.h"
#include "gb_reserved.h"
#include "gbc_read.h"

#include "gbc_class.h"

typedef
  struct {
    char *name;                        /* source file name */
    int line;                          /* current line number */
    char *source;                      /* source file contents */
    unsigned verbose : 1;              /* verbose compilation */
    unsigned debug : 1;                /* if debugging information must be generated */
    unsigned trans : 1;                /* if translation files must be generated */
    unsigned is_module : 1;            /* if the source file is a module */
    unsigned is_form : 1;              /* if the source is a class form */
    unsigned declared : 1;             /* ? */
    unsigned nobreak : 1;              /* no breakpoint */
    unsigned exported : 1;             /* there are some exported class */
    unsigned all : 1;                  /* compile everything */
    unsigned swap : 1;                 /* endianness must be swapped */
    unsigned public_module : 1;        /* modules symbols are by default */
    unsigned _reserved : 22;           /* reserved*/
    char *output;                      /* output file */
    PATTERN *pattern;                  /* lexical analyze */
    int pattern_count;                 /* number of patterns */
    PATTERN *current;                  /* position de traduction courante */
    PATTERN *end;                      /* fin de traduction */
    FUNCTION *func;                    /* fonction en cours de compilation */
    CLASS *class;                      /* classe en cours de compilation */
    char *form;                        /* nom du fichier formulaire */
    char *tname;                       /* nom du fichier *.pot */
    int default_library;               /* default library name for extern declarations */
    }
  PACKED
  COMPILE;

#ifndef __GBC_COMPILE_C

EXTERN COMPILE COMP_current;
EXTERN char *COMP_root;
EXTERN char *COMP_project;
EXTERN char *COMP_info_path;
EXTERN char *COMP_classes;

#endif

#define JOB (&COMP_current)

void COMPILE_init(void);
void COMPILE_load(void);
void COMPILE_exit(void);
void COMPILE_begin(const char *file, bool trans);
void COMPILE_end(void);
void COMPILE_export_class(char *name);

#endif
