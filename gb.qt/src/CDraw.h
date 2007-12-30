/***************************************************************************

  CDraw.h

  (c) 2000-2005 Beno� Minisini <gambas@users.sourceforge.net>

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

#ifndef __CDRAW_H
#define __CDRAW_H

#include "gambas.h"
#include <qfont.h>

#ifndef __CDRAW_C

extern GB_DESC CDrawDesc[];
extern GB_DESC CDrawClipDesc[];

#endif

#if 0
typedef
  struct {
    long (*GetBackground)(void);
    void (*SetBackground)(long);
    long (*GetForeground)(void);
    void (*SetForeground)(long);
    bool (*GetTransparent)(void);
    void (*SetTransparent)(bool);
    bool (*GetInvert)(void);
    void (*SetInvert)(bool);
    long (*GetLineWidth)(void);
    void (*SetLineWidth)(long);
    long (*GetLineStyle)(void);
    void (*SetLineStyle)(long);
    long (*GetFillColor)(void);
    void (*SetFillColor)(long);
    long (*GetFillStyle)(void);
    void (*SetFillStyle)(long);
    long (*GetFillX)(void);
    long (*GetFillY)(void);
    void (*SetFillOrg)(long, long);
    CFONT *(*GetFont)(void);
    void (*SetFont)(CFONT *);
    void (*DrawRect)(long, long, long, long);
    void (*DrawRoundRect)(long, long, long, long, double);
    void (*DrawEllipse)(long, long, long, long);
    void (*DrawPie)(long, long, long, long, double, double);
    void (*DrawLine)(long, long, long, long);
    void (*DrawPoint)(long, long);
		}
	DRAW_CLASS;
#endif

/*
enum
{
  DRAW_ON_NOTHING = 0,
  DRAW_ON_WINDOW = 1,
  DRAW_ON_PICTURE = 2,
  DRAW_ON_DRAWINGAREA = 3,
  DRAW_ON_WIDGET = 4,
  DRAW_ON_PRINTER = 5,
  DRAW_ON_ITEM = 6
};
*/

void DRAW_begin(void *device, QPainter *p, int w, int h, int dpi = 0);
void DRAW_end();
bool DRAW_must_resize_font();
int DRAW_status(void);
void DRAW_restore(int status);
void DRAW_set_font(QFont &f);

#endif
