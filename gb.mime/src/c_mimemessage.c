/***************************************************************************

  c_mimemessage.c

  gb.mime component

  (c) 2000-2017 Benoît Minisini <g4mba5@gmail.com>
  (c) 2018      Bastian Germann <bastiangermann@fishpost.de>

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

static GMimeMessage *_message = NULL;

CMIMEMESSAGE *CMIMEMESSAGE_create(GMimeMessage *message)
{
	CMIMEMESSAGE *mmsg;
	
	if (!message)
		return NULL;
	
	mmsg = (CMIMEMESSAGE *)g_object_get_data(G_OBJECT(message), "gambas-object");
	if (!mmsg)
	{
		_message = message;
		g_object_ref(message);
		mmsg = (CMIMEMESSAGE *)GB.New(GB.FindClass("MimeMessage"), NULL, NULL);
		_message = NULL;
	}
	
	return mmsg;
}

//-------------------------------------------------------------------------

BEGIN_METHOD(MimeMessage_new, GB_STRING contents)

	GMimeMessage *message = _message;
	
	if (!message)
	{
		if (MISSING(contents))
		{
			message = g_mime_message_new(FALSE);
			// Add a default part?
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
			g_object_unref(stream);
		
			/* parse the message from the stream */
			message = g_mime_parser_construct_message (parser NULLGM3);
		
			/* free the parser (and the stream) */
			g_object_unref(parser);
			
			if (!message)
			{
				GB.Error("Unable to parse message");
				return;
			}
		}
	}
	
	THIS->message = message;
	g_object_set_data(G_OBJECT(message), "gambas-object", THIS);

END_METHOD


BEGIN_METHOD_VOID(MimeMessage_free)

	if (MESSAGE)
	{
		g_object_set_data(G_OBJECT(MESSAGE), "gambas-object", NULL);
		g_object_unref(MESSAGE);
	}

END_METHOD


BEGIN_PROPERTY(MimeMessage_Subject)

	if (READ_PROPERTY)
		GB.ReturnNewZeroString(g_mime_message_get_subject(MESSAGE));
	else
		g_mime_message_set_subject(MESSAGE, GB.ToZeroString(PROP(GB_STRING)) NULLGM3);

END_PROPERTY


#define IMPLEMENT_STRING_PROPERTY(_name, _func) \
BEGIN_PROPERTY(MimeMessage_##_name) \
\
	if (READ_PROPERTY) \
		GB.ReturnNewZeroString(g_mime_message_get_##_func(MESSAGE)); \
	else \
		g_mime_message_set_##_func(MESSAGE, GB.ToZeroString(PROP(GB_STRING))); \
\
END_PROPERTY


#define IMPLEMENT_LIST_PROPERTY(_name, _func) \
BEGIN_PROPERTY(MimeMessage_##_name) \
\
	InternetAddressList *list; \
	InternetAddress *addr; \
	if (READ_PROPERTY) \
	{ \
		list = g_mime_message_get_##_func(MESSAGE); \
		addr = internet_address_list_get_address(list, 0); \
		GB.ReturnNewZeroString(internet_address_to_string(addr, NULL, FALSE)); \
	} \
	else \
	{ \
		addr = internet_address_mailbox_new("", GB.ToZeroString(PROP(GB_STRING))); \
		list = g_mime_message_get_##_func(MESSAGE); \
		internet_address_list_set_address(list, 0, addr); \
	} \
\
END_PROPERTY

#if GMIME_MAJOR_VERSION < 3
	IMPLEMENT_STRING_PROPERTY(Sender, sender)
	IMPLEMENT_STRING_PROPERTY(ReplyTo, reply_to)
	IMPLEMENT_STRING_PROPERTY(Id, message_id)
#else
	IMPLEMENT_LIST_PROPERTY(Sender, sender)
	IMPLEMENT_LIST_PROPERTY(ReplyTo, reply_to)
	IMPLEMENT_STRING_PROPERTY(Id, message_id)
#endif


#define IMPLEMENT_RECIPIENT_PROPERTY(_name, _type, _adr_func, _string) \
BEGIN_PROPERTY(MimeMessage_##_name) \
\
	InternetAddressList *addr = g_mime_message_get_##_adr_func(MESSAGE, _type); \
	\
	if (READ_PROPERTY) \
	{ \
		char *list = internet_address_list_to_string(addr NULLGM3, FALSE); \
		GB.ReturnNewZeroString(list); \
		g_free(list); \
	} \
	else \
	{ \
		InternetAddressList *new_addr; \
		\
		internet_address_list_clear(addr); \
		\
		new_addr = internet_address_list_parse##_string(GM3NULL GB.ToZeroString(PROP(GB_STRING))); \
		internet_address_list_append(addr, new_addr); \
		g_object_unref(new_addr); \
	} \
\
END_PROPERTY

#if GMIME_MAJOR_VERSION < 3
	IMPLEMENT_RECIPIENT_PROPERTY(To, GMIME_RECIPIENT_TYPE_TO, recipients, _string);
	IMPLEMENT_RECIPIENT_PROPERTY(Cc, GMIME_RECIPIENT_TYPE_CC, recipients, _string);
	IMPLEMENT_RECIPIENT_PROPERTY(Bcc, GMIME_RECIPIENT_TYPE_BCC, recipients, _string);
#else
	IMPLEMENT_RECIPIENT_PROPERTY(To, GMIME_ADDRESS_TYPE_TO, addresses,);
	IMPLEMENT_RECIPIENT_PROPERTY(Cc, GMIME_ADDRESS_TYPE_CC, addresses,);
	IMPLEMENT_RECIPIENT_PROPERTY(Bcc, GMIME_ADDRESS_TYPE_BCC, addresses,);
#endif

BEGIN_PROPERTY(MimeMessage_Part)

	if (READ_PROPERTY)
		GB.ReturnObject(CMIMEPART_create(g_mime_message_get_mime_part(MESSAGE)));
	else
	{
		CMIMEPART *mpart = VPROP(GB_OBJECT);
		g_mime_message_set_mime_part(MESSAGE, mpart ? mpart->part : NULL);
	}

END_PROPERTY

BEGIN_PROPERTY(MimeMessage_Body)

	GB.ReturnObject(CMIMEPART_create(g_mime_message_get_body(MESSAGE)));

END_PROPERTY

BEGIN_METHOD_VOID(MimeMessage_ToString)

	char *str = g_mime_object_to_string((GMimeObject *)MESSAGE NULLGM3);
	GB.ReturnNewZeroString(str);
	g_free(str);

END_METHOD

/*BEGIN_PROPERTY(MimeMessage_Date)

	if (READ_PROPERTY)
	{
		time_t time;
		GB_DATE date;
	
		g_mime_message_get_date(MESSAGE, &time, NULL);
		GB.MakeDateFromTime(time, 0, &date);
		GB.ReturnDate(&date);
	}
	else
	{
		GB_DATE_SERIAL *date;
		struct tm time;
		
		date = GB.SplitDate(PROP(GB_DATE));
		time.tm_sec = date.
	}

END_PROPERTY*/

//-------------------------------------------------------------------------

BEGIN_METHOD(MimeMessage_Headers_get, GB_STRING name)

	GB.ReturnNewZeroString(g_mime_object_get_header((GMimeObject *)MESSAGE, GB.ToZeroString(ARG(name))));

END_METHOD

BEGIN_METHOD(MimeMessage_Headers_put, GB_STRING value; GB_STRING name)

	if (LENGTH(name))
		g_mime_object_set_header((GMimeObject *)MESSAGE, GB.ToZeroString(ARG(name)), GB.ToZeroString(ARG(value)) NULLGM3);
	else
		g_mime_object_remove_header((GMimeObject *)MESSAGE, GB.ToZeroString(ARG(name)));

END_METHOD

//-------------------------------------------------------------------------

GB_DESC MimeMessageHeadersDesc[] = 
{
	GB_DECLARE_VIRTUAL(".MimeMessage.Headers"),
	
	GB_METHOD("_get", "s", MimeMessage_Headers_get, "(Name)s"),
	GB_METHOD("_put", NULL, MimeMessage_Headers_put, "(Value)s(Name)s"),

	GB_END_DECLARE
};

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
	
	GB_PROPERTY_SELF("Headers", ".MimeMessage.Headers"),
	
	GB_PROPERTY("Part", "MimePart", MimeMessage_Part),
	GB_PROPERTY_READ("Body", "MimePart", MimeMessage_Body),
	
	GB_METHOD("ToString", "s", MimeMessage_ToString, NULL),
	
	GB_END_DECLARE
};
