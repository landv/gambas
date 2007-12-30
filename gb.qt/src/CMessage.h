/***************************************************************************

  CMessage.h

  The message box class

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

#ifndef __CMESSAGE_H
#define __CMESSAGE_H

#include <qobject.h>
#include <qmessagebox.h>

#include "gambas.h"

#ifndef __CMESSAGE_CPP
extern GB_DESC CMessageDesc[];
#else

enum {
  MSG_INFO = 0,
  MSG_WARNING = 1,
  MSG_QUESTION = 2,
  MSG_ERROR = 3,
  MSG_DELETE = 4
  };

#endif

/*class MyMessageBox : public QMessageBox
{
  Q_OBJECT
  
public:

  MyMessageBox(const QString& caption, const QString &text, Icon icon,
               int button0, int button1, int button2);

  ~MyMessageBox() {}
  int run();
                 
protected:

  //void showEvent( QShowEvent * );  
    
private:

  bool center;
};
*/

class CMessage : public QObject
{
  Q_OBJECT

public:

  static CMessage manager;
  
protected:

  bool eventFilter(QObject *, QEvent *);
};




#endif
