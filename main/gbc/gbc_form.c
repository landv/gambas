/***************************************************************************

  form.c

  form description translator

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

#define __GBC_FORM_C

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <fcntl.h>
#include <ctype.h>

#include <sys/stat.h>
#include <sys/types.h>

#include <unistd.h>

#include "gb_common.h"
#include "gb_error.h"
#include "gb_limit.h"
#include "gb_file.h"
#include "gb_str.h"
#include "gbc_compile.h"
#include "gbc_form.h"

/*#define DEBUG*/

static const char *_source;
static const char *_current;

static FORM_PARENT form_parent[MAX_FORM_PARENT];
static int form_parent_level;

#if 0
static void print_fmt(const char *fmt, ...)
{
  va_list args;
  int len;
  char buffer[256];

  va_start(args, fmt);

  if (JOB->verbose)
  {
  	vprintf(fmt, args);
  	va_start(args, fmt);
	}

  len = vsnprintf(buffer, sizeof(buffer), fmt, args);
  if (len < 0)
    ERROR_panic("FORM_do: too short buffer");

  BUFFER_add(&JOB->source, buffer, len);
}
#endif

static void print_len(const char *buffer, int len)
{
  if (JOB->verbose)
  	printf("%.*s", len, buffer);

  BUFFER_add(&JOB->source, buffer, len);
}


static void print(const char *buffer)
{
  if (JOB->verbose)
    printf("%s", buffer);
  BUFFER_add(&JOB->source, buffer, strlen(buffer));
}


static void print_fmt(const char *before, const char *word, int len, const char *after)
{
  print(before);
  print_len(word, len);
  print(after);
}

static void print_var(const char *before, const char *word, int len, const char *after)
{
	print(before);
	if (!word)
		print("ME");
	else
	{
		print("{");
		print_len(word, len);
		print("}");
	}
	print(after);
}

static boolean read_line(const char **str, int *len)
{
  const char *start;
  const char *nospace;
  unsigned char car;

  for(;;)
  {
    car = *_current;
    if (!car)
      return TRUE;
    
    if (car > ' ')
      break;
    
    _current++;
  }

  start = nospace = _current;
  for(;;)
  {
    car = *_current++;
    if (car == '\n' || car =='\r' || !car)
      break;
    if (car > ' ')
      nospace = _current;
  }

  *str = start;
  *len = (int)(nospace - start);
  return FALSE;
}


static char *get_word(const char **str, int *len)
{
  char car;
  const char *pos;

  *len = 0;

  for(;;)
  {
    car = **str;
    if (car == '\n')
      return NULL;
    if (!isspace(car))
      break;
    (*str)++;
  }

  pos = *str;

  for(;;)
  {
    car = **str;
    if (isspace(car))
      break;
    (*str)++;
    (*len)++;
  }

  return (char *)pos;
}


static void parent_enter(const char *str, int len)
{
  if (form_parent_level >= MAX_FORM_PARENT)
    THROW("&1: too many nested containers", JOB->form);

  form_parent[form_parent_level].name = (char *)str;
  form_parent[form_parent_level].len = len;
  form_parent_level++;
}


static void parent_leave(void)
{
  if (form_parent_level == 0)
    THROW("&1: syntax error", JOB->form);

  form_parent_level--;
}


static void parent_get(char **str, int *len, int add)
{
  if (form_parent_level < add)
  {
    *str = NULL;
    *len = 0;
  }
  else
  {
    *str = form_parent[form_parent_level - add].name;
    *len = form_parent[form_parent_level - add].len;
  }
}


static void get_container(char **str, int *len)
{
  parent_get(str, len, 2);
}

static void get_current(char **str, int *len)
{
  parent_get(str, len, 1);
}

PUBLIC char *FORM_get_file(const char *file)
{
  char *form;

  if (strcmp(FILE_get_ext(file), "class"))
    return NULL;

  /*form = STR_cat(file, ".desc", NULL);*/
  form = STR_copy(FILE_set_ext(file, "form"));
  if (!FILE_exist(form))
  {
    STR_free(form);
    return NULL;
  }
  else
    return form;
}



static boolean FORM_init(void)
{
  BUFFER_create(&_source);
  BUFFER_load_file(&_source, JOB->form);
  BUFFER_add(&_source, "\n\0", 2);
  _current = _source;
  form_parent_level = 0;
  return FALSE;
}


static void FORM_exit(void)
{
  BUFFER_delete(&_source);
}


PUBLIC void FORM_do(bool ctrl_public)
{
  const char *line;
  char *word;
  char *win = NULL;
  char *twin = NULL;

  int len, len_win, len_twin;
  const char *pos_rewind;
  bool virtual;

  if (JOB->form == NULL)
    return;

  FORM_init();

  /* version */

  if (read_line(&line, &len))
    goto _ERROR;

  if (!strncasecmp(line, "Gambas Form File 1.0", len))
    THROW("Bad version");

  pos_rewind = _current;
  
  print("#SECTION\n");

  while (!read_line(&line, &len))
  {
    if (len == 0 || *line == '#' || *line == '\'')
      continue;

    if (*line == '{')
    {
      line++;

      word = get_word(&line, &len);
      if (word == NULL)
        goto _ERROR;

      if (win != NULL)
      {
        if (ctrl_public)
          print("PUBLIC");
        else
          print("PRIVATE");

        //print_fmt(" {%.*s} ", len, word);
        print_fmt(" {", word, len, "} ");
      }

      if (win == NULL)
      {
        win = word;
        len_win = len;
      }

      word = get_word(&line, &len);
      if (word == NULL)
        goto _ERROR;

      if (twin != NULL)
      {
        if (*word == '#')
        {
          word++;
          len--;
        }
        //print_fmt("AS %.*s\n", len, word);
        print_fmt("AS ", word, len, "\n");
      }

      if (twin == NULL)
      {
        twin = word;
        len_twin = len;

        //print_fmt("INHERITS %.*s\n\n", len_twin, twin);
        print_fmt("INHERITS ", twin, len_twin, "\n\n");
      }
    }
  }

  /*
  if (type != FT_NORMAL)
    stat = "Static ";
  else
    stat = "";

  if (type != FT_NORMAL)
    print("Static Private _%.*s As %s\n", len_win, win, JOB->class->name);

  print("%sPrivate _%.*sId As Integer\n\n", stat, len_win, win);
  */

  print("\nPUBLIC SUB {$load}()\n\n");

  _current = pos_rewind;

  while (!read_line(&line, &len))
  {
    if (len == 0 || *line == '#' || *line == '\'')
      continue;

    if (*line == '{')
    {
      line++;

      word = get_word(&line, &len);
      if (word == NULL)
        goto _ERROR;

      if (form_parent_level == 0)
      {
        parent_enter(NULL, 0);
        print("  WITH ME\n");
      }
      else
      {
        //print_fmt("  {%.*s} = NEW ", len, word);
        print_fmt("  {", word, len, "} = NEW ");
        parent_enter(word, len);

        word = get_word(&line, &len);
        if (word == NULL)
          goto _ERROR;

        if (*word == '#')
        {
          virtual = TRUE;
          word++;
          len--;
        }
        else
          virtual = FALSE;

        //print_fmt("%.*s", len, word);
        print_len(word, len);

        if (!virtual)
        {
          get_container(&word, &len);
          //print_fmt("(%.*s)", len, word);
          print_var("(", word, len, ")");
        }

        word = get_word(&line, &len);
        if (word == NULL)
          get_current(&word, &len);

        //print_fmt(" AS \"%.*s\"\n", len, word);
        print_fmt(" AS \"", word, len, "\"\n");

        get_current(&word, &len);
        //print_fmt("  WITH {%.*s}\n", len, word);
        print_var("  WITH ", word, len, "\n");
      }
    }
    else if (*line == '}')
    {
      print("  END WITH\n");
      parent_leave();
      if (form_parent_level == 0)
        break;
    }
    else
    {
      /*get_current(&word, &len_word);*/
      /*print("  %.*s", len_word, word);*/
      print("    .");
      print_len(line, len);
      print("\n");
    }
  }

  if (form_parent_level > 0)
    goto _ERROR;


  /*
  FORM_rewind(pos_rewind);
  first = TRUE;
  print("\n");

  while (!read_line(&line, &len))
  {
    if (len == 0 || *line == '#')
      continue;

    if (*line == '{')
    {
      line++;

      word = get_word(&line, &len);
      if (word == NULL)
        goto _ERROR;

      if (first)
        first = FALSE;
      else
        print("Object.Attach(%.*s, Me, \"%.*s\")\n", len, word, len, word);
    }
  }
  */

  print("\nEND\n\n");

  FORM_exit();
  return;

_ERROR:

  THROW("&1: syntax error in form file", JOB->form);
}

