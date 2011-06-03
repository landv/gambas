/***************************************************************************

  CDraw.h

  (c) 2004-2006 - Daniel Campos Fernández <dcamposf@gmail.com>

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

#ifndef __CDRAW_H
#define __CDRAW_H

#include "main.h"
#include "gdraw.h"
#include "gb.draw.h"

#ifndef __CDRAW_C

extern GB_DRAW_DESC DRAW_Interface;
extern DRAW_INTERFACE DRAW;

void DRAW_init();
void DRAW_begin(void *device);
void DRAW_end();
gDraw *DRAW_get_current();
void* DRAW_get_drawable(void *);
void* DRAW_get_style(void *);
int   DRAW_get_state(void *);
int   DRAW_get_shadow(void *);
void  DRAW_set_state(void *,int);
void DRAW_set_shadow(void *,int);

#endif



#endif

