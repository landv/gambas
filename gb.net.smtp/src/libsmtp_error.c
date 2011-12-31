/***************************************************************************

  libsmtp_error.c

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
  This is a library to send mail via SMTP

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

#ifdef WITH_MIME
  #include "libsmtp_mime.h"
#endif

char libsmtp_more_error[256];

const char *libsmtp_strerr_strings_fatal[] = {
  "No error",   /* 0 */
  "Unable to create local socket",
  "Mailserver unknown",
  "Connection to mailserver failed",
  "Unable to read from socket, fatal", /* 4 */
  "Mailserver didn't greet correctly conforming to RFC, we might not be welcome",
  "Can't find hostname or domainname",
  "Unable to send to socket", /* 7 */
  "Server won't accept sender",
  "Server rejected mail!!",
  "Server won't accept DATA command",
  "Authorization failed"
};

const char *libsmtp_strerr_strings_nonfatal[] = {
  "Error reading from socket",   /* 1024 */
  "Error sending to socket",
  "Bad arguments passed to libsmtp_function",
  "Server won't accept recipient",
  "Bad stage in libsmtp",
  "Server rejected QUIT command :-)"
};

const char *libsmtp_undef_errstr = "Undefined error";

const char *libsmtp_strerr (struct libsmtp_session_struct *libsmtp_session)
{
  /* This shouldn't really happen, but this is not C++, we can't prevent
     non-libsmtp functions from writing to these ...
     There are no higher error codes than the MIME ones */
  #ifdef WITH_MIME
    if (libsmtp_session->ErrorCode > LIBSMTP_MAX_MIME_ERRNO)
    {
      printf ("Undefined error code: %d\n", libsmtp_session->ErrorCode);
      return libsmtp_undef_errstr;
    }
  #else
    /* Or the nonfatal ones when not using MIME stuff */
    if (libsmtp_session->ErrorCode > LIBSMTP_MAX_NONFATAL_ERRNO)
    {
      printf ("Undefined error code: %d\n", libsmtp_session->ErrorCode);
      return libsmtp_undef_errstr;
    }
  #endif

  /* And there are no valid error codes between fatal and nonfatal */
  if ((libsmtp_session->ErrorCode > LIBSMTP_MAX_FATAL_ERRNO) && \
      (libsmtp_session->ErrorCode < LIBSMTP_MIN_NONFATAL_ERRNO))
  {
    printf ("Undefined error code: %d\n", libsmtp_session->ErrorCode);
    return libsmtp_undef_errstr;
  }

  /* Now send back the pointer - we have two tables */
  if (libsmtp_session->ErrorCode > LIBSMTP_MAX_FATAL_ERRNO)
    return libsmtp_strerr_strings_nonfatal [libsmtp_session->ErrorCode-1024];
  else
    return libsmtp_strerr_strings_fatal [libsmtp_session->ErrorCode];
}

int libsmtp_errno (struct libsmtp_session_struct *libsmtp_session)
{
  /* This shouldn't really happen, but this is not C++, we can't prevent
     non-libsmtp functions from writing to these ...
     There are no higher error codes than the nonfatal ones */
  if (libsmtp_session->ErrorCode > LIBSMTP_MAX_NONFATAL_ERRNO)
    return LIBSMTP_UNDEFERR;

  /* And there are no valid error codes betwenn fatal and nonfatal */
  if ((libsmtp_session->ErrorCode > LIBSMTP_MAX_FATAL_ERRNO) && \
      (libsmtp_session->ErrorCode < LIBSMTP_MIN_NONFATAL_ERRNO))
    return LIBSMTP_UNDEFERR;

  return libsmtp_session->ErrorCode;
}

