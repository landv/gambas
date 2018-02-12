/***************************************************************************

	CGLarea.cpp

	(c) 2000-2017 Benoît Minisini <g4mba5@gmail.com>

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

#define __CGLAREA_CPP

#include "gb.image.h"

#include "CGLarea.h"

DECLARE_EVENT(EVENT_Open);
DECLARE_EVENT(EVENT_Draw);
DECLARE_EVENT(EVENT_Resize);

BEGIN_METHOD(CGLAREA_new, GB_OBJECT parent)

	/*if (!QGLFormat::hasOpenGL())
	{
		GB.Error( "This system has no OpenGL support");
		return;
	}*/

	GLarea *area = new GLarea(QT.GetContainer(VARG(parent)), THIS);
	QT.InitWidget(area, _object, false);
	area->show();

END_METHOD

#if 0
BEGIN_METHOD_VOID(CGLAREA_update)

	WIDGET->updateGL();

END_METHOD
#endif

#if 0
BEGIN_METHOD_VOID(CGLAREA_select)

	WIDGET->makeCurrent();
	// really needed ?
	GL.Init();

END_METHOD
#endif

#if 0
BEGIN_METHOD(CGLAREA_text, GB_STRING text; GB_INTEGER x; GB_INTEGER y)

	QString text;
	int x, y;
	#ifdef GL_LIGHTING
	GLboolean _LIGHTING = glIsEnabled(GL_LIGHTING);
	#endif
	GLboolean _TEXTURE_2D = glIsEnabled(GL_TEXTURE_2D);

	#ifdef GL_LIGHTING
	if (_LIGHTING)
		glDisable(GL_LIGHTING);
	#endif
	if (_TEXTURE_2D)
		glDisable(GL_TEXTURE_2D);

	text = QSTRING_ARG(text);
	x = VARG(x);
	y = VARG(y);

	WIDGET->renderText(x, y, text, WIDGET->font());

	#ifdef GL_LIGHTING
	if (_LIGHTING)
		glEnable(GL_LIGHTING);
	#endif
	if (_TEXTURE_2D)
		glEnable(GL_TEXTURE_2D);

END_METHOD
#endif

/**************************************************************************/

GB_DESC CGlareaDesc[] =
{
	GB_DECLARE("GLArea", sizeof(CGLAREA)), GB_INHERITS("Control"),

	GB_METHOD("_new", NULL, CGLAREA_new, "(Parent)Container;"),

	GB_CONSTANT("_Group", "s", "Special"),

	GB_EVENT("Open", NULL, NULL, &EVENT_Open),
	GB_EVENT("Draw", NULL, NULL, &EVENT_Draw),
	GB_EVENT("Resize", NULL, NULL, &EVENT_Resize),

	GB_END_DECLARE
};

/* class GLarea */

GLarea::GLarea(QWidget *parent, CGLAREA *object): QOpenGLWidget(parent)
{
	setFocusPolicy(Qt::WheelFocus);
	setAttribute(Qt::WA_InputMethodEnabled, true);
	_area = object;
};


void GLarea::initializeGL()
{
	GL.Init();
	GB.Raise(_area, EVENT_Open, 0);
}

void GLarea::paintGL()
{
	/*static bool CleanupOnFirstShow = 0;
	
	uint color;
	
	if (!CleanupOnFirstShow)
	{
		// clear to avoid garbage
		color = QT.GetBackgroundColor(_area);
		if (color == GB_COLOR_DEFAULT)
			color = 0;

		qglClearColor(QColor((QRgb)color));
		glClear(GL_COLOR_BUFFER_BIT);

		CleanupOnFirstShow = true;
	}*/
	
	GB.Raise(_area, EVENT_Draw, 0);
}

void GLarea::resizeGL(int w, int h)
{
	GB.Raise(_area, EVENT_Resize, 0);
}

