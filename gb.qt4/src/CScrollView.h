/***************************************************************************

	CScrollView.h

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

#ifndef __CSCROLLVIEW_H
#define __CSCROLLVIEW_H

#include "gambas.h"

#include <qevent.h>
//Added by qt3to4:
#include <QResizeEvent>
#include <QShowEvent>
//#include <QChildEvent>
#include <QScrollArea>

#include "CWidget.h"
#include "CContainer.h"

#ifndef __CSCROLLVIEW_CPP
extern GB_DESC CScrollViewDesc[];
#else

#define THIS    ((CSCROLLVIEW *)_object)
#define WIDGET  ((MyScrollView *)((CWIDGET *)_object)->widget)

#endif

class MyContents;

typedef
	struct {
		CWIDGET widget;
		MyContents *container;
		CARRANGEMENT arrangement;
		}
	CSCROLLVIEW;


class MyScrollView : public QScrollArea
{
	Q_OBJECT

public:

	MyScrollView(QWidget *);
	bool _noscroll;
	int _scrollx, _scrolly;
	bool _scroll_sent;

	void doUpdateScrollbars();
	int getScrollbar();

protected:

	void showEvent(QShowEvent *);
};


class MyContents : public MyContainer
{
	Q_OBJECT

public:
	MyContents(MyScrollView *scrollview);
	void checkAutoResizeLater();
	//void afterArrange();

public slots:
	void autoResize(void);
	void checkWidget(QWidget *);

protected:
	void childEvent(QChildEvent *);

private:
	void findRightBottom(void);

	QWidget *right;
	QWidget *bottom;
	MyScrollView *sw;
	bool timer;
	bool _mustfind;
	bool _dirty;
};

class CScrollView : public QObject
{
	Q_OBJECT

public:
	static CScrollView manager;

public slots:
	void scrolled(void);
};

void CSCROLLVIEW_arrange(void *_object);

#endif
