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

/*
#include <qframe.h>
#include <qmainwindow.h>
#include <qpushbutton.h>
*/

#include "gambas.h"
#include "../gb.qt.h"
#include "main.h"

#include "CLCDNumber.h"
#include "CDial.h"
#include "CEditor.h"

extern "C" {

GB_INTERFACE GB EXPORT;
QT_INTERFACE QT;
EVAL_INTERFACE EVAL;

GB_DESC *GB_CLASSES[] EXPORT =
{
  CLCDNumberDesc,
  CDialDesc,

	CHighlightDesc,
  CEditorLinesDesc,
  CEditorSelectionDesc,
  CEditorStyleDesc,
  CEditorStylesDesc,
  CEditorFlagsDesc,
  CEditorDesc,

  NULL
};

int EXPORT GB_INIT(void)
{
  GB.GetInterface("gb.qt", QT_INTERFACE_VERSION, &QT);

  return FALSE;
}

void EXPORT GB_EXIT()
{
}

}

bool MAIN_load_eval_component(void)
{
  if (GB.LoadComponent("gb.eval"))
    return true;

  GB.GetInterface("gb.eval", EVAL_INTERFACE_VERSION, &EVAL);
  return false;
}

