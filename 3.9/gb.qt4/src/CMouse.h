/***************************************************************************

	CMouse.h

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

#ifndef __CMOUSE_H
#define __CMOUSE_H

#include <QCursor>
#include <QMouseEvent>

#include "gambas.h"

#include "CPicture.h"

typedef
	struct {
		int valid;
		int x;
		int y;
		int sx;
		int sy;
		Qt::MouseButton button;
		Qt::MouseButtons state;
		Qt::KeyboardModifiers modifier;
		int orientation;
		int delta;
		int screenX;
		int screenY;
		int dx;
		int dy;
		}
	MOUSE_INFO;
	
typedef
	struct {
		double tx;
		double ty;
		int xtilt;
		int ytilt;
		int type;
		double pressure;
		double rotation;
	}
	POINTER_INFO;

#ifndef __CMOUSE_CPP
extern GB_DESC CMouseDesc[];
extern GB_DESC CCursorDesc[];
extern GB_DESC CPointerDesc[];

extern MOUSE_INFO MOUSE_info;
extern POINTER_INFO POINTER_info;
#else

#define THIS ((CCURSOR *)_object)

#endif

typedef
	struct _CCURSOR {
		GB_BASE ob;
		int x;
		int y;
		QCursor *cursor;
		}
	CCURSOR;

#define CMOUSE_DEFAULT (-1)
#define CMOUSE_CUSTOM  (-2)


// ### QT_WIDGET_PROPERTIES must be modified with this constant

#define MOUSE_CONSTANTS \
	"Mouse,Default,Blank,Arrow,Cross,Wait,Text,SizeAll,SizeH,SizeV,SizeN,SizeS,SizeW,SizeE,SizeNWSE," \
	"SizeNESW,SplitH,SplitV,Pointing"

void CMOUSE_clear(int valid);
void CMOUSE_reset_translate(void);

#endif
