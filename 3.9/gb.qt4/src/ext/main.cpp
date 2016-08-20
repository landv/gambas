/***************************************************************************

  main.cpp

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

#include "main.h"

#include "CLCDNumber.h"
#include "CDial.h"
#include "CEditor.h"
#include "CTextEdit.h"

extern "C" {

GB_INTERFACE GB EXPORT;
QT_INTERFACE QT;
EVAL_INTERFACE EVAL;

GB_DESC *GB_CLASSES[] EXPORT =
{
	CLCDNumberDesc,
	
	CDialDesc,

	CHighlightDesc,
	CEditorLineDesc,
	CEditorLinesDesc,
	CEditorSelectionDesc,
	CEditorStyleDesc,
	CEditorStylesDesc,
	CEditorFlagsDesc,
	CEditorDesc,
	
	CTextEditSelectionDesc,
	CTextEditFormatDesc,
	CTextEditDesc,

	NULL
};

int EXPORT GB_INIT(void)
{
	GB.GetInterface("gb.qt4", QT_INTERFACE_VERSION, &QT);
	
	return 0;
}

void EXPORT GB_EXIT()
{
}

}

bool MAIN_load_eval_component(void)
{
	if (GB.Component.Load("gb.eval"))
		return true;

	GB.GetInterface("gb.eval", EVAL_INTERFACE_VERSION, &EVAL);
	return false;
}

