/***************************************************************************

  CMessage.cpp

  The message box class

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

#define __CMESSAGE_CPP


#include "gambas.h"
#include "main.h"

#include <qnamespace.h>
#include <qmessagebox.h>
#include <qpixmap.h>
#include <qsizepolicy.h>
#include <qeventloop.h>
#include <qapplication.h>

#include "gb.qt.h"
#include "CWindow.h"
#include "CPicture.h"
#include "CMessage.h"


#if 0
/* XPM */
static const char *_question_xpm[] = {
/* width height ncols cpp */
"32 32 6 2",
  /* Colors */
  "00 c #000000",
  "01 c #808080",
  "02 c #C0C0C0",
  "03 c #0000FF",
  "04 c #FFFFFF",
  ".. s None c None",
  "......................0101010101010101..........................",
  "................0101010204040404040402010101....................",
  "............010102040404040404040404040404020101................",
  "..........0102040404040404040404040404040404040201..............",
  "........01040404040404040404040404040404040404040400............",
  "......010404040404040402030303030303020404040404040400..........",
  "....0104040404040404020302040403030303020404040404040400........",
  "..01020404040404040403030404040403030303040404040404040200......",
  "..0104040404040404040303030304040303030304040404040404040001....",
  "01020404040404040404030303030402030303030404040404040404020001..",
  "01040404040404040404020303020403030303040404040404040404040001..",
  "0104040404040404040404040404020303030404040404040404040404000101",
  "0104040404040404040404040404030303040404040404040404040404000101",
  "0104040404040404040404040404030302040404040404040404040404000101",
  "0104040404040404040404040404030304040404040404040404040404000101",
  "0102040404040404040404040404040404040404040404040404040402000101",
  "..01040404040404040404040402030302040404040404040404040400010101",
  "..01020404040404040404040403030303040404040404040404040200010101",
  "....0104040404040404040404030303030404040404040404040400010101..",
  "......00040404040404040404020303020404040404040404040001010101..",
  "........0004040404040404040404040404040404040404040001010101....",
  "..........000204040404040404040404040404040404020001010101......",
  "............00000204040404040404040404040402000001010101........",
  "..............0100000002040404040404020000000101010101..........",
  "................010101000000020404040001010101010101............",
  "....................0101010100040404000101010101................",
  "..........................0100040404000101......................",
  "..............................000404000101......................",
  "................................0004000101......................",
  "..................................00000101......................",
  "....................................010101......................",
  "......................................0101......................",
};

/* XPM */
static char *_trash_xpm[]={
"32 32 5 1",
". c None",
"# c #000000",
"a c #808080",
"b c #c0c0c0",
"c c #ffffff",
"..............##................",
"...........a##bb#...............",
"..........aaaa#ac##a............",
"........aabbab###b#b#ab.........",
"......aabbbca#abab#bbba#........",
".....abbbccccbbba##abbbaa.......",
".....abbcccccccc#aaabbba#.......",
".....aabccccccbbbaaabbba#.......",
".....aabbcccccbbbbbbbbaa#.......",
".....abbbbbbccbbbbbaaaaa#.......",
".....abbbbbbbbbbbaaaaaaa#.......",
".....a#abbccbbbbbbaaaa##a.......",
"......aaaabbbbbbbbaa##a#........",
"......abbbbbaaaa###aaaa#........",
"......abbbcbbbbbbbaaaaa#........",
"......ababbcbbbcbabbb#a#........",
"......ababbcabbbbaaba#a#aaa.....",
"......ababbcabbbbaaba#a#aaaaa...",
"......ababbcabbbbaaba#a#aaaaaa..",
"......ababbcabbbbaaba#a#aaaaaaa.",
"......ababbcabbbbaaba#a#aaaaaaaa",
"......ababbcabbbbaaba#a#aaaaaaaa",
"......ababbcabbbbaaba#a#aaaaaaaa",
"......ababbcabbbbaaba#a#aaaaaaaa",
"......ababbcabbbbaaba#a#aaaaaaaa",
"......abbbbcabbbbaabaaa#aaaaaaa.",
"......abbbbcabbbbaabaaa#aaaaa...",
"......#abbcccabbbabbaaa#aaa.....",
"......a##bccbbbbbbbba###a.......",
"........aa#abbbbba#####.........",
"..........a##########...........",
"................................"};


static QPixmap *_question_pixmap = 0;
static QPixmap *_trash_pixmap = 0;
#endif

/*
static int my_message(GAMBAS_MESSAGE *msg)
{
  QMessageBox::Icon icon;
  int button[3];
  int i;

  switch (msg->icon)
  {
    case GB_MSG_INFORMATION:
      icon = QMessageBox::Information;
      break;
    case GB_MSG_WARNING:
      icon = QMessageBox::Warning;
      break;
    case GB_MSG_CRITICAL:
      icon = QMessageBox::Critical;
      break;
    default:
      icon = QMessageBox::Information;
  }

  button[0] = button[1] = button[2] = QMessageBox::NoButton;

  switch (msg->type)
  {
    case GB_MSG_USER_DEFINED:
      for (i = 0; i < 3; i++)
        button[i] = msg->button[i] ? (i + 1) : QMessageBox::NoButton;
      break;

    case GB_MSG_OK_ONLY:
      button[0] = QMessageBox::Ok | QMessageBox::Default | QMessageBox::Escape;
      break;

    case GB_MSG_OK_CANCEL:
      button[0] = QMessageBox::Ok | QMessageBox::Default;
      button[1] = QMessageBox::Cancel | QMessageBox::Escape;
      break;

    case GB_MSG_YES_NO:
      button[0] = QMessageBox::Yes | QMessageBox::Default;
      button[1] = QMessageBox::No | QMessageBox::Escape;
      break;

    case GB_MSG_YES_NO_CANCEL:
      button[0] = QMessageBox::Yes | QMessageBox::Default;
      button[1] = QMessageBox::No;
      button[2] = QMessageBox::Cancel | QMessageBox::Escape;
      break;
  }

  QMessageBox mb(msg->title, msg->text, icon, button[0], button[1], button[2]);

  if (msg->type == GB_MSG_USER_DEFINED)
  {
    for (i = 0; i < 3; i++)
    {
      if (msg->button[i] != NULL)
      {
        qDebug(msg->button[i]);
        mb.setButtonText(i + 1, msg->button[i]);
      }
    }
  }

  switch (mb.exec())
  {
    case QMessageBox::Yes:
      return GB_MSG_YES;
    case QMessageBox::No:
      return GB_MSG_NO;
    case QMessageBox::Ok:
      return GB_MSG_OK;
    case QMessageBox::Cancel:
      return GB_MSG_CANCEL;
    case QMessageBox::Abort:
      return GB_MSG_ABORT;
    case QMessageBox::Retry:
      return GB_MSG_RETRY;
    case QMessageBox::Ignore:
      return GB_MSG_IGNORE;
    default:
      return 0;
  }
}
*/

typedef
  struct {
    GB_STRING msg;
    GB_STRING btn[3];
    }
  MSG_PARAM;


static int make_message(int type, int nbmax, void *_param)
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
  char *stock = 0;

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
    if (btn[i])
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
  
  QMessageBox *mb = new QMessageBox(
    TO_QSTRING(GB.Application.Title()), msg, icon,
    mbtn[0], mbtn[1], mbtn[2], parent);
    
    //, 0, //qApp->activeWindow(),
    //0, true, 0);
    //CWINDOW_Current ? ((CWIDGET *)CWINDOW_Current)->widget : 0);

  for (i = 0; i < 3; i++)
  {
    if (btn[i])
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
  
  mb->adjustSize();
  if (mb->width() < 256)
    mb->resize(256, mb->height());
  
  mb->setMinimumSize(mb->width(), mb->height());
  mb->setMaximumSize(mb->width(), mb->height());
  mb->setSizePolicy(QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed));
  
  mb->reparent(parent, Qt::WType_TopLevel | Qt::WStyle_Customize | Qt::WStyle_DialogBorder | Qt::WStyle_Title | Qt::WStyle_SysMenu, mb->pos()); // WShowModal
  mb->installEventFilter(&CMessage::manager);
  
  //qDebug("message-box: parent = %p", mb->parentWidget());
  ret = mb->exec();
  
  if (ret == 0)
    ret = cancel + 1;

  delete mb;
    
  return ret;
}


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


GB_DESC CMessageDesc[] =
{
  GB_DECLARE("Message", 0), GB_VIRTUAL_CLASS(),

  //GB_STATIC_METHOD("_exit", NULL, CMESSAGE_exit, NULL),

  GB_STATIC_METHOD("_call", "i", CMESSAGE_info, "(Message)s[(Button)s]"),
  GB_STATIC_METHOD("Info", "i", CMESSAGE_info, "(Message)s[(Button)s]"),
  GB_STATIC_METHOD("Warning", "i", CMESSAGE_warning, "(Message)s[(Button1)s(Button2)s(Button3)s]"),
  GB_STATIC_METHOD("Question", "i", CMESSAGE_question, "(Message)s[(Button1)s(Button2)s(Button3)s]"),
  GB_STATIC_METHOD("Error", "i", CMESSAGE_error, "(Message)s[(Button1)s(Button2)s(Button3)s]"),
  GB_STATIC_METHOD("Delete", "i", CMESSAGE_delete, "(Message)s[(Button1)s(Button2)s(Button3)s]"),

  GB_END_DECLARE
};



/***************************************************************************

  MyMessageBox

***************************************************************************/

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

#if 0
void MyMessageBox::showEvent(QShowEvent *e)
{
  QMessageBox::showEvent(e);

  if (center)
  {
    QPoint p(
      (qApp->desktop()->width() - width()) / 2, 
      (qApp->desktop()->height() - height()) / 2
      );

    //qDebug("%d x %d / %d x %d", qApp->desktop()->width(), qApp->desktop()->height(), mb.width(), mb.height());
    //qDebug("(%d, %d) / (%d, %d)", p.x(), p.y(), mb.parentWidget()->x(), mb.parentWidget()->y());
    //p = mb.parentWidget()->mapFromGlobal(p);
    //qDebug("==> (%d, %d)", p.x(), p.y());
    //mb.reparent(mb.parentWidget(), Qt::WStyle_Customize | Qt::WStyle_DialogBorder | Qt::WStyle_Title | Qt::WStyle_SysMenu | Qt::WType_Dialog, p);
    //qDebug("move MessageBox to (%d %d)", mb.pos().x(), mb.pos().y());
    
    move(64, 64);  
    center = false;
  }  
}
#endif

int MyMessageBox::run()
{
  QPoint p(64, 64);
  
  clearWFlags(WDestructiveClose);
  clearWFlags(WShowModal);
    
  show();  
  reparent(qApp->activeWindow(), getWFlags() | WStyle_DialogBorder | WShowModal, p); // WShowModal
  move(p);
  show();

  qApp->eventLoop()->enterLoop();
  
  return result();  
}

  


/***************************************************************************

  CMessage

***************************************************************************/

CMessage CMessage::manager;

bool CMessage::eventFilter(QObject *o, QEvent *e)
{
  if (e->type() == QEvent::Show)
  {
    QMessageBox *mb = (QMessageBox *)o;
  
    mb->move((qApp->desktop()->width() - mb->width()) / 2, (qApp->desktop()->height() - mb->height()) / 2);
    mb->removeEventFilter(this);
  }
  
  return QObject::eventFilter(o, e);    // standard event processing
}
