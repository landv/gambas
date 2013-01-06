/***************************************************************************

  main.c

  (c) 2003-2004 Daniel Campos Fernández <danielcampos@netcourrier.com>

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

#define MAX_DRIVER 8
#define __MAIN_C

#include <stdio.h>

#include "CCompress.h"
#include "CUncompress.h"
#include "gb.compress.h"

#include "gb_common.h"

#include "main.h"



GB_INTERFACE GB EXPORT;

static COMPRESS_DRIVER *_drivers[MAX_DRIVER];
static int _drivers_count = 0;


static void COMPRESS_Register(COMPRESS_DRIVER *driver)
{
  if (_drivers_count >= MAX_DRIVER)
    return;

  _drivers[_drivers_count] = driver;
  _drivers_count++;

}

COMPRESS_DRIVER *COMPRESS_GetDriver(char *type)
{
  int i;
	char *comp;

  if (!type || !*type)
  {
    GB.Error("Driver name missing");
    return NULL;
  }

  comp = alloca(strlen(type) + 14);

	strcpy(comp, "gb.compress.");
  strcat(comp, type);

  if (GB.Component.Load(comp))
  {
    GB.Error("Cannot find driver for : &1", type);
    return NULL;
  }

  for (i = 0; i < _drivers_count; i++)
  {
    if (strcasecmp(_drivers[i]->name, type) == 0)
      return _drivers[i];
  }

  return NULL;
}

void *GB_COMPRESS_1[] EXPORT = {

  (void *)1,

  (void *)COMPRESS_Register,

  NULL
};

GB_DESC *GB_CLASSES[] EXPORT =
{
  CCompressDesc,
  CUncompressDesc,
  NULL
};

int EXPORT GB_INIT(void)
{
  return 0;
}

void EXPORT GB_EXIT()
{

}

