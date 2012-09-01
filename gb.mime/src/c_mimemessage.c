/***************************************************************************

  c_mimemessage.c

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

#define __C_MIMEMESSAGE_C

#include "c_mimemessage.h"

#define THIS ((CMIMEMESSAGE *)_object)
#define MESSAGE THIS->message

//-------------------------------------------------------------------------

BEGIN_METHOD(MimeMessage_new, GB_STRING contents)

	GMimeMessage *message;
	
	if (MISSING(contents))
	{
		message = g_mime_message_new(FALSE);
	}
	else
	{
		GMimeParser *parser;
		GMimeStream *stream;
	
		/* create a stream to read from memory */
		stream = g_mime_stream_mem_new_with_buffer(STRING(contents), LENGTH(contents));
	
		/* create a new parser object to parse the stream */
		parser = g_mime_parser_new_with_stream (stream);
	
		/* unref the stream (parser owns a ref, so this object does not actually get free'd until we destroy the parser) */
		g_object_unref (stream);
	
		/* parse the message from the stream */
		message = g_mime_parser_construct_message (parser);
	
		/* free the parser (and the stream) */
		g_object_unref (parser);
	}
	
	THIS->message = message;

END_METHOD


BEGIN_METHOD_VOID(MimeMessage_free)

	g_object_unref(MESSAGE);

END_METHOD


#define IMPLEMENT_STRING_PROPERTY(_name, _func) \
BEGIN_PROPERTY(MimeMessage_##_name) \
\
	if (READ_PROPERTY) \
		GB.ReturnNewZeroString(g_mime_message_get_##_func(MESSAGE)); \
	else \
		g_mime_message_set_##_func(MESSAGE, GB.ToZeroString(PROP(GB_STRING))); \
\
END_PROPERTY

IMPLEMENT_STRING_PROPERTY(Sender, sender)
IMPLEMENT_STRING_PROPERTY(ReplyTo, reply_to)
IMPLEMENT_STRING_PROPERTY(Subject, subject)
IMPLEMENT_STRING_PROPERTY(Id, message_id)


#define IMPLEMENT_RECIPIENT_PROPERTY(_name, _type) \
BEGIN_PROPERTY(MimeMessage_##_name) \
\
	InternetAddressList *addr = g_mime_message_get_recipients(MESSAGE, _type); \
	\
	if (READ_PROPERTY) \
	{ \
		char *list = internet_address_list_to_string(addr, FALSE); \
		GB.ReturnNewZeroString(list); \
		g_free(list); \
	} \
	else \
	{ \
		InternetAddressList *new_addr; \
		\
		internet_address_list_clear(addr); \
		\
		new_addr = internet_address_list_parse_string(GB.ToZeroString(PROP(GB_STRING))); \
		internet_address_list_append(addr, new_addr); \
		g_object_unref(new_addr); \
	} \
\
END_PROPERTY

IMPLEMENT_RECIPIENT_PROPERTY(To, GMIME_RECIPIENT_TYPE_TO);
IMPLEMENT_RECIPIENT_PROPERTY(Cc, GMIME_RECIPIENT_TYPE_CC);
IMPLEMENT_RECIPIENT_PROPERTY(Bcc, GMIME_RECIPIENT_TYPE_BCC);


//-------------------------------------------------------------------------

GB_DESC MimeMessageDesc[] = 
{
	GB_DECLARE("MimeMessage", sizeof(CMIMEMESSAGE)),
	
	GB_METHOD("_new", NULL, MimeMessage_new, "[(Contents)s]"),
	GB_METHOD("_free", NULL, MimeMessage_free, NULL),
	
	GB_PROPERTY("Sender", "s", MimeMessage_Sender),
	GB_PROPERTY("ReplyTo", "s", MimeMessage_ReplyTo),
	GB_PROPERTY("To", "s", MimeMessage_To),
	GB_PROPERTY("Cc", "s", MimeMessage_Cc),
	GB_PROPERTY("Bcc", "s", MimeMessage_Bcc),
	GB_PROPERTY("Subject", "s", MimeMessage_Subject),
	//GB_PROPERTY("Date", "d", MimeMessage_Date),
	GB_PROPERTY("Id", "s", MimeMessage_Id),
	
	//GB_PROPERTY("Part", "MimePart", MimeMessage_Part),
	//GB_PROPERTY_Read("Body", "MimePart", MimeMessage_Body)
	
	GB_END_DECLARE
};