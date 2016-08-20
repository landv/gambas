/***************************************************************************

  main.h

  (c) 2003-2004 Daniel Campos Fern�ndez <danielcampos@netcourrier.com>

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

#ifndef __MAIN_H
#define __MAIN_H

#include "gambas.h"
#include "gb_common.h"
#include "../gb.compress.h"

#ifndef __MAIN_C
extern GB_INTERFACE GB;
extern COMPRESS_INTERFACE COMPRESSION;
extern GB_STREAM_DESC ZStream;
#endif

static int CZ_stream_lof(GB_STREAM *stream, int64_t *len);
static int CZ_stream_seek(GB_STREAM *stream, int64_t offset, int whence);
static int CZ_stream_open(GB_STREAM *stream, const char *path, int mode, void *data);
static int CZ_stream_tell(GB_STREAM *stream, int64_t *npos);
static int CZ_stream_flush(GB_STREAM *stream);
static int CZ_stream_close(GB_STREAM *stream);
static int CZ_stream_write(GB_STREAM *stream, char *buffer, int len);
static int CZ_stream_eof(GB_STREAM *stream);
static int CZ_stream_read(GB_STREAM *stream, char *buffer, int len);

#endif /* __MAIN_H */
