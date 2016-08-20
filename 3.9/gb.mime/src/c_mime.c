/***************************************************************************

  c_mime.c

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

#define __C_MIME_C

#include "c_mime.h"

//-------------------------------------------------------------------------

BEGIN_METHOD(Mime_Encode, GB_STRING data; GB_INTEGER encoding)

	GMimeEncoding menc;
	int encoding = VARG(encoding);
	size_t outlen;
	char *outbuf;
	
	switch(encoding)
	{
		case GMIME_CONTENT_ENCODING_BASE64:
		case GMIME_CONTENT_ENCODING_QUOTEDPRINTABLE:
		case GMIME_CONTENT_ENCODING_UUENCODE:
			break;
		default:
			GB.Error("Bad encoding");
			return;
	}
	
	g_mime_encoding_init_encode(&menc, encoding);
	outlen = g_mime_encoding_outlen(&menc, LENGTH(data));
	outbuf = GB.NewString(NULL, outlen);
	outlen = g_mime_encoding_step(&menc, STRING(data), LENGTH(data), outbuf);
	outbuf = GB.ExtendString(outbuf, outlen);
	
	GB.ReturnString(GB.FreeStringLater(outbuf));
	
END_METHOD

BEGIN_METHOD(Mime_Decode, GB_STRING data; GB_INTEGER encoding)

	GMimeEncoding menc;
	int encoding = VARG(encoding);
	size_t outlen;
	char *outbuf;
	
	switch(encoding)
	{
		case GMIME_CONTENT_ENCODING_BASE64:
		case GMIME_CONTENT_ENCODING_QUOTEDPRINTABLE:
		case GMIME_CONTENT_ENCODING_UUENCODE:
			break;
		default:
			GB.Error("Bad encoding");
			return;
	}
	
	g_mime_encoding_init_decode(&menc, encoding);
	outlen = g_mime_encoding_outlen(&menc, LENGTH(data));
	outbuf = GB.NewString(NULL, outlen);
	outlen = g_mime_encoding_step(&menc, STRING(data), LENGTH(data), outbuf);
	outbuf = GB.ExtendString(outbuf, outlen);
	
	GB.ReturnString(GB.FreeStringLater(outbuf));
	
END_METHOD

//-------------------------------------------------------------------------



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
	
	GB_STATIC_METHOD("Encode", "s", Mime_Encode, "(Data)s(Encoding)i"),
	GB_STATIC_METHOD("Decode", "s", Mime_Decode, "(Data)s(Encoding)i"),
	
	GB_END_DECLARE
};