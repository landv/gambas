/***************************************************************************

  comm.c

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
/*
  libsmtp is a library to send mail via SMTP
     This is the MIME handling part for communcating with the server

Copyright  2001 Kevin Read <obsidian@berlios.de>

This software is available under the GNU Lesser Public License as described
in the COPYING file.

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free
Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

Kevin Read <obsidian@berlios.de>
Thu Aug 16 2001 */

/* This will only be included when MIME is enabled */

#include "gb_common.h"

/* #ifndef __G_LIB_H__ */
  #include <glib.h>
/* #endif */

//#include "../config.h"

#include "libsmtp.h"
#include "libsmtp_mime.h"


/* #define LIBSMTP_DEBUG */

int libsmtp_mime_headers (struct libsmtp_session_struct *libsmtp_session)
{
   /* If we use the MIME functionality, we need to send some stuff */
   int libsmtp_temp;
   GString *libsmtp_temp_gstring;
   char *libsmtp_temp_string;
   struct libsmtp_part_struct *libsmtp_temp_part;

   libsmtp_temp_gstring=g_string_new (NULL);

  /* Are we at the end of the dialogue stage, but haven't sent the
     DATA yet? */
  if ((libsmtp_session->Stage < LIBSMTP_RECIPIENT_STAGE) || \
      (libsmtp_session->Stage > LIBSMTP_HEADERS_STAGE))
  {
    libsmtp_session->ErrorCode = LIBSMTP_BADSTAGE;
    return LIBSMTP_BADSTAGE;
  }

  /* Maybe we are already in DATA mode so... */
  if (libsmtp_session->Stage < LIBSMTP_DATA_STAGE)
  {
    /* Great finality. After this no more dialogue can go on */
    g_string_assign (libsmtp_temp_gstring, "dAta\r\n");

    if (libsmtp_int_send (libsmtp_temp_gstring, libsmtp_session, 2))
      return LIBSMTP_ERRORSENDFATAL;

    /* What has he to say to a little bit of DATA? */

    if (libsmtp_int_read (libsmtp_temp_gstring, libsmtp_session, 2))
    {
      libsmtp_session->ErrorCode = LIBSMTP_ERRORREADFATAL;
      return LIBSMTP_ERRORREADFATAL;
    }

    if (libsmtp_session->LastResponseCode != 354)
    {
			libsmtp_close(libsmtp_session);
      libsmtp_session->ErrorCode = LIBSMTP_WONTACCEPTDATA;
      return LIBSMTP_WONTACCEPTDATA;
    }

    /* We enter the data stage now */
    libsmtp_session->Stage = LIBSMTP_DATA_STAGE;
  }

  /* If we use the MIME stuff we tell them this, too */
  g_string_assign (libsmtp_temp_gstring, "Mime-Version: 1.0\r\n");

  #ifdef LIBSMTP_DEBUG
    printf ("libsmtp_mime_headers: %s", libsmtp_temp_gstring->str);
  #endif

  if (libsmtp_int_send (libsmtp_temp_gstring, libsmtp_session, 1))
    return LIBSMTP_ERRORSENDFATAL;

  /* If there are no parts defined by now, we assume its
     text/plain MIME type and US-ASCII charset */

  if (!libsmtp_session->Parts)
  {
    g_string_assign (libsmtp_temp_gstring, "Content-Type: text/plain; charset=\"us-ascii\r\n\"");

    #ifdef LIBSMTP_DEBUG
      printf ("libsmtp_mime_headers: %s", libsmtp_temp_gstring->str);
    #endif

    if (libsmtp_int_send (libsmtp_temp_gstring, libsmtp_session, 1))
      return LIBSMTP_ERRORSENDFATAL;
  }
  else
  {
    libsmtp_temp_part=libsmtp_session->Parts->data;

    /* We should check for valied MIME settings first */
    if ((libsmtp_temp=libsmtp_int_check_part (libsmtp_temp_part)))
    {
      libsmtp_session->ErrorCode=libsmtp_temp;
      return libsmtp_temp;
    }

    /* Then we look up the names of the MIME settings of the main body part
       and send them as headers */

    g_string_sprintf (libsmtp_temp_gstring, "Content-Type: %s/%s", \
       libsmtp_int_lookup_mime_type (libsmtp_temp_part), \
       libsmtp_int_lookup_mime_subtype (libsmtp_temp_part));

    #ifdef LIBSMTP_DEBUG
      printf ("libsmtp_mime_headers: %s. Type: %d/%d\n", libsmtp_temp_gstring->str, \
         libsmtp_temp_part->Type, libsmtp_temp_part->Subtype);
    #endif

    if (libsmtp_int_send (libsmtp_temp_gstring, libsmtp_session, 1))
      return LIBSMTP_ERRORSENDFATAL;

    /* Multipart parts need a boundary set. We define it as a fixed string
       at the moment, with an added dynamic number. This is always 1 here. */

    if (libsmtp_temp_part->Type==LIBSMTP_MIME_MULTIPART)
    {
			libsmtp_set_boundary(libsmtp_temp_part, 0);

      #ifdef LIBSMTP_DEBUG
        printf ("libsmtp_mime_headers: %s", libsmtp_temp_part->Boundary->str);
      #endif

      g_string_sprintf (libsmtp_temp_gstring, "; boundary=\"%s\"",
          libsmtp_temp_part->Boundary->str);

      if (libsmtp_int_send (libsmtp_temp_gstring, libsmtp_session, 1))
        return LIBSMTP_ERRORSENDFATAL;
    }

    /* Text and message parts will have a charset setting */
    if ((libsmtp_temp_part->Type==LIBSMTP_MIME_TEXT) ||
        (libsmtp_temp_part->Type==LIBSMTP_MIME_MESSAGE))
      if ((libsmtp_temp_string = (char *)libsmtp_int_lookup_mime_charset(libsmtp_temp_part)))
      {
        g_string_sprintf (libsmtp_temp_gstring, "; charset=\"%s\"", \
           libsmtp_temp_string);

        if (libsmtp_int_send (libsmtp_temp_gstring, libsmtp_session, 1))
          return LIBSMTP_ERRORSENDFATAL;

        #ifdef LIBSMTP_DEBUG
          printf ("libsmtp_mime_headers: %s", libsmtp_temp_gstring->str);
        #endif
      }

    /* We need a transfer encoding, too */

    g_string_sprintf (libsmtp_temp_gstring, "\r\nContent-Transfer-Encoding: %s\r\n", \
       libsmtp_int_lookup_mime_encoding (libsmtp_temp_part));

    #ifdef LIBSMTP_DEBUG
      printf ("libsmtp_mime_headers: %s\n", libsmtp_temp_gstring);
    #endif

    if (libsmtp_int_send (libsmtp_temp_gstring, libsmtp_session, 1))
      return LIBSMTP_ERRORSENDFATAL;
  }

  /* We enter the data stage now */
  libsmtp_session->Stage = LIBSMTP_MIMEHEADERS_STAGE;

  return LIBSMTP_NOERR;
}


/* This function sends body data. It can only be used in the appropriate
   stage. The data to be sent will to be formatted according to RFC822 and
   the MIME standards. */

int libsmtp_part_send (char *libsmtp_body_data, unsigned int libsmtp_body_length, \
            struct libsmtp_session_struct *libsmtp_session)
{

  struct libsmtp_part_struct *libsmtp_temp_part;
  int libsmtp_int_errorstate;

  /* Headers must have been sent before body data goes out, but we must
     still be in body stage at most */
  if ((libsmtp_session->Stage < LIBSMTP_MIMEHEADERS_STAGE) ||
      (libsmtp_session->Stage > LIBSMTP_BODY_STAGE))
  {
    libsmtp_session->ErrorCode = LIBSMTP_BADSTAGE;
    return LIBSMTP_BADSTAGE;
  }

  if (libsmtp_session->Stage != LIBSMTP_BODY_STAGE)
  {
    /* If we just came from the headers stage, we have to send a blank line
     first */
    GString *libsmtp_temp_gstring = g_string_new (NULL);
    g_string_assign (libsmtp_temp_gstring, "\r\n");

    if (libsmtp_int_send (libsmtp_temp_gstring, libsmtp_session, 1))
      return LIBSMTP_ERRORSENDFATAL;

    /* We now enter the body stage */
    libsmtp_session->Stage = LIBSMTP_BODY_STAGE;
  }

  /* Check to see if we already are in a part */
  if (!libsmtp_session->PartNow)
  {
    /* We are not at the moment working on one part. Lets see if any parts
       are defined at all! */
    if (!libsmtp_session->Parts)
    {
      /* nope. bad mistake! */
      libsmtp_session->ErrorCode=LIBSMTP_NOPARENT;
      return LIBSMTP_NOPARENT;
    }

    /* So we try to lookup the first part that might contain data */
    if (libsmtp_int_nextpart (libsmtp_session))
      return LIBSMTP_PARTSERR;
  }
  /* Ok, we are in a part */

  libsmtp_temp_part=libsmtp_session->PartNow;

  /* We pass the data on to the encoding and sending function */
  switch (libsmtp_temp_part->Encoding)
  {
    case LIBSMTP_ENC_7BIT:
      libsmtp_int_errorstate = libsmtp_int_send_body (libsmtp_body_data, libsmtp_body_length, libsmtp_session);
    break;

    case LIBSMTP_ENC_8BIT:
      libsmtp_int_errorstate = libsmtp_int_send_body (libsmtp_body_data, libsmtp_body_length, libsmtp_session);
    break;

    case LIBSMTP_ENC_BINARY:
      libsmtp_int_errorstate = libsmtp_int_send_body (libsmtp_body_data, libsmtp_body_length, libsmtp_session);
    break;

    case LIBSMTP_ENC_BASE64:
      libsmtp_int_errorstate = libsmtp_int_send_base64 (libsmtp_body_data, libsmtp_body_length, libsmtp_session, 0);
    break;

    case LIBSMTP_ENC_QUOTED:
      libsmtp_int_errorstate = libsmtp_int_send_quoted (libsmtp_body_data, libsmtp_body_length, libsmtp_session);
    break;

    default:
      libsmtp_int_errorstate = libsmtp_int_send_body (libsmtp_body_data, libsmtp_body_length, libsmtp_session);
    break;
  }

  return libsmtp_int_errorstate;
}

/* This function moves on to the next body part. It can only be used in the
   appropriate stage. */

int libsmtp_part_next (struct libsmtp_session_struct *libsmtp_session)
{

/*  struct libsmtp_part_struct *libsmtp_temp_part;
  GString *libsmtp_temp_gstring; */
  int libsmtp_int_errorstate;

  /* Headers must have been sent before body data goes out, but we must
     still be in body stage at most */
  if ((libsmtp_session->Stage < LIBSMTP_MIMEHEADERS_STAGE) ||
      (libsmtp_session->Stage > LIBSMTP_BODY_STAGE))
  {
    libsmtp_session->ErrorCode = LIBSMTP_BADSTAGE;
    return LIBSMTP_BADSTAGE;
  }

  /* Check to see if we already are in a part */
  if (!libsmtp_session->PartNow)
  {
    /* We are not at the moment working on one part. Lets see if any parts
       are defined at all! */
    if (!libsmtp_session->Parts)
    {
      /* nope. bad mistake! */
      libsmtp_session->ErrorCode=LIBSMTP_NOPARENT;
      return LIBSMTP_NOPARENT;
    }
  }

  /* If we aren't in any stages, this will lookup the first stage for us.
     Otherwise it will just move on to the next stage */

  if ((libsmtp_int_errorstate = libsmtp_int_nextpart (libsmtp_session)))
    return libsmtp_int_errorstate;

  return LIBSMTP_NOERR;
}

