/***************************************************************************

  image_stat.c

  (c) 2000-2014 Benoît Minisini <gambas@users.sourceforge.net>

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

#define __IMAGE_STAT_C

#include <stdlib.h>
#include <stdio.h>

#include "image_stat.h"

const char *IMAGE_error = NULL;

static const char _sign_gif[3] = {'G', 'I', 'F'};
static const char _sign_bmp[2] = {'B', 'M'};
static const char _sign_jpg[3] = {0xff, 0xd8, 0xff};
static const char _sign_png[8] = {0x89, 0x50, 0x4e, 0x47, 0x0d, 0x0a, 0x1a, 0x0a};
static const char _sign_tif_ii[4] = {'I','I', 0x2A, 0x00};
static const char _sign_tif_mm[4] = {'M','M', 0x00, 0x2A};

static int stream_seek(IMAGE_STREAM *stream, int pos, int whence)
{
	switch (whence)
	{
		case SEEK_CUR:
			if ((stream->pos + pos) >= stream->len)
				return 1;
			if ((stream->pos + pos) < 0)
				return 1;
			stream->pos += pos;
			return 0;
		
		case SEEK_SET:
			if (pos < 0 || pos >= stream->len)
				return 1;
			stream->pos = pos;
			return 0;
		
		default:
			return 1;
	}
}

static int stream_read(IMAGE_STREAM *stream, void *addr, int len)
{
	int lmax = stream->len - stream->pos;
	
	if (len > lmax)
		len = lmax;
		
	memcpy(addr, stream->addr + stream->pos, len);
	stream->pos += len;
	return len;
}


static int stream_getc(IMAGE_STREAM *stream)
{
	if (stream->pos >= stream->len)
		return EOF;
		
	return (uchar)stream->addr[stream->pos++];
}


static ushort read_ushort(IMAGE_STREAM * stream)
{
	uchar a[2];

	/* returns 0 if we hit the end-of-file */
	if((stream_read(stream, a, sizeof(a))) <= 0)
		return 0;

	return (((ushort)a[0]) << 8) + ((ushort)a[1]);
}


static int get_ushort_at(void *pshort, int big_endian)
{
	if (big_endian)
		return (((uchar *)pshort)[0] << 8) | ((uchar *)pshort)[1];
	else
		return (((uchar *)pshort)[1] << 8) | ((uchar *)pshort)[0];
}

static signed short get_short_at(void *pshort, int big_endian)
{
	return (signed short)get_ushort_at(pshort, big_endian);
}

static int get_int_at(void *pint, int big_endian)
{
	if (big_endian)
		return  (((char*)pint)[0] << 24) | (((uchar *)pint)[1] << 16) | (((uchar *)pint)[2] << 8 ) | (((uchar *)pint)[3] << 0);
	else
		return  (((char*)pint)[3] << 24) | (((uchar *)pint)[2] << 16) | (((uchar *)pint)[1] << 8 ) | (((uchar *)pint)[0] << 0);
}

static unsigned get_uint_at(void *pint, int big_endian)
{
	return (uint)get_int_at(pint, big_endian) & 0xffffffff;
}


//-------------------------------------------------------------------------

static bool handle_gif (IMAGE_STREAM * stream, IMAGE_INFO *result)
{
	uchar dim[5];

	if (stream_seek(stream, 3, SEEK_CUR))
		return TRUE;

	if (stream_read(stream, dim, sizeof(dim)) != sizeof(dim))
		return TRUE;

	result->width = (uint)dim[0] | (((uint)dim[1]) << 8);
	result->height = (uint)dim[2] | (((uint)dim[3]) << 8);
	result->depth = (dim[4] & 0x80) ? ((((uint)dim[4]) & 0x07) + 1) : 0;
	return FALSE;
}

//-------------------------------------------------------------------------

static bool handle_bmp (IMAGE_STREAM * stream, IMAGE_INFO *result)
{
	uchar dim[16];
	int size;

	if (stream_seek(stream, 11, SEEK_CUR))
		return TRUE;

	if (stream_read(stream, dim, sizeof(dim)) != sizeof(dim))
		return TRUE;

	size   = (((uint)dim[3]) << 24) + (((uint)dim[2]) << 16) + (((uint)dim[1]) << 8) + ((uint) dim[0]);
	if (size == 12)
	{
		result->width = (((uint)dim[5]) << 8) + ((uint)dim[4]);
		result->height = (((uint)dim[7]) << 8) + ((uint) dim[6]);
		result->depth = ((uint)dim[11]);
		return FALSE;
	}
	else if (size > 12 && (size <= 64 || size == 108))
	{
		result->width = (((uint)dim[7]) << 24) + (((uint)dim[6]) << 16) + (((uint)dim[5]) << 8) + ((uint) dim[4]);
		result->height = (((uint)dim[11]) << 24) + (((uint)dim[10]) << 16) + (((uint)dim[9]) << 8) + ((uint) dim[8]);
		result->depth = (((uint)dim[15]) <<  8) +  ((uint)dim[14]);
		return FALSE;
	}

	return TRUE;
}

//-------------------------------------------------------------------------

static bool handle_png (IMAGE_STREAM * stream, IMAGE_INFO *result)
{
	uchar dim[13];
	
	if (stream_seek(stream, 8, SEEK_CUR))
		return TRUE;

	if((stream_read(stream, dim, sizeof(dim))) < sizeof(dim))
		return TRUE;

	result->width = (((uint)dim[0]) << 24) + (((uint)dim[1]) << 16) + (((uint)dim[2]) << 8) + ((uint)dim[3]);
	result->height = (((uint)dim[4]) << 24) + (((uint)dim[5]) << 16) + (((uint)dim[6]) << 8) + ((uint)dim[7]);
	
	switch (dim[9])
	{
		case 0:	result->depth = 8; break;
		case 2: result->depth = 24; break;
		case 3: result->depth = 8; break; 
		case 4: result->depth = 32; break; 
		case 6: result->depth = 32; break;
		default: result->depth = 32; break;
	}
	
	return FALSE;
}

//-------------------------------------------------------------------------

/* some defines for the different JPEG block types */
#define M_SOF0  0xC0			/* Start Of Frame N */
#define M_SOF1  0xC1			/* N indicates which compression process */
#define M_SOF2  0xC2			/* Only SOF0-SOF2 are now in common use */
#define M_SOF3  0xC3
#define M_SOF5  0xC5			/* NB: codes C4 and CC are NOT SOF markers */
#define M_SOF6  0xC6
#define M_SOF7  0xC7
#define M_SOF9  0xC9
#define M_SOF10 0xCA
#define M_SOF11 0xCB
#define M_SOF13 0xCD
#define M_SOF14 0xCE
#define M_SOF15 0xCF
#define M_SOI   0xD8
#define M_EOI   0xD9			/* End Of Image (end of datastream) */
#define M_SOS   0xDA			/* Start Of Scan (begins compressed data) */
#define M_APP0  0xE0
#define M_APP1  0xE1
#define M_APP2  0xE2
#define M_APP3  0xE3
#define M_APP4  0xE4
#define M_APP5  0xE5
#define M_APP6  0xE6
#define M_APP7  0xE7
#define M_APP8  0xE8
#define M_APP9  0xE9
#define M_APP10 0xEA
#define M_APP11 0xEB
#define M_APP12 0xEC
#define M_APP13 0xED
#define M_APP14 0xEE
#define M_APP15 0xEF
#define M_COM   0xFE            /* COMment                                  */

#define M_PSEUDO 0xFFD8			/* pseudo marker for start of image(byte 0) */

static uint next_marker(IMAGE_STREAM * stream, int last_marker, int comment_correction, int ff_read)
{
	int a=0, marker;

	/* get marker byte, swallowing possible padding                           */
	if (last_marker == M_COM && comment_correction)
	{
		/* some software does not count the length bytes of COM section           */
		/* one company doing so is very much envolved in JPEG... so we accept too */
		/* by the way: some of those companies changed their code now...          */
		comment_correction = 2;
	}
	else
	{
		last_marker = 0;
		comment_correction = 0;
	}

	if (ff_read)
		a = 1; /* already read 0xff in filetype detection */

	do
	{
		if ((marker = stream_getc(stream)) == EOF)
			return M_EOI;/* we hit EOF */

		if (last_marker == M_COM && comment_correction > 0)
		{
			if (marker != 0xFF)
			{
				marker = 0xff;
				comment_correction--;
			}
			else
			{
				last_marker = M_PSEUDO; /* stop skipping non 0xff for M_COM */
			}
		}

		if (++a > 25)
		{
			/* who knows the maxim amount of 0xff? though 7 */
			/* but found other implementations              */
			return M_EOI;
		}
	}
	while (marker == 0xff);

	if (a < 2)
		return M_EOI; /* at least one 0xff is needed before marker code */

	if (last_marker == M_COM && comment_correction)
		return M_EOI; /* ah illegal: char after COM section not 0xFF */

	return (uint)marker;
}


/* skip over a variable-length block; assumes proper length marker */
static int skip_variable(IMAGE_STREAM * stream)
{
	int length = read_ushort(stream);

	if (length < 2)
		return 0;

	length -= 2;
	(void)stream_seek(stream, length, SEEK_CUR);
	return 1;
}


static bool handle_jpeg(IMAGE_STREAM * stream, IMAGE_INFO *result)
{
	uint marker = M_PSEUDO;
	ushort ff_read = 1;
	bool ret = TRUE;

	for(;;)
	{
		marker = next_marker(stream, marker, 1, ff_read);
		ff_read = 0;

		switch (marker)
		{
			case M_SOF0:
			case M_SOF1:
			case M_SOF2:
			case M_SOF3:
			case M_SOF5:
			case M_SOF6:
			case M_SOF7:
			case M_SOF9:
			case M_SOF10:
			case M_SOF11:
			case M_SOF13:
			case M_SOF14:
			case M_SOF15:

				stream_getc(stream);
				result->height = read_ushort(stream);
				result->width = read_ushort(stream);
				stream_getc(stream);
				result->depth = 24;
				return FALSE;

			case M_APP0:
			case M_APP1:
			case M_APP2:
			case M_APP3:
			case M_APP4:
			case M_APP5:
			case M_APP6:
			case M_APP7:
			case M_APP8:
			case M_APP9:
			case M_APP10:
			case M_APP11:
			case M_APP12:
			case M_APP13:
			case M_APP14:
			case M_APP15:

				if (!skip_variable(stream))
					return ret;

				break;

			case M_SOS:
			case M_EOI:

				return ret;	/* we're about to hit image data, or are at EOF. stop processing. */
			
			default:

				if (!skip_variable(stream)) /* anything else isn't interesting */
					return ret;

				break;
		}
	}

	return ret; /* perhaps image broken -> no info but size */
}

//-------------------------------------------------------------------------

static const int _tiff_bytes_per_format[] = {0, 1, 1, 2, 4, 8, 1, 1, 2, 4, 8, 4, 8};

/* uncompressed only */
#define TAG_IMAGEWIDTH              0x0100
#define TAG_IMAGEHEIGHT             0x0101
/* compressed images only */
#define TAG_COMP_IMAGEWIDTH         0xA002
#define TAG_COMP_IMAGEHEIGHT        0xA003

#define TAG_FMT_BYTE       1
#define TAG_FMT_STRING     2
#define TAG_FMT_USHORT     3
#define TAG_FMT_ULONG      4
#define TAG_FMT_URATIONAL  5
#define TAG_FMT_SBYTE      6
#define TAG_FMT_UNDEFINED  7
#define TAG_FMT_SSHORT     8
#define TAG_FMT_SLONG      9
#define TAG_FMT_SRATIONAL 10
#define TAG_FMT_SINGLE    11
#define TAG_FMT_DOUBLE    12

static bool handle_tiff(IMAGE_STREAM * stream, IMAGE_INFO *result, bool big_endian)
{
	int i, num_entries;
	uchar *dir_entry;
	size_t ifd_size, dir_size, entry_value, ifd_addr;
	int entry_tag , entry_type;
	char *ifd_data, ifd_ptr[4];
	int width = 0, height = 0;

	if (stream_read(stream, ifd_ptr, 4) != 4)
		return TRUE;

	ifd_addr = get_uint_at(ifd_ptr, big_endian);
	if (stream_seek(stream, ifd_addr - 8, SEEK_CUR))
		return TRUE;

	ifd_size = 2;
	GB.Alloc(POINTER(&ifd_data), ifd_size);

	if (stream_read(stream, ifd_data, 2) != 2)
	{
		GB.Free(POINTER(&ifd_data));
		return TRUE;
	}

	num_entries = get_ushort_at(ifd_data, big_endian);

	// dir_size = <num dir entries> + <length of entry> * <num_entries> + <offset to next ifd (points to thumbnail or NULL)>
	dir_size = 2 + 12 * num_entries + 4;

	ifd_size = dir_size;
	GB.Realloc(POINTER(&ifd_data), ifd_size);

	if (stream_read(stream, ifd_data + 2, dir_size - 2) != dir_size - 2)
	{
		GB.Free(POINTER(&ifd_data));
		return TRUE;
	}

	/* now we have the directory we can look how long it should be */
	ifd_size = dir_size;
	for(i = 0; i < num_entries; i++)
	{
		dir_entry = (uchar *)(ifd_data + 2 + i * 12);
		entry_tag = get_ushort_at(dir_entry + 0, big_endian);
		entry_type = get_ushort_at(dir_entry + 2, big_endian);

		switch(entry_type)
		{
			case TAG_FMT_BYTE:
			case TAG_FMT_SBYTE:
				entry_value  = (size_t)(dir_entry[8]);
				break;
			case TAG_FMT_USHORT:
				entry_value  = get_ushort_at(dir_entry+8, big_endian);
				break;
			case TAG_FMT_SSHORT:
				entry_value  = get_short_at(dir_entry+8, big_endian);
				break;
			case TAG_FMT_ULONG:
				entry_value  = get_uint_at(dir_entry+8, big_endian);
				break;
			case TAG_FMT_SLONG:
				entry_value  = get_int_at(dir_entry+8, big_endian);
				break;
			default:
				continue;
		}

		switch(entry_tag)
		{
			case TAG_IMAGEWIDTH:
			case TAG_COMP_IMAGEWIDTH:
				width  = entry_value;
				break;

			case TAG_IMAGEHEIGHT:
			case TAG_COMP_IMAGEHEIGHT:
				height = entry_value;
				break;
		}
	}

	GB.Free(POINTER(&ifd_data));

	if (width > 0 && height > 0)
	{
		result->height = height;
		result->width = width;
		result->depth = 24;
		return FALSE;
	}

	return TRUE;
}

//-------------------------------------------------------------------------

static char *image_type_to_mime_type(int type)
{
	switch(type)
	{
		case IMAGE_TYPE_GIF:
			return "image/gif";
		case IMAGE_TYPE_JPEG:
			return "image/jpeg";
		case IMAGE_TYPE_PNG:
			return "image/png";
		case IMAGE_TYPE_BMP:
			return "image/bmp";
		case IMAGE_TYPE_TIFF_INTEL:
		case IMAGE_TYPE_TIFF_MOTOROLA:
			return "image/tiff";
		default:
		case IMAGE_TYPE_UNKNOWN:
			return "application/octet-stream";
	}
}

static int get_image_type(IMAGE_STREAM * stream)
{
	char buffer[12];

	if((stream_read(stream, buffer, 3)) != 3)
	{
		IMAGE_error = "Read error";
		return IMAGE_TYPE_ERROR;
	}

	if (memcmp(buffer, _sign_gif, 3) == 0)
		return IMAGE_TYPE_GIF;

	if (memcmp(buffer, _sign_jpg, 3) == 0)
		return IMAGE_TYPE_JPEG;

	if (memcmp(buffer, _sign_png, 3) == 0)
	{
		if (stream_read(stream, buffer + 3, 5) != 5)
		{
			IMAGE_error = "Read error";
			return IMAGE_TYPE_ERROR;
		}

		if (memcmp(buffer, _sign_png, 8) == 0)
			return IMAGE_TYPE_PNG;

		IMAGE_error = "PNG file is corrupted";
		return IMAGE_TYPE_ERROR;
	}

	if (memcmp(buffer, _sign_bmp, 2) == 0)
		return IMAGE_TYPE_BMP;

	if (stream_read(stream, buffer + 3, 1) != 1)
	{
		IMAGE_error = "Read error";
		return IMAGE_TYPE_ERROR;
	}

	if (memcmp(buffer, _sign_tif_ii, 4) == 0)
		return IMAGE_TYPE_TIFF_INTEL;

	if (memcmp(buffer, _sign_tif_mm, 4) == 0)
		return IMAGE_TYPE_TIFF_MOTOROLA;

	return IMAGE_TYPE_UNKNOWN;
}


bool IMAGE_get_info(IMAGE_STREAM *stream, IMAGE_INFO *result)
{
	int type;
	bool err;

	type = get_image_type(stream);
	if (type == IMAGE_TYPE_ERROR)
		return TRUE;
		
	result->type = image_type_to_mime_type(type);
	
	switch(type)
	{
		case IMAGE_TYPE_GIF:
			err = handle_gif(stream, result);
			break;
		case IMAGE_TYPE_JPEG:
			err = handle_jpeg(stream, result);
			break;
		case IMAGE_TYPE_PNG:
			err = handle_png(stream, result);
			break;
		case IMAGE_TYPE_BMP:
			err = handle_bmp(stream, result);
			break;
		case IMAGE_TYPE_TIFF_INTEL:
			err = handle_tiff(stream, result, FALSE);
			break;
		case IMAGE_TYPE_TIFF_MOTOROLA:
			err = handle_tiff(stream, result, TRUE);
			break;
		default:
		case IMAGE_TYPE_UNKNOWN:
			err = FALSE;
			break;
	}

	if (err)
		IMAGE_error = "Cannot read file";

	return err;
}

