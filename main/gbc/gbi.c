/***************************************************************************

  gbi.c

  component informer

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

#include <config.h>

#include "gb_limit.h"
#include "gb_common.h"
#include "gb_alloc.h"

#ifdef __GNU_LIBRARY__
//#define _GNU_SOURCE already defined before
#include <getopt.h>
#endif

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>

#include <sys/stat.h>
#include <sys/types.h>
#include <sys/time.h>
#include <dirent.h>

#include <dlfcn.h>

#ifdef OS_LINUX
	#define lt_dlinit() (0)
	#define lt_dlhandle void *
	#define lt_dlopenext(_path) dlopen(_path, RTLD_LAZY)
	#define lt_dlsym(_handle, _symbol) dlsym(_handle, _symbol)
	#define lt_dlclose(_handle) dlclose(_handle)
	#define lt_dlerror() dlerror()
#else
	#include <ltdl.h>
#endif

#include <ctype.h>

#include "gb_component.h"
#include "gb_file.h"
#include "gb_str.h"
#include "gb_arch.h"
#include "gb_common_swap.h"
#include "gambas.h"

#include "gb_arch_temp.h"


static char _root[MAX_PATH + 1] = { 0 };
static char _lib_path[MAX_PATH + 1];
static char _info_path[MAX_PATH + 1];
static char _buffer[MAX_PATH + 1];

static FILE *out_info;
static FILE *out_list;
static bool _verbose = FALSE;
static bool _format = FALSE;
static bool _nopreload = FALSE;

static void analyze(const char *comp, bool include);

#ifdef __GNU_LIBRARY__
static struct option LongOptions[] =
{
  { "version", 0, NULL, 'V' },
  { "verbose", 0, NULL, 'v' },
  { "format", 0, NULL, 'f' },
  { "help", 0, NULL, 'h' },
  { "root", 1, NULL, 'r' },
  { 0 }
};
#endif


static void print(const char *fmt, ...)
{
  va_list args;

  va_start(args, fmt);

  vfprintf(out_info, fmt, args);
  
  va_end(args);
}

static void warning(const char *fmt, ...)
{
  va_list args;

	fprintf(stderr, "gbi" GAMBAS_VERSION_STRING ": warning: ");
		
  va_start(args, fmt);

  vfprintf(stderr, fmt, args);
  
  putc('\n', stderr);
  
  va_end(args);
}

static void error(bool must_exit, const char *fmt, ...)
{
  va_list args;

	fprintf(stderr, "gbi" GAMBAS_VERSION_STRING ": ERROR: ");
		
  va_start(args, fmt);

  vfprintf(stderr, fmt, args);
  
  putc('\n', stderr);
  
  va_end(args);

  if (must_exit)
  	exit(1);
}

// static void error2(const char *msg, const char *msg2, bool must_exit)
// {
//   fprintf(stderr, "gbi" GAMBAS_VERSION_STRING ": ERROR: %s: %s%s%s\n", msg, msg2, (errno != 0) ? ": " : "", (errno != 0) ? strerror(errno) : "");
//   if (must_exit)
//   	exit(1);
// }


static void init(void)
{
  const char *path;

  /* chemin d'installation de Gambas */

  if (!_root[0])
  {
    path = FILE_find_gambas();
    strncpy(_root, FILE_get_dir(FILE_get_dir(path)), MAX_PATH);
  }

  #ifdef OS_64BITS
  strcpy(_lib_path, FILE_cat(_root, GAMBAS_LIB64_PATH, NULL));
  if (access(FILE_cat(_lib_path, "gb.component", NULL), F_OK))
  #endif
  	strcpy(_lib_path, FILE_cat(_root, GAMBAS_LIB_PATH, NULL));
  	
  strcpy(_info_path, FILE_cat(_root, "share/gambas" GAMBAS_VERSION_STRING "/info", NULL));

  if (lt_dlinit())
    error(TRUE, "Cannot initialize plug-in management: %s", lt_dlerror());

  /*if (putenv("LD_BIND_NOW=true"))
    error2("Cannot set LD_BIND_NOW", strerror(errno));

  if (putenv("KDE_MALLOC=0"))
    error2("Cannot set KDE_MALLOC", strerror(errno));*/
}


static void newline()
{
  print("\n");
}

#if 0
static bool print_type(const char *type)
{
  switch (*type)
  {
    case 'b': print("Boolean"); break;
    case 'i': print("Integer"); break;
    case 's': print("String"); break;
    case 'd': print("Date"); break;
    case 'f': print("Float"); break;
    case 'v': print("Variant"); break;
    case 'o': print("Object"); break;

    default:

      while (*type && *type != ';')
      {
        print("%c", *type);
        type++;
      }
      return TRUE;
  }

  return FALSE;
}
#endif

static void dump_value(const char *type, intptr_t value)
{
  char *p;

  switch(*type)
  {
    case 'i':
      print("%d", value);
      break;

    case 'f':
      print("%s", (char *)value);
      break;

    case 's':
      p = (char *)value;
      while (*p)
      {
        if (*p == '\n')
          print("\\n");
        else if (*p == '\t')
          print("\\t");
        else
          print("%c", *p);
        p++;
      }
      break;

    default:
      print("?");
      break;
  }
}

static void dump_symbol(GB_DESC *desc)
{
  const char *name;

  name = &desc->name[1];

  print("%s\n", name);
  print("%c\n", *desc->name);
  if (desc->val1)
    print("%s", desc->val1);
  newline();

  if (*desc->name == 'C')
    dump_value((char *)desc->val1, desc->val2);
  else if (desc->val3)
    print("%s", (char *)desc->val3);

  newline();
}


static GB_DESC *_sort_symbol;

static int sort_symbol(const int *a, const int *b)
{
  return strcmp(_sort_symbol[*a].name, _sort_symbol[*b].name);
}

static void analyze_class(GB_DESC *desc)
{
  const char *name = desc->name;
  char *parent = NULL;
  bool autocreate = FALSE;
  bool nocreate = FALSE;
  uintptr_t hook;
  int nsymbol;
  int *sort;
  GB_DESC *p;
  int i;

  if (out_list && name[0] != '.')
    fprintf(out_list, "%s\n", name);

  desc++;

  while (desc->name)
  {
    hook = (uintptr_t)desc->name;

    if (hook == (intptr_t)GB_INHERITS_ID)
      parent = (char *)desc->val1;
    else if ((hook == (intptr_t)GB_NOT_CREATABLE_ID))
      nocreate = TRUE;
    else if (hook == (intptr_t)GB_AUTO_CREATABLE_ID)
      autocreate = TRUE;
    else if (hook > 16)
      break;

    desc++;
  }

  p = desc;
  nsymbol = 0;
  while (p->name)
  {
    nsymbol++;
    p++;
  }

  if (_format)
  {
    print("CLASS %s\n", name);
    if (parent) print("INHERITS %s\n", parent);
    if (!nocreate) print("CREATABLE\n");
    if (autocreate) print("AUTOCREATABLE\n");
  }
  else
  {
    print("#%s\n", name);
    if (parent)
      print("%s", parent);
    newline();
    if (!nocreate)
      print("C");
    if (autocreate)
      print("A");
    newline();
  }

  ALLOC(&sort, sizeof(int) * nsymbol, "analyze_class");
  for (i = 0; i < nsymbol; i++)
    sort[i] = i;
  
  _sort_symbol = desc;
  qsort(sort, nsymbol, sizeof(int), (int (*)(const void *, const void *))sort_symbol);

  for (i = 0; i < nsymbol; i++)
    dump_symbol(&desc[sort[i]]);

  FREE(&sort, "analyze_class");
  
  if (_format)
    newline();
}


static GB_DESC **_sort_desc;

static int sort_desc(const int *a, const int *b)
{
  return strcmp(_sort_desc[*a]->name, _sort_desc[*b]->name);
}

static bool analyze_native_component(const char *path)
{
  lt_dlhandle lib;
  GB_DESC **desc;
  char **include;
  GB_DESC **p;
  bool ret = FALSE;
  int nclass;
  int *sort;
  int i;

  if (_verbose)
    fprintf(stderr, "Loading native component: %s\n", path);

  //lt_dlopen_flag = RTLD_LAZY; /* | RTLD_GLOBAL;*/

  lib = lt_dlopenext(path);
  if (!lib)
  {
    error(FALSE, "Cannot load shared library: %s", lt_dlerror());
    return TRUE;
	}

	include = lt_dlsym(lib, LIB_INCLUDE);
	if (include)
	{
		if (_verbose)
			fprintf(stderr, "Including %s\n", *include);
		analyze(*include, TRUE);
	}

  desc = lt_dlsym(lib, LIB_CLASS);

  if (desc)
  {
    p = desc;
    nclass = 0;
    while (*p)
    {
      nclass++;
      p++;
    }
  
    ALLOC(&sort, sizeof(int) * nclass, "analyze_native_component");
    for (i = 0; i < nclass; i++)
      sort[i] = i;
    
    _sort_desc = desc;
    qsort(sort, nclass, sizeof(int), (int (*)(const void *, const void *))sort_desc);
  
    for (i = 0; i < nclass; i++)
      analyze_class(desc[sort[i]]);
      
    FREE(&sort, "analyze_native_component");
  }
  else
  {
  	warning("cannot find '" LIB_CLASS "' symbol in shared library.");
    ret = TRUE;
	}

	// Do not close shared libraries,except on openbsd that seems to feel better with
	#ifdef OS_OPENBSD
	lt_dlclose(lib);
	#endif

  return ret;
}


static bool analyze_gambas_component(const char *path)
{
  ARCH *arch;
  ARCH_FIND find;

  if (_verbose)
    fprintf(stderr, "Loading gambas component: %s\n", path);

  arch = ARCH_open(path);

  if (ARCH_find(arch, ".info", 0, &find))
  {
  	warning(".info file not found in component archive.");
    return TRUE;
	}

  fwrite(&arch->addr[find.pos], 1, find.len, out_info);

  if (ARCH_find(arch, ".list", 0, &find))
  {
  	warning(".list file not found in component archive.");
    return TRUE;
	}

  fwrite(&arch->addr[find.pos], 1, find.len, out_list);

  ARCH_close(arch);
  return FALSE;
}


static void preload(char **argv, char *lib)
{
#if DO_PRELOADING
  char buf[256];

  if (_nopreload || getenv("GB_PRELOAD") || !lib || !*lib)
    return;

  snprintf(buf, sizeof(buf), "LD_PRELOAD=%s", lib);
  putenv(buf);
  putenv("GB_PRELOAD=1");

  execv(argv[0], argv);
#endif
}


static void analyze(const char *comp, bool include)
{
  bool native, gambas;
  char *name;
  char *path_list = NULL;
  char *path_info = NULL;
  bool ok;

  name = STR_copy(comp);

	snprintf(_buffer, sizeof(_buffer), LIB_PATTERN, _lib_path, name);
	native = (access(_buffer, F_OK) == 0);
		
 	snprintf(_buffer, sizeof(_buffer), ARCH_PATTERN, _lib_path, name);
 	gambas = (access(_buffer, F_OK) == 0);

  if (!native && !gambas)
  {
    warning("component %s not found", name);
    STR_free(name);
    return;
  }

  if (!include)
  {
		path_info = STR_cat(FILE_cat(_info_path, name, NULL), ".info", NULL);
		path_list = STR_cat(FILE_cat(_info_path, name, NULL), ".list", NULL);
	
		out_info = fopen(path_info, "w");
		if (!out_info)
			error(TRUE, "Cannot write file: %s", path_info);
	
		out_list = fopen(path_list, "w");
		if (!out_list)
			error(TRUE, "Cannot write file: %s", path_list);
	}
	
  fflush(stdout);
  ok = TRUE;

  if (native)
  {
    snprintf(_buffer, sizeof(_buffer), LIB_PATTERN, _lib_path, name);

    if (analyze_native_component(_buffer))
      ok = FALSE;
  }

  if (gambas)
  {
    snprintf(_buffer, sizeof(_buffer), ARCH_PATTERN, _lib_path, name);

    if (analyze_gambas_component(_buffer))
    	if (!native)
      	ok = FALSE;
  }

	if (!include)
	{
		fclose(out_info);
		fclose(out_list);
	
		if (!ok)
		{
			unlink(path_info);
			unlink(path_list);
		}

		STR_free(path_info);
		STR_free(path_list);
	}
	
  STR_free(name);
}


int main(int argc, char **argv)
{
  DIR *dir;
  struct dirent *dirent;
  const char *name;
  int opt;
  int save_fd;
  #ifdef __GNU_LIBRARY__
  int ind = 0;
  #endif

  /*#ifdef __FreeBSD__
  optind = 1;
  #else
  optind = 0;
  #endif*/

  for(;;)
  {
    #ifdef __GNU_LIBRARY__
      opt = getopt_long(argc, argv, "vVhafpr:", LongOptions, &ind);
    #else
      opt = getopt(argc, argv, "vVhafpr:");
    #endif
    if (opt < 0) break;

    switch (opt)
    {
      case 'V':
        printf(VERSION "\n");
        exit(0);

      case 'v':
        _verbose = TRUE;
        break;

      case 'p':
        _nopreload = TRUE;
        break;

      case 'r':
        strncpy(_root, optarg, MAX_PATH);
        break;

      case 'h':
        printf(
          "\n"
          "GAMBAS Component Informer version " VERSION " " __DATE__ " " __TIME__ "\n"
          COPYRIGHT
          "Usage: gbi" GAMBAS_VERSION_STRING " [options] [components]\n"
          "Options:"
          #ifdef __GNU_LIBRARY__
          "\n"
          "  -V  --version              display version\n"
          "  -h  --help                 display this help\n"
          "  -p                         disable preloading\n"
          "  -r  --root <directory>     gives the gambas installation directory\n"
          #else
          " (no long options on this system)\n"
          "  -V                         display version\n"
          "  -h                         display this help\n"
          "  -p                         disable preloading\n"
          "  -r <directory>             gives the gambas installation directory\n"
          #endif
          "\n"
          );

        exit(0);
    }
  }

  init();

  if (optind == argc
  #ifdef OS_SOLARIS  /* solaris bug ? */
      || optind == 0
  #endif
      )
  {
    preload(argv,
      "libqt-mt.so.3 "
      "libkdecore.so.4 "
      );

    if (_verbose)
    {
      fprintf(stderr, "component path: %s\n", _lib_path);
      fprintf(stderr, "info path: %s\n", _info_path);
    }

    dir = opendir(_lib_path);
    if (dir == NULL)
      error(TRUE, "Cannot read directory: %s", _lib_path);

    save_fd = dup(STDOUT_FILENO);

    while ((dirent = readdir(dir)) != NULL)
    {
      name = dirent->d_name;
      if (strcmp(FILE_get_ext(name), "component"))
        continue;
      name = FILE_get_basename(name);
      puts(name);
      analyze(name, FALSE);
    }

    closedir(dir);
  }
  else
  {
  #ifdef __GNU_LIBRARY__
 	if (!getenv("GB_PRELOAD"))
  	{
			for (ind = optind; ind < argc; ind++)
			{
				name = argv[ind];			
				if (strncmp(name, "gb.qt.kde", 9) == 0)
					preload(argv, "libqt-mt.so.3 libkdecore.so.4");
				else if (strncmp(name, "gb.qt", 5) == 0)
					preload(argv, "libqt-mt.so.3");
			}
		}
  
  	for (ind = optind; ind < argc; ind++)
  	{
  		name = argv[ind];
			puts(name);
			analyze(name, FALSE);
		}
  #endif
  }

  exit(0);
}

