/***************************************************************************

  cwebframe.h

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

#ifndef __CWEBFRAME_H
#define __CWEBFRAME_H

#include "main.h"

#include <QUrl>
#include <QWebFrame>
#include <QWebPage>

#ifndef __CWEBFRAME_CPP

extern GB_DESC WebFrameChildrenDesc[];
extern GB_DESC WebFrameDesc[];

#else

#define THIS      ((CWEBFRAME *)_object)
#define FRAME     (THIS->frame)

#endif

typedef
	struct 
	{
		GB_BASE ob;
		QWebFrame *frame;
	}
	CWEBFRAME;

CWEBFRAME *CWEBFRAME_get(QWebFrame *frame);
void CWEBFRAME_eval(QWebFrame *frame, const QString &javascript);

#endif
