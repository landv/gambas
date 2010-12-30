/***************************************************************************

  CScreen.h

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

#ifndef __CSCREEN_H
#define __CSCREEN_H

#include "gambas.h"

#ifndef __CSCREEN_CPP
extern GB_DESC DesktopDesc[];
extern GB_DESC ApplicationTooltipDesc[];
extern GB_DESC ApplicationDesc[];
extern GB_DESC ScreensDesc[];
extern GB_DESC ScreenDesc[];

extern char *CAPPLICATION_Theme;
#else

#define SCREEN ((CSCREEN *)_object)

#endif

typedef
	struct
	{
		GB_BASE ob;
		int index;
	}
	CSCREEN;
	
#endif
