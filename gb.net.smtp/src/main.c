/***************************************************************************

  main.c

  gb.net.smtp component

  (c) 2006 Benoît Minisini <gambas@users.sourceforge.net>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 1, or (at your option)
  any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

***************************************************************************/

#define __MAIN_C

#include "CSmtpClient.h"

#include "main.h"

#ifdef __cplusplus
extern "C" {
#endif

GB_INTERFACE GB EXPORT;

GB_DESC *GB_CLASSES[] EXPORT =
{
  CSmtpClientDesc,
  NULL
};

int EXPORT GB_INIT(void)
{
  return 0;
}

void EXPORT GB_EXIT(void)
{
}

#ifdef _cpluscplus
}
#endif

