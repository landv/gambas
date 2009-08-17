/***************************************************************************

  CDraw.h

  (c) 2000-2009 Benoît Minisini <gambas@users.sourceforge.net>

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
  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

***************************************************************************/

#ifndef __CDRAW_H
#define __CDRAW_H

#include "gambas.h"
#include "gb.draw.h"

#include <QPainter>
#include <QString>

#ifndef __CDRAW_C

extern GB_DRAW_DESC DRAW_Interface;

#endif

//bool DRAW_must_resize_font();
//int DRAW_status(void);
//void DRAW_restore(int status);
void DRAW_init();
void DRAW_begin(void *device);
void DRAW_end();
QPainter *DRAW_get_current();
void DRAW_rich_text(QPainter *p, int x, int y, int w, int h, int align, QString &text, QPainter *p2 = 0);
void DRAW_aligned_pixmap(QPainter *p, const QPixmap &pix, int x, int y, int w, int h, int align);

#endif
