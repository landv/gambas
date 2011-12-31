/***************************************************************************

  libsmtp_mime.h

  (c) 2000-2012 Benoît Minisini <gambas@users.sourceforge.net>

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
  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
  MA 02110-1301, USA.

***************************************************************************/

#ifndef __LIBSMTP_MIME_H
#define __LIBSMTP_MIME_H

/* #ifdef LIBSMTP_USE_MIME */

#ifndef __G_LIB_H__
	#include <glib.h>
#endif

#define LIB_SMTP_MIME_H


/* Encoding types */

#define LIBSMTP_ENC_7BIT	0
#define LIBSMTP_ENC_8BIT	1 /* not really used in current version */
#define LIBSMTP_ENC_BINARY	2 /* not really used in current version */
#define LIBSMTP_ENC_BASE64	3
#define LIBSMTP_ENC_QUOTED	4 /* not really used in current version */

#define LIBSMTP_MAX_ENC	4


struct libsmtp_part_struct {
	int internal_id;	/* internal id number */
	GString *Type;	/* MIME type */
	GString *Subtype;	/* MIME subtype */
	int Encoding;	/* MIME transfer encoding */
	GString *Charset;	/* optional charset for text MIME types */
	GString *Description;	/* MIME part description */
	GString *Boundary;	 /* optional Multipart boundary string */
	int Tag; /* tag for user */
	int length; // part length
};

bool libsmtp_part_is_type(struct libsmtp_part_struct *part, const char *type);

struct libsmtp_part_struct *libsmtp_part_new
    (struct libsmtp_part_struct *libsmtp_int_parent_part, const char *type,
    const char *subtype, int libsmtp_int_encoding, const char *charset,
    char *libsmtp_int_desc, int length, struct libsmtp_session_struct *libsmtp_session);
		
struct libsmtp_part_struct *libsmtp_part_query (struct libsmtp_session_struct *);

int libsmtp_mime_headers (struct libsmtp_session_struct *);

int libsmtp_part_send (char *, unsigned int, struct libsmtp_session_struct *);

int libsmtp_part_next (struct libsmtp_session_struct *);

/* internal functions */

const char *libsmtp_int_lookup_mime_encoding (struct libsmtp_part_struct *);

int libsmtp_int_nextpart (struct libsmtp_session_struct *);

void libsmtp_set_boundary(struct libsmtp_part_struct *part, int index);


/* MIME related error codes >= 2048 */
#define LIBSMTP_BADMIME	2048	/* You gave a bad type/subtype combo */
#define LIBSMTP_NOMULTIPART	2049	/* Parent is not multipart */
#define LIBSMTP_BADENCODING	2050	/* You gave a bad MIME/encoding combo */
#define LIBSMTP_NOPARENT	2051	/* There is no parent */
#define LIBSMTP_PART_EXISTS	2052	/* This part exists already */
#define LIBSMTP_PARTSERR	2053	/* Generic parts error */
#define LIBSMTP_PARTSFINISHED	2054	/* All parts finished */
#define LIBSMTP_BADCHARSET	2055

#define LIBSMTP_MAX_MIME_ERRNO	2055
/* #endif LIBSMTP_USE_MIME */

#endif
