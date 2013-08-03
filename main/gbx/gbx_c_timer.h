/***************************************************************************

  gbx_c_timer.h

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

#ifndef __GBX_C_TIMER_H
#define __GBX_C_TIMER_H

#include "gambas.h"
#include "gbx_variant.h"
#include "gb_list.h"

typedef
	GB_TIMER CTIMER;

#ifndef __GBX_C_TIMER_C
extern GB_DESC NATIVE_Timer[];
#else

#define THIS ((CTIMER *)_object)

#endif

void CTIMER_raise(void *_object);
CTIMER *CTIMER_every(int delay, GB_TIMER_CALLBACK callback, intptr_t param);

#endif
