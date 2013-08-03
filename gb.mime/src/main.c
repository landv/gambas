/***************************************************************************

  main.c

  gb.mime component

  (c) 2000-2013 Benoît Minisini <gambas@users.sourceforge.net>

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

#define __MAIN_C

#include "main.h"

#include "c_mime.h"
#include "c_mimemessage.h"
#include "c_mimepart.h"
#include "main.h"

GB_INTERFACE GB EXPORT;

GB_DESC *GB_CLASSES[] EXPORT = 
{
	MimeDesc,
	MimePartHeadersDesc,
	MimePartDesc,
	MimeMessageHeadersDesc,
	MimeMessageDesc,
	NULL
};

int EXPORT GB_INIT()
{
	g_mime_init(0);
	return 0;
}


void EXPORT GB_EXIT()
{
	g_mime_shutdown();
}
