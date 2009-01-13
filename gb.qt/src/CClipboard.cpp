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
void *CDRAG_destination = 0;

static void *CLASS_Image;

//static CCURSOR *_cursor = 0;
static CPICTURE *_picture = 0;
static int _picture_x = -1;
static int _picture_y = -1;

static QDragObject *_current_drag = 0;

CDragManager CDragManager::manager;

void CDragManager::destroy(QObject *o)
{
	if (_current_drag == o)
		_current_drag = 0;
}

static int get_type(QMimeSource *src)
{
  if (QTextDrag::canDecode(src))
    return 1;
  else if (QImageDrag::canDecode(src))
    return 2;
  else
    return 0;
}

static QCString get_format(QMimeSource *src, int i = 0, bool charset = false)
{
  QCString format = src->format(i);

	if (!charset)
	{
		int pos = format.find(';');
		if (pos >= 0)
			format = format.left(pos);
	}

  return format;
}

static void get_formats(QMimeSource *src, GB_ARRAY array)
{
  int i, j;
  QCString fmt;
  char *str;
  
  for (i = 0;; i++)
  {
    if (!src->format(i))
      break;
      
    fmt = get_format(src, i, true); 
    if (*fmt < 'a' || *fmt > 'z')
      continue;
    for (j = 0; j < GB.Array.Count(array); j++)
    {
      if (strcasecmp(fmt, *((char **)GB.Array.Get(array, j))) == 0)
        break;
    }
    if (j < GB.Array.Count(array))
      continue;
    //fmt = get_format(src, i);
		GB.NewString(&str, fmt, 0);
		*((char **)GB.Array.Add(array)) = str;
  }
}

static void paste(QMimeSource *data, const char *fmt)
{
  CIMAGE *img;

  if (fmt)
  {
    if (!data->provides(fmt))
    {
      GB.ReturnNull();
      return;
    }
  }

  if (QTextDrag::canDecode(data))
  {
    QString text;
    QCString subtype;
    
    if (fmt)
    {
      subtype = fmt;
      if (subtype.left(5) == "text/")
        subtype = subtype.mid(5);
      else
        subtype = 0;
    }

    QTextDrag::decode(data, text, subtype);
    GB.ReturnNewZeroString(TO_UTF8(text));
  }
  else if (QImageDrag::canDecode(data))
  {
    GB.New(POINTER(&img), GB.FindClass("Image"), 0, 0);
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


BEGIN_PROPERTY(CCLIPBOARD_formats)

  GB_ARRAY array;
  
  GB.Array.New(&array, GB_T_STRING, 0);
  get_formats(QApplication::clipboard()->data(), array);
  GB.ReturnObject(array);

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

    drag->setText(TO_QSTRING(VARG(data)._string.value));
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
  QWidget(parent, 0, Qt::WStyle_Customize | Qt::WStyle_NoBorder | Qt::WX11BypassWM)
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
	
	_frame[0]->setGeometry(x, y, w, 1);
	_frame[1]->setGeometry(x, y, 1, h);
	_frame[2]->setGeometry(x + w - 1, y, 1, h);
	_frame[3]->setGeometry(x, y + h - 1, w, 1);
	
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

extern bool qt_xdnd_handle_badwindow();

void *CDRAG_drag(CWIDGET *source, GB_VARIANT_VALUE *data, GB_STRING *fmt)
{
  QDragObject *drag;
  QCString format;
  CIMAGE *img;
  void *dest;

  if (GB.CheckObject(source))
    return NULL;

	if (CDRAG_dragging)
	{
		GB.Error("Undergoing drag");
		return NULL;
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
		if (_picture_x >= 0 && _picture_y >= 0)
    	drag->setPixmap(*(_picture->pixmap), QPoint(_picture_x, _picture_y));
		else
    	drag->setPixmap(*(_picture->pixmap));
  }

	CDRAG_dragging = true;
	
	GB.Unref(POINTER(&CDRAG_destination));
	CDRAG_destination = 0;
	
	if (_current_drag)
	{
		//qDebug("_current_drag = %p", _current_drag);
		//qDebug("qt_xdnd_handle_badwindow = %d", qt_xdnd_handle_badwindow());
		//qDebug("_current_drag now = %p", _current_drag);
		qt_xdnd_handle_badwindow(); // Destroys the forgotten DragObject
	}
	
  QObject::connect(drag, SIGNAL(destroyed(QObject *)), &CDragManager::manager, SLOT(destroy(QObject *)));
	
	_current_drag = drag;
	//qDebug("Start drag %p", drag);
  drag->drag();
  
  hide_frame(NULL);
  GB.Post((GB_POST_FUNC)post_exit_drag, 0);

	if (CDRAG_destination)
		GB.Unref(POINTER(&CDRAG_destination));
	
	dest = CDRAG_destination;
	CDRAG_destination = 0;
		
  return dest;

_BAD_FORMAT:

  GB.Error("Bad drag format");
  return NULL;
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
	CDRAG_destination = control;
	GB.Ref(CDRAG_destination);

	p = e->pos();
	p = w->mapTo(QWIDGET(control), p);
	CDRAG_info.x = p.x();
	CDRAG_info.y = p.y();

	GB.Raise(CDRAG_destination, EVENT_Drop, 0);
	CDRAG_clear(false);
}



BEGIN_METHOD(CDRAG_call, GB_OBJECT source; GB_VARIANT data; GB_STRING format)

  GB.ReturnObject(CDRAG_drag((CWIDGET *)VARG(source), &VARG(data), MISSING(format) ? NULL : ARG(format)));

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

  GB.ReturnInteger(get_type(CDRAG_info.drop));

END_PROPERTY


BEGIN_PROPERTY(CDRAG_format)

  CHECK_VALID();

  GB.ReturnNewZeroString(get_format(CDRAG_info.drop));

END_PROPERTY


BEGIN_PROPERTY(CDRAG_formats)

  GB_ARRAY array;
  
  CHECK_VALID();
  
  GB.Array.New(&array, GB_T_STRING, 0);
  get_formats(CDRAG_info.drop, array);
  GB.ReturnObject(array);

END_PROPERTY


BEGIN_PROPERTY(CDRAG_data)

  if (!CDRAG_info.valid)
  {
    GB.ReturnNull();
    return;
  }

  paste(CDRAG_info.drop, NULL);

END_PROPERTY


BEGIN_METHOD(CDRAG_paste, GB_STRING format)

  if (!CDRAG_info.valid)
  {
    GB.ReturnNull();
    return;
  }

  paste(CDRAG_info.drop, MISSING(format) ?  NULL : GB.ToZeroString(ARG(format)));

END_METHOD


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

  GB_STATIC_METHOD("_call", "Control", CDRAG_call, "(Source)Control;(Data)v[(Format)s]"),
  GB_STATIC_METHOD("_exit", NULL, CDRAG_exit, NULL),
  GB_STATIC_METHOD("Show", NULL, CDRAG_show, "(Control)Control;[(X)i(Y)i(Width)i(Height)i]"),
  GB_STATIC_METHOD("Hide", NULL, CDRAG_hide, NULL),
  GB_STATIC_METHOD("Paste", "v", CDRAG_paste, "[(Format)s]"),

  GB_END_DECLARE
};



