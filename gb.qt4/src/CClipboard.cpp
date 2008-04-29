/***************************************************************************

  CClipboard.cpp

  (c) 2000-2007 Benoit Minisini <gambas@users.sourceforge.net>

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
#include <qevent.h>
#include <qcolor.h>
//Added by qt3to4:
#include <QDragMoveEvent>
#include <QDropEvent>
#include <QDragEnterEvent>
#include <QMimeData>

#include "CWidget.h"
#include "CImage.h"
#include "CClipboard.h"
#include "CTreeView.h"

CDRAG_INFO CDRAG_info = { 0 };
bool CDRAG_dragging = false;

static void *CLASS_Image;

//static CCURSOR *_cursor = 0;
static CPICTURE *_picture = 0;
static int _picture_x = -1;
static int _picture_y = -1;

static int get_type(const QMimeData *src)
{
  if (src->formats().indexOf(QRegExp("text/.*")) >= 0)
    return 1;
  else if (src->hasImage())
    return 2;
  else
    return 0;
}

static QString get_format(const QMimeData *src, int i = 0, bool charset = false)
{
  QString format = src->formats().at(i);

	if (!charset)
	{
		int pos = format.indexOf(';');
		if (pos >= 0)
			format = format.left(pos);
	}

  return format;
}

static void get_formats(const QMimeData *src, GB_ARRAY array)
{
  int i, j;
  QStringList formats = src->formats();
  QString fmt;
  char *str;
  
  for (i = 0; i < formats.count(); i++)
  {
    fmt = get_format(src, i, true); 
    if (!fmt[0].isLower())
      continue;
    for (j = 0; j < GB.Array.Count(array); j++)
    {
      if (strcasecmp(fmt.toUtf8().data(), *((char **)GB.Array.Get(array, j))) == 0)
        break;
    }
    if (j < GB.Array.Count(array))
      continue;
    //fmt = get_format(src, i);
		GB.NewString(&str, fmt.toUtf8().data(), 0);
		*((char **)GB.Array.Add(array)) = str;
  }
}

static void paste(const QMimeData *data, const char *fmt)
{
  CIMAGE *img;
  QString format = fmt;

	if (!data->hasFormat(format))
	{
		GB.ReturnNull();
		return;
	}

	switch(get_type(data))
	{
		case 1:
			GB.ReturnNewZeroString(data->data(format).data());
			break;
		
		case 2:
			GB.New(POINTER(&img), GB.FindClass("Image"), 0, 0);
			*img->image = qvariant_cast<QImage>(data->imageData());
			img->image->convertToFormat(QImage::Format_ARGB32);
			GB.ReturnObject(img);
			break;
		
		default:
			GB.ReturnNull();
	}
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

  GB.ReturnNewZeroString(TO_UTF8(get_format(QApplication::clipboard()->mimeData())));

END_PROPERTY


BEGIN_PROPERTY(CCLIPBOARD_formats)

  GB_ARRAY array;
  
  GB.Array.New(&array, GB_T_STRING, 0);
  get_formats(QApplication::clipboard()->mimeData(), array);
  GB.ReturnObject(array);

END_PROPERTY


BEGIN_PROPERTY(CCLIPBOARD_type)

  GB.ReturnInteger(get_type(QApplication::clipboard()->mimeData()));

END_PROPERTY


BEGIN_METHOD(CCLIPBOARD_copy, GB_VARIANT data; GB_STRING format)

  QString format;
  QMimeData *data = new QMimeData();

  if (VARG(data).type == GB_T_STRING)
  {
    if (MISSING(format))
      format = "plain";
    else
    {
      format = TO_QSTRING(GB.ToZeroString(ARG(format)));
      if (format.left(5) != "text/")
        goto _BAD_FORMAT;
      if (format.length() == 5)
        goto _BAD_FORMAT;
    }

    data->setData(format, QByteArray(VARG(data)._string.value, GB.StringLength(VARG(data)._string.value)));
    QApplication::clipboard()->setMimeData(data);
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

  paste(QApplication::clipboard()->mimeData(), MISSING(format) ?  NULL : GB.ToZeroString(ARG(format)));

END_METHOD

GB_DESC CClipboardDesc[] =
{
  GB_DECLARE("Clipboard", 0), GB_VIRTUAL_CLASS(),

  GB_CONSTANT("None", "i", 0),
  GB_CONSTANT("Text", "i", 1),
  GB_CONSTANT("Image", "i", 2),

  GB_STATIC_METHOD("_init", NULL, CCLIPBOARD_init, NULL),

  GB_STATIC_METHOD("Clear", NULL, CCLIPBOARD_clear, NULL),

  GB_STATIC_PROPERTY_READ("Format", "s", CCLIPBOARD_format),
  GB_STATIC_PROPERTY_READ("Formats", "String[]", CCLIPBOARD_formats),
  GB_STATIC_PROPERTY_READ("Type", "i", CCLIPBOARD_type),

  GB_STATIC_METHOD("Copy", NULL, CCLIPBOARD_copy, "(Data)v[(Format)s]"),
  GB_STATIC_METHOD("Paste", "v", CCLIPBOARD_paste, "[(Format)s]"),

  GB_END_DECLARE
};

/** Drag frame ***********************************************************/

//MyDragFrame::MyDragFrame() : QWidget(0, 0, Qt::WType_TopLevel | Qt::WStyle_Customize | Qt::WStyle_NoBorder | Qt::WStyle_StaysOnTop | Qt::WX11BypassWM)
MyDragFrame::MyDragFrame(QWidget *parent) : 
  QWidget(parent, Qt::FramelessWindowHint | Qt::X11BypassWindowManagerHint)
{
	QPalette pal(palette());
	pal.setColor(QPalette::Window, Qt::black);
	setPalette(pal);
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
	QWidget *wid;
	//QPoint p = wid->mapToGlobal(QPoint(0, 0));
	int i;
	
	if (GB.Is(control, CLASS_Container))
	 wid = QCONTAINER(control);
 else
   wid = QWIDGET(control);
	
	if (w <= 0 || h <= 0)
	{
		x = y = 0;
		w = wid->width();
		h = wid->height();
	}
	
	//x += p.x();
	//y += p.y();
	
	if (!_frame_visible)
	{
		for (i = 0; i < 4; i++)
			_frame[i] = new MyDragFrame(wid);
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

static void post_exit_drag(intptr_t param)
{
	CDRAG_dragging = false;
}

void CDRAG_drag(CWIDGET *source, GB_VARIANT_VALUE *data, GB_STRING *fmt)
{
  QDrag *drag;
  QMimeData *mimeData;
  QString format;
  CIMAGE *img;

  if (GB.CheckObject(source))
    return;

	if (CDRAG_dragging)
	{
		GB.Error("Undergoing drag");
		return;
	}

	drag = new QDrag(source->widget);
	mimeData = new QMimeData();

  if (data->type == GB_T_STRING)
  {
    if (fmt == NULL)
      format = "text/plain";
    else
    {
      format = TO_QSTRING(GB.ToZeroString(fmt));
      if (format.left(5) != "text/")
        goto _BAD_FORMAT;
      if (format.length() == 5)
        goto _BAD_FORMAT;
    }
    
    mimeData->setData(format, QByteArray(data->_string.value, GB.StringLength(data->_string.value)));
  }
  else if (data->type >= GB_T_OBJECT && GB.Is(data->_object.value, CLASS_Image))
  {
    if (fmt)
      goto _BAD_FORMAT;

    img = (CIMAGE *)data->_object.value;

		mimeData->setImageData(*(img->image));
  }
  else
    goto _BAD_FORMAT;

  drag->setMimeData(mimeData);
  
  if (_picture)
  {
   	drag->setPixmap(*(_picture->pixmap));
		if (_picture_x >= 0 && _picture_y >= 0)
    	drag->setHotSpot(QPoint(_picture_x, _picture_y));
  }

	CDRAG_dragging = true;
	
  drag->exec();
  
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
			e->acceptProposedAction();
		else
			e->ignore();
		return true;
	}
	
	CDRAG_clear(true);
	CDRAG_info.event = e;

	cancel = GB.Raise(control, EVENT_Drag, 0);
	
	CDRAG_clear(false);
	
	if (cancel)
		e->ignore();
	else
		e->acceptProposedAction();
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
			e->acceptProposedAction();
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
	CDRAG_info.event = e;

	p = e->pos();
	p = w->mapTo(QWIDGET(control), p);
	CDRAG_info.x = p.x();
	CDRAG_info.y = p.y();

	cancel = GB.Raise(control, EVENT_DragMove, 0);
	if (cancel)
		e->ignore();
	else
		e->acceptProposedAction();

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
	CDRAG_info.event = e;

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

  GB.Unref(POINTER(&_picture));

END_METHOD

BEGIN_PROPERTY(CDRAG_icon)

  if (READ_PROPERTY)
    GB.ReturnObject(_picture);
  else
    GB.StoreObject(PROP(GB_OBJECT), POINTER(&_picture));

END_PROPERTY

BEGIN_PROPERTY(CDRAG_icon_x)

  if (READ_PROPERTY)
    GB.ReturnInteger(_picture_x);
  else
    _picture_x = VPROP(GB_INTEGER);

END_PROPERTY

BEGIN_PROPERTY(CDRAG_icon_y)

  if (READ_PROPERTY)
    GB.ReturnInteger(_picture_y);
  else
    _picture_y = VPROP(GB_INTEGER);

END_PROPERTY

#define CHECK_VALID() \
  if (!CDRAG_info.valid) \
  { \
    GB.Error("No drag data"); \
    return; \
  }


BEGIN_PROPERTY(CDRAG_type)

  CHECK_VALID();

  GB.ReturnInteger(get_type(CDRAG_info.event->mimeData()));

END_PROPERTY


BEGIN_PROPERTY(CDRAG_format)

  CHECK_VALID();

  GB.ReturnNewZeroString(TO_UTF8(get_format(CDRAG_info.event->mimeData())));

END_PROPERTY


BEGIN_PROPERTY(CDRAG_formats)

  GB_ARRAY array;
  
  CHECK_VALID();
  
  GB.Array.New(&array, GB_T_STRING, 0);
  get_formats(CDRAG_info.event->mimeData(), array);
  GB.ReturnObject(array);

END_PROPERTY


BEGIN_PROPERTY(CDRAG_data)

  if (!CDRAG_info.valid)
  {
    GB.ReturnNull();
    return;
  }

  paste(CDRAG_info.event->mimeData(), NULL);

END_PROPERTY


BEGIN_METHOD(CDRAG_paste, GB_STRING format)

  if (!CDRAG_info.valid)
  {
    GB.ReturnNull();
    return;
  }

  paste(CDRAG_info.event->mimeData(), MISSING(format) ?  NULL : GB.ToZeroString(ARG(format)));

END_METHOD


BEGIN_PROPERTY(CDRAG_action)

  CHECK_VALID();

  switch(CDRAG_info.event->dropAction())
  {
    case Qt::LinkAction:
      GB.ReturnInteger(1);
      break;

    case Qt::MoveAction:
      GB.ReturnInteger(2);
      break;

    default:
      GB.ReturnInteger(0);
      break;
  }

END_PROPERTY


BEGIN_PROPERTY(CDRAG_source)

  CHECK_VALID();

  GB.ReturnObject(CWidget::get(CDRAG_info.event->source()));

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


GB_DESC CDragDesc[] =
{
  GB_DECLARE("Drag", 0), GB_VIRTUAL_CLASS(),

  GB_CONSTANT("None", "i", 0),
  GB_CONSTANT("Text", "i", 1),
  GB_CONSTANT("Image", "i", 2),

  GB_CONSTANT("Copy", "i", 0),
  GB_CONSTANT("Link", "i", 1),
  GB_CONSTANT("Move", "i", 2),

  GB_STATIC_PROPERTY("Icon", "Picture", CDRAG_icon),
  GB_STATIC_PROPERTY("IconX", "i", CDRAG_icon_x),
  GB_STATIC_PROPERTY("IconY", "i", CDRAG_icon_y),

  GB_STATIC_PROPERTY_READ("Data", "v", CDRAG_data),
  GB_STATIC_PROPERTY_READ("Format", "s", CDRAG_format),
  GB_STATIC_PROPERTY_READ("Formats", "String[]", CDRAG_formats),
  GB_STATIC_PROPERTY_READ("Type", "i", CDRAG_type),
  GB_STATIC_PROPERTY_READ("Action", "i", CDRAG_action),
  GB_STATIC_PROPERTY_READ("Source", "Control", CDRAG_source),
  GB_STATIC_PROPERTY_READ("X", "i", CDRAG_x),
  GB_STATIC_PROPERTY_READ("Y", "i", CDRAG_y),
  GB_STATIC_PROPERTY_READ("Pending", "b", CDRAG_pending),

  GB_STATIC_METHOD("_call", NULL, CDRAG_call, "(Source)Control;(Data)v[(Format)s]"),
  GB_STATIC_METHOD("_exit", NULL, CDRAG_exit, NULL),
  GB_STATIC_METHOD("Show", NULL, CDRAG_show, "(Control)Control;[(X)i(Y)i(Width)i(Height)i]"),
  GB_STATIC_METHOD("Hide", NULL, CDRAG_hide, NULL),
  GB_STATIC_METHOD("Paste", "v", CDRAG_paste, "[(Format)s]"),

  GB_END_DECLARE
};



