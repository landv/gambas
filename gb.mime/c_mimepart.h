/***************************************************************************

  c_mimepart.h

  gb.mime component

  (c) 2000-2017 Benoît Minisini <gambas@users.sourceforge.net>

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
  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
  MA 02110-1301, USA.

***************************************************************************/

#ifndef __C_MIMEPART_H
#define __C_MIMEPART_H

#include "main.h"

#ifndef __C_MIMEPART_C
extern GB_DESC MimePartDesc[];
extern GB_DESC MimePartHeadersDesc[];
#endif

typedef
	struct {
		GB_BASE ob;
		GMimeObject *part;
	}
	CMIMEPART;

CMIMEPART *CMIMEPART_create(GMimeObject *part);
	
#endif /* __MAIN_H */
