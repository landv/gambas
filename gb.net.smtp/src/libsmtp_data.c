/***************************************************************************

  libsmtp_data.c

  (c) 2000-2011 Benoît Minisini <gambas@users.sourceforge.net>

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
    These are the utility data functions.

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

#include "gb_common.h"

#include <glib.h>

//#include "../config.h"

#include "libsmtp.h"


/* This function returns a pointer to an allocated libsmtp_session_struct
   All GStrings are initialized. */

struct libsmtp_session_struct *libsmtp_session_initialize (bool debug, void *stream)
{
  struct libsmtp_session_struct *session;

  /* We use calloc here to clear the memory. GLists are initialized when
     they point to NULL, so it must be cleared. */
  session = (struct libsmtp_session_struct *)calloc (1, sizeof(struct libsmtp_session_struct));

  if (!session)
    return NULL;

  /* The GStrings must be initialized */
  session->From = g_string_new (NULL);
  session->Subject = g_string_new (NULL);
  session->LastResponse = g_string_new (NULL);

  #ifdef WITH_MIME
    session->Parts = NULL;
    session->NumParts = 0;
  #endif
  
  session->socket = -1;

	session->debug = debug;
	session->stream = stream;
	
  return session;
}

#ifdef WITH_MIME
gboolean free_part(GNode *node, gpointer data)
{
	free(node->data);
	return FALSE;
}
#endif

/* This frees the specified libsmtp_session_struct and all accompanying
   GStrings and GLists */

int libsmtp_free (struct libsmtp_session_struct *libsmtp_session)
{
  int libsmtp_temp;

  /* Lets see if we gotta close the socket */

	libsmtp_close(libsmtp_session);

  /* All GStrings and GLists must be freed */
  g_list_free (libsmtp_session->To);
  g_list_free (libsmtp_session->CC);
  g_list_free (libsmtp_session->BCC);

  /* Now we free all elements of the Lists we allocated with strdup */
  libsmtp_session->ToResponse = g_list_first (libsmtp_session->ToResponse);
  for (libsmtp_temp=0; libsmtp_temp<g_list_length (libsmtp_session->ToResponse); \
       libsmtp_temp++)
    free (g_list_nth_data (libsmtp_session->ToResponse, libsmtp_temp));

  libsmtp_session->CCResponse = g_list_first (libsmtp_session->CCResponse);
  for (libsmtp_temp=0; libsmtp_temp<g_list_length (libsmtp_session->CCResponse); \
       libsmtp_temp++)
    free (g_list_nth_data (libsmtp_session->CCResponse, libsmtp_temp));

  libsmtp_session->BCCResponse = g_list_first (libsmtp_session->BCCResponse);
  for (libsmtp_temp=0; libsmtp_temp<g_list_length (libsmtp_session->BCCResponse); \
       libsmtp_temp++)
    free (g_list_nth_data (libsmtp_session->BCCResponse, libsmtp_temp));

  g_list_free (libsmtp_session->ToResponse);
  g_list_free (libsmtp_session->CCResponse);
  g_list_free (libsmtp_session->BCCResponse);

  g_string_free (libsmtp_session->From,1);
  g_string_free (libsmtp_session->Subject,1);
  g_string_free (libsmtp_session->LastResponse,1);

	#ifdef WITH_MIME
	g_node_traverse(libsmtp_session->Parts, G_IN_ORDER, G_TRAVERSE_ALL, -1, free_part, NULL);
	g_node_destroy(libsmtp_session->Parts);
	#endif

  /* Ok, lets free the malloced session struct */
  free (libsmtp_session);

  return 0;
}

/* This function sets the environment for the session. At the moment it
   just sets subject and sender address. SSL and auth stuff should be set
   here in the future. */

int libsmtp_set_environment (char *libsmtp_int_From, char *libsmtp_int_Subject,\
      unsigned int libsmtp_int_flags, struct libsmtp_session_struct *libsmtp_session)
{
  if ((!strlen (libsmtp_int_From)) || (!strlen (libsmtp_int_Subject)))
  {
    libsmtp_session->ErrorCode = LIBSMTP_BADARGS;
    return LIBSMTP_BADARGS;
  }

  g_string_assign (libsmtp_session->From, libsmtp_int_From);
  g_string_assign (libsmtp_session->Subject, libsmtp_int_Subject);

  return LIBSMTP_NOERR;
}

int libsmtp_add_recipient (int libsmtp_int_rec_type, char *libsmtp_int_address,
      struct libsmtp_session_struct *libsmtp_session)
{
  /* Do we need a copy? */
  char *libsmtp_int_copy;

  /* Lets just check that rec_type isn't an invalid value */
  if ((libsmtp_int_rec_type < 0) || (libsmtp_int_rec_type > LIBSMTP_REC_MAX))
  {
    libsmtp_session->ErrorCode = LIBSMTP_BADARGS;
    return LIBSMTP_BADARGS;
  }

  /* Zero length string as argument? */
  if (!strlen (libsmtp_int_address))
  {
    libsmtp_session->ErrorCode = LIBSMTP_BADARGS;
    return LIBSMTP_BADARGS;
  }

  libsmtp_int_copy = strdup(libsmtp_int_address);

  switch (libsmtp_int_rec_type)
  {
    case (LIBSMTP_REC_TO):
      libsmtp_session->To = g_list_append (libsmtp_session->To, libsmtp_int_copy);
      break;

    case (LIBSMTP_REC_CC):
      libsmtp_session->CC = g_list_append (libsmtp_session->CC, libsmtp_int_copy);
      break;

    case (LIBSMTP_REC_BCC):
      libsmtp_session->BCC = g_list_append (libsmtp_session->BCC, libsmtp_int_copy);
      break;

    default:
      /* Lets just check that rec_type isn't an invalid value */
      libsmtp_session->ErrorCode = LIBSMTP_BADARGS;
      return LIBSMTP_BADARGS;
      break;
  }

  return LIBSMTP_NOERR;
}

