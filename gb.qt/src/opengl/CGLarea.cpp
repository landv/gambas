/***************************************************************************

  CGLarea.h

  OpenGL widget support for gb.qt

  (c) 2006 Laurent Carlier <lordheavy@users.sourceforge.net>
           Benoît Minisini <gambas@users.sourceforge.net>

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

#define __CGLAREA_CPP

#include "main.h"
#include "gambas.h"
#include "CGLarea.h"

#include <qgl.h>

DECLARE_EVENT(EVENT_Open);
DECLARE_EVENT(EVENT_Draw);
DECLARE_EVENT(EVENT_Resize);

BEGIN_METHOD(CGLAREA_new, GB_OBJECT parent)

	if (!QGLFormat::hasOpenGL())
	{
		GB.Error( "This system has no OpenGL support");
		return;
	}

	GLarea *area = new GLarea(QT.GetContainer(VARG(parent)), THIS);

	QT.InitWidget(area, _object);
	area->show();

END_METHOD

BEGIN_METHOD_VOID(CGLAREA_update)

	WIDGET->updateGL();

END_METHOD

BEGIN_METHOD_VOID(CGLAREA_select)

	WIDGET->makeCurrent();

END_METHOD

/**************************************************************************/

GB_DESC CGlareaDesc[] =
{
  GB_DECLARE("GLarea", sizeof(CGLAREA)), GB_INHERITS("Control"),

  GB_METHOD("_new", NULL, CGLAREA_new, "(Parent)Container;"),
  GB_METHOD("Update", NULL, CGLAREA_update, NULL),
  GB_METHOD("Refresh", NULL, CGLAREA_update, NULL),
  GB_METHOD("Select", NULL, CGLAREA_select, NULL),

  GB_CONSTANT("_Properties", "s", CGLAREA_PROPERTIES),

  GB_EVENT("Open", NULL, NULL, &EVENT_Open),
  GB_EVENT("Draw", NULL, NULL, &EVENT_Draw),
  GB_EVENT("Resize", NULL, NULL, &EVENT_Resize),

  GB_END_DECLARE
};

/* class GLarea */

void GLarea::initializeGL()
{
	// clear to avoid garbage
	qglClearColor(black);
	GB.Raise(_area, EVENT_Open, 0);
}

void GLarea::paintGL()
{
	GB.Raise(_area, EVENT_Draw, 0);
}

void GLarea::resizeGL(int w, int h)
{
	GB.Raise(_area, EVENT_Resize, 0);
}
