/***************************************************************************

  gbi.c

  component informer

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
#include <ltdl.h>
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
}


static void error2(const char *msg, const char *msg2)
{
  fprintf(stderr, "gbi: %s: %s%s%s\n", msg, msg2, (errno != 0) ? ": " : "", (errno != 0) ? strerror(errno) : "");
  exit(1);
}

static void init(void)
{
  const char *path;

  /* chemin d'installation de Gambas */

  if (!_root[0])
  {
    path = FILE_find_gambas();
    strncpy(_root, FILE_get_dir(FILE_get_dir(path)), MAX_PATH);
  }

  strcpy(_lib_path, FILE_cat(_root, "lib/gambas" GAMBAS_VERSION_STRING, NULL));
  strcpy(_info_path, FILE_cat(_root, "share/gambas" GAMBAS_VERSION_STRING "/info", NULL));

  if (lt_dlinit())
    error2("Cannot initialize plug-in management", lt_dlerror());

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

static void dump_value(const char *type, long value)
{
  char *p;

  switch(*type)
  {
    case 'i':
      print("%ld", value, value);
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

#if 0
static void print_value(const char *type, long value)
{
  char *p;

  switch(*type)
  {
    case 'i':
      print("%ld ( &H%lX )", value, value);
      break;

    case 'f':
      print("\"%s\"", (char *)value);
      break;

    case 's':
      p = (char *)value;
      print("\"");
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
      print("\"");
      break;

    default:
      print("?");
      break;
  }
}

static void print_signature(const char *sign)
{
  char mode = 0;
  char c;
  bool comma = FALSE;

  if (!sign)
    return;

  while ((c = *sign))
  {
    if (!mode)
    {
      if (c == '(')
      {
        if (comma)
          print(" ,");
        print(_format ? " <i>" : " ");

        mode = ')';
      }
      else if (c == '<')
        mode = '>';
      else if (c == '[')
        print(_format ? " <b>[</b>" : " [");
      else if (c == ']')
        print(_format ? " <b>]</b>" : " ]");
      else if (c == '.')
        print(" ...");
      else
      {
        print(" AS ");
        if (print_type(sign))
          mode = ';';
        comma = TRUE;
      }
    }
    else if (c == mode)
    {
      if (mode == ')' && _format)
        print("</i>");
      mode = 0;
    }
    else if (mode == ')')
    {
      print("%c", c);
    }

    sign++;
  }
}

static void analyze_symbol(GB_DESC *desc)
{
  char type;
  char *name;

  type = *desc->name;

  name = &desc->name[1];

  print("%s\n", name);

  if (isupper(type))
  {
    if (type != 'C') print("STATIC ");
    type = tolower(type);
  }

  if (type == ':')
    name++;

  switch (type)
  {
    case 'c':
      print(_format ? "CONST <b>%s</b> AS " : "CONST %s AS ", name);
      print_type((char *)desc->val1);
      print(" = ");
      print_value((char *)desc->val1, desc->val2);
      newline();
      break;

    case 'p':
      print(_format ? "PROPERTY <b>%s</b> AS " : "PROPERTY %s AS ", name);
      print_type((char *)desc->val1);
      newline();
      break;

    case 'r':
      print(_format ? "PROPERTY READ <b>%s</b> AS " : "PROPERTY READ %s AS ", name);
      print_type((char *)desc->val1);
      newline();
      break;

    case 'm': case ':':
      if (type == ':')
        print(_format ? "EVENT <b>%s</b> (" : "EVENT %s (", name);
      else
        print(_format ? "%s <b>%s</b> (" : "%s %s (", desc->val1 ? "FUNCTION" : "SUB", name);
      print_signature((char *)desc->val3);
      print(" )");
      if (desc->val1)
      {
        print(" AS ");
        print_type((char *)desc->val1);
      }
      newline();
      break;
  }
}
#endif


static void dump_symbol(GB_DESC *desc)
{
  char *name;

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


static void analyze_class(GB_DESC *desc)
{
  char *name = desc->name;
  char *parent = NULL;
  bool nonew = FALSE;
  bool autocreate = FALSE;
  ulong hook;

  if (out_list && name[0] != '.')
    fprintf(out_list, "%s\n", name);

  desc++;

  while (desc->name)
  {
    hook = (ulong)desc->name;

    if (hook == (ulong)GB_INHERITS_ID)
      parent = (char *)desc->val1;
    else if ((hook == (ulong)GB_HOOK_NEW_ID) && desc->val1 == 0)
      nonew = TRUE;
    else if (hook == (ulong)GB_AUTO_CREATABLE_ID)
      autocreate = TRUE;
    else if (hook > 16)
      break;

    desc++;
  }

  if (_format)
  {
    print("CLASS %s\n", name);
    if (parent) print("INHERITS %s\n", parent);
    if (!nonew) print("CREATABLE\n");
    if (autocreate) print("AUTOCREATABLE\n");
  }
  else
  {
    print("#%s\n", name);
    if (parent)
      print("%s", parent);
    newline();
    if (!nonew)
      print("C");
    if (autocreate)
      print("A");
    newline();
  }

  while (desc->name)
  {
    /*if (_format)
      analyze_symbol(desc);
    else*/
    dump_symbol(desc);
    desc++;
  }

  if (_format)
    newline();
}


static bool analyze_native_component(const char *path)
{
  lt_dlhandle lib;
  GB_DESC **desc;
  bool ret = FALSE;

  if (_verbose)
    fprintf(stderr, "Loading native component: %s\n", path);

  //lt_dlopen_flag = RTLD_LAZY; /* | RTLD_GLOBAL;*/

  lib = lt_dlopenext(path);
  if (!lib)
    error2(path, lt_dlerror());

  desc = lt_dlsym(lib, LIB_CLASS);

  if (desc)
  {
    while (*desc)
    {
      analyze_class(*desc);
      desc++;
    }
  }
  else
  {
  	fprintf(stderr, "warning: cannot find '" LIB_CLASS "' symbol in shared library.\n");
    ret = TRUE;
	}

  lt_dlclose(lib);

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
  	fprintf(stderr, "warning: .info file not found in component archive.\n");
    return TRUE;
	}

  fwrite(&arch->addr[find.pos], 1, find.len, out_info);

  if (ARCH_find(arch, ".list", 0, &find))
  {
  	fprintf(stderr, "warning: .list file not found in component archive.\n");
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

  if (_nopreload || getenv("GAMBAS_PRELOAD"))
    return;

  sprintf(buf, "LD_PRELOAD=%s", lib);
  putenv(buf);
  putenv("GAMBAS_PRELOAD=1");

  execv(argv[0], argv);
#endif
}


static void analyze(const char *comp)
{
  bool native, gambas;
  char *name;
  char *path_list;
  char *path_info;
  bool ok;

  name = STR_copy(comp);

  sprintf(_buffer, LIB_PATTERN, _lib_path, name);
  native = (access(_buffer, F_OK) == 0);
  sprintf(_buffer, ARCH_PATTERN, _lib_path, name);
  gambas = (access(_buffer, F_OK) == 0);

  if (!native && !gambas)
  {
    fprintf(stderr, "gbi2: warning: component %s not found\n", name);
    STR_free(name);
    return;
  }

  path_info = STR_cat(FILE_cat(_info_path, name, NULL), ".info", NULL);
  path_list = STR_cat(FILE_cat(_info_path, name, NULL), ".list", NULL);

	puts(name);

  //puts(path_info);
  out_info = fopen(path_info, "w");
  if (!out_info)
    error2("Cannot write file", path_info);

  //puts(path_list);
  out_list = fopen(path_list, "w");
  if (!out_list)
    error2("Cannot write file", path_list);

  fflush(stdout);
  ok = TRUE;

  if (native)
  {
    sprintf(_buffer, LIB_PATTERN, _lib_path, name);

    if (analyze_native_component(_buffer))
      ok = FALSE;
  }

  if (gambas)
  {
    sprintf(_buffer, ARCH_PATTERN, _lib_path, name);

    if (analyze_gambas_component(_buffer))
    	if (!native)
      	ok = FALSE;
  }

  fclose(out_info);
  fclose(out_list);

  if (!ok)
  {
    unlink(path_info);
    unlink(path_list);
  }

  STR_free(path_info);
  STR_free(path_list);

  STR_free(name);
}


int main(int argc, char **argv)
{
  DIR *dir;
  struct dirent *dirent;
  char *name;
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
          "Usage: gbi" GAMBAS_VERSION_STRING " [options] [component]\n"
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
          "  -r  --root <directory>     gives the gambas installation directory\n"
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
      #if HAVE_KDE_COMPONENT
      "libkdecore.so.4 "
      #endif
      );

    if (_verbose)
    {
      fprintf(stderr, "component path: %s\n", _lib_path);
      fprintf(stderr, "info path: %s\n", _info_path);
    }

    dir = opendir(_lib_path);
    if (dir == NULL)
      error2("Cannot read directory", _lib_path);

    save_fd = dup(STDOUT_FILENO);

    while ((dirent = readdir(dir)) != NULL)
    {
      name = dirent->d_name;
      if (strcmp(FILE_get_ext(name), "component"))
        continue;
      analyze(FILE_get_basename(name));
    }

    closedir(dir);
  }
  else
  {
    if (strncmp(argv[optind], "gb.qt.kde", 9) == 0)
      preload(argv, "libqt-mt.so.3 libkdecore.so.4");
    else if (strncmp(argv[optind], "gb.qt", 5) == 0)
      preload(argv, "libqt-mt.so.3");

    analyze(argv[optind]);
  }

  exit(0);
}

