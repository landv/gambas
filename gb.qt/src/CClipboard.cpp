/***************************************************************************

  CClipboard.cpp

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


#define __CCLIPBOARD_CPP


#include "gambas.h"

#include <qapplication.h>
#include <qmime.h>
#include <qclipboard.h>
#include <qimage.h>
#include <qpicture.h>
#include <qdragobject.h>
#include <qcstring.h>
#include <qevent.h>
#include <qcolor.h>

#include "CWidget.h"
#include "CImage.h"
#include "CClipboard.h"
#include "CTreeView.h"

CDRAG_INFO CDRAG_info = { 0 };
bool CDRAG_dragging = false;

static void *CLASS_Image;

//static CCURSOR *_cursor = 0;
static CPICTURE *_picture = 0;


static int get_type(QMimeSource *src)
{
  if (QTextDrag::canDecode(src))
    return 1;
  else if (QImageDrag::canDecode(src))
    return 2;
  else
    return 0;
}

static QCString get_format(QMimeSource *src)
{
  QCString format = src->format();
  int pos;

  pos = format.find(';');
  if (pos >= 0)
    format = format.left(pos);

  return format;
}

static void paste(QMimeSource *data, char *fmt)
{
  CIMAGE *img;

  if (fmt)
  {
    if (get_format(data) != QCString(fmt))
    {
      GB.ReturnNull();
      return;
    }
  }

  if (QTextDrag::canDecode(data))
  {
    QString text;

    QTextDrag::decode(data, text);
    GB.ReturnNewZeroString(text.latin1());
  }
  else if (QImageDrag::canDecode(data))
  {
    GB.New((void **)&img, GB.FindClass("Image"), 0, 0);
    QImageDrag::decode(data, *(img->image));
    img->image->convertDepth(32);
    GB.ReturnObject(img);
  }
  else
    GB.ReturnNull();
}



/***************************************************************************

  Clipboard

***************************************************************************/

BEGIN_METHOD_VOID(CCLIPBOARD_init)

  CLASS_Image = GB.FindClass("Image");

END_METHOD

BEGIN_METHOD_VOID(CCLIPBOARD_clear)

  QApplication::clipboard()->clear();

END_METHOD


BEGIN_PROPERTY(CCLIPBOARD_format)

  GB.ReturnNewZeroString(get_format(QApplication::clipboard()->data()));

END_PROPERTY


BEGIN_PROPERTY(CCLIPBOARD_type)

  GB.ReturnInteger(get_type(QApplication::clipboard()->data()));

END_PROPERTY


BEGIN_METHOD(CCLIPBOARD_copy, GB_VARIANT data; GB_STRING format)

  QCString format;

  if (VARG(data).type == GB_T_STRING)
  {
    QTextDrag *drag = new QTextDrag();

    if (MISSING(format))
      format = "plain";
    else
    {
      format = GB.ToZeroString(ARG(format));
      if (format.left(5) != "text/")
        goto _BAD_FORMAT;
      format = format.mid(5);
      if (format.length() == 0)
        goto _BAD_FORMAT;
    }

    drag->setText(VARG(data)._string.value);
    drag->setSubtype(format);

    QApplication::clipboard()->setData(drag);
  }
  else if (VARG(data).type >= GB_T_OBJECT && GB.Is(VARG(data)._object.value, CLASS_Image))
  {
    CIMAGE *img;

    if (!MISSING(format))
      goto _BAD_FORMAT;

    img = (CIMAGE *)VARG(data)._object.value;

    QApplication::clipboard()->setImage(*(img->image));
  }
  else
    goto _BAD_FORMAT;

  return;

_BAD_FORMAT:

  GB.Error("Bad clipboard format");

END_METHOD


BEGIN_METHOD(CCLIPBOARD_paste, GB_STRING format)

  paste(QApplication::clipboard()->data(), MISSING(format) ?  NULL : GB.ToZeroString(ARG(format)));

END_METHOD

#include "CClipboard_desc.h"

/** Drag frame ***********************************************************/

MyDragFrame::MyDragFrame() : QWidget(0, 0, Qt::WType_TopLevel | Qt::WStyle_Customize | Qt::WStyle_NoBorder | Qt::WStyle_StaysOnTop | Qt::WX11BypassWM)
{
	setPaletteBackgroundColor(Qt::black);
}

/*MyDragFrame::paintEvent(QPaintEvent *e)
{
	
}*/

static QWidget *_frame[4] = { 0 };
static bool _frame_visible = false;
static CWIDGET *_frame_control =0;

static void hide_frame(CWIDGET *control)
{
	int i;
	
	if (!_frame_visible)
		return;
		
	if (control && control != _frame_control)
		return;
		
	for (i = 0; i < 4; i++)
		delete _frame[i];
		
	_frame_visible = false;
}

void CDRAG_hide_frame(CWIDGET *control)
{
	hide_frame(control);
}

static void show_frame(CWIDGET *control, int x, int y, int w, int h)
{
	QWidget *wid = control->widget;
	QPoint p = wid->mapToGlobal(QPoint(0, 0));
	int i;
	
	if (w <= 0 || h <= 0)
	{
		x = y = 0;
		w = wid->width();
		h = wid->height();
	}
	
	x += p.x();
	y += p.y();
	
	if (!_frame_visible)
	{
		for (i = 0; i < 4; i++)
			_frame[i] = new MyDragFrame();
	}
	
	//x -= 2;
	//y -= 2;
	//w += 4;
	//h += 4;
	if (w < 2 || h < 2)
		return;
	
	_frame[0]->setGeometry(x, y, w, 2);
	_frame[1]->setGeometry(x, y, 2, h);
	_frame[2]->setGeometry(x + w - 2, y, 2, h);
	_frame[3]->setGeometry(x, y + h - 2, w, 2);
	
	for (i = 0; i < 4; i++)
		_frame[i]->show();
	
	_frame_control = control;
	
	_frame_visible = true;
}


/** Drag *****************************************************************/

void CDRAG_clear(bool valid)
{
  if (valid)
    CDRAG_info.valid++;
  else
    CDRAG_info.valid--;

  if (CDRAG_info.valid == 0)
    CLEAR(&CDRAG_info);
}

static void post_exit_drag(long param)
{
	CDRAG_dragging = false;
}

void CDRAG_drag(CWIDGET *source, GB_VARIANT_VALUE *data, GB_STRING *fmt)
{
  QDragObject *drag;
  QCString format;
  CIMAGE *img;

  if (GB.CheckObject(source))
    return;

	if (CDRAG_dragging)
	{
		GB.Error("Undergoing drag");
		return;
	}

  if (data->type == GB_T_STRING)
  {
     drag = new QTextDrag(source->widget);

    if (fmt == NULL)
      format = "plain";
    else
    {
      format = GB.ToZeroString(fmt);
      if (format.left(5) != "text/")
        goto _BAD_FORMAT;
      format = format.mid(5);
      if (format.length() == 0)
        goto _BAD_FORMAT;
    }

    ((QTextDrag *)drag)->setText(data->_string.value);
    ((QTextDrag *)drag)->setSubtype(format);
  }
  else if (data->type >= GB_T_OBJECT && GB.Is(data->_object.value, CLASS_Image))
  {
    if (fmt)
      goto _BAD_FORMAT;

    drag = new QImageDrag(source->widget);

    img = (CIMAGE *)data->_object.value;

    ((QImageDrag *)drag)->setImage(*(img->image));
  }
  else
    goto _BAD_FORMAT;

  if (_picture)
  {
    //QPoint p(_cursor->x, _cursor->y);
    //pict = _cursor->picture;

    drag->setPixmap(*(_picture->pixmap));
  }

	CDRAG_dragging = true;
  drag->drag();
  hide_frame(NULL);
  GB.Post((GB_POST_FUNC)post_exit_drag, 0);

  return;

_BAD_FORMAT:

  GB.Error("Bad drag format");
}


bool CDRAG_drag_enter(QWidget *w, CWIDGET *control, QDropEvent *e)
{
	bool cancel;

	//qDebug("CDRAG_drag_enter: (%s %p) %d", GB.GetClassName(control), control, w->inherits("QListView"));

	// Hack for QScrollView
	if (CWIDGET_test_flag(control, WF_SCROLLVIEW) && QWIDGET(control)->inherits("MyListView"))
		((MyListView *)QWIDGET(control))->contentsDragEnterEvent((QDragEnterEvent *)e);

	if (!GB.CanRaise(control, EVENT_Drag))
	{
		if (!GB.CanRaise(control, EVENT_DragMove) && GB.CanRaise(control, EVENT_Drop))
			e->acceptAction();
		else
			e->ignore();
		return true;
	}
	
	CDRAG_clear(true);
	CDRAG_info.drop = e;

	cancel = GB.Raise(control, EVENT_Drag, 0);
	
	CDRAG_clear(false);
	
	if (cancel)
		e->ignore();
	else
		e->acceptAction(true);
	return cancel;
}


bool CDRAG_drag_move(QWidget *w, CWIDGET *control, QDropEvent *e)
{
	bool accepted;
	bool cancel;
	QPoint p;

	//qDebug("CDRAG_drag_move: widget = %p  control = %p", w, control);
	//qDebug("CDRAG_drag_move: (%s %p) %d", GB.GetClassName(control), control, w->inherits("QListView"));

	/*if (!e->isAccepted())
		return true;*/

	// Hack for QScrollView
	if (CWIDGET_test_flag(control, WF_SCROLLVIEW) && QWIDGET(control)->inherits("MyListView"))
	{
		accepted = e->isAccepted();
		((MyListView *)QWIDGET(control))->contentsDragMoveEvent((QDragMoveEvent *)e);
		if (accepted)
			e->acceptAction();
		else
			e->ignore();
	}

	if (!GB.CanRaise(control, EVENT_DragMove))
	{
		/*if (GB.CanRaise(control, EVENT_Drop))
			e->accept();
		else
			e->ignore();*/
		return true;
	}

	CDRAG_clear(true);
	CDRAG_info.drop = e;

	p = e->pos();
	p = w->mapTo(QWIDGET(control), p);
	CDRAG_info.x = p.x();
	CDRAG_info.y = p.y();

	cancel = GB.Raise(control, EVENT_DragMove, 0);
	if (cancel)
		CDRAG_info.drop->ignore();
	else
		CDRAG_info.drop->acceptAction(true);

	CDRAG_clear(false);
	return cancel;
}

void CDRAG_drag_drop(QWidget *w, CWIDGET *control, QDropEvent *e)
{
	QPoint p;

	//hide_frame();

	if (!GB.CanRaise(control, EVENT_Drop))
		return;

	// Hack for QScrollView
	if (CWIDGET_test_flag(control, WF_SCROLLVIEW) && QWIDGET(control)->inherits("MyListView"))
		((MyListView *)QWIDGET(control))->contentsDropEvent((QDragMoveEvent *)e);
	
	CDRAG_clear(true);
	CDRAG_info.drop = e;

	p = e->pos();
	p = w->mapTo(QWIDGET(control), p);
	CDRAG_info.x = p.x();
	CDRAG_info.y = p.y();

	GB.Raise(control, EVENT_Drop, 0);

	CDRAG_clear(false);
}



BEGIN_METHOD(CDRAG_call, GB_OBJECT source; GB_VARIANT data; GB_STRING format)

  CDRAG_drag((CWIDGET *)VARG(source), &VARG(data), MISSING(format) ? NULL : ARG(format));

END_METHOD


BEGIN_METHOD_VOID(CDRAG_exit)

  GB.Unref((void **)&_picture);

END_METHOD

BEGIN_PROPERTY(CDRAG_picture)

  if (READ_PROPERTY)
    GB.ReturnObject(_picture);
  else
    GB.StoreObject(PROP(GB_OBJECT), (void **)&_picture);

END_PROPERTY

#define CHECK_VALID() \
  if (!CDRAG_info.valid) \
  { \
    GB.Error("No drag data"); \
    return; \
  }


BEGIN_PROPERTY(CDRAG_type)

  CHECK_VALID();

  GB.ReturnInteger(get_type(CDRAG_info.drop));

END_PROPERTY


BEGIN_PROPERTY(CDRAG_format)

  CHECK_VALID();

  GB.ReturnNewZeroString(get_format(CDRAG_info.drop));

END_PROPERTY


BEGIN_PROPERTY(CDRAG_data)

  if (!CDRAG_info.valid)
  {
    GB.ReturnNull();
    return;
  }

  paste(CDRAG_info.drop, NULL);

END_PROPERTY


BEGIN_PROPERTY(CDRAG_action)

  CHECK_VALID();

  switch(CDRAG_info.drop->action())
  {
    case QDropEvent::Link:
      GB.ReturnInteger(1);
      break;

    case QDropEvent::Move:
      GB.ReturnInteger(2);
      break;

    default:
      GB.ReturnInteger(0);
      break;
  }

END_PROPERTY


BEGIN_PROPERTY(CDRAG_source)

  CHECK_VALID();

  GB.ReturnObject(CWidget::get(CDRAG_info.drop->source()));

END_PROPERTY


BEGIN_PROPERTY(CDRAG_x)

  CHECK_VALID();

  GB.ReturnInteger(CDRAG_info.x);

END_PROPERTY


BEGIN_PROPERTY(CDRAG_y)

  CHECK_VALID();

  GB.ReturnInteger(CDRAG_info.y);

END_PROPERTY

BEGIN_PROPERTY(CDRAG_pending)

  GB.ReturnBoolean(CDRAG_dragging);

END_PROPERTY

BEGIN_METHOD(CDRAG_show, GB_OBJECT control; GB_INTEGER x; GB_INTEGER y; GB_INTEGER w; GB_INTEGER h)

	if (GB.CheckObject(VARG(control)))
		return;
		
	if (!CDRAG_dragging)
	{
		GB.Error("No undergoing drag");
		return;
	}

	if (MISSING(x) || MISSING(y) || MISSING(w) || MISSING(h))
		show_frame((CWIDGET *)VARG(control), 0, 0, -1, -1);
	else
		show_frame((CWIDGET *)VARG(control), VARG(x), VARG(y), VARG(w), VARG(h));

END_METHOD

BEGIN_METHOD_VOID(CDRAG_hide)

	hide_frame(NULL);

END_METHOD

#include "CDrag_desc.h"

