/***************************************************************************

  component.c

  GAMBAS component management routines

  (c) 2000-2005 Beno� Minisini <gambas@users.sourceforge.net>

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

#define __GBX_COMPONENT_C

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "gb_common.h"
#include "gb_common_buffer.h"
#include "gb_common_case.h"
#include "gb_error.h"
#include "gb_alloc.h"
#include "gbx_list.h"
#include "gb_replace.h"
#include "gb_file.h"
#include "gbx_debug.h"

#include <fcntl.h>
#include <stdlib.h>
#include <stdarg.h>
#include <time.h>
#include <ctype.h>

#include "gb_error.h"

#include "gbx_class.h"
#include "gbx_exec.h"

#include "gbx_local.h"
#include "gbx_archive.h"
#include "gbx_library.h"
#include "gbx_project.h"

#include "gbx_component.h"


/*#define DEBUG*/
/*#define DEBUG_PRELOAD*/

PUBLIC COMPONENT *COMPONENT_current = NULL;
PUBLIC COMPONENT *COMPONENT_main;
PUBLIC int COMPONENT_count = 0;
PUBLIC char *COMPONENT_path;
PUBLIC char *COMPONENT_user_path;

static COMPONENT *_component_list = NULL;


PUBLIC void COMPONENT_init(void)
{
  LIBRARY_init();
  ARCHIVE_init();
}


PUBLIC void COMPONENT_exit(void)
{
  COMPONENT *comp;

  LIST_for_each(comp, _component_list)
    COMPONENT_unload(comp);

  while (_component_list)
    COMPONENT_delete(_component_list);

  LIBRARY_exit();
  ARCHIVE_exit();

  STRING_free(&COMPONENT_path);
  STRING_free(&COMPONENT_user_path);
}



PUBLIC void COMPONENT_load_all(void)
{
  COMPONENT *comp;

  if (EXEC_debug)
  {
  	COMPONENT_create("gb.debug");
    COMPONENT_create("gb.eval");
	}

  LIST_for_each(comp, _component_list)
  {
    comp->preload = TRUE;
    COMPONENT_load(comp);
  }
}


PUBLIC COMPONENT *COMPONENT_find(const char *name)
{
  COMPONENT *comp;

	/* A null name is the main archive */
	if (!name)
		return NULL;

  LIST_for_each(comp, _component_list)
  {
    if (strcmp(comp->name, name) == 0)
      return comp;
  }

  return NULL;
}


PUBLIC COMPONENT *COMPONENT_create(const char *name)
{
  COMPONENT *comp;
  char *path;
  bool can_archive;

  comp = COMPONENT_find(name);
  if (comp)
    return comp;

  ALLOC_ZERO(&comp, sizeof(COMPONENT), "COMPONENT_create");

  comp->class = CLASS_Component;
  comp->ref = 1;

  STRING_new(&comp->name, name, 0);

	// Don't load the archive if it has the same name as the project

	if (PROJECT_name)
		can_archive = strcmp(name, PROJECT_name);
	else
		can_archive = TRUE;

	// System wide component, located in /usr/lib/gambas2 (by default)

  path = FILE_buffer();
  sprintf(path, LIB_PATTERN, COMPONENT_path, name);

  if (FILE_exist(path))
    comp->library = LIBRARY_create(name);

	if (can_archive)
	{
		path = FILE_buffer();
		sprintf(path, ARCH_PATTERN, COMPONENT_path, name);

		if (FILE_exist(path))
			comp->archive = ARCHIVE_create(name);
	}

  if (comp->library || comp->archive)
  	goto __OK;

	// User specific component, located in ~/.gambas/lib/gambas2

  path = FILE_buffer();
  sprintf(path, LIB_PATTERN, COMPONENT_user_path, name);

  if (FILE_exist(path))
    comp->library = LIBRARY_create(name);

	if (can_archive)
	{
		path = FILE_buffer();
		sprintf(path, ARCH_PATTERN, COMPONENT_user_path, name);

		if (FILE_exist(path))
			comp->archive = ARCHIVE_create(name);
	}

	comp->user = TRUE;

  if (comp->library || comp->archive || !can_archive)
  	goto __OK;

	THROW(E_LIBRARY, comp->name, "cannot find library file");

__OK:

  LIST_insert((void **)&_component_list, comp, &comp->list);
  COMPONENT_count++;

  return comp;
}


PUBLIC void COMPONENT_delete(COMPONENT *comp)
{
  COMPONENT_unload(comp);
  LIST_remove((void **)&_component_list, comp, &comp->list);
  COMPONENT_count--;

  if (comp->library)
    LIBRARY_delete(comp->library);

  if (comp->archive)
    ARCHIVE_delete(comp->archive);

  STRING_free(&comp->name);

  FREE(&comp, "COMPONENT_delete");
}


PUBLIC void COMPONENT_load(COMPONENT *comp)
{
  COMPONENT *current;

  if (comp->loaded)
    return;

  #if DEBUG_COMP
    fprintf(stderr, "Loading component %s\n", comp->name);
  #endif

  current = COMPONENT_current;
  COMPONENT_current = comp;

  if (comp->library)
    LIBRARY_load(comp->library);
  if (comp->archive)
    ARCHIVE_load(comp->archive);

  comp->loaded = TRUE;
  COMPONENT_current = current;
}


PUBLIC void COMPONENT_unload(COMPONENT *comp)
{
  if (comp->library)
    LIBRARY_unload(comp->library);

  /* Do not exist yet */
  //if (comp->archive)
  //  ARCHIVE_unload(comp->archive);
}


PUBLIC COMPONENT *COMPONENT_next(COMPONENT *comp)
{
  if (comp)
    return (COMPONENT *)(comp->list.next);
  else
    return _component_list;
}


PUBLIC void COMPONENT_translation_must_be_reloaded(void)
{
  COMPONENT *comp;

  LIST_for_each(comp, _component_list)
  {
    if (comp->archive)
      comp->archive->translation_loaded = FALSE;
  }
  
  if (ARCHIVE_main)
  	ARCHIVE_main->translation_loaded = FALSE;
}


PUBLIC void COMPONENT_signal(int signal, void *param)
{
  COMPONENT *comp;

  LIST_for_each(comp, _component_list)
  {
    if (comp->library && comp->library->signal)
      (*comp->library->signal)(signal, param);
  }
}
