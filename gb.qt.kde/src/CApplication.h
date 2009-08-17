/***************************************************************************

  CApplication.h

  (c) 2000-2009 Benoît Minisini <gambas@users.sourceforge.net>

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
  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

***************************************************************************/

#ifndef __CAPPLICATION_H
#define __CAPPLICATION_H

#include <qasciidict.h>
#include <qdict.h>
#include <qcstring.h>
#include <qstring.h>
#include <qstringlist.h>

#include <dcopref.h>

#include "gambas.h"

#ifndef __CAPPLICATION_CPP
extern GB_DESC CKDEObjectDesc[];
extern GB_DESC CKDEApplicationDesc[];
extern GB_DESC CApplicationDesc[];
extern GB_DESC CKDEDCOPRefDesc[];
#else

#define THIS ((CAPPLICATION *)_object)

#endif

class CFunction
{
public:
  
  QCString name;
  QCString dcopName;
  int type;
  int *args;
  
  CFunction() { args = 0; }
  ~CFunction() { if (args) delete[] args; }
};

typedef
  struct {
    GB_BASE ob;
    DCOPRef *ref;
    }
  CDCOPREF;

typedef
  struct
  {
    GB_BASE ob;
    char *name;
    char *object;
    QAsciiDict< QDict<CFunction> > *cache;
  }
  CAPPLICATION;

#endif
