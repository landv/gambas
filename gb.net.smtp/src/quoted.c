/***************************************************************************

	quoted.c

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
		This part will encode data into quoted-printable format

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

/* Encoded line length (max 76)*/
#define LINELEN 72

//#include "../config.h"

#include "libsmtp.h"
#include "libsmtp_mime.h"


/* This function will send data in a part, encoded in quoted printable format.
	It will not perform any checks whatsoever. */

int libsmtp_int_send_quoted (char *libsmtp_int_data, unsigned int libsmtp_int_length,
				struct libsmtp_session_struct *libsmtp_session)
{
	/* These are the input buffer and the output buffer */
	char ogroup[2056], obuffer[4];
	unsigned char c;
	//unsigned char libsmtp_int_last_char;
	int finished = 0, outbytes = 0, width = 0;
	int newline = 0;
	/* This points into the data stream to the byte we are reading ATM */
	unsigned int current = 0;

	/* Lets clear the buffers */
	bzero (obuffer, 4);
	bzero (ogroup, 2056);
	c = 0;

	/* The main parsing loop */
	while (!finished)
	{
		/* We fetch a character from the input buffer */
		//libsmtp_int_last_char = c;
		c = libsmtp_int_data[current++];

		/* Lets check that we don't read over the end of the input buffer */
		if (current >= libsmtp_int_length)
			finished = 1;

		/* We decide which characters to encode and which not */
		switch (c)
		{
			/* Don't encode whitespace (except at end of line - we check for that
				below */
			case 9:
			case 32:
				if (width == 0)
					sprintf(obuffer, "=%02X", c);
				else
					obuffer[0]=0;
			break;

			/* We need to encode 61 */
			case 61:
				strcpy(obuffer, "=3D");
			break;

			/* Newlines need special observation */
			case '\n':

				/* Newline characters don't need to be encoded, but if they follow
					whitespace, the whitespace has to be suffixed with = */
				obuffer[0]=0;
				newline=1;
			break;

			default:
				/* All values below 33 need to be encoded except newline/CR and tab
					which we have already dealt with above. All values above 126 needs
					to be encoded as well */
				if ((c < 33) || (c > 126 ))
					sprintf(obuffer, "=%02X", c);
				else if (c == '.' && width == 0)
					strcpy(obuffer, "=2E");
				else
					/* The rest can go unencoded */
					obuffer[0]=0;
			break;
		}

		/* Lets see ... */
		if (obuffer[0])
		{
			/* Something encoded */
			strcpy(&ogroup[outbytes], obuffer);
			outbytes += 3;
			width += 3;

			#ifdef LIBSMTP_QUOTED_DEBUG
				printf ("Got encoded: %c = %s. Group: %s (%d)\n", c,
						obuffer, ogroup, outbytes);
			#endif
		}
		else
		{
			/* An unencoded char */
			if (newline)
			{
				newline=0;
				/* We have to check if the char before us is whitespace */
				if (outbytes)
				{
					if (ogroup[outbytes-1] <= ' ')
					{
						/* We need to add a = after the whitespace */
						strcpy(&ogroup[outbytes], "=\r\n");
						//ogroup[outbytes+1]= c;
						//ogroup[outbytes+2]=0;
						outbytes += 3;
						/* We have a new line */
						width=0;
					}
				}

				strcpy(&ogroup[outbytes], "\r\n");
				outbytes += 2;
			}
			else
			{
				/* An unencoded character is added */
				ogroup[outbytes] = c;
				ogroup[outbytes+1] = 0;
				outbytes++;
				width++;
			}

			#ifdef LIBSMTP_QUOTED_DEBUG
				printf ("Got unencoded: %c = %s. Group: %s (%d)\n", c,
						obuffer, ogroup, outbytes);
			#endif
		}

		/* After LINELEN characters in a line we need a soft linebreak */
		if (width >= LINELEN )
		{
			ogroup[outbytes++]='=';
			ogroup[outbytes++]='\r';
			ogroup[outbytes++]='\n';
			width=0;
		}

		/* If we have more than 2K of data, we send it */
		if (outbytes >= 2048)
		{
			ogroup[outbytes]='\0';
			if (libsmtp_int_send_body (ogroup, outbytes, libsmtp_session))
				return LIBSMTP_ERRORSENDFATAL;

			#ifdef LIBSMTP_DEBUG
				printf ("libsmtp_send_quoted: out: %s\n", ogroup);
			#endif

			/* We reset the pointer into our outbuffer, too */
			outbytes=0;
		}
	}

	/* We send the rest of the data out anyway. It is better to send a linebreak
		here so the next pack won't go over 72 characters a line */

	ogroup[outbytes++]='\r';
	ogroup[outbytes++]='\n';
	ogroup[outbytes]='\0';
	
	if (libsmtp_int_send_body (ogroup, outbytes, libsmtp_session))
		return LIBSMTP_ERRORSENDFATAL;
	
	#ifdef LIBSMTP_DEBUG
		printf ("libsmtp_send_quoted: out: %s\n", ogroup);
	#endif

	return LIBSMTP_NOERR;
}

static const char _utf8_char_length[256] =
{
	1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
	1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
	1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
	1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
	1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
	1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
	2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,
	3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,4,4,4,4,4,4,4,4,5,5,5,5,6,6,1,1
};

/* This function will send data in an header, encoded in utf-8 quoted printable format.
	It will not perform any checks whatsoever. */

int libsmtp_int_send_quoted_header (const char *header, char *libsmtp_int_data, unsigned int libsmtp_int_length,
				struct libsmtp_session_struct *libsmtp_session)
{
	/* These are the input buffer and the output buffer */
	char ogroup[2056], obuffer[4];
	unsigned char c;
	//unsigned char libsmtp_int_last_char;
	int finished=0, outbytes=0, width=0;
	/* This points into the data stream to the byte we are reading ATM */
	unsigned int current=0;
	int len;

	/* Lets clear the buffers */
	bzero (obuffer, 4);
	bzero (ogroup, 2056);
	c = 0;

	/* Adds the header name */
	outbytes = sprintf(ogroup, "%s =?utf-8?q?", header);
	width = outbytes;	

	/* The main parsing loop */
	while (!finished)
	{
		/* We fetch a character from the input buffer */
		//libsmtp_int_last_char = c;
		c = libsmtp_int_data[current++];

		obuffer[0] = 0;
		len = 1;
		
		if (c == 32)
			c = '_';
		else if (c < 33 || c > 126 || c == '_' || c == '=' || c == '?')
			len = sprintf(obuffer, "=%02X", c);
		
		
		if (c >= 128)
			len += (_utf8_char_length[c] - 1) * 3;
		
		/* After LINELEN characters in a line we need a soft linebreak */
		if ((width + len) >= LINELEN)
		{
			outbytes += sprintf(&ogroup[outbytes], "?=\r\n");
			width = sprintf(&ogroup[outbytes], " =?utf-8?q?");
			outbytes += width;
		}
		
		/* Lets see ... */
		if (obuffer[0])
		{
			/* Something encoded */
			strcpy(&ogroup[outbytes], obuffer);
			outbytes += 3;
			width += 3;

			#ifdef LIBSMTP_QUOTED_DEBUG
				printf ("Got encoded: %c = %s. Group: %s (%d)\n", c,
						obuffer, ogroup, outbytes);
			#endif
		}
		else
		{
			/* An unencoded character is added */
			ogroup[outbytes] = c;
			ogroup[outbytes + 1] = 0;
			outbytes++;
			width++;

			#ifdef LIBSMTP_QUOTED_DEBUG
				printf ("Got unencoded: %c = %s. Group: %s (%d)\n", c,
						obuffer, ogroup, outbytes);
			#endif
		}

		/* Lets check that we don't read over the end of the input buffer */
		if (current >= libsmtp_int_length)
		{
			finished = TRUE;
			outbytes += sprintf(&ogroup[outbytes], "?=\r\n");
		}

		/* If we have more than 2K of data, we send it */
		if (outbytes >= 2048 || finished)
		{
			ogroup[outbytes] = 0;
			if (libsmtp_int_send_body (ogroup, outbytes, libsmtp_session))
				return LIBSMTP_ERRORSENDFATAL;

			#ifdef LIBSMTP_DEBUG
				printf ("libsmtp_send_quoted: out: %s\n", ogroup);
			#endif

			/* We reset the pointer into our outbuffer, too */
			outbytes=0;
		}
	}

	return LIBSMTP_NOERR;
}

