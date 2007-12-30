/***************************************************************************

  main.h

  (c) 2000-2005 Benoît Minisini <gambas@users.sourceforge.net>

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

#ifndef __MAIN_H
#define __MAIN_H

#include "gambas.h"
#include "../gb.qt.h"

#define QT_THREAD_SUPPORT
#include <qstring.h>
#include <kapplication.h>

#ifndef __MAIN_C
extern "C" GB_INTERFACE GB;
extern "C" QT_INTERFACE QT;
#endif

class MyApplication: public KApplication
{
  Q_OBJECT

public:

  bool eventFilter(QObject *, QEvent *);
};

#endif
