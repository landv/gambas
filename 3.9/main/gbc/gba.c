/***************************************************************************

	gba.c

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

#define __GBA_C

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

#include "gb_common.h"
#include "gb_error.h"
#include "gb_str.h"
#include "gb_file.h"
#include "gb_array.h"
#include "gb_common_buffer.h"

#include "gbc_archive.h"

#if HAVE_GETOPT_LONG
static struct option Long_options[] =
{
	{ "version", 0, NULL, 'V' },
	{ "help", 0, NULL, 'h' },
	{ "swap", 0, NULL, 's' },
	{ "verbose", 0, NULL, 'v' },
	{ "output", 1, NULL, 'o' },
	{ "extract", 1, NULL, 'x' },
	{ 0 }
};
#endif

static char **path_list;
static int path_current;

static const char *allowed_hidden_files[] = { ".gambas", ".info", ".list", ".lang", ".action", ".connection", ".component", ".public", NULL };
//static const char *remove_ext_root[] = { "module", "class", "form", "gambas", NULL };
static const char *remove_ext_lang[] = { "pot", "po", NULL };

static bool _extract = FALSE;
static char *_extract_archive;
static char *_extract_file = NULL;

static void get_arguments(int argc, char **argv)
{
	int opt;
	#if HAVE_GETOPT_LONG
	int index = 0;
	#endif

	for(;;)
	{
		#if HAVE_GETOPT_LONG
			opt = getopt_long(argc, argv, "vVLhso:x:", Long_options, &index);
		#else
			opt = getopt(argc, argv, "vVLhso:x:");
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

			case 'v':
				ARCH_verbose = TRUE;
				break;

			case 's':
				ARCH_swap = TRUE;
				break;

			case 'o':
				ARCH_define_output(optarg);
				_extract = FALSE;
				break;
				
			case 'x':
				_extract_archive = optarg;
				_extract = TRUE;
				break;
				
			case 'L':
				printf(
					"\nGAMBAS Archiver version " VERSION " " __DATE__ " " __TIME__ "\n"
					COPYRIGHT
					);
				exit(0);

			case 'h': case '?':
				printf(
					"\nCreate a standalone one-file executable from a Gambas project.\n"
					"Or extract a specific file from a Gambas executable (-x option).\n"
					"\nUsage: gba" GAMBAS_VERSION_STRING " [options] [<project directory>]\n"
					"       gba" GAMBAS_VERSION_STRING " -x <archive path> <file>\n\n"
					"Options:"
					#if HAVE_GETOPT_LONG
					"\n"
					"  -o  --output=ARCHIVE       archive path [<project directory>/<project name>.gambas]\n"
					"  -x  --extract=ARCHIVE      archive path\n"
					"  -v  --verbose              verbose output\n"
					"  -s  --swap                 swap endianness\n"
					"  -V  --version              display version\n"
					"  -L  --license              display license\n"
					"  -h  --help                 display this help\n"
					#else
					" (no long options on this system)\n"
					"  -o=ARCHIVE             archive path [<project directory>/<project name>.gambas]\n"
					"  -x=ARCHIVE             archive path\n"
					"  -v                     verbose output\n"
					"  -s                     swap endianness\n"
					"  -V                     display version\n"
					"  -L                     display license\n"
					"  -h                     display this help\n"
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
		fprintf(stderr, "gba: too many arguments.\n");
		exit(1);
	}

	if (_extract)
	{
		if (optind == argc)
		{
			fprintf(stderr, "gba: not enough arguments.\n");
			exit(1);
		}
		_extract_file = argv[optind];
	}
	else
	{
		if (optind == argc)
			ARCH_define_project(NULL);
		else
			ARCH_define_project(argv[optind]);

		if (!FILE_exist(ARCH_project))
		{
			fprintf(stderr, "gba: project file not found: %s\n", ARCH_project);
			exit(1);
		}
	}
}


static void path_add(const char *path)
{
	*((char **)ARRAY_add(&path_list)) = STR_copy(path);
}


static void path_init(const char *first)
{
	ARRAY_create(&path_list);

	if (*first)
		FILE_chdir(first);

	path_add(FILE_get_current_dir());

	path_current = 0;
}


static void path_exit(void)
{
	ARRAY_delete(&path_list);
}


static int path_count(void)
{
	return ARRAY_count(path_list);
}


int main(int argc, char **argv)
{
	DIR *dir;
	const char *path;
	struct dirent *dirent;
	char *file_name;
	const char *file;
	struct stat info;
	const char *ext;
	int len;
	const char **p;
	int len_prefix;
	const char **remove_ext;
	ARCH *arch;
	ARCH_FIND find;

	get_arguments(argc, argv);
	COMMON_init();

	TRY
	{
		if (_extract) // Extract a file from an archive
		{
			arch = ARCH_open(_extract_archive);
			
			if (ARCH_find(arch, _extract_file, 0, &find))
				fprintf(stderr, "gba: file not found in archive\n");
			else
				fwrite(&arch->addr[find.pos], sizeof(char), find.len, stdout);
			
			ARCH_close(arch);
		}
		else // Create an archive
		{
			ARCH_init();

			file = FILE_get_dir(ARCH_project);
			len_prefix = strlen(file);
			path_init(file);

			/* .startup and .project file always first ! */
			
			path = FILE_cat(FILE_get_dir(ARCH_project), ".startup", NULL);
			if (FILE_exist(path)) ARCH_add_file(path);
			
			path = FILE_cat(FILE_get_dir(ARCH_project), ".project", NULL);
			if (FILE_exist(path)) ARCH_add_file(path);

			for(;;)
			{
				if (path_current >= path_count())
					break;

				path = path_list[path_current++];
				dir = opendir(path);

				if (dir == NULL)
				{
					fprintf(stderr, "gba: warning: Cannot open dir: %s\n", path);
					goto _NEXT_PATH;
				}

				if (chdir(path) != 0)
				{
					fprintf(stderr, "gba: warning: Cannot change dir: %s\n", path);
					goto _NEXT_PATH;
				}

				while ((dirent = readdir(dir)) != NULL)
				{
					file_name = dirent->d_name;
					len = strlen(file_name);

					if (*file_name == '.')
					{
						for (p = allowed_hidden_files; *p; p++)
						{
							if (strcmp(file_name, *p) == 0)
								break;
						}

						if (*p == NULL)
							continue;
					}

					if (file_name[len - 1] == '~')
						continue;

					//if (strcmp(file_name, ARCH_project_name) == 0)
					//  continue;
						
					if ((len == 4) && (strncmp(file_name, "core", 4) == 0))
						continue;

					if ((len > 5) && (strncmp(file_name, "core.", 5) == 0))
						continue;

					if ((len > 7) && (strncmp(file_name, "vgcore.", 5) == 0))
						continue;

					if ((len > 10) && (strncmp(file_name, "callgrind.", 5) == 0))
						continue;

					file = FILE_cat(path, file_name, NULL);
					
					// Do not put the archive file inside itself.
					if (!strcmp(file, ARCH_output))
						continue;

					if (stat(file_name, &info))
					{
						fprintf(stderr, "gba: warning: cannot stat file: %s\n", file);
						continue;
					}

					if (S_ISDIR(info.st_mode))
					{
						if (strcmp(file_name, "CVS") == 0)
							continue;
							
						path_add(file);
						ARCH_add_file(file);
					}
					else
					{
						ext = FILE_get_ext(file_name);

						//printf("path = %s\n", &path[len_prefix]);

						//if (path[len_prefix] == 0)
						//	remove_ext = remove_ext_root;
						if (strcmp(&path[len_prefix], "/.lang") == 0)
							remove_ext = remove_ext_lang;
						else
							remove_ext = 0;
						
						if (remove_ext)
						{
							for (p = remove_ext; *p; p++)
							{
								if (strcasecmp(ext, *p) == 0)
									break;
							}
						
							if (*p != NULL)
								continue;
						}
						
						if (strcmp(ext, "gambas") == 0)
							continue;

						ARCH_add_file(file);
					}
				}

	_NEXT_PATH:
				if (dir != NULL) closedir(dir);
				FREE((char **)&path);
			}

			path_exit();

			ARCH_exit();
			/*MEM_check();*/
		}
	}
	CATCH
	{
		fflush(NULL);
		fprintf(stderr, "gba: ERROR: ");
		ERROR_print();
		exit(1);
	}
	END_TRY

	return 0;
}

