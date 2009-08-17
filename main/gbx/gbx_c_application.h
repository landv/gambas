/***************************************************************************

  gbx_c_application.h

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

#ifndef __GBX_C_APPLICATION_H
#define __GBX_C_APPLICATION_H

#include "gambas.h"

#ifndef __GBX_C_APPLICATION_C
extern GB_DESC NATIVE_AppEnv[];
extern GB_DESC NATIVE_AppArgs[];
extern GB_DESC NATIVE_App[];
extern GB_DESC NATIVE_System[];
extern GB_DESC NATIVE_User[];
#endif

void CAPP_init(void);
void CAPP_got_signal(void);

#endif
