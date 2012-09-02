/***************************************************************************

  c_mime.c

  gb.mime component

  (c) 2012 Benoît Minisini <gambas@users.sourceforge.net>

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

#define __C_MIME_C

#include "c_mime.h"

GB_DESC MimeDesc[] = 
{
	GB_DECLARE_VIRTUAL("Mime"),
	
	GB_CONSTANT("Default", "i", GMIME_CONTENT_ENCODING_DEFAULT),
	GB_CONSTANT("7Bit", "i", GMIME_CONTENT_ENCODING_7BIT),
	GB_CONSTANT("8Bit", "i", GMIME_CONTENT_ENCODING_8BIT),
	GB_CONSTANT("Binary", "i", GMIME_CONTENT_ENCODING_BINARY),
	GB_CONSTANT("Base64", "i", GMIME_CONTENT_ENCODING_BASE64),
	GB_CONSTANT("QuotedPrintable", "i", GMIME_CONTENT_ENCODING_QUOTEDPRINTABLE),
	GB_CONSTANT("UUEncode", "i", GMIME_CONTENT_ENCODING_UUENCODE),
	
	GB_END_DECLARE
};