/***************************************************************************

  parts.c

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
     This is the MIME handling part

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

/* We need some definitions here first. These arrays hold the name tags
   of all MIME settings */

char *libsmtp_mime_types[] = {
  "text", "message", "image", "audio", "video", \
  "application", "multipart", "custom!!"};

char *libsmtp_mime_subtypes0[] = {
  "plain", "html", "english", "richtext"};

char *libsmtp_mime_subtypes1[] = {
  "rfc822", "partial"};

char *libsmtp_mime_subtypes2[] = {
  "gif", "jpeg", "png", "tiff", "x-ms-bmp", "x-xbitmap", \
  "x-xpixmap", "x-portable-anymap", "x-portable-bitmap", \
  "x-portable-graymap", "x-portable-pixmap"};

char *libsmtp_mime_subtypes3[] = {
  "mpeg", "midi", "x-wav", "x-aiff"};

char *libsmtp_mime_subtypes4[] = {
  "mpeg", "x-ms-video", "quicktime", "fli"};

char *libsmtp_mime_subtypes5[] = {
  "rtf", "postscript", "pdf", "zip", "x-debian-package", \
      "x-executable", "x-gtar", "x-shellscript", "x-tar", "octet-stream"};

char *libsmtp_mime_subtypes6[] = {
  "mixed", "parallel", "digest", "alternative"};

char *libsmtp_mime_encodings[] = {
  "7bit", "8bit", "binary", "base64", "quoted-printable"};

char *libsmtp_mime_charsets[] = {
  "us-ascii", "iso-8859-1", "iso-8859-2", "iso-8859-3", "iso-8859-15", "utf-8"};

/* This function creates a new body part, checks for conformance to RFC822
   and RFC 2045 and maybe attaches it to the session. It is taken care in here
   that only multipart and message parts can contain children! Charset is
   ignored unless you set a text or message part */

struct libsmtp_part_struct *libsmtp_part_new \
    (struct libsmtp_part_struct *libsmtp_int_parent_part, int libsmtp_int_type,\
    int libsmtp_int_subtype, int libsmtp_int_encoding, int libsmtp_int_charset, \
    char *libsmtp_int_desc, int length, struct libsmtp_session_struct *libsmtp_session)
{
  struct libsmtp_part_struct *libsmtp_int_part;
  GNode *libsmtp_int_temp_node;

  /* First we have to check if the session already has a main type */

  if (libsmtp_session->NumParts)
  {
    /* Yes, there already is a main part. Now lets see if he has passed
       us a non-NULL parent pointer */

    if (libsmtp_int_parent_part)
    {
      /* Ok, it is non-null. Now the parent part this pointer points to has
         to be some kind of multipart */

      if ((libsmtp_int_parent_part->Type!=LIBSMTP_MIME_MULTIPART) &&
          (libsmtp_int_parent_part->Type!=LIBSMTP_MIME_MESSAGE))
      {
        /* No, it isn't multipart. We can't append new parts to it. */
        libsmtp_session->ErrorCode=LIBSMTP_NOMULTIPART;
        return NULL;
      }
    }
    else
    {
      /* We already have a parent part but he passed a NULL pointer;
         This won't do!! */
      libsmtp_session->ErrorCode = LIBSMTP_PART_EXISTS;
      return NULL;
    }
  }
  else
  {
    /* This session hasn't got a main type yet. Lets see if he wants to
       define it. */
    if (libsmtp_int_parent_part)
    {
      /* He doesn't want to define the main part!! */
      libsmtp_session->ErrorCode=LIBSMTP_NOPARENT;
      return NULL;
    }
  }

  /* Ok. If we got so far the parent argument should be ok. */

  /* We use calloc here to clear the memory. GLists are initialized when
     they point to NULL, so it must be cleared. */

  libsmtp_int_part = (struct libsmtp_part_struct *)calloc (1, sizeof(struct libsmtp_part_struct));

  if (libsmtp_int_part == NULL)
    return NULL;

  /* The GStrings must be initialized */
  libsmtp_int_part->CustomType = g_string_new (NULL);
  libsmtp_int_part->CustomSubtype = g_string_new (NULL);
  libsmtp_int_part->Description = g_string_new (NULL);
  libsmtp_int_part->Boundary = g_string_new (NULL);

  libsmtp_int_part->Type = libsmtp_int_type;
  libsmtp_int_part->Subtype=libsmtp_int_subtype;
  libsmtp_int_part->Encoding=libsmtp_int_encoding;
  libsmtp_int_part->Description=g_string_new (libsmtp_int_desc);
  libsmtp_int_part->Charset=libsmtp_int_charset;
  
  libsmtp_int_part->length = length;

  if (libsmtp_int_check_part (libsmtp_int_part))
  {
    libsmtp_session->ErrorCode = LIBSMTP_BADARGS;
    return NULL;
  }

  /* We adjust the counters */
  libsmtp_session->NumParts++;

  if (libsmtp_int_parent_part)
  {
    /* This is a sibling part. We search the N-Tree for the data */
    libsmtp_int_temp_node = g_node_find (libsmtp_session->Parts, \
        G_IN_ORDER, G_TRAVERSE_ALL, libsmtp_int_parent_part);
    g_node_append_data (libsmtp_int_temp_node, libsmtp_int_part);

    #ifdef LIBSMTP_DEBUG
      printf ("libsmtp_part_new: Me: %s\n", libsmtp_int_part->Description->str);
    #endif
    //libsmtp_int_part=libsmtp_int_temp_node->data;

    #ifdef LIBSMTP_DEBUG
      printf ("libsmtp_part_new: Parent: %s\n", ((struct libsmtp_part_struct *)libsmtp_int_temp_node->data)->Description->str);
    #endif
  }
  else
  {
    /* This is the main part. */
    libsmtp_session->Parts=g_node_new (libsmtp_int_part);

    #ifdef LIBSMTP_DEBUG
      printf ("libsmtp_part_new: Me: %s\nlibsmtp_part_new: Parent: None\n", libsmtp_int_part->Description->str);
    #endif
  }

  return libsmtp_int_part;
}


int libsmtp_mime_type_custom (char *libsmtp_int_custom_type, \
       struct libsmtp_part_struct *libsmtp_int_part)
{
  /* Is this a custom type ? */
  if (libsmtp_int_part->Type != LIBSMTP_MIME_CUSTOM)
    return LIBSMTP_BADMIME;

  g_string_assign (libsmtp_int_part->CustomType, libsmtp_int_custom_type);

  return LIBSMTP_NOERR;
}

int libsmtp_mime_subtype_custom (char *libsmtp_int_custom_subtype, \
       struct libsmtp_part_struct *libsmtp_int_part)
{
  /* Is this a custom subtype ? */
  if (libsmtp_int_part->Subtype != LIBSMTP_MIME_SUB_CUSTOM)
    return LIBSMTP_BADMIME;

  g_string_assign (libsmtp_int_part->CustomSubtype, libsmtp_int_custom_subtype);

  return LIBSMTP_NOERR;
}

void libsmtp_set_boundary(struct libsmtp_part_struct *part, int index)
{
	static char pattern[33];
	static char digits[] = "0123456789ABCDEF";
	int i;
	
	for(i = 0; i < 32; i++)
		pattern[i] = digits[(random() >> 4) & 0xF];
	
	pattern[32] = 0;

	g_string_sprintf(part->Boundary, "----%s%02d", pattern, index);
}

/* We use this function internally to set the session to the next part to
   send. This function relies on the caller to check that all arguments
   are ok */
int libsmtp_int_nextpart (struct libsmtp_session_struct *libsmtp_session)
{
  GNode *libsmtp_temp_now;
  struct libsmtp_part_struct *libsmtp_temp_part;
  GString *libsmtp_temp_gstring=0;
  char *libsmtp_temp_string;
  int libsmtp_temp_int, libsmtp_int_travel=0;

  libsmtp_temp_gstring = g_string_new (NULL);

  /* Are we in a part already? */
  if (!libsmtp_session->PartNowNode)
  {
    /* No, we start with the main part */
    libsmtp_session->PartNowNode = libsmtp_session->Parts;
    libsmtp_session->PartNow = libsmtp_session->PartNowNode->data;
    #ifdef LIBSMTP_DEBUG
      printf ("libsmtp_int_nextpart: Starting with main part %s\n", libsmtp_session->PartNow->Description->str);
    #endif

    /* If this is a Multipart part, we must send the standard MIME blurb */
    if (libsmtp_session->PartNow->Type == LIBSMTP_MIME_MULTIPART)
    {
      g_string_assign (libsmtp_temp_gstring, \
         "This is a MIME multipart message.\r\n");

      if (libsmtp_int_send (libsmtp_temp_gstring, libsmtp_session, 2))
        return LIBSMTP_ERRORSENDFATAL;
    }
  }
  else
  {
    #ifdef LIBSMTP_DEBUG
      printf ("libsmtp_int_nextpart: Starting with non-main part %s, travelling\n", libsmtp_session->PartNow->Description->str);
    #endif
    libsmtp_int_travel=1;
  }

  /* This is the scanning loop. It scans for the next non-Multipart part */
  for(;;)
  {
    libsmtp_temp_part = (struct libsmtp_part_struct *)libsmtp_session->PartNow;

    /* Should we travel over this one anyway? */

    if (libsmtp_int_travel)
    {
      /* Yes, so we'll have to take the next sibling. O Brother were art
         thou? */
      #ifdef LIBSMTP_DEBUG
        printf ("libsmtp_int_nextpart: Travelled over %s\n", libsmtp_session->PartNow->Description->str);
      #endif

      libsmtp_int_travel=0;
      if (libsmtp_session->PartNowNode->next)
      {
        /* Yeah, so we take it. */
        libsmtp_session->PartNowNode=libsmtp_session->PartNowNode->next;
        libsmtp_session->PartNow=libsmtp_session->PartNowNode->data;

        #ifdef LIBSMTP_DEBUG
          printf ("libsmtp_int_nextpart: Now at sibling %s\n", libsmtp_session->PartNow->Description->str);
        #endif

        /* Jump the gun, Mr. Ed */
        continue;
      }
      else
      {
        #ifdef LIBSMTP_DEBUG
          printf ("libsmtp_int_nextpart: %s has no more siblings\n", libsmtp_session->PartNow->Description->str);
        #endif

        /* No, so we need the parent */
        if (libsmtp_session->PartNowNode->parent)
        {
          /* Ok, here it is */
          libsmtp_session->PartNowNode = libsmtp_session->PartNowNode->parent;
          libsmtp_temp_part = libsmtp_session->PartNow = libsmtp_session->PartNowNode->data;

					/* We must send the boundary */

          g_string_sprintf (libsmtp_temp_gstring, "\r\n--%s--\r\n", \
             libsmtp_temp_part->Boundary->str);

          if (libsmtp_int_send (libsmtp_temp_gstring, libsmtp_session, 1))
            return LIBSMTP_ERRORSENDFATAL;

          #ifdef LIBSMTP_DEBUG
            printf ("libsmtp_int_nextpart: Now at parent %s\n", libsmtp_session->PartNow->Description->str);
          #endif
          /* We will travel over this one, of course. It is already finished */
          libsmtp_int_travel=1;
          continue;
        }
        else
        {
          /* No more parents here. We are finished. */

          #ifdef LIBSMTP_DEBUG
            printf ("libsmtp_int_nextpart: Finished at %s\n", libsmtp_session->PartNow->Description->str);
          #endif
          return LIBSMTP_PARTSFINISHED;
        }
      }
    }
    else
    {
      /* Ok, we don't need to travel. Is this a multipart part? */
      if (libsmtp_temp_part->Type==LIBSMTP_MIME_MULTIPART)
      {
        /* Yes, is the boundary string set? */
        if (libsmtp_temp_part->Boundary->len == 0)
        {
          /* No, we really should set that */
					libsmtp_set_boundary(libsmtp_temp_part, g_node_depth (libsmtp_session->PartNowNode));

					#ifdef LIBSMTP_DEBUG
						printf ("libsmtp_int_nextpart: Part %s is Multipart, Setting boundary to '%s'\n", libsmtp_temp_part->Description->str, libsmtp_temp_part->Boundary->str);
					#endif
        }
      }

      {
        /* No, this is our man! */
        libsmtp_session->PartNow = libsmtp_session->PartNowNode->data;

        /* Maybe we have to send out a boundary and next part data... */
        if (g_node_depth (libsmtp_session->PartNowNode) > 1)
        {
          libsmtp_temp_now=libsmtp_session->PartNowNode->parent;
          libsmtp_temp_part=libsmtp_temp_now->data;

          g_string_sprintf (libsmtp_temp_gstring, "\r\n\r\n--%s\r\n", \
             libsmtp_temp_part->Boundary->str);

          #ifdef LIBSMTP_DEBUG
            printf ("libsmtp_mime_headers: %s", libsmtp_temp_gstring->str);
          #endif

          if (libsmtp_int_send (libsmtp_temp_gstring, libsmtp_session, 1))
            return LIBSMTP_ERRORSENDFATAL;

          libsmtp_temp_part=libsmtp_session->PartNow;

          /* We should check for valied MIME settings first */
          if ((libsmtp_temp_int=libsmtp_int_check_part (libsmtp_temp_part)))
          {
            libsmtp_session->ErrorCode=libsmtp_temp_int;
            return libsmtp_temp_int;
          }

          /* Then we look up the names of the MIME settings of the main body part
             and send them as headers */

          g_string_sprintf (libsmtp_temp_gstring, "Content-Type: %s/%s", \
             libsmtp_int_lookup_mime_type (libsmtp_temp_part), \
             libsmtp_int_lookup_mime_subtype (libsmtp_temp_part));

          if (strlen(libsmtp_temp_part->Description->str))
          {
            g_string_append (libsmtp_temp_gstring, "; name=\"");
            g_string_append (libsmtp_temp_gstring, libsmtp_temp_part->Description->str);
            g_string_append (libsmtp_temp_gstring, "\"");
          }

          #ifdef LIBSMTP_DEBUG
            printf ("libsmtp_mime_headers: %s. Type: %d/%d\n", libsmtp_temp_gstring->str, \
            libsmtp_temp_part->Type, libsmtp_temp_part->Subtype);
          #endif

          if (libsmtp_int_send (libsmtp_temp_gstring, libsmtp_session, 1))
            return LIBSMTP_ERRORSENDFATAL;

					/* Multiparts have a boundary */

      		if (libsmtp_temp_part->Type == LIBSMTP_MIME_MULTIPART)
      		{
        		g_string_sprintf (libsmtp_temp_gstring, "; boundary=\"%s\"", libsmtp_temp_part->Boundary->str);

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

					if (libsmtp_temp_part->length > 0)
					{
						g_string_sprintf (libsmtp_temp_gstring, "\r\nContent-Length: %d", libsmtp_temp_part->length);

						#ifdef LIBSMTP_DEBUG
							printf ("libsmtp_int_nextpart: %s\n", libsmtp_temp_gstring);
						#endif

						if (libsmtp_int_send (libsmtp_temp_gstring, libsmtp_session, 1))
							return LIBSMTP_ERRORSENDFATAL;
					}

          /* We need a transfer encoding, too */

      		if (libsmtp_temp_part->Type != LIBSMTP_MIME_MULTIPART)
      		{
						g_string_sprintf (libsmtp_temp_gstring, "\r\nContent-Transfer-Encoding: %s\r\n", \
							 libsmtp_int_lookup_mime_encoding (libsmtp_temp_part));

						#ifdef LIBSMTP_DEBUG
							printf ("libsmtp_mime_headers: %s\n", libsmtp_temp_gstring);
						#endif

						if (libsmtp_int_send (libsmtp_temp_gstring, libsmtp_session, 1))
							return LIBSMTP_ERRORSENDFATAL;
					}
					
					/* Adds a blank line */
					g_string_assign (libsmtp_temp_gstring, "\r\n");
					
					if (libsmtp_int_send(libsmtp_temp_gstring, libsmtp_session, 1))
						return LIBSMTP_ERRORSENDFATAL;
        }
      }

      if (libsmtp_temp_part->Type==LIBSMTP_MIME_MULTIPART)
      {
        #ifdef LIBSMTP_DEBUG
          printf ("libsmtp_int_nextpart: Part %s is Multipart, so we jump to the first child\n", libsmtp_session->PartNow->Description->str);
        #endif

        /* Set PartNowNode to the first child */
        libsmtp_session->PartNowNode=libsmtp_session->PartNowNode->children;
        libsmtp_session->PartNow=libsmtp_session->PartNowNode->data;

        #ifdef LIBSMTP_DEBUG
          printf ("libsmtp_int_nextpart: Jumped to %s\n", libsmtp_session->PartNow->Description->str);
        #endif

        /* and continue to the next loop */
        continue;
      }

			#ifdef LIBSMTP_DEBUG
				printf ("libsmtp_int_nextpart: I chose %s\n", libsmtp_session->PartNow->Description->str);
			#endif
			return LIBSMTP_NOERR;
    }
  }
}


/* This function returns the current part. It should be used directly after
   sending the headers to find out which part will be transmitted first. It
   can be used at any time to find out what part is currently being sent, of
   course. */

struct libsmtp_part_struct *libsmtp_part_query \
     (struct libsmtp_session_struct *libsmtp_session)
{
  /* Are we in data stage? */
  if ((libsmtp_session->Stage < LIBSMTP_HEADERS_STAGE) ||
      (libsmtp_session->Stage > LIBSMTP_BODY_STAGE))
  {
    libsmtp_session->ErrorCode = LIBSMTP_BADSTAGE;
    return NULL;
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
      return NULL;
    }

    /* So we try to lookup the first part that might contain data */
    if (libsmtp_int_nextpart (libsmtp_session))
      return NULL;
  }

  /* Ok, return the current part */
  return libsmtp_session->PartNow;
}


/* libsmtp_int_check_part checks a part for correct settings */

int libsmtp_int_check_part (struct libsmtp_part_struct *libsmtp_int_part)
{

  /* Now we check if any invalid MIME arguments have been given.*/

  if ((libsmtp_int_part->Type < 0) || (libsmtp_int_part->Type > LIBSMTP_MAX_MIME))
  {
    return LIBSMTP_BADARGS;
  }

  /* Now the same for the subtype argument. This must correspond to the
     selected type */

  switch (libsmtp_int_part->Type)
  {
    case (LIBSMTP_MIME_TEXT):
      if ((libsmtp_int_part->Subtype < 0) || (libsmtp_int_part->Subtype > LIBSMTP_MAX_MIME_SUB0))
      {
        return LIBSMTP_BADMIME;
      }
      /* Text types can have any encoding */
      if ((libsmtp_int_part->Encoding < 0) || (libsmtp_int_part->Encoding > LIBSMTP_MAX_ENC))
      {
        return LIBSMTP_BADENCODING;
      }
      /* Text types must have a valid charset */
      if ((libsmtp_int_part->Charset < 0) || (libsmtp_int_part->Charset > LIBSMTP_MAX_CHARSET))
      {
        return LIBSMTP_BADCHARSET;
      }

    break;

    case (LIBSMTP_MIME_MESSAGE):
      if ((libsmtp_int_part->Subtype < 1000) || (libsmtp_int_part->Subtype > LIBSMTP_MAX_MIME_SUB1))
      {
        return LIBSMTP_BADMIME;
      }

      /* Message types can have any encoding */
      if ((libsmtp_int_part->Encoding < 0) || (libsmtp_int_part->Encoding > LIBSMTP_MAX_ENC))
      {
        return LIBSMTP_BADENCODING;
      }
      /* Message types must have a valid charset */
      if ((libsmtp_int_part->Charset < 0) || (libsmtp_int_part->Charset > LIBSMTP_MAX_CHARSET))
      {
        return LIBSMTP_BADCHARSET;
      }

    break;

    case (LIBSMTP_MIME_IMAGE):
      if ((libsmtp_int_part->Subtype < 2000) || (libsmtp_int_part->Subtype > LIBSMTP_MAX_MIME_SUB2))
      {
        return LIBSMTP_BADMIME;
      }

      /* Image types must be in a non-text encoding */
      if ((libsmtp_int_part->Encoding < LIBSMTP_ENC_BINARY) || (libsmtp_int_part->Encoding > LIBSMTP_MAX_ENC))
      {
        return LIBSMTP_BADENCODING;
      }

      /* Charset is set to -1 because it won't matter here */
      libsmtp_int_part->Charset = -1;
    break;

    case (LIBSMTP_MIME_AUDIO):
      if ((libsmtp_int_part->Subtype < 3000) || (libsmtp_int_part->Subtype > LIBSMTP_MAX_MIME_SUB3))
      {
        return LIBSMTP_BADMIME;
      }

      /* Audio types must be in a non-text encoding */
      if ((libsmtp_int_part->Encoding < LIBSMTP_ENC_BINARY) || (libsmtp_int_part->Encoding > LIBSMTP_MAX_ENC))
      {
        return LIBSMTP_BADENCODING;
      }

      /* Charset is set to -1 because it won't matter here */
      libsmtp_int_part->Charset = -1;
    break;

    case (LIBSMTP_MIME_VIDEO):
      if ((libsmtp_int_part->Subtype < 4000) || (libsmtp_int_part->Subtype > LIBSMTP_MAX_MIME_SUB4))
      {
        return LIBSMTP_BADMIME;
      }

      /* Video types must be in a non-text encoding */
      if ((libsmtp_int_part->Encoding < LIBSMTP_ENC_BINARY) || (libsmtp_int_part->Encoding > LIBSMTP_MAX_ENC))
      {
        return LIBSMTP_BADENCODING;
      }

      /* Charset is set to -1 because it won't matter here */
      libsmtp_int_part->Charset = -1;
    break;

    case (LIBSMTP_MIME_APPLICATION):
      if ((libsmtp_int_part->Subtype < 5000) || (libsmtp_int_part->Subtype > LIBSMTP_MAX_MIME_SUB5))
      {
        return LIBSMTP_BADMIME;
      }

      /* Application types must be in a non-text encoding */
      if ((libsmtp_int_part->Encoding < LIBSMTP_ENC_BINARY) || (libsmtp_int_part->Encoding > LIBSMTP_MAX_ENC))
      {
        return LIBSMTP_BADENCODING;
      }

      /* Charset is set to -1 because it won't matter here */
      libsmtp_int_part->Charset = -1;
    break;

    case (LIBSMTP_MIME_MULTIPART):
      if ((libsmtp_int_part->Subtype < 6000) || (libsmtp_int_part->Subtype > LIBSMTP_MAX_MIME_SUB6))
      {
        return LIBSMTP_BADMIME;
      }

      /* Application types must be in a text encoding, and should only be
         7bit */
      if (libsmtp_int_part->Encoding != LIBSMTP_ENC_7BIT)
      {
        return LIBSMTP_BADENCODING;
      }

      /* Charset is set to -1 because it won't matter here */
      libsmtp_int_part->Charset = -1;
    break;

    case (LIBSMTP_MIME_CUSTOM):
      if (libsmtp_int_part->Subtype != LIBSMTP_MIME_SUB_CUSTOM)
      {
        return LIBSMTP_BADMIME;
      }

      /* Custom type can have any encoding, of course */
      if ((libsmtp_int_part->Encoding < 0) || (libsmtp_int_part->Encoding > LIBSMTP_MAX_ENC))
      {
        return LIBSMTP_BADENCODING;
      }

      /* Custom types must have a valid charset or NOCHARSET */
      if ((libsmtp_int_part->Charset < LIBSMTP_CHARSET_NOCHARSET) || (libsmtp_int_part->Charset > LIBSMTP_MAX_CHARSET))
      {
        return LIBSMTP_BADCHARSET;
      }
    break;
  }

  return 0;
}

/* These functions lookup the name of types, subtypes and encodings for a
   part. They only perform glancing checking of parameters, so you should
   check the mime settings beforehand with libsmtp_int_check_parts */

const char *libsmtp_int_lookup_mime_type (struct libsmtp_part_struct *libsmtp_int_part)
{
  if ((libsmtp_int_part->Type >= 0) && (libsmtp_int_part->Type <= LIBSMTP_MAX_MIME))
  {
    if (libsmtp_int_part->Type == LIBSMTP_MIME_CUSTOM)
      return libsmtp_int_part->CustomType->str;
    else
      return libsmtp_mime_types[libsmtp_int_part->Type];
  }
  else
    return NULL;
}

const char *libsmtp_int_lookup_mime_subtype (struct libsmtp_part_struct *libsmtp_int_part)
{
    switch (libsmtp_int_part->Type)
    {
      case LIBSMTP_MIME_TEXT:
        return libsmtp_mime_subtypes0[libsmtp_int_part->Subtype];

      case LIBSMTP_MIME_MESSAGE:
        return libsmtp_mime_subtypes1[libsmtp_int_part->Subtype-1000];

      case LIBSMTP_MIME_IMAGE:
        return libsmtp_mime_subtypes2[libsmtp_int_part->Subtype-2000];

      case LIBSMTP_MIME_AUDIO:
        return libsmtp_mime_subtypes3[libsmtp_int_part->Subtype-3000];

      case LIBSMTP_MIME_VIDEO:
        return libsmtp_mime_subtypes4[libsmtp_int_part->Subtype-4000];

      case LIBSMTP_MIME_APPLICATION:
        return libsmtp_mime_subtypes5[libsmtp_int_part->Subtype-5000];

      case LIBSMTP_MIME_MULTIPART:
        return libsmtp_mime_subtypes6[libsmtp_int_part->Subtype-6000];

      case LIBSMTP_MIME_CUSTOM:
        return libsmtp_int_part->CustomSubtype->str;

      default:
        return NULL;
    }
}

const char *libsmtp_int_lookup_mime_charset (struct libsmtp_part_struct *libsmtp_int_part)
{
  /* Only textual parts can have a charset */
  if ((libsmtp_int_part->Type == LIBSMTP_MIME_TEXT) || \
      (libsmtp_int_part->Type == LIBSMTP_MIME_MESSAGE))
    {
    if ((libsmtp_int_part->Charset >= 0) && (libsmtp_int_part->Charset <= LIBSMTP_MAX_CHARSET))
    {
      return libsmtp_mime_charsets[libsmtp_int_part->Charset];
    }
  }

  return NULL;
}

const char *libsmtp_int_lookup_mime_encoding (struct libsmtp_part_struct *libsmtp_int_part)
{
  if ((libsmtp_int_part->Encoding >= 0) && (libsmtp_int_part->Encoding <= LIBSMTP_MAX_ENC))
    return libsmtp_mime_encodings[libsmtp_int_part->Encoding];
  else
    return NULL;
}

