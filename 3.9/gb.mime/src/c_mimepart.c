/***************************************************************************

  c_mimepart.c

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

#define __C_MIMEPART_C

#include <string.h>

#include "c_mimemessage.h"
#include "c_mimepart.h"

#define THIS ((CMIMEPART *)_object)
#define PART THIS->part
#define MPART ((GMimePart *)THIS->part)
#define MMPART ((GMimeMultipart *)THIS->part)
#define MSPART ((GMimeMessagePart *)THIS->part)

//-------------------------------------------------------------------------

static bool _do_not_create_part = FALSE;

CMIMEPART *CMIMEPART_create(GMimeObject *part)
{
	CMIMEPART *mpart;
	
	if (!part)
		return NULL;
	
	mpart = (CMIMEPART *)g_object_get_data(G_OBJECT(part), "gambas-object");
	if (!mpart)
	{
		_do_not_create_part = TRUE;
		mpart = (CMIMEPART *)GB.New(GB.FindClass("MimePart"), NULL, NULL);
		_do_not_create_part = FALSE;
		mpart->part = part;
		g_object_ref(part);
		g_object_set_data(G_OBJECT(part), "gambas-object", (gpointer)mpart);
	}
	
	return mpart;
}

//-------------------------------------------------------------------------

BEGIN_METHOD(MimePart_new, GB_STRING ctype)

	GMimeObject *part;
	GMimeContentType *ctype;
	
	if (_do_not_create_part)
		return;
	
	if (MISSING(ctype))
		ctype = g_mime_content_type_new_from_string("text/plain;charset=utf-8");
	else
		ctype = g_mime_content_type_new_from_string(GB.ToZeroString(ARG(ctype)));
	
	if (g_mime_content_type_is_type(ctype, "multipart", "*"))
		part = (GMimeObject *)g_mime_multipart_new_with_subtype(g_mime_content_type_get_media_subtype(ctype));
	else if (g_mime_content_type_is_type(ctype, "message", "*"))
		part = (GMimeObject *)g_mime_message_part_new(g_mime_content_type_get_media_subtype(ctype));
	else
	{
		part = (GMimeObject* )g_mime_part_new();
		g_mime_object_set_content_type(part, ctype);

		if (g_mime_content_type_is_type(ctype, "text", "*"))
			g_mime_part_set_content_encoding((GMimePart *)part, GMIME_CONTENT_ENCODING_QUOTEDPRINTABLE);
		else
			g_mime_part_set_content_encoding((GMimePart *)part, GMIME_CONTENT_ENCODING_BASE64);
	}
	
	PART = part;
	//g_object_ref(part);
	g_object_set_data(G_OBJECT(part), "gambas-object", (gpointer)THIS);
	
END_METHOD

BEGIN_METHOD_VOID(MimePart_free)

	g_object_set_data(G_OBJECT(PART), "gambas-object", NULL);
	g_object_unref(PART);

END_METHOD

#define IMPLEMENT_STRING_PROPERTY(_name, _func) \
BEGIN_PROPERTY(MimePart_##_name) \
\
	if (READ_PROPERTY) \
		GB.ReturnNewZeroString(g_mime_object_get_##_func(PART)); \
	else \
		g_mime_object_set_##_func(PART, GB.ToZeroString(PROP(GB_STRING))); \
\
END_PROPERTY

IMPLEMENT_STRING_PROPERTY(Disposition, disposition);
IMPLEMENT_STRING_PROPERTY(ContentId, content_id);

BEGIN_PROPERTY(MimePart_ContentType)

	GMimeContentType *ctype;

	if (READ_PROPERTY)
	{
		ctype = g_mime_object_get_content_type(PART);
		char *str = g_mime_content_type_to_string(ctype);
		GB.ReturnNewZeroString(str);
		g_free(str);
	}
	else
	{
		ctype = g_mime_content_type_new_from_string(GB.ToZeroString(PROP(GB_STRING)));
		g_mime_object_set_content_type(PART, ctype);
		g_object_unref(ctype);
	}

END_PROPERTY


BEGIN_PROPERTY(MimePart_ContentDisposition)

	GMimeContentDisposition *cdisp;

	//if (READ_PROPERTY)
	//{
		cdisp = g_mime_object_get_content_disposition(PART);
		char *str = g_mime_content_disposition_to_string(cdisp, TRUE);
		GB.ReturnNewZeroString(str);
		g_free(str);
	/*}
	else
	{
		cdisp = g_mime_content_disposition_new_from_string(GB.ToZeroString(PROP(GB_STRING)));
		g_mime_object_set_content_disposition(PART, cdisp);
		g_object_unref(cdisp);
	}*/

END_PROPERTY


BEGIN_METHOD_VOID(MimePart_ToString)

	char *str = g_mime_object_to_string(PART);
	GB.ReturnNewZeroString(str);
	g_free(str);

END_METHOD


//-------------------------------------------------------------------------

#define CHECK_PART() if (!GMIME_IS_PART(PART)) { GB.Error("Not a part"); return; }

BEGIN_PROPERTY(MimePart_ContentEncoding)

	CHECK_PART();
	
	if (READ_PROPERTY)
		GB.ReturnInteger(g_mime_part_get_content_encoding(MPART));
	else
		g_mime_part_set_content_encoding(MPART, VPROP(GB_INTEGER));

END_PROPERTY


BEGIN_PROPERTY(MimePart_Data)

	GMimeDataWrapper *content;
	GMimeStream *stream;
	GByteArray *array;
	
	if (READ_PROPERTY)
	{
		if (!GMIME_IS_PART(PART))
		{
			GB.ReturnNull();
			return;
		}
		
		content = g_mime_part_get_content_object(MPART);
	
		array = g_byte_array_new();
		
		/* create a new stream for writing to memory */
		stream = g_mime_stream_mem_new_with_byte_array(array);
		g_mime_stream_mem_set_owner((GMimeStreamMem *)stream, TRUE);
		
		/* write the contents to the stream */
		g_mime_data_wrapper_write_to_stream(content, stream);
		
		if (!array->data)
			GB.ReturnVoidString();
		else
			GB.ReturnNewString((char *)array->data, (int)array->len);
		
		/* free the output stream */
		g_object_unref (stream);
		
	}
	else
	{
		CHECK_PART();

		/* create the parts' content stream */
		stream = g_mime_stream_mem_new_with_buffer(PSTRING(), PLENGTH());
	
		/* create the content object - since the stream is not base64
			or QP encoded or anything, we can use
			GMIME_CONTENT_ENCODING_DEFAULT as the encoding type (_DEFAULT
			is the same as saying "nothing specified") */
		content = g_mime_data_wrapper_new_with_stream (stream, GMIME_CONTENT_ENCODING_DEFAULT);
		g_object_unref (stream);
	
		/* set the content object on the new mime part */
		g_mime_part_set_content_object(MPART, content);
		g_object_unref(content);
	
		/* if we want, we can tell GMime that the content should be base64 encoded when written to disk... */
		//g_mime_part_set_content_encoding (mime_part, GMIME_CONTENT_ENCODING_BASE64);
	}

END_PROPERTY


BEGIN_PROPERTY(MimePart_FileName)

	if (READ_PROPERTY)
	{
		if (GMIME_IS_PART(PART))
			GB.ReturnNewZeroString(g_mime_part_get_filename(MPART));
		else
			GB.ReturnNull();
	}
	else
	{
		CHECK_PART();
		g_mime_part_set_filename(MPART, GB.ToZeroString(PROP(GB_STRING)));
	}

END_PROPERTY

//-------------------------------------------------------------------------

BEGIN_PROPERTY(MimePart_Count)

	if (!GMIME_IS_MULTIPART(PART))
		GB.ReturnInteger(0);
	else
		GB.ReturnInteger(g_mime_multipart_get_count(MMPART));

END_PROPERTY


BEGIN_METHOD(MimePart_get, GB_INTEGER index)

	int count;
	int index;

	if (!GMIME_IS_MULTIPART(PART))
	{
		GB.Error(GB_ERR_BOUND);
		return;
	}
	
	count = g_mime_multipart_get_count(MMPART);
	index = VARG(index);
	
	if (index < 0 || index >= count)
	{
		GB.Error(GB_ERR_BOUND);
		return;
	}
	
	GB.ReturnObject(CMIMEPART_create(g_mime_multipart_get_part(MMPART, index)));

END_METHOD


BEGIN_METHOD_VOID(MimePart_next)

	int count;
	int *index;

	if (!GMIME_IS_MULTIPART(PART))
		goto __STOP;
	
	count = g_mime_multipart_get_count(MMPART);
	index = (int *)GB.GetEnum();
	
	if (*index >= count)
		goto __STOP;
	
	GB.ReturnObject(CMIMEPART_create(g_mime_multipart_get_part(MMPART, *index)));
	(*index)++;
	return;

__STOP:

	GB.StopEnum();
	
END_METHOD


BEGIN_METHOD(MimePart_Add, GB_OBJECT part)

	CMIMEPART *part = VARG(part);
	
	if (GB.CheckObject(part))
		return;

	if (!GMIME_IS_MULTIPART(PART))
	{
		GB.Error("Not a multipart");
		return;
	}

	g_mime_multipart_add(MMPART, part->part);

END_METHOD

//-------------------------------------------------------------------------

BEGIN_PROPERTY(MimePart_Message)

	if (READ_PROPERTY)
	{
		if (!GMIME_IS_MESSAGE_PART(PART))
			GB.ReturnNull();
		else
			GB.ReturnObject(CMIMEMESSAGE_create(g_mime_message_part_get_message(MSPART)));
	}
	else
	{
		CMIMEMESSAGE *mmsg;
		
		if (!GMIME_IS_MESSAGE_PART(PART))
		{
			GB.Error("Not a message part");
			return;
		}
		
		mmsg = VPROP(GB_OBJECT);
		g_mime_message_part_set_message(MSPART, mmsg ? mmsg->message : NULL);
	}

END_PROPERTY

//-------------------------------------------------------------------------

BEGIN_METHOD(MimePart_Headers_get, GB_STRING name)

	GB.ReturnNewZeroString(g_mime_object_get_header(PART, GB.ToZeroString(ARG(name))));

END_METHOD

BEGIN_METHOD(MimePart_Headers_put, GB_STRING value; GB_STRING name)

	if (LENGTH(name))
		g_mime_object_set_header(PART, GB.ToZeroString(ARG(name)), GB.ToZeroString(ARG(value)));
	else
		g_mime_object_remove_header(PART, GB.ToZeroString(ARG(name)));

END_METHOD

//-------------------------------------------------------------------------

GB_DESC MimePartHeadersDesc[] = 
{
	GB_DECLARE_VIRTUAL(".MimePart.Headers"),
	
	GB_METHOD("_get", "s", MimePart_Headers_get, "(Name)s"),
	GB_METHOD("_put", NULL, MimePart_Headers_put, "(Value)s(Name)s"),

	GB_END_DECLARE
};

GB_DESC MimePartDesc[] = 
{
	GB_DECLARE("MimePart", sizeof(CMIMEPART)),
	
	GB_METHOD("_new", NULL, MimePart_new, "[(ContentType)s]"),
	GB_METHOD("_free", NULL, MimePart_free, NULL),
	
	GB_PROPERTY("ContentType", "s", MimePart_ContentType),
	GB_PROPERTY("Disposition", "s", MimePart_Disposition),
	GB_PROPERTY("ContentDisposition", "s", MimePart_ContentDisposition),
	GB_PROPERTY("ContentId", "s", MimePart_ContentId),
	
	GB_PROPERTY_SELF("Headers", ".MimePart.Headers"),
	
	GB_METHOD("ToString", "s", MimePart_ToString, NULL),
	
	// Specific to GMimePart
	
	//GB_PROPERTY("ContentDescription", "s", MimePart_ContentDescription),
	//GB_PROPERTY("ContentMD5", "s", MimePart_ContentMD5),
	//GB_PROPERTY("ContentLocation", "s", MimePart_ContentLocation),
	GB_PROPERTY("ContentEncoding", "i", MimePart_ContentEncoding),
	GB_PROPERTY("FileName", "s", MimePart_FileName),
	GB_PROPERTY("Data", "s", MimePart_Data),
	GB_PROPERTY("Message", "MimeMessage", MimePart_Message),
	
	GB_PROPERTY_READ("Count", "i", MimePart_Count),
	//GB_METHOD("Clear", NULL, MimePart_Clear, NULL),
	GB_METHOD("_get", "MimePart", MimePart_get, "(Index)i"),
	GB_METHOD("_next", "MimePart", MimePart_next, NULL),
	GB_METHOD("Add", NULL, MimePart_Add, "(Part)MimePart"),
	
	GB_END_DECLARE
};