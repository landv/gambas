/***************************************************************************

	gbx_component.c

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

#define __GBX_COMPONENT_C

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "gb_common.h"
#include "gb_common_buffer.h"
#include "gb_common_case.h"
#include "gb_error.h"
#include "gb_alloc.h"
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


//#define DEBUG_COMP
/*#define DEBUG_PRELOAD*/

COMPONENT *COMPONENT_current = NULL;
COMPONENT *COMPONENT_main;
int COMPONENT_count = 0;
char *COMPONENT_path;

static COMPONENT *_component_list = NULL;
static COMPONENT *_component_load = NULL;

static bool _load_all = FALSE;

void COMPONENT_init(void)
{
	LIBRARY_init();
	ARCHIVE_init();
}


void COMPONENT_exit(void)
{
	COMPONENT *comp;
	int order;
	int max_order = 0;

	_component_load = NULL;
	
	LIST_for_each(comp, _component_list)
	{
		if (comp->order > max_order)
			max_order = comp->order;
	}
	
	// if order < 0, the component is not unloaded
	
	for (order = 0; order <= max_order; order++)
	{
		LIST_for_each(comp, _component_list)
		{
			if (comp->order == order)
				COMPONENT_unload(comp);
		}
	}

	/*LIST_for_each(comp, _component_list)
	{
		if (comp->loaded)
			COMPONENT_unload(comp);
	}*/

	while (_component_list)
		COMPONENT_delete(_component_list);

	LIBRARY_exit();
	ARCHIVE_exit();

	STRING_free(&COMPONENT_path);
}



void COMPONENT_load_all(void)
{
	COMPONENT *comp;

	if (EXEC_debug)
	{
		COMPONENT_create("gb.eval");
		COMPONENT_create("gb.debug");
	}

	_load_all = TRUE;
	
	LIST_for_each(comp, _component_list)
	{
		COMPONENT_load(comp);
	}
	
	_load_all = FALSE;
}

void COMPONENT_load_all_finish(void)
{
	COMPONENT *comp;

	LIST_for_each_name(comp, _component_load, load)
	{
		ARCHIVE_load_exported_class(comp->archive, AR_FIND_ONLY);
	}
	LIST_for_each_name(comp, _component_load, load)
	{
		ARCHIVE_load_exported_class(comp->archive, AR_LOAD_ONLY);
	}
}


COMPONENT *COMPONENT_find(const char *name)
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

bool COMPONENT_exist(const char *name)
{
	return COMPONENT_find(name) != NULL;
}

COMPONENT *COMPONENT_create(const char *name)
{
	COMPONENT *comp;
	char *path = NULL;
	bool can_archive;
	bool library = FALSE;
	bool same_name_as_project = FALSE;

	if (*name == '/') // user library
	{
		library = TRUE;
		path = (char *)name;
		name = FILE_get_name(name);
	}
	
	comp = COMPONENT_find(name);
	if (comp)
		return comp;

	ALLOC_ZERO(&comp, sizeof(COMPONENT));

	comp->class = CLASS_Component;
	comp->ref = 1;

	comp->name = STRING_new_zero(name);

	if (library)
	{
		comp->archive = ARCHIVE_create(name, path);
		comp->user = TRUE;
	}
	else
	{
		// Don't load the archive if it has the same name as the project
		
		if (PROJECT_name)
			same_name_as_project = strcmp(name, PROJECT_name) == 0;
		
		can_archive = !same_name_as_project;

		// System wide component

		path = FILE_buffer();
		sprintf(path, LIB_PATTERN, COMPONENT_path, name);
		//fprintf(stderr, "COMPONENT_create: %s\n", path);

		if (FILE_exist(path))
			comp->library = LIBRARY_create(name);

		if (can_archive)
		{
			path = FILE_buffer();
			sprintf(path, ARCH_PATTERN, COMPONENT_path, name);

			if (FILE_exist(path))
				comp->archive = ARCHIVE_create(name, NULL);
		}
	}

	//fprintf(stderr, "insert %s\n", comp->name);
	LIST_insert(&_component_list, comp, &comp->list);
	COMPONENT_count++;

	if (!comp->library && !comp->archive && !same_name_as_project)
	{
		COMPONENT_delete(comp);
		THROW(E_LIBRARY, name, "cannot find component");
	}

	return comp;
}


void COMPONENT_delete(COMPONENT *comp)
{
	COMPONENT_unload(comp);
	LIST_remove(&_component_list, comp, &comp->list);
	COMPONENT_count--;

	if (comp->library)
		LIBRARY_delete(comp->library);

	if (comp->archive)
		ARCHIVE_delete(comp->archive);

	STRING_free(&comp->name);

	FREE(&comp);
}


void COMPONENT_load(COMPONENT *comp)
{
	COMPONENT *current;

	if (comp->loaded || comp->loading)
		return;

	#if DEBUG_COMP
		fprintf(stderr, "Loading component %s\n", comp->name);
	#endif
		
	comp->loading = TRUE;

	current = COMPONENT_current;
	COMPONENT_current = comp;

	if (comp->library)
	{
		comp->order = LIBRARY_load(comp->library);
		comp->library->persistent = _load_all;
	}
	
	if (comp->archive)
	{
		if (_load_all)
		{
			//fprintf(stderr, "load later: %s\n", comp->name);
			LIST_insert(&_component_load, comp, &comp->load);
		}
		
		ARCHIVE_load(comp->archive, !_load_all);
	}

	comp->loading = FALSE;
	comp->loaded = TRUE;
	COMPONENT_current = current;
}


void COMPONENT_unload(COMPONENT *comp)
{
	if (!comp->loaded)
		return;

	#if DEBUG_COMP
		fprintf(stderr, "Unloading component %s [%d]\n", comp->name, comp->order);
	#endif

	if (comp->library)
		LIBRARY_unload(comp->library);

	/* Do not exist yet */
	//if (comp->archive)
	//  ARCHIVE_unload(comp->archive);

	comp->loaded = FALSE;
}


COMPONENT *COMPONENT_next(COMPONENT *comp)
{
	if (comp)
		return (COMPONENT *)(comp->list.next);
	else
		return _component_list;
}


void COMPONENT_translation_must_be_reloaded(void)
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


void COMPONENT_signal(int signal, void *param)
{
	COMPONENT *comp;

	LIST_for_each(comp, _component_list)
	{
		if (comp->library && comp->library->signal)
			(*comp->library->signal)(signal, param);
	}
}

bool COMPONENT_get_info(const char *key, void **value)
{
	COMPONENT *comp;
	
	LIST_for_each(comp, _component_list)
	{
		if (comp->library && comp->library->info)
			if ((*comp->library->info)(key, value))
				return FALSE;
	}
	
	return TRUE;
}

void COMPONENT_exec(const char *name, int argc, char **argv)
{
	COMPONENT *comp;
	
	comp = COMPONENT_create(name);
	
	COMPONENT_load(comp);
	
	if (comp->library)
		LIBRARY_exec(comp->library, argc, argv);
}

