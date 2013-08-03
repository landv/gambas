/***************************************************************************

  CMessage.h

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

#ifndef __CMESSAGE_H
#define __CMESSAGE_H

#include <qobject.h>
#include <qmessagebox.h>
//Added by qt3to4:
#include <QEvent>

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

class CMessage : public QObject
{
  Q_OBJECT

public:

  static CMessage manager;
  
protected:

  bool eventFilter(QObject *, QEvent *);
};




#endif
