/***************************************************************************

  CConst.h

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

#ifndef __CCONST_H
#define __CCONST_H

#include "gambas.h"
#include "gb.form.const.h"

#ifndef __CCONST_CPP
extern GB_DESC CAlignDesc[];
extern GB_DESC CArrangeDesc[];
extern GB_DESC CBorderDesc[];
extern GB_DESC CScrollDesc[];
extern GB_DESC CLineDesc[];
extern GB_DESC CFillDesc[];
extern GB_DESC CSelectDesc[];
#endif

int CCONST_convert(int *tab, int value, int def, bool to_qt);
int CCONST_alignment(int value, int def, bool to_qt);
int CCONST_horizontal_alignment(int value, int def, bool to_qt);
int CCONST_line_style(int value, int def, bool to_qt);
int CCONST_fill_style(int value, int def, bool to_qt);

#endif
