/*
  libsmtp is a library to send mail via SMTP
     This part will encode data into base64 format

     Base64 routines are borrowed from code by John Walker
		       http://www.fourmilab.ch/

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

#define LINELEN 72		      /* Encoded line length (max 76) */

//#include "../config.h"

#include "libsmtp.h"
#include "libsmtp_mime.h"


/* We declare some global variables *yuck* */
static unsigned char libsmtp_int_dtable[256];	      /* Encode / decode table */
int libsmtp_int_dtable_init=0; /* Is it initialized? */

/* This function will initialize the base64 encoding tables */

int libsmtp_int_init_base64 (void)
{
  int libsmtp_int_temp;
  /*	Fill dtable with character encodings.  */

  for (libsmtp_int_temp = 0; libsmtp_int_temp < 26; libsmtp_int_temp++) {
    libsmtp_int_dtable[libsmtp_int_temp] = 'A' + libsmtp_int_temp;
    libsmtp_int_dtable[26 + libsmtp_int_temp] = 'a' + libsmtp_int_temp;
  }
  for (libsmtp_int_temp = 0; libsmtp_int_temp < 10; libsmtp_int_temp++) {
    libsmtp_int_dtable[52 + libsmtp_int_temp] = '0' + libsmtp_int_temp;
  }
  libsmtp_int_dtable[62] = '+';
  libsmtp_int_dtable[63] = '/';

  return 0;
}

/* This function will send data in a part, encoded in base64. It will not
   perform any checks whatsoever. */

int libsmtp_int_send_base64 (char *libsmtp_int_data, unsigned int libsmtp_int_length, \
         struct libsmtp_session_struct *libsmtp_session)
{
  int libsmtp_int_counter;

  /* These are the input buffer and the output buffer */
  unsigned char libsmtp_int_igroup[3], libsmtp_int_ogroup[2056];
  unsigned char libsmtp_int_char;
  int libsmtp_int_finished=0, libsmtp_int_outbytes=0, libsmtp_int_width=0;
  /* This points into the data stream to the byte we are reading ATM */
  unsigned int libsmtp_int_data_ptr=0;
  //GString *libsmtp_int_gstring=g_string_new (NULL);

  if (!libsmtp_int_dtable_init)
    libsmtp_int_init_base64();

  /* The main parsing loop */
  while (!libsmtp_int_finished)
  {
    /* We now fetch 3 bytes from the input data */
    libsmtp_int_igroup[0] = libsmtp_int_igroup[1] = libsmtp_int_igroup[2] = 0;
    for (libsmtp_int_counter = 0; libsmtp_int_counter < 3; \
         libsmtp_int_counter++)
    {
      libsmtp_int_char = libsmtp_int_data[libsmtp_int_data_ptr++];

      /* Lets check that we don't read over the end of the input buffer */
      if (libsmtp_int_data_ptr > libsmtp_int_length)
      {
        libsmtp_int_finished = 1;
        break;
      }

      /* Assign the fetched data to the input buffer. This could all be
         optimized */
      libsmtp_int_igroup[libsmtp_int_counter] = (unsigned char) libsmtp_int_char;
    }

    /* Only encode if we have fetched any bytes */
    if (libsmtp_int_counter > 0)
    {
      /* This is the encoding stuff - courtesy of John Walker */
      libsmtp_int_ogroup[libsmtp_int_outbytes] = libsmtp_int_dtable[libsmtp_int_igroup[0] >> 2];
      libsmtp_int_ogroup[libsmtp_int_outbytes+1] = libsmtp_int_dtable[((libsmtp_int_igroup[0] & 3) << 4) | (libsmtp_int_igroup[1] >> 4)];
      libsmtp_int_ogroup[libsmtp_int_outbytes+2] = libsmtp_int_dtable[((libsmtp_int_igroup[1] & 0xF) << 2) | (libsmtp_int_igroup[2] >> 6)];
      libsmtp_int_ogroup[libsmtp_int_outbytes+3] = libsmtp_int_dtable[libsmtp_int_igroup[2] & 0x3F];

      /* Replace characters in output stream with "=" pad
         characters if fewer than three characters were
         read from the end of the input stream. */

      if (libsmtp_int_counter < 3)
      {
        libsmtp_int_ogroup[libsmtp_int_outbytes+3] = '=';
        if (libsmtp_int_counter < 2)
        {
          libsmtp_int_ogroup[libsmtp_int_outbytes+2] = '=';
        }
      }
      libsmtp_int_outbytes+=4;
      libsmtp_int_width+=4;

      /* After 72 characters in a line we need a linebreak */
      if (libsmtp_int_width > 72 )
      {
        libsmtp_int_ogroup[libsmtp_int_outbytes++]='\r';
        libsmtp_int_ogroup[libsmtp_int_outbytes++]='\n';
        libsmtp_int_width=0;
      }

      /* If we have more than 2K of data, we send it */
      if (libsmtp_int_outbytes >=2048)
      {
        libsmtp_int_ogroup[libsmtp_int_outbytes]='\0';
        if (libsmtp_int_send_body ((char *)libsmtp_int_ogroup, libsmtp_int_outbytes, libsmtp_session))
          return LIBSMTP_ERRORSENDFATAL;

        #ifdef LIBSMTP_DEBUG
          printf ("libsmtp_send_base64: out: %s\n", libsmtp_int_ogroup);
        #endif
        /* We reset the pointer into our outbuffer, too */
        libsmtp_int_outbytes=0;
      }
    }
  }

  /* We send the rest of the data out anyway. It is better to send a linebreak
     here so the next pack won't go over 72 characters a line */

  libsmtp_int_ogroup[libsmtp_int_outbytes++]='\r';
  libsmtp_int_ogroup[libsmtp_int_outbytes++]='\n';
  libsmtp_int_ogroup[libsmtp_int_outbytes]='\0';
  if (libsmtp_int_send_body ((char *)libsmtp_int_ogroup, libsmtp_int_outbytes, libsmtp_session))
    return LIBSMTP_ERRORSENDFATAL;

  #ifdef LIBSMTP_DEBUG
    printf ("libsmtp_send_base64: out: %s\n", libsmtp_int_ogroup);
  #endif

  return LIBSMTP_NOERR;
}

