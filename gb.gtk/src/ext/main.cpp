/***************************************************************************

  main.cpp

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

#define __MAIN_C

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>

#include "main.h"

#include "CGridView.h"
#include "CPaint.h"

GTK_INTERFACE GTK;
DRAW_INTERFACE DRAW;

extern "C" {

GB_INTERFACE GB EXPORT;

GB_DESC *GB_CLASSES[] EXPORT =
{
	CGridViewColumnDesc,
	CGridViewColumnsDesc,
	CGridViewDesc,
	CStateDesc,
	CShadowDesc,
	CPaintDesc,
        CDrawDesc,
  NULL
};

int EXPORT GB_INIT(void)
{
  GB.GetInterface("gb.gtk", GTK_INTERFACE_VERSION, &GTK);
  GB.GetInterface("gb.draw", DRAW_INTERFACE_VERSION, &DRAW);	
  return FALSE;
}

void EXPORT GB_EXIT()
{
}

}
