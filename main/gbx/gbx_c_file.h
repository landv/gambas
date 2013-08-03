/***************************************************************************

  gbx_c_file.h

  (c) 2000-2013 Benoît Minisini <gambas@users.sourceforge.net>

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

#ifndef __GBX_C_FILE_H
#define __GBX_C_FILE_H

#include "gambas.h"

#include "gbx_value.h"
#include "gbx_stream.h"
#include "gbx_object.h"
#include "gb_file.h"

typedef
  struct {
    OBJECT ob;
    STREAM stream;
    GB_VARIANT_VALUE tag;
    }
  CSTREAM;

typedef
  struct {
    CSTREAM ob;
    int watch_fd;
    }
  CFILE;

typedef
  struct {
    OBJECT ob;
    FILE_STAT info;
    char *path;
    }
  CSTAT;

#ifndef __GBX_C_FILE_C
extern GB_DESC NATIVE_StreamLines[];
extern GB_DESC NATIVE_Stream[];
extern GB_DESC NATIVE_File[];
extern GB_DESC NATIVE_Stat[];
extern GB_DESC NATIVE_StatPerm[];
extern CFILE *CFILE_in;
extern CFILE *CFILE_out;
extern CFILE *CFILE_err;
#else
#define THIS ((CFILE *)_object)
#define THIS_STREAM ((CSTREAM *)_object)
#define THIS_STAT ((CSTAT *)_object)
#endif

#define CSTREAM_stream(_cstream) (&((CSTREAM *)(void *)(_cstream))->stream)

CFILE *CFILE_create(STREAM *stream, int mode);
void CFILE_init(void);
void CFILE_exit(void);
void CFILE_init_watch(void);

#endif
