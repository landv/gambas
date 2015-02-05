/***************************************************************************

  CMessage.cpp

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

#define __CMESSAGE_CPP

#include "gambas.h"
#include "main.h"

#include <QPixmap>
#include <QSizePolicy>
#include <QEventLoop>
#include <QApplication>
#include <QShowEvent>
#include <QEvent>
#include <QDesktopWidget>
#include <QMessageBox>

#include "gb.qt.h"
#include "CWindow.h"
#include "CPicture.h"
#include "CMessage.h"

typedef
	struct {
		GB_STRING msg;
		GB_STRING btn[3];
		}
	MSG_PARAM;

typedef
	struct {
		const char *text;
		//QMessageBox::ButtonRole role;
		QMessageBox::StandardButton button;
		}
	MSG_BUTTON_ROLE;
	
static char *_title = 0;

/*static MSG_BUTTON_ROLE _button_role[] =
{
	{ "Apply", QMessageBox::Apply },
	{ "Abort", QMessageBox::Abort },
	{ "Cancel", QMessageBox::Cancel },
	{ "Close", QMessageBox::Close },
	{ "Discard", QMessageBox::Discard },
	{ "Don't save", QMessageBox::Discard },
	{ "Do not save", QMessageBox::Discard },
	{ "Help", QMessageBox::Help },
	{ "Ignore", QMessageBox::Ignore },
	{ "No", QMessageBox::No },
	{ "OK", QMessageBox::Ok },
	{ "Open", QMessageBox::Open },
	{ "Reset", QMessageBox::Reset },
	{ "Restore defaults", QMessageBox::RestoreDefaults },
	{ "Retry", QMessageBox::Retry },
	{ "Save", QMessageBox::Save },
	{ "Save all", QMessageBox::SaveAll },
	{ "Yes", QMessageBox::Yes },
	{ NULL }
};

static bool compare_text(char *text, char *button)
{
	int it, ib;
	
	it = ib = 0;
	for(;;)
	{
		if (button[ib] == '&')
			ib++;
		
		if (text[it] != button[ib])
			return false;
		if (!text[it])
			return true;
		it++;
		ib++;
	}
}

static QMessageBox::StandardButton get_standard_button(char *text)
{
	MSG_BUTTON_ROLE *pb;
	
	for (pb = _button_role; pb->text; pb++)
	{
		if (compare_text(text, GB.Translate(pb->text)))
			return pb->button;
	}
	
	return QMessageBox::NoButton;
}*/

static int make_message(int type, int nbmax, void *_param)
{
	MSG_PARAM *_p = (MSG_PARAM *)_param;
	QString msg = QSTRING_ARG(msg);
	QPushButton *button[3];
	int ret, nbutton;
	//QMessageBox::ButtonRole role;
	//QMessageBox::StandardButton std;
	QMessageBox::Icon icon;
	const char *stock;
	QString title;
	QWidget *parent;
	int i, n;
	QMessageBox *mb;
	
	if (MAIN_CHECK_INIT())
		return 0;

	if (MAIN_in_message_box)
	{
		GB.Error("Message box already displayed");
		return 0;
	}
	
	MAIN_in_message_box++;

	// Make message box
	
	parent = qApp->activeWindow();
	if (!parent)
	{
		if (CWINDOW_Current)
			parent = CWINDOW_Current->widget.widget;
		else if (CWINDOW_Main)
			parent = CWINDOW_Main->widget.widget;
	}
	
	mb = new QMessageBox(parent);
	
	// Number of buttons
	
	nbutton = 0;
	for (i = 0; i < nbmax; i++)
	{
		if (MISSING(btn[i]))
			continue;
		nbutton++;
	}
		
	// Create the buttons with their role
	
	for (i = 0, n = 0; i < nbmax; i++)
	{
		if (MISSING(btn[i]))
			continue;
		
		/*std = get_standard_button(GB.ToZeroString(ARG(btn[i])));
		if (std)
		{
			button[n] = mb->addButton(std);
		}
		else
		{*/
			/*if (n == 0)
				role = QMessageBox::AcceptRole;
			else if (n == (nbutton - 1))
				role = QMessageBox::RejectRole;
			else
				role = QMessageBox::ActionRole;*/
		
			button[n] = mb->addButton(QSTRING_ARG(btn[i]), QMessageBox::ActionRole);
		//}
		
		n++;
	}

	// Default and cancel buttons
	
	if (nbutton)
	{
		mb->setDefaultButton(button[0]);
		mb->setEscapeButton(button[nbutton - 1]);
	}

	// Icon

	switch (type)
	{
		case MSG_INFO:
			icon = QMessageBox::Information;
			stock = "icon:/48/info";
			break;
		case MSG_WARNING:
			icon = QMessageBox::Warning;
			stock = "icon:/48/warning";
			break;
		case MSG_ERROR:
			icon = QMessageBox::Critical;
			stock = "icon:/48/error";
			break;
		case MSG_QUESTION:
			icon = QMessageBox::Question;
			stock = "icon:/48/question";
			break;
		case MSG_DELETE:
			icon = QMessageBox::Warning;
			stock = "icon:/48/trash";
			break;
		default:
			icon = QMessageBox::Information;
			stock = 0;
	}

	mb->setIcon(icon);
	if (stock)
	{
		CPICTURE *pict = CPICTURE_get_picture(stock);
		if (pict)
			mb->setIconPixmap(*pict->pixmap);
	}
	
	// Title

	if (_title && *_title)
	{
		title = TO_QSTRING(_title);
		GB.FreeString(&_title);
	}
	else
		title = TO_QSTRING(GB.Application.Title());

	mb->setWindowTitle(title);
	
	// Message
	
	mb->setText(msg);
	
	// Run the message box
	
	GB.Debug.EnterEventLoop();
	mb->exec();
	GB.Debug.LeaveEventLoop();
	
	//CWINDOW_ensure_active_window();
	if (parent)
		parent->activateWindow();
	
	// Returned value
	
	if (nbutton)
	{
		ret = nbutton;
		for (i = 0; i < nbutton; i++)
		{
			if (button[i] == mb->clickedButton())
				ret = i + 1;
		}
	}
	else
		ret = 1;
	
	MAIN_in_message_box--;
	MAIN_check_quit();
	
	delete mb;
	
	return ret;
}

#if 0
static int old_make_message(int type, int nbmax, void *_param)
{
	MSG_PARAM *_p = (MSG_PARAM *)_param;

	QString msg = QSTRING_ARG(msg);
	QString btn[3];
	QString swap;
	int mbtn[3];
	int i, cancel, ret;
	QMessageBox::Icon icon;
	QPoint p;
	QWidget *parent;
	const char *stock = 0;
	QString title;

	if (_global_lock)
	{
		GB.Error("Message box already displayed");
		return 0;
	}
	
	_global_lock++;

	if (!MISSING(btn[0]))
		btn[0] = QSTRING_ARG(btn[0]);

	if (nbmax >= 2 && !MISSING(btn[1]))
		btn[1] = QSTRING_ARG(btn[1]);

	if (nbmax >= 3 && !MISSING(btn[2]))
		btn[2] = QSTRING_ARG(btn[2]);

	if (btn[0].isNull() && !btn[1].isNull())
		swap = btn[0], btn[0] = btn[1], btn[1] = swap;

	if (btn[1].isNull() && !btn[2].isNull())
		swap = btn[1], btn[1] = btn[2], btn[2] = swap;

	for (i = 0; i < 3; i++)
		mbtn[i] = btn[i].isNull() ? QMessageBox::NoButton : (i + 1) ;

	mbtn[0] |= QMessageBox::Default;

	cancel = 0;

	for (i = 2; i >= 0; i--)
	{
		if (!btn[i].isNull())
		{
			cancel = i;
			break;
		}
	}

	mbtn[cancel] |= QMessageBox::Escape;

	switch (type)
	{
		case MSG_INFO:
			icon = QMessageBox::Information;
			stock = "icon:/32/info";
			break;
		case MSG_WARNING:
			icon = QMessageBox::Warning;
			stock = "icon:/32/warning";
			break;
		case MSG_ERROR:
			icon = QMessageBox::Critical;
			stock = "icon:/32/error";
			break;
		case MSG_QUESTION:
			icon = QMessageBox::Information;
			stock = "icon:/32/question";
			break;
		case MSG_DELETE:
			icon = QMessageBox::Information;
			stock = "icon:/32/trash";
			break;
		default:
			icon = QMessageBox::Information;
	}

	parent = qApp->activeWindow();
	if (!parent)
	{
		if (CWINDOW_Main)
			parent = CWINDOW_Main->widget.widget;
	}
	
	if (_title && *_title)
	{
		title = TO_QSTRING(_title);
		GB.FreeString(&_title);
	}
	else
		title = TO_QSTRING(GB.Application.Title());
	
	QMessageBox *mb = new QMessageBox(title, msg, icon, mbtn[0], mbtn[1], mbtn[2], parent);
	
	for (i = 0; i < 3; i++)
	{
		if (!btn[i].isNull())
			mb->setButtonText(i + 1, btn[i]);
	}

	if (stock)
	{
		CPICTURE *pict = CPICTURE_get_picture(stock);
		if (pict)
			mb->setIconPixmap(*pict->pixmap);
	}

	#if 0
	if (type == MSG_QUESTION)
	{
		if (_question_pixmap == 0)
			_question_pixmap = new QPixmap(_question_xpm);

		mb->setIconPixmap(*_question_pixmap);
	}
	else if (type == MSG_DELETE)
	{
		if (_trash_pixmap == 0)
			_trash_pixmap = new QPixmap(_trash_xpm);

		mb->setIconPixmap(*_trash_pixmap);
	}
	#endif
	
	//mb->adjustSize();
	if (mb->width() < 256)
		mb->resize(256, mb->height());
	
	/*mb->setMinimumSize(mb->width(), mb->height());
	mb->setMaximumSize(mb->width(), mb->height());
	mb->setSizePolicy(QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed));*/
	
	mb->setParent(parent, mb->windowFlags()); // WShowModal
	mb->installEventFilter(&CMessage::manager);
	
	//qDebug("message-box: parent = %p", mb->parentWidget());
	ret = mb->exec();
	
	if (ret == 0)
		ret = cancel + 1;

	delete mb;
		
	_global_lock--;
	
	return ret;
}
#endif

BEGIN_METHOD(CMESSAGE_info, GB_STRING msg; GB_STRING btn)

	GB.ReturnInteger(make_message(MSG_INFO, 1, (void *)_p));

END_METHOD


BEGIN_METHOD(CMESSAGE_warning, GB_STRING msg; GB_STRING btn1; GB_STRING btn2; GB_STRING btn3)

	GB.ReturnInteger(make_message(MSG_WARNING, 3, (void *)_p));

END_METHOD


BEGIN_METHOD(CMESSAGE_question, GB_STRING msg; GB_STRING btn1; GB_STRING btn2; GB_STRING btn3)

	GB.ReturnInteger(make_message(MSG_QUESTION, 3, (void *)_p));

END_METHOD


BEGIN_METHOD(CMESSAGE_delete, GB_STRING msg; GB_STRING btn1; GB_STRING btn2; GB_STRING btn3)

	GB.ReturnInteger(make_message(MSG_DELETE, 3, (void *)_p));

END_METHOD


BEGIN_METHOD(CMESSAGE_error, GB_STRING msg; GB_STRING btn1; GB_STRING btn2; GB_STRING btn3)

	GB.ReturnInteger(make_message(MSG_ERROR, 3, (void *)_p));

END_METHOD


BEGIN_METHOD_VOID(CMESSAGE_exit)

	GB.FreeString(&_title);

END_METHOD


BEGIN_PROPERTY(CMESSAGE_title)

	if (READ_PROPERTY)
		GB.ReturnString(_title);
	else
		GB.StoreString(PROP(GB_STRING), &_title);

END_PROPERTY

GB_DESC CMessageDesc[] =
{
	GB_DECLARE("Message", 0), GB_VIRTUAL_CLASS(),

	GB_STATIC_METHOD("_exit", NULL, CMESSAGE_exit, NULL),

	GB_STATIC_METHOD("_call", "i", CMESSAGE_info, "(Message)s[(Button)s]"),
	GB_STATIC_METHOD("Info", "i", CMESSAGE_info, "(Message)s[(Button)s]"),
	GB_STATIC_METHOD("Warning", "i", CMESSAGE_warning, "(Message)s[(Button1)s(Button2)s(Button3)s]"),
	GB_STATIC_METHOD("Question", "i", CMESSAGE_question, "(Message)s[(Button1)s(Button2)s(Button3)s]"),
	GB_STATIC_METHOD("Error", "i", CMESSAGE_error, "(Message)s[(Button1)s(Button2)s(Button3)s]"),
	GB_STATIC_METHOD("Delete", "i", CMESSAGE_delete, "(Message)s[(Button1)s(Button2)s(Button3)s]"),
	
	GB_STATIC_PROPERTY("Title", "s", CMESSAGE_title),
	
	GB_END_DECLARE
};



/***************************************************************************

	MyMessageBox

***************************************************************************/

#if 0
MyMessageBox::MyMessageBox(
		const QString& caption, const QString &text, Icon icon,
		int button0, int button1, int button2)
: QMessageBox(caption, text, icon, button0, button1, button2, qApp->activeWindow())
{
	center = true;

	adjustSize();
	if (width() < 256)
		resize(256, height());
	
	setMinimumSize(width(), height());
	setMaximumSize(width(), height());
	setSizePolicy(QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed));
}

void MyMessageBox::showEvent(QShowEvent *e)
{
	QMessageBox::showEvent(e);

	if (center)
	{
		QPoint p(
			(qApp->desktop()->width() - width()) / 2, 
			(qApp->desktop()->height() - height()) / 2
			);

		//qDebug("%d x %d / %d x %d", qApp->desktop()->width(), qApp->desktop()->height(), mb->width(), mb->height());
		//qDebug("(%d, %d) / (%d, %d)", p.x(), p.y(), mb->parentWidget()->x(), mb->parentWidget()->y());
		//p = mb->parentWidget()->mapFromGlobal(p);
		//qDebug("==> (%d, %d)", p.x(), p.y());
		//mb->reparent(mb->parentWidget(), Qt::WStyle_Customize | Qt::WStyle_DialogBorder | Qt::WStyle_Title | Qt::WStyle_SysMenu | Qt::WType_Dialog, p);
		//qDebug("move MessageBox to (%d %d)", mb->pos().x(), mb->pos().y());
		
		move(64, 64);  
		center = false;
	}  
}

int MyMessageBox::run()
{
	QPoint p(64, 64);
	
	clearWFlags(Qt::WDestructiveClose);
	clearWFlags(Qt::WShowModal);
		
	show();  
	reparent(qApp->activeWindow(), getWFlags() | Qt::WStyle_DialogBorder | Qt::WShowModal, p); // WShowModal
	move(p);
	show();

	qApp->eventLoop()->enterLoop();
	
	return result();  
}
#endif

	


/***************************************************************************

	CMessage

***************************************************************************/

CMessage CMessage::manager;

bool CMessage::eventFilter(QObject *o, QEvent *e)
{
	if (e->type() == QEvent::Show)
	{
		QMessageBox *mb = (QMessageBox *)o;
	
		mb->move((qApp->desktop()->availableGeometry().width() - mb->width()) / 2, (qApp->desktop()->availableGeometry().height() - mb->height()) / 2);
		mb->removeEventFilter(this);
	}
	
	return QObject::eventFilter(o, e);    // standard event processing
}
