/***************************************************************************

  CSmtpClient.c

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

#define __CSMTPCLIENT_C

#include "CSmtpClient.h"

//#define DEBUG_ME

static char *_tmp = NULL;

typedef
	struct {
		char *constant;
		int value;
		}
	MIME_FIND;

static MIME_FIND _types[] =
{
	{ "text",           		LIBSMTP_MIME_TEXT									},
	{ "message",        		LIBSMTP_MIME_MESSAGE							},
	{ "image",          		LIBSMTP_MIME_IMAGE	        			},
	{ "audio",          		LIBSMTP_MIME_AUDIO	        			},
	{ "video",          		LIBSMTP_MIME_VIDEO	        			},
	{ "application",    		LIBSMTP_MIME_APPLICATION	  			},
	{ "multipart",      		LIBSMTP_MIME_MULTIPART	    			},
	{ "custom!!",       		LIBSMTP_MIME_CUSTOM	        			},

	{ "plain",              LIBSMTP_MIME_SUB_PLAIN	          },
	{ "html",               LIBSMTP_MIME_SUB_HTML	            },
	{ "english",            LIBSMTP_MIME_SUB_ENGLISH	        },
	{ "richtext",           LIBSMTP_MIME_SUB_RICHTEXT	        },

	{ "rfc822",             LIBSMTP_MIME_SUB_RFC822	          },
	{ "partial",            LIBSMTP_MIME_SUB_PARTIAL          },

	{ "gif",                LIBSMTP_MIME_SUB_GIF              },
	{ "jpeg",               LIBSMTP_MIME_SUB_JPG              },
	{ "png",                LIBSMTP_MIME_SUB_PNG              },
	{ "tiff",               LIBSMTP_MIME_SUB_TIFF             },
	{ "x-ms-bmp",           LIBSMTP_MIME_SUB_MS_BMP	          },
	{ "x-xbitmap",          LIBSMTP_MIME_SUB_XBITMAP          },
	{ "x-xpixmap",          LIBSMTP_MIME_SUB_XPIXMAP          },
	{ "x-portable-anymap",  LIBSMTP_MIME_SUB_PORTABLE_ANYMAP	},
	{ "x-portable-bitmap",  LIBSMTP_MIME_SUB_PORTABLE_BITMAP	},
	{ "x-portable-graymap", LIBSMTP_MIME_SUB_PORTABLE_GRAYMAP },
	{ "x-portable-pixmap",  LIBSMTP_MIME_SUB_PORTABLE_PIXMAP	},

	{ "mpeg",               LIBSMTP_MIME_SUB_MPEGAUD          },
	{ "midi",               LIBSMTP_MIME_SUB_MIDI             },
	{ "x-wav",              LIBSMTP_MIME_SUB_WAV	            },
	{ "x-aiff",             LIBSMTP_MIME_SUB_AIFF	            },

	{ "mpeg",               LIBSMTP_MIME_SUB_MPEGVID          },
	{ "x-ms-video",         LIBSMTP_MIME_SUB_MSVIDEO	        },
	{ "quicktime",          LIBSMTP_MIME_SUB_QUICKTIME	      },
	{ "fli",                LIBSMTP_MIME_SUB_FLI	            },

	{ "rtf",                LIBSMTP_MIME_SUB_RTF	            },
	{ "postscript",         LIBSMTP_MIME_SUB_POSTSCRIPT	      },
	{ "pdf",                LIBSMTP_MIME_SUB_PDF              },
	{ "zip",                LIBSMTP_MIME_SUB_ZIP	            },
	{ "x-debian-package",   LIBSMTP_MIME_SUB_DEBIAN_PACKAGE	  },
	{ "x-executable",       LIBSMTP_MIME_SUB_EXECUTABLE	      },
	{ "x-gtar",             LIBSMTP_MIME_SUB_GTAR	            },
	{ "x-shellscript",      LIBSMTP_MIME_SUB_SHELLSCRIPT	    },
	{ "x-tar",              LIBSMTP_MIME_SUB_TAR	            },
	{ "octet-stream",       LIBSMTP_MIME_SUB_OCTET_STREAM	    },

	{ "mixed",              LIBSMTP_MIME_SUB_MIXED	          },
	{ "parallel",           LIBSMTP_MIME_SUB_PARALLEL	        },
	{ "digest",             LIBSMTP_MIME_SUB_DIGEST	          },
	{ "alternative",        LIBSMTP_MIME_SUB_ALTERNATIVE      },

	{ NULL, 0 }
};

static MIME_FIND _charsets[] =
{
	{ "us-ascii",           LIBSMTP_CHARSET_USASCII	          },
	{ "iso-8859-1",         LIBSMTP_CHARSET_ISO8859_1	        },
	{ "iso-8859-2",         LIBSMTP_CHARSET_ISO8859_2	        },
	{ "iso-8859-3",         LIBSMTP_CHARSET_ISO8859_3	        },
	{ "iso-8859-15",        LIBSMTP_CHARSET_ISO8859_15        },
	{ "utf-8",              LIBSMTP_CHARSET_UTF_8             },

	{ NULL, 0 }
};

static char *get_address(char *address)
{
	long len;

	GB.FreeString(&_tmp);

	if (!address || *address == 0)
		return "";

	len = GB.StringLength(address);

	if (address[0] == '<' && address[len - 1] == '>')
		return address;

	GB.NewString(&_tmp, "<", 1);
	GB.AddString(&_tmp, address, len);
	GB.AddString(&_tmp, ">", 1);
	return _tmp;
}

static bool send_recipient(struct libsmtp_session_struct *session, GB_ARRAY rec, int type)
{
	int i;

	if (!rec)
		return FALSE;

	for (i = 0; i < GB.Array.Count(rec); i++)
	{
		if (libsmtp_add_recipient(type, get_address(*(char **)GB.Array.Get(rec, i)), session))
			return TRUE;
	}

	return FALSE;
}

static int find_constant(MIME_FIND *table, char *str, int len)
{
	if (len <= 0)
		len = strlen(str);

	if (len >= 2 && str[0] == '"' && str[len -1] == '"')
	{
		str++;
		len--;
	}

	while (table->constant)
	{
		if (strncasecmp(table->constant, str, len) == 0)
			return table->value;
		table++;
	}

	return (-1);
}

static int decode_mime(char *mime, int *type, int *subtype, int *encoding, int *charset)
{
	char *p, *p2;

  *charset = find_constant(_charsets, GB.System.Charset(), 0);

	if (!mime || !*mime)
	{
		*type = LIBSMTP_MIME_TEXT;
		*subtype = LIBSMTP_MIME_SUB_PLAIN;
		*encoding = LIBSMTP_ENC_QUOTED;
		return FALSE;
	}

	p = mime;
	p2 = index(p, '/');
	if (!p2)
	{
		GB.Error("Cannot find MIME subtype");
		return TRUE;
	}

	*type = find_constant(_types, p, p2 - p);
	if (*type < 0)
	{
		GB.Error("Unknown MIME type");
		return TRUE;
	}

	p = p2 + 1;
	p2 = index(p, ';');
	if (p2)
	{
		*subtype = find_constant(_types, p, p2 - p);
		p = p2 + 1;
		if (strncasecmp(p, "CHARSET=", 8))
		{
			GB.Error("Syntax error in MIME charset");
			return TRUE;
		}
		p += 8;
		*charset = find_constant(_charsets, p, &mime[strlen(mime)] - p);
	}
	else
	{
		*subtype = find_constant(_types, p, &mime[strlen(mime)] - p);
	}

	if (*subtype < 0)
	{
		GB.Error("Unknown MIME subtype");
		return TRUE;
	}

	if (*type == LIBSMTP_MIME_TEXT || *type == LIBSMTP_MIME_MESSAGE)
	{
		*encoding = LIBSMTP_ENC_QUOTED;

		if (*charset < 0)
		{
			GB.Error("Unknown MIME charset");
			return TRUE;
		}
	}
	else
	{
		*charset = LIBSMTP_CHARSET_NOCHARSET;

		if (*type == LIBSMTP_MIME_MULTIPART)
			*encoding = LIBSMTP_ENC_7BIT;
		else
			*encoding = LIBSMTP_ENC_BASE64;
	}

	return FALSE;
}


#if 0
static int old_add_part(CSMTPCLIENT *_object, char *mime, char *name, int length)
{
	struct libsmtp_part_struct *part;
	struct libsmtp_part_struct *parent_part;
	int id;
	int type, subtype, encoding, charset;
	char buffer[24];

	if (!THIS->main)
	{
		if (THIS->alternative)
			THIS->main = libsmtp_part_new(NULL, LIBSMTP_MIME_MULTIPART, LIBSMTP_MIME_SUB_ALTERNATIVE, LIBSMTP_ENC_7BIT, LIBSMTP_CHARSET_NOCHARSET, "MIME main part", -1, THIS->session);
		else
			THIS->main = libsmtp_part_new(NULL, LIBSMTP_MIME_MULTIPART, LIBSMTP_MIME_SUB_MIXED, LIBSMTP_ENC_7BIT, LIBSMTP_CHARSET_NOCHARSET, "MIME main part", -1, THIS->session);
		THIS->main->Tag = -1;
		THIS->parent = -1;
	}

	id = GB.Count(THIS->parts);

	if (THIS->parent < 0 || THIS->parent >= id)
		parent_part = THIS->main;
	else
		parent_part = THIS->parts[THIS->parent];

	if (decode_mime(mime, &type, &subtype, &encoding, &charset))
		return -1;

	if (!name || !*name)
	{
		sprintf(buffer, "MIME part #%d", id);
		name = buffer;
	}

  part = libsmtp_part_new(parent_part, type, subtype, encoding, charset, name, -1, THIS->session);
  if (!part)
  {
    GB.Error("Cannot add part: &1", libsmtp_strerr(THIS->session));
    return -1;    
  }
  
  part->Tag = id;
  #ifdef DEBUG_ME
  printf("part = %p  parent_part = %p  Tag = %d\n", part, parent_part, id);
  #endif

	*((struct libsmtp_part_struct **)GB.Add(&THIS->parts)) = part;
	return id;
}
#endif

static bool begin_session(CSMTPCLIENT *_object)
{
	int npart;
	CSMTPPART *p;
	struct libsmtp_part_struct *main_part;
	struct libsmtp_part_struct *part;
	int parent;
	struct libsmtp_part_struct *parent_part;
	int i;
	int type, subtype, encoding, charset;
	char buffer[24];
	char *name;
	
	if (THIS->session)
		return TRUE;
		
	#ifdef DEBUG_ME
	fprintf(stderr, "begin_session\n");	
	#endif
	
	THIS->session = libsmtp_session_initialize(THIS->debug, THIS->stream ? GB.Stream.Get(THIS->stream) : NULL);
	
	npart = GB.Count(THIS->parts);
	if (npart == 0)
	{
		#ifdef DEBUG_ME
		fprintf(stderr, "Add void part\n");	
		#endif	
		p = (CSMTPPART *)GB.Add(&THIS->parts);
		GB.NewString(&p->data, "\n", 1);
		npart = 1;
	}
	
	if (npart == 1)
	{
		p = &THIS->parts[0];
		
		if (decode_mime(p->mime, &type, &subtype, &encoding, &charset))
			return TRUE;
	
		name = p->name;
		if (!name || !*name)
			name = "MIME part";
			
		parent_part = NULL;
	
		part = libsmtp_part_new(parent_part, type, subtype, encoding, charset, name, -1, THIS->session);
		if (!part)
		{
			GB.Error("Cannot add part: &1", libsmtp_strerr(THIS->session));
			return TRUE;    
		}
		
		part->Tag = 0;
		#ifdef DEBUG_ME
		fprintf(stderr, "part = %p  parent_part = %p  Tag = 0\n", part, parent_part);
		#endif
		p->part = part;
	}
	else
	{
		// Check mimetype
		
		for (i = 0; i < npart; i++)
		{
			if (decode_mime(THIS->parts[i].mime, &type, &subtype, &encoding, &charset))
				return TRUE;
		}
	
		if (THIS->alternative)
			main_part = libsmtp_part_new(NULL, LIBSMTP_MIME_MULTIPART, LIBSMTP_MIME_SUB_ALTERNATIVE, LIBSMTP_ENC_7BIT, LIBSMTP_CHARSET_NOCHARSET, "MIME main part", -1, THIS->session);
		else
			main_part = libsmtp_part_new(NULL, LIBSMTP_MIME_MULTIPART, LIBSMTP_MIME_SUB_MIXED, LIBSMTP_ENC_7BIT, LIBSMTP_CHARSET_NOCHARSET, "MIME main part", -1, THIS->session);
	
		#ifdef DEBUG_ME
		fprintf(stderr, "main part = %p  alternative = %d\n", main_part, THIS->alternative);	
		#endif
		
		main_part->Tag = -1;
		parent = -1;
		
		for (i = 0; i < npart; i++)
		{
			p = &THIS->parts[i];
			
			parent_part = main_part;
		
			decode_mime(p->mime, &type, &subtype, &encoding, &charset);
		
			name = p->name;
			if (!name || !*name)
			{
				sprintf(buffer, "MIME part #%d", i + 1);
				name = buffer;
			}
		
			part = libsmtp_part_new(parent_part, type, subtype, encoding, charset, name, -1, THIS->session);
			if (!part)
			{
				GB.Error("Cannot add part: &1", libsmtp_strerr(THIS->session));
				return TRUE;    
			}
			
			part->Tag = i;
			#ifdef DEBUG_ME
			fprintf(stderr, "part = %p  parent_part = %p  Tag = %d\n", part, parent_part, i);
			#endif
			p->part = part;
		}
	}
	
	return FALSE;
}

static void free_parts(CSMTPCLIENT *_object)
{
	int i;
	CSMTPPART *part;
	
	#ifdef DEBUG_ME
	fprintf(stderr, "free_parts\n");	
	#endif
	
	for (i = 0; i < GB.Count(THIS->parts); i++)
	{
		part = &THIS->parts[i];
		GB.FreeString(&part->name);
		GB.FreeString(&part->mime);
		GB.FreeString(&part->data);
	}

	GB.FreeArray(&THIS->parts);
	GB.NewArray(&THIS->parts, sizeof(CSMTPPART), 0);
}

static void end_session(CSMTPCLIENT *_object)
{
	if (!THIS->session)
		return;
	
	#ifdef DEBUG_ME
	fprintf(stderr, "end_session\n");	
	#endif
  
  libsmtp_quit(THIS->session);
	
	libsmtp_close(THIS->session);
	libsmtp_free(THIS->session);
	THIS->session = NULL;
}


BEGIN_METHOD_VOID(SmtpClient_new)

	GB.Array.New(&THIS->to, GB_T_STRING, 0);
	GB.Ref(THIS->to);
	GB.Array.New(&THIS->cc, GB_T_STRING, 0);
	GB.Ref(THIS->cc);
	GB.Array.New(&THIS->bcc, GB_T_STRING, 0);
	GB.Ref(THIS->bcc);

	GB.NewArray(&THIS->parts, sizeof(CSMTPPART), 0);
	
END_METHOD


BEGIN_METHOD_VOID(SmtpClient_free)

	end_session(THIS);
	free_parts(THIS);

	GB.FreeString(&THIS->host);
	GB.FreeString(&THIS->from);
	GB.FreeString(&THIS->subject);
	GB.FreeString(&THIS->user);
	GB.FreeString(&THIS->password);

	GB.Unref((void **)&THIS->to);
	GB.Unref((void **)&THIS->cc);
	GB.Unref((void **)&THIS->bcc);

	GB.FreeArray(&THIS->parts);
	
	GB.Unref(&THIS->stream);

	GB.FreeString(&_tmp);

END_METHOD


BEGIN_METHOD_VOID(SmtpClient_send)

  struct libsmtp_session_struct	*session;
  int i;
  const char *error;
  const char *where;
  char *addr;
  char buf[8];
  
  addr = get_address(THIS->from);
  if (*addr == 0)
  {
  	GB.Error("The From property must be set");
  	return;
	}

	if (begin_session(THIS))
	{
		end_session(THIS);
		return;
	}
		
	session = THIS->session;

  libsmtp_set_environment(addr, THIS->subject, 0, session);

	if (send_recipient(session, THIS->to, LIBSMTP_REC_TO)) { where = "setting TO recipient"; goto __ERROR; }
	if (send_recipient(session, THIS->cc, LIBSMTP_REC_CC)) { where = "setting CC recipient"; goto __ERROR; }
	if (send_recipient(session, THIS->bcc, LIBSMTP_REC_BCC)) { where = "setting BCC recipient"; goto __ERROR; }

  if (libsmtp_connect(THIS->host ? THIS->host : "localhost", THIS->port, 0, session)) { where = "connecting to SMTP server"; goto __ERROR; }
  
  if (THIS->user)
	{
		if (libsmtp_authenticate(session, THIS->user, THIS->password ? THIS->password : ""))
		{
			where = "sending authorization"; goto __ERROR;
		}
	}
  
  if (libsmtp_dialogue(session)) { where = "starting dialog"; goto __ERROR; }
  if (libsmtp_headers(session)) { where = "sending headers"; goto __ERROR; }
  if (libsmtp_mime_headers(session)) { where = "sending mime headers"; goto __ERROR; }

	//if (libsmtp_body_send_raw((char *)test, strlen(test), session)) goto __ERROR;

	if (GB.Count(THIS->parts))
	{
		#ifdef DEBUG_ME
		fprintf(stderr, "**** Send parts\n");
		#endif
	
		for (;;)
		{
			#ifdef DEBUG_ME
			fprintf(stderr, ">>>> libsmtp_part_next\n");
			#endif
			if (libsmtp_part_next(session))
			{
				if (libsmtp_errno(session))
				{
					where = "reading next part";
					goto __ERROR;
				}
				else
					break;
			}
			#ifdef DEBUG_ME
			fprintf(stderr, "<<<< libsmtp_part_next\n");
			#endif
	
			i = libsmtp_part_query(session)->Tag;
			#ifdef DEBUG_ME
			fprintf(stderr, "**** part = %p  Tag = %d\n", libsmtp_part_query(session), i);
			#endif
			if (THIS->parts[i].data)
			{
				if (libsmtp_part_send(THIS->parts[i].data, GB.StringLength(THIS->parts[i].data), session))
				{
					where = "sending part";
					goto __ERROR;
				}
			}
			else
			{
				if (libsmtp_part_send("", 0, session))
				{
					where = "sending part";
					goto __ERROR;
				}
			}
		}
	}

  if (libsmtp_body_end(session)) { where = "ending dialog"; goto __ERROR; }

	end_session(THIS);
	free_parts(THIS);
	return;

__ERROR:

	sprintf(buf, "%d", session->LastResponseCode);
	error = libsmtp_strerr(session);
	//if (!error || !*error)
	//	error = "Unknown error";
	GB.Error("&1 while &2 (SMTP error code #&3)", error, where, buf);
	end_session(THIS);

END_METHOD


#define IMPLEMENT_STRING_PROPERTY(_property) \
BEGIN_PROPERTY(SmtpClient_##_property) \
\
	if (READ_PROPERTY) \
		GB.ReturnString(THIS->_property); \
	else \
		GB.StoreString(PROP(GB_STRING), &THIS->_property); \
\
END_PROPERTY

IMPLEMENT_STRING_PROPERTY(host);
IMPLEMENT_STRING_PROPERTY(from);
IMPLEMENT_STRING_PROPERTY(subject);
IMPLEMENT_STRING_PROPERTY(user);
IMPLEMENT_STRING_PROPERTY(password);

BEGIN_PROPERTY(SmtpClient_port)

	if (READ_PROPERTY)
		GB.ReturnInteger(THIS->port);
	else
		THIS->port = VPROP(GB_INTEGER);

END_PROPERTY

#define IMPLEMENT_RECIPIENT(_recipient) \
BEGIN_PROPERTY(SmtpClient_##_recipient) \
\
		GB.ReturnObject(THIS->_recipient); \
\
END_PROPERTY

IMPLEMENT_RECIPIENT(to);
IMPLEMENT_RECIPIENT(cc);
IMPLEMENT_RECIPIENT(bcc);


BEGIN_METHOD(SmtpClient_Add, GB_STRING data; GB_STRING mime; GB_STRING name)

	CSMTPPART *part = (CSMTPPART *)GB.Add(&THIS->parts);
	
	if (!MISSING(name))
		GB.StoreString(ARG(name), &part->name);
	if (!MISSING(mime))
		GB.StoreString(ARG(mime), &part->mime);
	GB.StoreString(ARG(data), &part->data);
	
END_METHOD


BEGIN_PROPERTY(SmtpClient_Alternative)

	if (READ_PROPERTY)
		GB.ReturnBoolean(THIS->alternative);
	else
		THIS->alternative = VPROP(GB_BOOLEAN);

END_PROPERTY


BEGIN_PROPERTY(SmtpClient_Count)

	GB.ReturnInteger(GB.Count(THIS->parts));

END_PROPERTY


BEGIN_PROPERTY(SmtpClient_Debug)

	if (READ_PROPERTY)
		GB.ReturnBoolean(THIS->debug);
	else
		THIS->debug = VPROP(GB_BOOLEAN);

END_PROPERTY


BEGIN_PROPERTY(SmtpClient_Stream)

	if (READ_PROPERTY)
		GB.ReturnObject(THIS->stream);
	else
		GB.StoreObject(PROP(GB_OBJECT), POINTER(&THIS->stream));

END_PROPERTY


GB_DESC CSmtpClientDesc[] =
{
  GB_DECLARE("SmtpClient", sizeof(CSMTPCLIENT)),

  GB_METHOD("_new", NULL, SmtpClient_new, NULL),
  GB_METHOD("_free", NULL, SmtpClient_free, NULL),

	GB_PROPERTY("Debug", "b", SmtpClient_Debug),
	GB_PROPERTY("_Stream", "Stream", SmtpClient_Stream),

	GB_PROPERTY("Host", "s", SmtpClient_host),
	GB_PROPERTY("Port", "i", SmtpClient_port),
	
  GB_PROPERTY("User", "s", SmtpClient_user),
  GB_PROPERTY("Password", "s", SmtpClient_password),
	
  GB_PROPERTY("From", "s", SmtpClient_from),
  GB_PROPERTY("Subject", "s", SmtpClient_subject),
  GB_PROPERTY_READ("To", "String[]", SmtpClient_to),
  GB_PROPERTY_READ("Cc", "String[]", SmtpClient_cc),
  GB_PROPERTY_READ("Bcc", "String[]", SmtpClient_bcc),

  GB_PROPERTY("Alternative", "b", SmtpClient_Alternative),
  GB_METHOD("Add", NULL, SmtpClient_Add, "(Data)s[(MimeType)s(Name)s]"),
  GB_PROPERTY_READ("Count", "i", SmtpClient_Count),
 
  GB_METHOD("Send", NULL, SmtpClient_send, NULL),

  GB_CONSTANT("_IsControl", "b", TRUE),
  GB_CONSTANT("_IsVirtual", "b", TRUE),
  GB_CONSTANT("_Group", "s", "Network"),
  GB_CONSTANT("_Properties", "s", "Host,Port,Debug"),

  GB_END_DECLARE
};

