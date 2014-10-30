/***************************************************************************

  CClipboard.cpp

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

#define __CCLIPBOARD_CPP

#include "gambas.h"

#include <QApplication>
#include <QClipboard>
#include <QImage>
#include <QEvent>
#include <QColor>
#include <QDragMoveEvent>
#include <QDropEvent>
#include <QDragEnterEvent>
#include <QMimeData>
#include <QLineEdit>
#include <QTextEdit>
#include <QSpinBox>

#include "CWidget.h"
#include "CImage.h"
#include "CClipboard.h"

CDRAG_INFO CDRAG_info = { 0 };
bool CDRAG_dragging = false;
void *CDRAG_destination = 0;

static CPICTURE *_picture = 0;
static int _picture_x = -1;
static int _picture_y = -1;

enum { MIME_UNKNOWN, MIME_TEXT, MIME_IMAGE };

static int get_type(const QMimeData *src)
{
	if (src->hasImage())
		return MIME_IMAGE;
	else if (src->formats().indexOf(QRegExp("text/.*")) >= 0)
		return MIME_TEXT;
	else
		return MIME_UNKNOWN;
}

static QString get_format(const QMimeData *src, int i = 0, bool charset = false)
{
	QStringList formats = src->formats();
	QString format;
	
	if (i >= 0 && i < formats.count())
	{
		format = formats.at(i);

		if (!charset)
		{
			int pos = format.indexOf(';');
			if (pos >= 0)
				format = format.left(pos);
		}
	}

	return format;
}

static void get_formats(const QMimeData *src, GB_ARRAY array)
{
	int i, j;
	QStringList formats = src->formats();
	QString fmt;
	
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
		
		*((char **)GB.Array.Add(array)) = GB.NewZeroString(fmt.toUtf8().data());
	}
}

static QString get_first_format(const QMimeData *src)
{
	int i;
	QString format;
	
	for (i = 0;; i++)
	{
		format = get_format(src, i);
		if (format.length() && !format[0].isLower())
			continue;
		break;
	}
	
	return format;
}

static void paste(const QMimeData *data, const char *fmt)
{
	QString format;
	QByteArray ba;
	int type;

	if (fmt)
		format = fmt;
	else
		format = get_first_format(data);
		
	if (!data->hasFormat(format))
	{
		GB.ReturnVariant(NULL);
		return;
	}

	if (format.startsWith("text/"))
		type = MIME_TEXT;
	else
		type = get_type(data);

	switch(type)
	{
		case MIME_TEXT:
			ba = data->data(format);
			if (ba.size())
				GB.ReturnNewString(ba.constData(), ba.size());
			else
				GB.ReturnNull();
			break;
		
		case MIME_IMAGE:
			{
				QImage *image = new QImage();
				*image = qvariant_cast<QImage>(data->imageData());
				image->convertToFormat(QImage::Format_ARGB32_Premultiplied);
				GB.ReturnObject(CIMAGE_create(image));
			}
			break;
		
		default:
			GB.ReturnNull();
	}
	
	GB.ReturnConvVariant();
}



/***************************************************************************

	Clipboard

***************************************************************************/

static GB_ARRAY _clipboard_formats = NULL;

void CLIPBOARD_has_changed()
{
	GB.Unref(POINTER(&_clipboard_formats));
	_clipboard_formats = NULL;
}

static GB_ARRAY load_clipboard_formats()
{
	if (!_clipboard_formats)
	{
		//qDebug("load clipboard formats");
		GB.Array.New(&_clipboard_formats, GB_T_STRING, 0);
		get_formats(QApplication::clipboard()->mimeData(), _clipboard_formats);
		GB.Ref(_clipboard_formats);
	}
	
	return _clipboard_formats;
}

static int get_clipboard_type()
{
	int i;
	QString format;
	
	load_clipboard_formats();
	
	for (i = 0; i < GB.Array.Count(_clipboard_formats); i++)
	{
		format = *((char **)GB.Array.Get(_clipboard_formats, i));
		if (format.startsWith("text/"))
			return MIME_TEXT;
		else if (format.startsWith("image/"))
			return MIME_IMAGE;
		else if (format == "application/x-qt-image")
			return MIME_IMAGE;
	}
	
	return MIME_UNKNOWN;
}

BEGIN_METHOD_VOID(Clipboard_exit)

	CLIPBOARD_has_changed();

END_METHOD


BEGIN_METHOD_VOID(CCLIPBOARD_clear)

	QApplication::clipboard()->clear();

END_METHOD


BEGIN_PROPERTY(CCLIPBOARD_format)

	load_clipboard_formats();
	if (GB.Array.Count(_clipboard_formats) == 0)
		GB.ReturnVoidString();
	else
		GB.ReturnString(*((char **)GB.Array.Get(_clipboard_formats, 0)));

END_PROPERTY


BEGIN_PROPERTY(CCLIPBOARD_formats)

	GB.ReturnObject(load_clipboard_formats());

END_PROPERTY


BEGIN_PROPERTY(CCLIPBOARD_type)

	GB.ReturnInteger(get_clipboard_type());

END_PROPERTY


BEGIN_METHOD(CCLIPBOARD_copy, GB_VARIANT data; GB_STRING format)

	QString format;
	QMimeData *data = new QMimeData();

	if (VARG(data).type == GB_T_STRING)
	{
		if (MISSING(format))
			format = "text/plain";
		else
		{
			format = TO_QSTRING(GB.ToZeroString(ARG(format)));
			if (format.left(5) != "text/")
				goto _BAD_FORMAT;
			if (format.length() == 5)
				goto _BAD_FORMAT;
		}

		data->setData(format, QByteArray(VARG(data).value._string, GB.StringLength(VARG(data).value._string)));
		QApplication::clipboard()->setMimeData(data);
	}
	else if (VARG(data).type >= GB_T_OBJECT && GB.Is(VARG(data).value._object, CLASS_Image))
	{
		QImage img;

		if (!MISSING(format))
			goto _BAD_FORMAT;

		img = *CIMAGE_get((CIMAGE *)VARG(data).value._object);
		img.detach();

		QApplication::clipboard()->setImage(img);
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

	GB_STATIC_METHOD("_exit", NULL, Clipboard_exit, NULL),

	GB_CONSTANT("None", "i", 0),
	GB_CONSTANT("Text", "i", 1),
	GB_CONSTANT("Image", "i", 2),

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
	setAutoFillBackground(true);
	QPalette pal(palette());
	pal.setColor(QPalette::Window, QColor(0, 0, 0, 128));
	setPalette(pal);
	//setWindowOpacity(0.5);
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
	GB.Unref(POINTER(&_frame_control));
	_frame_control = NULL;
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
	
	if (control != _frame_control)
	{
		hide_frame(NULL);
		_frame_control = control;
		GB.Ref(control);
	}
	
	if (!_frame_visible)
	{
		for (i = 0; i < 4; i++)
			_frame[i] = new MyDragFrame(wid);
	}
	
	//x -= 2;
	//y -= 2;
	//w += 4;
	//h += 4;
	if (w < 4 || h < 4)
		return;
	
	_frame[0]->setGeometry(x, y, w, 2);
	_frame[3]->setGeometry(x, y + h - 2, w, 2);
	_frame[1]->setGeometry(x, y + 2, 2, h - 4);
	_frame[2]->setGeometry(x + w - 2, y + 2, 2, h - 4);
	
	for (i = 0; i < 4; i++)
		_frame[i]->show();
	
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

void *CDRAG_drag(CWIDGET *source, GB_VARIANT_VALUE *data, GB_STRING *fmt)
{
	QDrag *drag;
	QMimeData *mimeData;
	QString format;
	void *dest;

	if (GB.CheckObject(source))
		return NULL;

	if (CDRAG_dragging)
	{
		GB.Error("Undergoing drag");
		return NULL;
	}

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
		
		mimeData->setData(format, QByteArray(data->value._string, GB.StringLength(data->value._string)));
	}
	else if (data->type >= GB_T_OBJECT && GB.Is(data->value._object, CLASS_Image))
	{
		QImage img;

		if (fmt)
			goto _BAD_FORMAT;

		img = *CIMAGE_get((CIMAGE *)data->value._object);
		img.detach();
		
		mimeData->setImageData(img);
	}
	else
		goto _BAD_FORMAT;

	source->flag.dragging = true;
	
	drag = new QDrag(source->widget);
	drag->setMimeData(mimeData);
	
	if (_picture)
	{
		drag->setPixmap(*(_picture->pixmap));
		if (_picture_x >= 0 && _picture_y >= 0)
			drag->setHotSpot(QPoint(_picture_x, _picture_y));
	}

	CDRAG_dragging = true;
	
	GB.Unref(POINTER(&CDRAG_destination));
	CDRAG_destination = 0;
	
	//qDebug("start drag");
	drag->exec();

	source->flag.dragging = false;
	//qDebug("end drag");
	
	hide_frame(NULL);
	GB.Post((GB_CALLBACK)post_exit_drag, 0);

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

	//qDebug("CDRAG_drag_enter: (%s %p) %p", GB.GetClassName(control), control, qobject_cast<MyListView *>(QWIDGET(control)));

	// Hack for QScrollView
	/*if (CWIDGET_test_flag(control, WF_SCROLLVIEW) && qobject_cast<MyListView *>(QWIDGET(control)))
		((MyListView *)QWIDGET(control))->contentsDragEnterEvent((QDragEnterEvent *)e);*/

	if (!GB.CanRaise(control, EVENT_Drag))
	{
		if (GB.CanRaise(control, EVENT_DragMove) || GB.CanRaise(control, EVENT_Drop))
			e->acceptProposedAction();
		else
		{
			if (qobject_cast<QLineEdit *>(w) || qobject_cast<QTextEdit *>(w))
				return false;

			e->ignore();
		}
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

#define EXT(_ob) (((CWIDGET *)_ob)->ext)

void CDRAG_drag_leave(CWIDGET *control)
{
	CDRAG_hide_frame(control);
	
	//while (EXT(control) && EXT(control)->proxy)
	//	control = (CWIDGET *)(EXT(control)->proxy);

__DRAG_LEAVE_TRY_PROXY:

	GB.Raise(control, EVENT_DragLeave, 0);

	if (EXT(control) && EXT(control)->proxy)
	{
		control = (CWIDGET *)(EXT(control)->proxy);
		goto __DRAG_LEAVE_TRY_PROXY;
	}
}


bool CDRAG_drag_move(QWidget *w, CWIDGET *control, QDropEvent *e)
{
	bool cancel;
	QPoint p;

	//qDebug("CDRAG_drag_move: (%s %p) %p", GB.GetClassName(control), control, qobject_cast<MyListView *>(QWIDGET(control)));

	// Hack for QScrollView
	
	/*if (CWIDGET_test_flag(control, WF_SCROLLVIEW) && qobject_cast<MyListView *>(QWIDGET(control)))
	{
		accepted = e->isAccepted();
		((MyListView *)QWIDGET(control))->contentsDragMoveEvent((QDragMoveEvent *)e);
		if (accepted)
			e->acceptProposedAction();
		else
			e->ignore();
	}*/

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

bool CDRAG_drag_drop(QWidget *w, CWIDGET *control, QDropEvent *e)
{
	QPoint p;

	//hide_frame();

	if (!GB.CanRaise(control, EVENT_Drop))
		return false;

	// Hack for QScrollView
	/*if (CWIDGET_test_flag(control, WF_SCROLLVIEW) && qobject_cast<MyListView *>(QWIDGET(control)))
		((MyListView *)QWIDGET(control))->contentsDropEvent((QDragMoveEvent *)e);*/
	
	CDRAG_clear(true);
	CDRAG_info.event = e;
	CDRAG_destination = control;
	GB.Ref(CDRAG_destination);

	p = e->pos();
	p = w->mapTo(QWIDGET(control), p);
	CDRAG_info.x = p.x();
	CDRAG_info.y = p.y();

	GB.Raise(control, EVENT_Drop, 0);

	if (!CDRAG_dragging) // DnD run outside of the application
	{
		GB.Unref(&CDRAG_destination);
		hide_frame(control);
	}
	
	CDRAG_clear(false);
	
	return true;
}

BEGIN_METHOD(CDRAG_call, GB_OBJECT source; GB_VARIANT data; GB_STRING format)

	GB.ReturnObject(CDRAG_drag((CWIDGET *)VARG(source), &VARG(data), MISSING(format) ? NULL : ARG(format)));

END_METHOD

BEGIN_METHOD_VOID(CDRAG_exit)

	hide_frame(NULL);
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
		GB.ReturnVariant(NULL);
		return;
	}

	paste(CDRAG_info.event->mimeData(), NULL);

END_PROPERTY


BEGIN_METHOD(CDRAG_paste, GB_STRING format)

	if (!CDRAG_info.valid)
	{
		GB.ReturnVariant(NULL);
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
		
	/*if (!CDRAG_dragging)
	{
		GB.Error("No undergoing drag");
		return;
	}*/

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

	GB_CONSTANT("None", "i", MIME_UNKNOWN),
	GB_CONSTANT("Text", "i", MIME_TEXT),
	GB_CONSTANT("Image", "i", MIME_IMAGE),

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



