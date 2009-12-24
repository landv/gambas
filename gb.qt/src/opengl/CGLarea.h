/***************************************************************************

  CGLarea.h

  (c) 2006 Laurent Carlier <lordheavy@users.sourceforge.net>

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

#ifndef __CGLAREA_H
#define __CGLAREA_H

#include "gambas.h"
#include "main.h"
#include "../gb.qt.h"

#include <GL/glew.h>
#include <qgl.h>
#include "gl.h"

typedef
  struct {
    QT_WIDGET widget;
   }
  CGLAREA;

#ifndef __CGLAREA_CPP
extern GB_DESC CGlareaDesc[];
#else

#define THIS    ((CGLAREA *)_object)
#define WIDGET  ((GLarea *)((QT_WIDGET *)_object)->widget)

#define CGLAREA_PROPERTIES QT_WIDGET_PROPERTIES

#endif /* __CGLAREA_CPP */

class GLarea : public QGLWidget
{
Q_OBJECT
public:
	GLarea(QWidget *parent,CGLAREA *object, QGLWidget *sharing = 0);
	~GLarea() {};
protected:
	void initializeGL();
	void paintGL();
	void resizeGL(int w, int h);
private:
	CGLAREA *_area;
};

#endif
