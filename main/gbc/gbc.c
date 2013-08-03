/***************************************************************************

  gbc.c

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

#define __GBC_C

#include "config.h"
#include "trunk_version.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>

#include <sys/stat.h>
#include <sys/types.h>
#include <sys/time.h>
#include <dirent.h>

#include <unistd.h>

#include "gb_common.h"
#include "gb_error.h"
#include "gb_str.h"
#include "gb_file.h"
#include "gb_common_buffer.h"

#include "gbc_compile.h"

#include "gb_reserved.h"
#include "gbc_read.h"
#include "gbc_form.h"
#include "gbc_trans.h"
#include "gbc_header.h"
#include "gbc_output.h"


#if HAVE_GETOPT_LONG
static struct option Long_options[] =
{
	{ "debug", 0, NULL, 'g' },
	{ "version", 0, NULL, 'V' },
	{ "help", 0, NULL, 'h' },
	{ "license", 0, NULL, 'L' },
	{ "verbose", 0, NULL, 'v' },
	{ "translate", 0, NULL, 't' },
	{ "public-control", 0, NULL, 'p' },
	{ "public-module", 0, NULL, 'm' },
	{ "swap", 0, NULL, 's' },
	{ "class", 1, NULL, 'c' },
	/*{ "dump", 0, NULL, 'd' },*/
	{ "root", 1, NULL, 'r' },
	{ "all", 0, NULL, 'a' },
	{ "translate-errors", 0, NULL, 'e' },
	{ "no-old-read-write-syntax", 0, NULL, 1 },
	{ 0 }
};
#endif

static bool main_debug = FALSE;
static bool main_exec = FALSE;
static bool main_verbose = FALSE;
static bool main_compile_all = FALSE;
static bool main_trans = FALSE;
static bool main_warnings = FALSE;
static bool main_public = FALSE;
static bool main_public_module = FALSE;
static bool main_swap = FALSE;
static bool main_no_old_read_syntax = FALSE;
//static char *main_class_file = NULL;

static char **_files = NULL;

static void get_arguments(int argc, char **argv)
{
	const char *dir;
	int opt;
	#if HAVE_GETOPT_LONG
	int index = 0;
	#endif

	for(;;)
	{
		#if HAVE_GETOPT_LONG
			opt = getopt_long(argc, argv, "gxvaVhLwtpmser:", Long_options, &index);
		#else
			opt = getopt(argc, argv, "gxvaVhLwtpmser:");
		#endif
		if (opt < 0) break;

		switch (opt)
		{
			case 'V':
				#ifdef TRUNK_VERSION
				printf(VERSION " r" TRUNK_VERSION "\n");
				#else
				printf(VERSION "\n");
				#endif
				exit(0);

			case 'g':
				main_debug = TRUE;
				break;

			case 'x':
				main_exec = TRUE;
				break;

			case 'v':
				main_verbose = TRUE;
				break;

			case 'a':
				main_compile_all = TRUE;
				break;

			case 't':
				main_trans = TRUE;
				break;

			case 'w':
				main_warnings = TRUE;
				break;

			case 'p':
				main_public = TRUE;
				break;

			case 'm':
				main_public_module = TRUE;
				break;

			case 's':
				main_swap = TRUE;
				break;

			//case 'c':
			//  main_class_file = optarg;
			//:  break;

			case 'r':
				if (COMP_root)
				{
					fprintf(stderr, "gbc: option '-r' already specified.\n");
					exit(1);
				}
				COMP_root = STR_copy(optarg);
				break;
				
			case 'e':
				ERROR_translate = TRUE;
				break;
				
			case 1:
				main_no_old_read_syntax = TRUE;
				break;
			
			case 'L':
				printf(
					"\nGAMBAS Compiler version " VERSION " " __DATE__ " " __TIME__ "\n"
					COPYRIGHT
					);
				exit(0);
				
			case 'h': case '?':
				printf(
					"\nCompile Gambas projects into architecture-independent bytecode.\n"
					"\nUsage: gbc" GAMBAS_VERSION_STRING " [options] [<project directory>]\n\n"
					"Options:"
					#if HAVE_GETOPT_LONG
					"\n"
					"  -g  --debug                add debugging information\n"
					"  -v  --verbose              verbose output\n"
					"  -a  --all                  compile all\n"
					"  -w  --warnings             display warnings\n"
					"  -t  --translate            output translation files\n"
					"  -p  --public-control       form controls are public\n"
					"  -m  --public-module        module symbols are public by default\n"
					"  -s  --swap                 swap endianness\n"
					"  -r  --root <directory>     gives the gambas installation directory\n"
					"  -e  --translate-errors     display translatable error messages\n"
					"  -x  --exec                 define the 'Exec' preprocessor constant\n"
					"  -V  --version              display version\n"
					"  -L  --license              display license\n"
					"  -h  --help                 display this help\n"
					#else
					" (no long options on this system)\n"
					"  -g                         add debugging information\n"
					"  -v                         verbose output\n"
					"  -a                         compile all\n"
					"  -w                         display warnings\n"
					"  -t                         output translation files\n"
					"  -p                         form controls are public\n"
					"  -m                         module symbols are public by default\n"
					"  -s                         swap endianness\n"
					"  -r <directory>             gives the gambas installation directory\n"
					"  -e                         display translatable error messages\n"
					"  -x                         define the 'Exec' preprocessor constant\n"
					"  -V                         display version\n"
					"  -L                         display license\n"
					"  -h                         display this help\n"
					#endif
					"\n"
					);

				exit(0);

			default:
				exit(1);

		}
	}

	if (optind < (argc - 1))
	{
		fprintf(stderr, "gbc: too many arguments.\n");
		exit(1);
	}

	/*COMP_project = STR_copy(FILE_cat(argv[optind], "Gambas", NULL));*/
	if (optind < argc)
		FILE_chdir(argv[optind]);

	dir = FILE_get_current_dir();
	if (!dir)
	{
		fprintf(stderr, "gbc: no current directory.\n");
		exit(1);
	}    

	COMP_project = STR_copy(FILE_cat(dir, ".project", NULL));

	if (!FILE_exist(COMP_project))
	{
		fprintf(stderr, "gbc: project file not found: %s\n", COMP_project);
		exit(1);
	}
}


static void compile_file(const char *file)
{
	int i;
	time_t time_src, time_form, time_pot, time_output;
	char *source;

	COMPILE_begin(file, main_trans, main_debug);

	if (!main_compile_all)
	{
		if (FILE_exist(JOB->output))
		{
			time_src = FILE_get_time(JOB->name);
			time_output = FILE_get_time(JOB->output);

			if (JOB->form)
				time_form = FILE_get_time(JOB->form);
			else
				time_form = time_src;

			if (main_trans)
				time_pot = FILE_get_time(JOB->tname);
			else
				time_pot = time_src;

			if (time_src <= time_output && time_src <= time_pot && time_form <= time_output)
				goto _FIN;
		}
	}

	JOB->all = main_compile_all;
	JOB->exec = main_exec;
	JOB->verbose = main_verbose;
	JOB->warnings = main_warnings;
	JOB->swap = main_swap;
	JOB->public_module = main_public_module;
	JOB->no_old_read_syntax = main_no_old_read_syntax;
	//JOB->class_file = main_class_file;

	if (JOB->verbose)
	{
		putchar('\n');
		for (i = 1; i <= 9; i++)
			printf("--------");
		printf("\nCompiling %s...\n", FILE_get_name(JOB->name));
	}

	JOB->first_line = 1;

	if (JOB->form)
	{
		JOB->first_line = FORM_FIRST_LINE;
		BUFFER_add(&JOB->source, "#Line " FORM_FIRST_LINE_STRING "\n", -1);
		
		BUFFER_create(&source);
		BUFFER_load_file(&source, JOB->form);
		BUFFER_add(&source, "\n\0", 2);

		switch (JOB->family->type)
		{
			case FORM_WEBPAGE:
				FORM_webpage(source);
				break;
				
			case FORM_NORMAL:
			default:
				FORM_do(source, main_public);
				break;
		}
				
		BUFFER_delete(&source);
		
		BUFFER_add(&JOB->source, "#Line 1\n", -1);
	}
	
	COMPILE_load();
	BUFFER_add(&JOB->source, "\n\0", 2);
	
	#if 0
	fprintf(stderr, "-----------------\n");
	fputs(JOB->source, stderr);
	fprintf(stderr, "-----------------\n");
	#endif

	READ_do();

	#ifdef DEBUG
	TABLE_print(JOB->class->table, TRUE);
	#endif

	HEADER_do();
	TRANS_code();

	#ifdef DEBUG
	TABLE_print(JOB->class->string, FALSE);
	#endif

	OUTPUT_do(main_swap);
	CLASS_export();

_FIN:
	COMPILE_end();
}


static int compare_path(char **a, char **b)
{
	return strcmp(*a, *b);
}

static void fill_files(const char *root, bool recursive)
{
	DIR *dir;
	char *path;
	struct dirent *dirent;
	char *file_name;
	const char *name;
	const char *file;
	struct stat info;
	const char *ext;
	
	path = STR_copy(root);
	
	dir = opendir(path);
	if (!dir)
	{
		fprintf(stderr, "gbc: cannot browse directory: %s\n", path);
		exit(1);
	}
	
	while ((dirent = readdir(dir)) != NULL)
	{
		file_name = dirent->d_name;
		if (*file_name == '.')
			continue;

		file = FILE_cat(path, file_name, NULL);

		if (stat(file, &info))
		{
			fprintf(stderr, "gbc: warning: cannot stat file: %s\n", file);
			continue;
		}

		if (S_ISDIR(info.st_mode))
		{
			if (recursive)
				fill_files(file, TRUE);
		}
		else
		{
			ext = FILE_get_ext(file);

			if ((strcmp(ext, "module") == 0)
					|| (strcmp(ext, "class") == 0))
			{
				*((char **)ARRAY_add(&_files)) = STR_copy(file);
				// Add the class to the list of classes
				name = FILE_get_basename(file_name);
				COMPILE_add_class(name, strlen(name));
			}
		}
	}

	closedir(dir);
	STR_free(path);
}

static void init_files(const char *first)
{
	bool recursive;
	
	ARRAY_create(&_files);

	if (*first)
		FILE_chdir(first);
	
	recursive = chdir(".src") == 0;
	fill_files(FILE_get_current_dir(), recursive);
	if (recursive) FILE_chdir("..");

	// Sort paths
	qsort(_files, ARRAY_count(_files), sizeof(*_files), (int (*)(const void *, const void *))compare_path);
	
	// End the list of classes
	COMPILE_end_class();
}


static void exit_files(void)
{
	int i;
	
	for (i = 0; i < ARRAY_count(_files); i++)
		STR_free(_files[i]);
	
	ARRAY_delete(&_files);
}


int main(int argc, char **argv)
{
	int i;
	
	MEMORY_init();
	COMMON_init();

	TRY
	{
		get_arguments(argc, argv);

		COMPILE_init();

		// Remove information files if we are compiling everything
		
		if (main_compile_all)
		{
			if (main_verbose)
				puts("Removing .info and .list files");
			FILE_chdir(FILE_get_dir(COMP_project));
			FILE_unlink(".info");
			FILE_unlink(".list");
		}

		init_files(FILE_get_dir(COMP_project));

		for (i = 0; i < ARRAY_count(_files); i++)
			compile_file(_files[i]);

		exit_files();
		
		COMPILE_exit();
		
		puts("OK");
	}
	CATCH
	{
		fflush(NULL);
		
		COMPILE_print(MSG_ERROR, -1, NULL);
		ERROR_print();
		exit(1);
	}
	END_TRY

	return 0;
}

