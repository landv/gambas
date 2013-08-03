/***************************************************************************

  CFont.h

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

#ifndef __CFONT_H
#define __CFONT_H

#include <qfont.h>

#include "gambas.h"
//#include "main.h"
#include "CWidget.h"

#ifndef NO_X_WINDOW
//#define USE_DPI
#endif

#ifndef __CFONT_CPP
extern GB_DESC CFontDesc[];
extern GB_DESC CFontsDesc[];

#ifdef USE_DPI
extern int CFONT_dpi;
#endif

#else

#define THIS OBJECT(CFONT)

#endif

typedef
  void (*FONT_FUNC)(QFont &, void *);

typedef
  struct {
    GB_BASE ob;
    QString *family;
    }
  CFONTINFO;

typedef
  struct {
    GB_BASE ob;
    QFont *font;
    FONT_FUNC func;
    void *object;
    enum { Name, Size, Grade, Bold, Italic, Underline, Strikeout };
    }
  CFONT;

//#define CFONT_NORMAL        0
//#define CFONT_APPLICATION   1
//#define CFONT_DRAW          2
  
CFONT *CFONT_create(const QFont &font, FONT_FUNC func = 0, void *object = 0);
void CFONT_set(FONT_FUNC func, void *font, void *object);
//CFONT *CFONT_create_control(CWIDGET *control);
double CFONT_size_real_to_virtual(double);
double CFONT_size_virtual_to_real(double);

#define SIZE_REAL_TO_VIRTUAL(_size) CFONT_size_real_to_virtual((double)(_size))
#define SIZE_VIRTUAL_TO_REAL(_size) CFONT_size_virtual_to_real((double)(_size))

#endif
