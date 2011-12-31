/***************************************************************************

  gbi.c

  (c) 2000-2012 Benoît Minisini <gambas@users.sourceforge.net>

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

#include <config.h>

#include "gb_limit.h"
#include "gb_common.h"
#include "gb_alloc.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>

#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <dirent.h>

#include <dlfcn.h>

#if defined(OS_LINUX) || defined(OS_OPENBSD) || defined(OS_FREEBSD) || defined(OS_CYGWIN)
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
#include "gb_array.h"
#include "gambas.h"

static char _root[PATH_MAX + 1] = { 0 };
static char _lib_path[PATH_MAX + 1];
static char _info_path[PATH_MAX + 1];
static char _buffer[PATH_MAX + 1];
static char _env[PATH_MAX + 16];

static FILE *out_info;
static FILE *out_list;
static bool _verbose = FALSE;
static bool _format = FALSE;
static bool _nopreload = FALSE;
static bool _root_set = FALSE;
static bool _analyze = FALSE;

static char **_components = NULL;

static void analyze(const char *comp, bool include);

#if HAVE_GETOPT_LONG
static struct option LongOptions[] =
{
	{ "version", 0, NULL, 'V' },
	{ "verbose", 0, NULL, 'v' },
	{ "help", 0, NULL, 'h' },
	{ "license", 0, NULL, 'L' },
	{ "root", 1, NULL, 'r' },
	{ 0 }
};
#endif


static int compare_components(char **a, char **b)
{
	return strcmp(*a, *b);
}

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
		const char *dir;
		
		path = FILE_find_gambas();
		dir = FILE_get_dir(FILE_get_dir(path));
		if (dir)
			strncpy(_root, dir, PATH_MAX);
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
			
		case 'b':
			print("%s", value ? "T" : "");
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

static void analyze_include(char *include_list)
{
	char *includes[8];
	int nincludes;
	char *include;
	int i;
	
	include_list = STR_copy(include_list);
	
	if (_verbose)
		fprintf(stderr, "Including %s\n", include_list);
	
	nincludes = 0;
	include = strtok(include_list, ",");
	while (include && nincludes < 8)
	{
		includes[nincludes++] = include;
		include = strtok(NULL, ",");
	}
	
	for (i = 0; i < nincludes; i++)
		analyze(includes[i], TRUE);
		
	STR_free(include_list);
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
		analyze_include(*include);

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
		if (_verbose)
			warning("cannot find '" LIB_CLASS "' symbol in shared library.");
		//ret = TRUE;
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
	bool ret = TRUE;

	if (_verbose)
		fprintf(stderr, "Loading gambas component: %s\n", path);

	arch = ARCH_open(path);

	if (ARCH_find(arch, ".info", 0, &find))
	{
		warning(".info file not found in component archive.");
		goto __RETURN;
	}

	fwrite(&arch->addr[find.pos], 1, find.len, out_info);

	if (ARCH_find(arch, ".list", 0, &find))
	{
		warning(".list file not found in component archive.");
		goto __RETURN;
	}

	fwrite(&arch->addr[find.pos], 1, find.len, out_list);
	ret = FALSE;

__RETURN:
	
	ARCH_close(arch);
	return ret;
}


static void preload(char **argv, char *lib)
{
#if DO_PRELOADING
	if (_nopreload || getenv("GB_PRELOAD") || !lib || !*lib)
		return;

	snprintf(_env, sizeof(_env), "LD_PRELOAD=%s", lib);
	putenv(_env);
	putenv("GB_PRELOAD=1");

	execvp(argv[0], argv);
#endif
}

static bool find_native_component(const char *name)
{
	snprintf(_buffer, sizeof(_buffer), LIB_PATTERN, _lib_path, name);
	return (access(_buffer, F_OK) == 0);
}

static void analyze(const char *comp, bool include)
{
	bool native, gambas;
	char *name;
	char *path_list = NULL;
	char *path_info = NULL;
	bool ok;

	name = STR_copy(comp);

	if (_verbose)
		fprintf(stderr, "%s component %s\n", include ? "Including" : "Analyzing", name);
	else if (!include)
		puts(name);
	
	native = find_native_component(name);
		
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
		{
			error(FALSE, "Cannot write file: %s", path_info);
			return;
		}
	
		out_list = fopen(path_list, "w");
		if (!out_list)
		{
			error(FALSE, "Cannot write file: %s", path_list);
			return;
		}
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
			FILE_unlink(path_info);
			FILE_unlink(path_list);
		}
		else if (_verbose)
		{
			fprintf(stderr, "wrote %s\n", path_info);
			fprintf(stderr, "wrote %s\n", path_list);
		}

		STR_free(path_info);
		STR_free(path_list);
	}
	
	STR_free(name);
}

static void run_myself(const char *path, const char *name)
{
	const char *argv[10];
	int n = 0;
	pid_t pid;
	int status;
	
	if (_verbose)
		fprintf(stderr, "Running myself for component %s\n", name);
	
	argv[n++] = path;
	if (_verbose)
		argv[n++] = "-v";
	if (_nopreload)
		argv[n++] = "-p";
	if (_root_set)
	{
		argv[n++] = "-r";
		argv[n++] = _root;
	}
	argv[n++] = "-a";
	argv[n++] = name;
	argv[n] = NULL;
	
	if (find_native_component(name))
	{
		snprintf(_env, sizeof(_env), "LD_PRELOAD=%s", _buffer);
		putenv(_env);
	}

	pid = fork();
	switch (pid)
	{
		case 0:
			execvp(path, (char **)argv);
		case -1:
			error(FALSE, "Cannot run sub-process: %s", strerror(errno));
			exit(1);
		default:
			waitpid(pid, &status, 0);
	}
}

static void make_component_list()
{
	DIR *dir;
	struct dirent *dirent;
	const char *name;
			
	dir = opendir(_lib_path);
	if (dir == NULL)
		error(TRUE, "Cannot read directory: %s", _lib_path);

	//save_fd = dup(STDOUT_FILENO);

	ARRAY_create(&_components);
	
	while ((dirent = readdir(dir)) != NULL)
	{
		name = dirent->d_name;
		if (strcmp(FILE_get_ext(name), "component"))
			continue;
		name = FILE_get_basename(name);
		*((char **)ARRAY_add(&_components)) = STR_copy(name);
	}

	closedir(dir);
	
	qsort(_components, ARRAY_count(_components), sizeof(*_components), (int (*)(const void *, const void *))compare_components);
}

int main(int argc, char **argv)
{
	int i;
	char *name;
	int opt;
	int ind = 0;

	/*#ifdef __FreeBSD__
	optind = 1;
	#else
	optind = 0;
	#endif*/

	//dup(STDOUT_FILENO);

	//_verbose = TRUE;

	for(;;)
	{
		#if HAVE_GETOPT_LONG
			opt = getopt_long(argc, argv, "vVhLpar:", LongOptions, &ind);
		#else
			opt = getopt(argc, argv, "vVhLpar:");
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
				strncpy(_root, optarg, PATH_MAX);
				_root_set = TRUE;
				break;
				
			case 'a':
				_analyze = TRUE;
				break;
				
			case 'L':
				printf(
					"GAMBAS Component Informer version " VERSION " " __DATE__ " " __TIME__ "\n"
					COPYRIGHT
					);
				exit(0);

			case 'h':
				printf(
					"Usage: gbi" GAMBAS_VERSION_STRING " [options] [components]\n"
					"Options:"
					#if HAVE_GETOPT_LONG
					"\n"
					"  -V  --version              display version\n"
					"  -h  --help                 display this help\n"
          "  -L  --license              display license\n"
					"  -p                         disable preloading\n"
					"  -r  --root <directory>     gives the gambas installation directory\n"
					#else
					" (no long options on this system)\n"
					"  -V                         display version\n"
					"  -h                         display this help\n"
          "  -L                         display license\n"
					"  -p                         disable preloading\n"
					"  -r <directory>             gives the gambas installation directory\n"
					#endif
					"\n"
					);

				exit(0);
		}
	}

	init();
	
	if (_analyze)
	{
		if (_verbose)
			fprintf(stderr, "LD_PRELOAD=%s\n", getenv("LD_PRELOAD"));

		analyze(argv[optind], FALSE);
	}
	else
	{
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
			
			make_component_list();
			
			for (i = 0; i < ARRAY_count(_components); i++)
			{
				name = _components[i];
				run_myself(argv[0], name);
				STR_free(name);
			}
			
  		ARRAY_delete(&_components);
		}
		else
		{
			if (!getenv("GB_PRELOAD"))
			{
				for (ind = optind; ind < argc; ind++)
				{
					name = argv[ind];			
					if (strncmp(name, "gb.qt.kde", 9) == 0)
						preload(argv, "libqt-mt.so.3 libkdecore.so.4");
					else if (strcmp(name, "gb.qt") == 0 || strncmp(name, "gb.qt.", 6) == 0)
						preload(argv, "libqt-mt.so.3");
				}
			}
		
			for (ind = optind; ind < argc; ind++)
			{
				name = argv[ind];
				//analyze(name, FALSE);
				run_myself(argv[0], name);
			}
		}
	}

	exit(0);
}

