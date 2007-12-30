/***************************************************************************

  form.c

  form description translator

  (c) 2000-2006 Benoît Minisini <gambas@users.sourceforge.net>

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

static char *form_source;
static long form_current;

static FORM_PARENT form_parent[MAX_FORM_PARENT];
static long form_parent_level;

static void print(const char *fmt, ...)
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


static void print_len(const char *buffer, long len)
{
  if (JOB->verbose)
  	printf("%.*s", (int)len, buffer);

  BUFFER_add(&JOB->source, buffer, len);
}


static boolean read_line(const char **str, long *len)
{
  long start, nospace;
  char car;

  for(;;)
  {
    if (form_current >= BUFFER_length(form_source))
      return TRUE;

    car = form_source[form_current];
    if (!isspace(car))
      break;
    form_current++;
  }

  start = nospace = form_current;
  for(;;)
  {
    car = form_source[form_current++];
    if (car == '\n' || form_current >= BUFFER_length(form_source))
      break;
    if (!isspace(car))
      nospace = form_current;
  }

  *str = &form_source[start];
  *len = nospace - start;
  return FALSE;
}


static char *get_word(const char **str, long *len)
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


static void parent_enter(const char *str, long len)
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


static void parent_get(char **str, long *len, int add)
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


static void get_container(char **str, long *len)
{
  parent_get(str, len, 2);
}

static void get_current(char **str, long *len)
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
  BUFFER_create(&form_source);
  BUFFER_load_file(&form_source, JOB->form);
  BUFFER_add(&form_source, "\n", 1);
  form_current = 0;
  form_parent_level = 0;
  return FALSE;
}


static void FORM_rewind(long pos)
{
  form_current = pos;
}


static void FORM_exit(void)
{
  BUFFER_delete(&form_source);
}


PUBLIC void FORM_do(bool ctrl_public)
{
  const char *line;
  char *word;
  char *win = NULL;
  char *twin = NULL;

  long len, len_win, len_twin;
  long pos_rewind;
  bool virtual;

  if (JOB->form == NULL)
    return;

  FORM_init();

  /* version */

  if (read_line(&line, &len))
    goto _ERROR;

  if (!strncasecmp(line, "Gambas Form File 1.0", len))
    THROW("Bad version");

  pos_rewind = form_current;

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

        print(" {%.*s} ", len, word);
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
        print("AS %.*s\n", len, word);
      }

      if (twin == NULL)
      {
        twin = word;
        len_twin = len;

        print("INHERITS %.*s\n\n", len_twin, twin);
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

  FORM_rewind(pos_rewind);

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
        parent_enter("ME", 2);
        print("  WITH ME\n");
      }
      else
      {
        print("  {%.*s} = NEW ", len, word);
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

        print("%.*s", len, word);

        if (!virtual)
        {
          get_container(&word, &len);
          print("(%.*s)", len, word);
        }

        word = get_word(&line, &len);
        if (word == NULL)
          get_current(&word, &len);

        print(" AS \"%.*s\"\n", len, word);
        /*print("\n");*/

        get_current(&word, &len);
        print("  WITH {%.*s}\n", len, word);
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

