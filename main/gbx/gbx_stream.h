/***************************************************************************

  stream.h

  The stream management routines

  (c) 2000-2007 Benoit Minisini <gambas@users.sourceforge.net>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 1, or (at your option)
  any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

***************************************************************************/

#ifndef __GBX_STREAM_H
#define __GBX_STREAM_H

#include "gbx_value.h"
#include "gbx_archive.h"

union STREAM;

typedef
  struct STREAM_CLASS {
    int (*open)(union STREAM *stream, const char *path, int mode, void *data);
    int (*close)(union STREAM *stream);
    int (*read)(union STREAM *stream, char *buffer, long len);
    int (*getchar)(union STREAM *stream, char *buffer);
    int (*write)(union STREAM *stream, char *buffer, long len);
    int (*seek)(union STREAM *stream, long long pos, int whence);
    int (*tell)(union STREAM *stream, long long *pos);
    int (*flush)(union STREAM *stream);
    int (*eof)(union STREAM *stream);
    int (*lof)(union STREAM *stream, long long *len);
    int (*handle)(union STREAM *stream);
    }
  STREAM_CLASS;

typedef
  struct {
    STREAM_CLASS *type;
    short mode;
    unsigned swap:1;
    unsigned eol:2;
    unsigned eof:1;
    unsigned _reserved:12;
    }
  PACKED
  STREAM_COMMON;


typedef
  struct {
    STREAM_COMMON common;
    long _reserved[6];
    }
  PACKED
  STREAM_RESERVED;

typedef
  struct {
    STREAM_COMMON common;
    long long size;
    long fd;
    unsigned is_term : 1;
    unsigned _reserved : 31;
    }
  PACKED
  STREAM_DIRECT;

typedef
  struct {
    STREAM_COMMON common;
    long long size;
    FILE *file;
    unsigned is_term : 1;
    unsigned _reserved : 31;
    }
  PACKED
  STREAM_BUFFER;

typedef
  struct {
    STREAM_COMMON common;
    void *addr;
    //long size;
    long pos;
    }
  PACKED
  STREAM_MEMORY;

typedef
  struct {
    STREAM_COMMON common;
    ARCHIVE *arch;
    long size;
    long start;
    long pos;
    }
  PACKED
  STREAM_ARCH;

typedef
  struct {
    STREAM_COMMON common;
    long fdr;
    long fdw;
    unsigned read_something : 1;
    unsigned _reserved : 31;
    }
  PACKED
  STREAM_PROCESS;


typedef
  union STREAM {
    STREAM_CLASS *type;
    STREAM_COMMON common;
    STREAM_RESERVED _reserved;
    STREAM_DIRECT direct;
    STREAM_BUFFER buffer;
    STREAM_DIRECT pipe;
    STREAM_MEMORY memory;
    STREAM_ARCH arch;
    STREAM_PROCESS process;
    }
  STREAM;

enum {
  ST_READ = 1,
  ST_WRITE = 2,
  ST_READ_WRITE = 3,
  ST_MODE = 3,
  ST_APPEND = 4,
  ST_CREATE = 8,
  ST_ACCESS = 15,
  ST_DIRECT = 16,
  ST_LINE = 32,
  ST_WATCH = 64,
  ST_PIPE = 128
  };

enum {
	ST_EOL_UNIX = 0,
	ST_EOL_WINDOWS = 1,
	ST_EOL_MAC = 2
	};

EXTERN long STREAM_eff_read;

#ifndef __STREAM_IMPL_C

EXTERN STREAM_CLASS STREAM_direct;
EXTERN STREAM_CLASS STREAM_buffer;
EXTERN STREAM_CLASS STREAM_pipe;
EXTERN STREAM_CLASS STREAM_memory;
EXTERN STREAM_CLASS STREAM_arch;
EXTERN STREAM_CLASS STREAM_process;
/*EXTERN STREAM_CLASS STREAM_null;*/

#else

#define DECLARE_STREAM(stream) \
PUBLIC STREAM_CLASS stream = \
{ \
  (void *)stream_open, \
  (void *)stream_close, \
  (void *)stream_read, \
  (void *)stream_getchar, \
  (void *)stream_write, \
  (void *)stream_seek, \
  (void *)stream_tell, \
  (void *)stream_flush, \
  (void *)stream_eof, \
  (void *)stream_lof, \
  (void *)stream_handle \
}

#endif


PUBLIC void STREAM_exit(void);

PUBLIC bool STREAM_in_archive(const char *path);
//PUBLIC int STREAM_get_readable(int fd, long *len);

PUBLIC void STREAM_open(STREAM *stream, const char *path, int mode);

PUBLIC void STREAM_close(STREAM *stream);
PUBLIC void STREAM_write(STREAM *stream, void *addr, long len);
PUBLIC void STREAM_line_input(STREAM *stream, char **addr);
PUBLIC void STREAM_input(STREAM *stream, char **addr);
PUBLIC long long STREAM_tell(STREAM *stream);
PUBLIC void STREAM_seek(STREAM *stream, long long pos, int whence);
PUBLIC void STREAM_read(STREAM *stream, void *addr, long len);
PUBLIC char STREAM_getchar(STREAM *stream);
PUBLIC void STREAM_read_type(STREAM *stream, TYPE type, VALUE *value, long len);
PUBLIC void STREAM_write(STREAM *stream, void *addr, long len);
PUBLIC void STREAM_write_type(STREAM *stream, TYPE type, VALUE *value, long len);
PUBLIC void STREAM_write_eol(STREAM *stream);
PUBLIC void STREAM_flush(STREAM *stream);
PUBLIC int STREAM_handle(STREAM *stream);
PUBLIC void STREAM_lof(STREAM *stream, long long *len);
PUBLIC bool STREAM_eof(STREAM *stream);

PUBLIC void STREAM_load(const char *path, char **buffer, long *len);
PUBLIC bool STREAM_map(const char *path, char **buffer, long *len);

PUBLIC int STREAM_read_direct(int fd, char *buffer, long len);
PUBLIC int STREAM_write_direct(int fd, char *buffer, long len);
//PUBLIC int STREAM_read_buffered(FILE *file, char *buffer, long len);
//PUBLIC int STREAM_write_buffered(FILE *file, char *buffer, long len);

PUBLIC void STREAM_lock(STREAM *stream);

#define STREAM_is_closed(_stream) ((_stream)->type == NULL)

#endif
