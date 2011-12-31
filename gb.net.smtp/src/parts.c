/***************************************************************************

  parts.c

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
Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
	MA 02110-1301, USA

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

bool libsmtp_part_is_type(struct libsmtp_part_struct *part, const char *type)
{
	if (g_string_is_void(part->Type))
		return FALSE;
	else
		return strcmp(part->Type->str, type) == 0;
}
	
/* This function creates a new body part, checks for conformance to RFC822
   and RFC 2045 and maybe attaches it to the session. It is taken care in here
   that only multipart and message parts can contain children! Charset is
   ignored unless you set a text or message part */

struct libsmtp_part_struct *libsmtp_part_new
    (struct libsmtp_part_struct *libsmtp_int_parent_part, const char *type,
    const char *subtype, int libsmtp_int_encoding, const char *charset,
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

      if (!libsmtp_part_is_type(libsmtp_int_parent_part, "multipart") && !libsmtp_part_is_type(libsmtp_int_parent_part, "message"))
      {
        /* No, it isn't multipart. We can't append new parts to it. */
        libsmtp_session->ErrorCode = LIBSMTP_NOMULTIPART;
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
  libsmtp_int_part->Description = g_string_new (NULL);
  libsmtp_int_part->Boundary = g_string_new (NULL);

  libsmtp_int_part->Type = g_string_new(type);
  libsmtp_int_part->Subtype = g_string_new(subtype);
  libsmtp_int_part->Encoding = libsmtp_int_encoding;
  libsmtp_int_part->Description = g_string_new(libsmtp_int_desc);
  libsmtp_int_part->Charset = g_string_new(charset);
  
  libsmtp_int_part->length = length;

  /*if (libsmtp_int_check_part (libsmtp_int_part))
  {
    libsmtp_session->ErrorCode = LIBSMTP_BADARGS;
    return NULL;
  }*/

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
  GString *libsmtp_temp_gstring = NULL;
  int libsmtp_int_travel = 0;

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
    if (libsmtp_part_is_type(libsmtp_session->PartNow, "multipart"))
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
      if (libsmtp_part_is_type(libsmtp_temp_part, "multipart"))
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
          /*if ((libsmtp_temp_int=libsmtp_int_check_part (libsmtp_temp_part)))
          {
            libsmtp_session->ErrorCode=libsmtp_temp_int;
            return libsmtp_temp_int;
          }*/

          /* Then we look up the names of the MIME settings of the main body part
             and send them as headers */

          g_string_sprintf(libsmtp_temp_gstring, "Content-Type: %s/%s", libsmtp_temp_part->Type->str, libsmtp_temp_part->Subtype->str);

          if (!g_string_is_void(libsmtp_temp_part->Description))
          {
            g_string_append(libsmtp_temp_gstring, "; name=\"");
            g_string_append(libsmtp_temp_gstring, libsmtp_temp_part->Description->str);
            g_string_append(libsmtp_temp_gstring, "\"");
          }

          #ifdef LIBSMTP_DEBUG
            printf ("libsmtp_mime_headers: %s. Type: %d/%d\n", libsmtp_temp_gstring->str, \
            libsmtp_temp_part->Type, libsmtp_temp_part->Subtype);
          #endif

          if (libsmtp_int_send (libsmtp_temp_gstring, libsmtp_session, 1))
            return LIBSMTP_ERRORSENDFATAL;

					/* Multiparts have a boundary */

      		if (libsmtp_part_is_type(libsmtp_temp_part, "multipart"))
      		{
        		g_string_sprintf (libsmtp_temp_gstring, "; boundary=\"%s\"", libsmtp_temp_part->Boundary->str);

	          if (libsmtp_int_send(libsmtp_temp_gstring, libsmtp_session, 1))
  	          return LIBSMTP_ERRORSENDFATAL;
					}

        	/* Text and message parts will have a charset setting */

					if (!g_string_is_void(libsmtp_temp_part->Charset))
					{
						g_string_sprintf (libsmtp_temp_gstring, "; charset=\"%s\"", libsmtp_temp_part->Charset->str);

						if (libsmtp_int_send(libsmtp_temp_gstring, libsmtp_session, 1))
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

      		if (!libsmtp_part_is_type(libsmtp_temp_part, "multipart"))
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

      if (libsmtp_part_is_type(libsmtp_temp_part, "multipart"))
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

struct libsmtp_part_struct *libsmtp_part_query(struct libsmtp_session_struct *libsmtp_session)
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


/* These functions lookup the name of types, subtypes and encodings for a
   part. They only perform glancing checking of parameters, so you should
   check the mime settings beforehand with libsmtp_int_check_parts */

const char *libsmtp_int_lookup_mime_encoding (struct libsmtp_part_struct *libsmtp_int_part)
{
  if ((libsmtp_int_part->Encoding >= 0) && (libsmtp_int_part->Encoding <= LIBSMTP_MAX_ENC))
    return libsmtp_mime_encodings[libsmtp_int_part->Encoding];
  else
    return NULL;
}

