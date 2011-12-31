/***************************************************************************

  libsmtp_connection.c

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
#include "main.h"

#include <glib.h>
#include <sys/socket.h>
#include <errno.h>
#include <netdb.h>
#include <netinet/in.h>

//#include "../config.h"
#include "libsmtp.h"

int libsmtp_connect (char *libsmtp_server, unsigned int libsmtp_port, unsigned int libsmtp_flags, struct libsmtp_session_struct *libsmtp_session)
{
  int libsmtp_socket; 		/* The temporary socket handle */
  //int libsmtp_bytes_read=0;     /* How many bytes read? */
  struct hostent *libsmtp_mailhost;	/* We need this to convert the hostname to an IP */
  struct sockaddr_in libsmtp_sock;	/* We need this for the connection */
  char libsmtp_temp_buffer[4096];	/* Temp string for reads and writes */
  //char *libsmtp_search_buffer;		/* Used for searching in strings */
  GString *libsmtp_temp_gstring;	/* Temp gstring */

  /* We clear up the variable space and instantiate the GStrings */

  bzero (libsmtp_temp_buffer, sizeof(libsmtp_temp_buffer));

  libsmtp_temp_gstring = g_string_new (NULL);

  /* We enter the connect stage now */
  libsmtp_session->Stage = LIBSMTP_CONNECT_STAGE;

	if (!libsmtp_session->stream)
	{
		/* We need a socket anyway :) */
		libsmtp_socket = socket (PF_INET, SOCK_STREAM, 0);

		/* Socket ok? */
		if (libsmtp_socket < 0)
		{
			libsmtp_session->ErrorCode = LIBSMTP_SOCKETNOCREATE;
			return LIBSMTP_SOCKETNOCREATE;
		}

		/* Now we need to get the IP from the hostname... */
		if ((libsmtp_mailhost=gethostbyname((const char *)libsmtp_server))==NULL)
			return libsmtp_error(libsmtp_session, LIBSMTP_HOSTNOTFOUND);

		/* This struct is needed for the connect call */
		libsmtp_sock.sin_family = AF_INET;
		libsmtp_sock.sin_addr = *(struct in_addr *)libsmtp_mailhost->h_addr;
		if (!libsmtp_port)
			libsmtp_sock.sin_port = htons (25);
		else
			libsmtp_sock.sin_port = htons (libsmtp_port);

		/* Now we make the connection to the smart host on the specified port */

		if (connect (libsmtp_socket, (struct sockaddr *) &libsmtp_sock, sizeof (libsmtp_sock) ) < 0)
		{
			libsmtp_close(libsmtp_session);
			libsmtp_session->ErrorCode = LIBSMTP_CONNECTERR;
			return LIBSMTP_CONNECTERR;
		}

		/* Ok, lets set the session socket to the right handler */
		libsmtp_session->socket = libsmtp_socket;
	}
	else
	{
		// Put the stream in blocking mode
		libsmtp_session->was_blocking = GB.Stream.Block(libsmtp_session->stream, TRUE);
		
		/*// Ignore all data sent by the server until now
		for(;;)
		{
			usleep(250000);
			if (GB.Stream.Read(libsmtp_session->stream, libsmtp_temp_buffer, (-sizeof(libsmtp_temp_buffer))) <= 0)
				break;
		}
		
		GB.Stream.Block(libsmtp_session->stream, TRUE);*/
	}

	/* We enter the greet stage now */
	libsmtp_session->Stage = LIBSMTP_GREET_STAGE;

	if (!libsmtp_session->no_greeting)
	{
		/* Now we should read the mail servers greeting */
		if (libsmtp_int_read (libsmtp_temp_gstring, libsmtp_session, 2))
			return LIBSMTP_ERRORREADFATAL;

		if (libsmtp_session->LastResponseCode != 220)
		{
			libsmtp_close(libsmtp_session);
			libsmtp_session->ErrorCode = LIBSMTP_NOTWELCOME;
			return LIBSMTP_NOTWELCOME;
		}
	}

	/* Now we need to know our hostname */
  // BM: HELO command needs a FQDN according to the RFC

  /*if (gethostname ((char *)libsmtp_temp_buffer, sizeof(libsmtp_temp_buffer)))
  {
    libsmtp_session->ErrorCode = LIBSMTP_WHATSMYHOSTNAME;
    close (libsmtp_session->socket);
    libsmtp_session->socket=0;
    return LIBSMTP_WHATSMYHOSTNAME;
  }*/
  
  strcpy(libsmtp_temp_buffer, GB.System.DomainName());
  if (!*libsmtp_temp_buffer)
  {
		libsmtp_close(libsmtp_session);
    libsmtp_session->ErrorCode = LIBSMTP_WHATSMYHOSTNAME;
    return LIBSMTP_WHATSMYHOSTNAME;
  }

  /* We enter the hello stage now */
  libsmtp_session->Stage = LIBSMTP_HELLO_STAGE;

  /* Ok, lets greet him back */

  g_string_sprintf (libsmtp_temp_gstring, "ehlo %s\r\n", libsmtp_temp_buffer);

  if (libsmtp_int_send (libsmtp_temp_gstring, libsmtp_session, 2))
    return LIBSMTP_ERRORSENDFATAL;

  /* After this he will send us his capabilities. He may not be able to
     recognize EHLO though, so we have to send HELO instead. */

  if (libsmtp_int_read (libsmtp_temp_gstring, libsmtp_session, 2))
    return LIBSMTP_ERRORSENDFATAL;

  if (libsmtp_session->LastResponseCode < 300)
  {
    /* Ok, he loves us. Lets parse the response for the capabilities. */

    if (strstr (libsmtp_session->LastResponse->str, "8BITMIME"))
      libsmtp_session->serverflags |= LIBSMTP_HAS_8BIT;

    if (strstr (libsmtp_session->LastResponse->str, "PIPELINING"))
      libsmtp_session->serverflags |= LIBSMTP_HAS_PIPELINING;

    if (strstr (libsmtp_session->LastResponse->str, "DSN"))
      libsmtp_session->serverflags |= LIBSMTP_HAS_DSN;

    if (strstr (libsmtp_session->LastResponse->str, "STARTTLS"))
      libsmtp_session->serverflags |= LIBSMTP_HAS_TLS;

    if (strstr (libsmtp_session->LastResponse->str, "AUTH"))
		{
      libsmtp_session->serverflags |= LIBSMTP_HAS_AUTH;
			if (strstr (libsmtp_session->LastResponse->str, "PLAIN"))
				libsmtp_session->serverflags |= LIBSMTP_HAS_AUTH_PLAIN;
		}

    if (strstr (libsmtp_session->LastResponse->str, "SIZE"))
      libsmtp_session->serverflags |= LIBSMTP_HAS_SIZE;

    if (strstr (libsmtp_session->LastResponse->str, "ETRN"))
      libsmtp_session->serverflags |= LIBSMTP_HAS_ETRN;

    if (strstr (libsmtp_session->LastResponse->str, "ENHANCEDSTATUSCODES"))
      libsmtp_session->serverflags |= LIBSMTP_HAS_ENHANCEDSTATUSCODES;

    /* Ok, now we're ready for business */

  }
  else
  {
    /* Ok, he doesn't understand EHLO, so we will send HELO instead */
    g_string_sprintf (libsmtp_temp_gstring, "helo %s\r\n", libsmtp_temp_buffer);

    if (libsmtp_int_send (libsmtp_temp_gstring, libsmtp_session, 2))
      return LIBSMTP_ERRORSENDFATAL;

    /* Lets see if he likes us now. */

    if (libsmtp_session->LastResponseCode > 299)
    {
      /* We are not welcome here it seems - I don't know if this is fatal
         to SMTP transactions - FIXME */
			libsmtp_close(libsmtp_session);
      libsmtp_session->ErrorCode=LIBSMTP_NOTWELCOME;
      return LIBSMTP_NOTWELCOME;
    }

    /* All capabilities are at 0 - This server is really stupid i. e. Exchange */
  }

  /* Ok, we're ready now. The connection is set up. */

  return LIBSMTP_NOERR;
}


int libsmtp_authenticate(struct libsmtp_session_struct *session, const char *user, const char *password)
{
  GString *temp_gstring;	/* Temp gstring */
	int len_user = strlen(user);
	int len_password = strlen(password);
	int len_auth = strlen("auth plain ");
	char buffer[len_user + len_password + len_auth + 3];
	char *p;

  temp_gstring = g_string_new (NULL);

  /* We enter the auth stage now */
  session->Stage = LIBSMTP_AUTH_STAGE;

  /* Send the AUTH PLAIN authentification */

	p = buffer;
	strcpy(p, "auth plain "); p += len_auth;
	*p++ = 0;
	strcpy(p, user); p += len_user;
	*p++ = 0;
	strcpy(p, password); p += len_password;
	strcpy(p, "\r\n"); p += 2;

	if (libsmtp_int_send_base64(buffer, p - buffer, session, len_auth))
		return LIBSMTP_ERRORSENDFATAL;

  if (libsmtp_int_read(temp_gstring, session, 2))
    return LIBSMTP_ERRORSENDFATAL;

  if (session->LastResponseCode != 235)
	{
		/* We couldn't authenticate */
		libsmtp_close(session);
		session->ErrorCode = LIBSMTP_AUTHERR;
		return LIBSMTP_AUTHERR;
	}

  return LIBSMTP_NOERR;	
}


int libsmtp_close(struct libsmtp_session_struct *libsmtp_session)
{
  if (!libsmtp_session->stream)
	{
		if (libsmtp_session->socket >= 0)
		{
			close (libsmtp_session->socket);
			libsmtp_session->socket = -1;
		}
	}
	else
	{
		GB.Stream.Block(libsmtp_session->stream, libsmtp_session->was_blocking);
	}

  return LIBSMTP_NOERR;
}


int libsmtp_error(struct libsmtp_session_struct *session, int error)
{
	libsmtp_close(session);
	session->ErrorCode = error;
	return error;
}
