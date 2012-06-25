/***************************************************************************

  (c) 2012 Adrien Prokopowicz <prokopy@users.sourceforge.net>

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

#ifndef MAIN_H
#define MAIN_H

#include "../gambas.h"

#define GB_STRCOMP_BINARY   0
#define GB_STRCOMP_NOCASE   1
#define GB_STRCOMP_LANG     2
#define GB_STRCOMP_LIKE     4
#define GB_STRCOMP_NATURAL  8

#define VARGOBJ(_type, _ob) ((_type*)VARG(_ob))

#include <iostream>

using namespace std;

#define DEBUG std::cerr << "XMLDBG : (" << __FILE__ << ":" <<__LINE__ << ") "
#define DEBUGH DEBUG << endl

extern "C" GB_INTERFACE GB;

#endif // MAIN_H
