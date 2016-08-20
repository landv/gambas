/***************************************************************************

	CPictureBox.h

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

#ifndef __CPICTUREBOX_H
#define __CPICTUREBOX_H

#include "gambas.h"

#include <qlabel.h>

#include "CWidget.h"
#include "CPicture.h"
#include "CContainer.h"

#ifndef __CPICTUREBOX_CPP
extern GB_DESC CPictureBoxDesc[];
#else

#define WIDGET ((MyPictureBox *)((CWIDGET *)_object)->widget)
#define THIS ((CPICTUREBOX *)_object)

#endif

typedef
	struct {
		CWIDGET widget;
		CPICTURE *picture;
		}
	CPICTUREBOX;

class MyPictureBox : public QLabel
{
	Q_OBJECT

public:
	MyPictureBox(QWidget *parent);
	void updateBackground();
	void setAutoResize(bool);
	bool isAutoResize() const { return _autoresize; }
	void updateSize();
	void adjustSize();
	int border() const { return _border; }
	void setBorder(int border) { CCONTAINER_set_border(&_border, border, this); }

protected:
	virtual void setPalette(const QPalette &);
	virtual void resizeEvent(QResizeEvent *);
	virtual void paintEvent(QPaintEvent *);

private:
	bool _autoresize;
	char _border;
};

#endif
