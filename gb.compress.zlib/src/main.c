/***************************************************************************

  main.c

  ZLIB Compression driver

  (c) 2003-2004 Daniel Campos Fernández <danielcampos@netcourrier.com>

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

#define __MAIN_C

/* Use 64 bits I/O */
#define _FILE_OFFSET_BITS 64

#include <stdio.h>
#include <zlib.h>
#include <errno.h>

#include "main.h"

#define GB_Z_BUFFER 8192
#define MODE_READ 0
#define MODE_WRITE 1

GB_INTERFACE EXPORT GB;
COMPRESS_INTERFACE EXPORT COMPRESSION;

static COMPRESS_DRIVER _driver;

GB_STREAM_DESC ZStream = {
	CZ_stream_open,
	CZ_stream_close,
	CZ_stream_read,
	CZ_stream_write,
	CZ_stream_seek,
	CZ_stream_tell,
	CZ_stream_flush,
	CZ_stream_eof,
	CZ_stream_lof
};

/*****************************************************************************

  The driver interface

*****************************************************************************/

/*********************************************************************************
 The Driver must provide this function:

 int max_compression(void)

 It must return the maximum compression level that user can
 assign, for instance gzip (in this moment) provides ten values, from zero to
 9 , beeing 9 the best compression level (more compression),
 so the driver here returns '9'
 *********************************************************************************/

static int max_compression(void)
{
	return Z_BEST_COMPRESSION;
}

/*********************************************************************************
 The Driver must provide this function:

 int min_compression(void)

 It must return the minimum compression level that user can
 assign, for instance gzip (in this moment) provides ten values, from zero to
 9 , beeing 0 the worst compression level (no compression at all),
 so the driver here returns '0'
 *********************************************************************************/
static int min_compression(void)
{
	return Z_NO_COMPRESSION;
}
/*********************************************************************************
 The Driver must provide this function:

 int default_compression(void)

 It must return the default compression level, that is, the value to send when
 Compression object methods are called without the optional parameter 'Level'.
 In this gzip wrapper, for example, this value is -1
 *********************************************************************************/
static int default_compression(void)
{
	return Z_DEFAULT_COMPRESSION;
}
/*********************************************************************************
 The Driver must provide this function:

 void c_String(char **target,unsigned long *lent,char *source,unsigned long len,int level)

 It is called to compress a String and return it compressed. The object will
 pass the following values:

 target = NULL
 lent   = 0
 source = a pointer to the original string
 len    = length of the original string
 level  = compression level

 You will never receive a zero lenght string, nor a erroneus 'level' value

 The function must store the compressed string in 'target', and its length in 'lent',
 or NULL and zero if it fails by any reason

 *********************************************************************************/
static void c_String(char **target,unsigned long *lent,char *source,unsigned long len,int level)
{
	*lent=0;
	*target=NULL;

	if (!len) return;

	*lent=len + (len*0.1) + 15;

	GB.Alloc ((void**)target,sizeof(char)*(*lent));

	if (Z_OK != compress2 ((Bytef*)(*target),lent,source,len,level))
	{
		GB.Free((void**)target);
		*lent=0;
		*target=NULL;
		GB.Error("Unable to compress string");
		return;
	}

}
/*********************************************************************************
 The Driver must provide this function:

 void u_String(char **target,unsigned long *lent,char *source,unsigned long len,int level)

 It is called to decompress a String and return it decompressed. The object will
 pass the following values:

 target = NULL
 lent   = 0
 source = a pointer to the original string
 len    = length of the original string
 level  = compression level

  You will never receive a zero lenght string, nor a erroneus 'level' value

 The function must store the decompressed string in 'target', and its length in 'lent',
 or NULL and zero if it fails by any reason
 *********************************************************************************/
static void u_String(char **target,unsigned long *lent,char *source,unsigned long len)
{
	int myok=Z_BUF_ERROR;

	/* we assume src len * 1.8 as target len */
	*lent=1.8*len;
	GB.Alloc ((void**)target,(*lent)*sizeof(char));

	while (myok==Z_BUF_ERROR)
	{
		myok=uncompress ((Bytef*)(*target),lent,source,len);
		switch (myok)
		{
			case Z_OK: break;
			case Z_DATA_ERROR:
				*lent=0;
				if (*target) GB.Free((void**)target);
				GB.Error ("Invalid compressed string");
				return;
			case Z_BUF_ERROR: /* test and error method ! */
				if ((*lent)<=10)
					*lent+=(*lent);
				else
					*lent+=((*lent)*0.5);
				GB.Realloc ((void**)target,(*lent)*sizeof(char));
				break;
			case Z_MEM_ERROR:
				*lent=0;
				if (*target) GB.Free((void**)target);
				GB.Error ("Not enough memory: String too long");
				return;
			default:
				*lent=0;
				GB.Free((void**)target);
				GB.Error ("Unable to inflate string");
				return;
		}
	}

}
/*********************************************************************************
 The Driver must provide this function:

static void c_File(char *source,char *target,int level)

 It is called to compress a file. The object will pass the following values:


 source = path of the file to be compressed
 target = path of the new compressed file to create
 level  = compression level

  You will never receive a erroneus 'level' value
 *********************************************************************************/
static void c_File(char *source,char *target,int level)
{
	FILE *src;
	gzFile dst;
	unsigned long len;
	char buf[GB_Z_BUFFER];
	char bufmode[4]={'w','b',0,0};

	if (level != Z_DEFAULT_COMPRESSION ) bufmode[2]=(char)(level+48);

	if ( (src=fopen(source,"r"))==NULL) {
		GB.Error ("Unable to open file for reading"); return;
	}

	if ( (dst=gzopen(target,bufmode))==NULL) {
		fclose(src);
		GB.Error ("Unable to open file for writing");
		return;
	}

	while (!feof(src))
	{
		len=fread((void*)buf,sizeof(char),GB_Z_BUFFER,src);
		if (len<GB_Z_BUFFER)
		{
			if ( ferror(src) )
			{
				fclose(src);
				gzclose(dst);
				GB.Error("Error while reading data");
				return;
			}
		}
		if (!gzwrite(dst,buf,len))
		{
			fclose(src);
			gzclose(dst);
			GB.Error("Error while writing data");
			return;
		}
		gzflush(dst,Z_SYNC_FLUSH);
	}

	fclose(src);
	gzflush(dst,Z_SYNC_FLUSH);
	gzclose(dst);

}

/*********************************************************************************
 The Driver must provide this function:

static void u_File(char *source,char *target,int level)

 It is called to decompress a file. The object will pass the following values:


 source = path of the file to be decompressed
 target = path of the new decompressed file to create
 level  = compression level

  You will never receive a erroneus 'level' value
 *********************************************************************************/
static void u_File(char *source,char *target)
{
	gzFile src;
	FILE *dst;
	char buf[GB_Z_BUFFER];
	unsigned long len;

	if ( (src=gzopen(source,"rb"))==NULL)
	{
		GB.Error ("Unable to open file for reading");
		return;
	}

	if ( (dst=fopen(target,"w"))==NULL)
	{
		gzclose(src);
		GB.Error ("Unable to open file for writing");
		return;
	}

	while (!gzeof(src))
	{
		len=gzread(src,buf,sizeof(char)*GB_Z_BUFFER);
		if (len==-1)
		{
			fclose(dst);
			gzclose(src);
			GB.Error("Error while reading data");
			return;
		}
		if (len)
		{
			if (len != fwrite((void*)buf,sizeof(char),len,dst) )
			{
				fclose(src);
				gzclose(dst);
				GB.Error("Error while writing data");
				return;
			}
		}
	}

	fclose(dst);
	gzclose(src);
}

static void c_Open(char *path,int level,GB_STREAM *stream)
{

	char mode[4]={'w','b',0,0};

	stream->desc=&ZStream;
	if (level != Z_DEFAULT_COMPRESSION ) mode[2]=(char)(level+48);
	stream->_free[0]=MODE_WRITE;
	stream->_free[1]=(long)gzopen(path,mode);

	if (stream->_free[1]) return;

	stream->desc=NULL;
	if ( errno == Z_MEM_ERROR )
	{
		GB.Error("Not enough memory to manage selected file");
		return;
	}

	GB.Error("Unable to open selected file");


}

static void u_Open(char *path,GB_STREAM *stream)
{
	char mode[3]={'r','b',0};

	stream->desc=&ZStream;
	stream->_free[0]=MODE_READ;
	stream->_free[1]=(long)gzopen(path,mode);

	if (stream->_free[1]) return;

	stream->desc=NULL;
	if ( errno == Z_MEM_ERROR )
	{
		GB.Error("Not enough memory to manage selected file");
		return;
	}

	GB.Error("Unable to open selected file");
}

/*************************************************************************
 Stream related stuff
**************************************************************************/
/* not allowed stream methods */
static int CZ_stream_lof(GB_STREAM *stream, long long *len){return -1;}
static int CZ_stream_seek(GB_STREAM *stream, long long offset, int whence){	return -1;}
static int CZ_stream_open(GB_STREAM *stream, const char *path, int mode, void *data){return -1;}
/* allowed stream methods */
static int CZ_stream_tell(GB_STREAM *stream, long long *npos)
{
	*npos=gztell ((gzFile)stream->_free[1]);
	if ((*npos)!=-1) return 0;
	gzclose ((gzFile)stream->_free[1]);
	stream->desc=NULL;
	return -1;
}

static int CZ_stream_flush(GB_STREAM *stream)
{
	gzflush((gzFile)stream->_free[1],Z_SYNC_FLUSH);
	return 0;
}

int CZ_stream_close(GB_STREAM *stream)
{
	gzclose ((gzFile)stream->_free[1]);
	stream->desc=NULL;
	return 0;
}

static int CZ_stream_write(GB_STREAM *stream, char *buffer, long len)
{
	if (stream->_free[0]==MODE_READ) return -1;
	if ( gzwrite ((gzFile)stream->_free[1], (voidp)buffer, (unsigned)len) == len) return 0;
	gzclose ((gzFile)stream->_free[1]);
	stream->desc=NULL;
	return -1;
}

static int CZ_stream_eof(GB_STREAM *stream)
{
	return gzeof ((gzFile)stream->_free[1]);
}

static int CZ_stream_read(GB_STREAM *stream, char *buffer, long len)
{
	if (stream->_free[0]==MODE_WRITE) return -1;
	if ( gzread ((gzFile)stream->_free[1], (voidp)buffer, (unsigned)len) == len) return 0;
	gzclose ((gzFile)stream->_free[1]);
	stream->desc=NULL;
	return -1;
}




/****************************************************************************
 This array of functions defines what functions the compression component
 must call to perform its actions
 ****************************************************************************/
static COMPRESS_DRIVER _driver =
{
    "zlib",

    (void*)max_compression,
    (void*)min_compression,
    (void*)default_compression,

    {
    	(void*)c_String,
	(void*)c_File,
	(void*)c_Open,
	(void*)CZ_stream_close,
    },

    {
    	(void*)u_String,
	(void*)u_File,
	(void*)u_Open,
	(void*)CZ_stream_close
    }

};
/*****************************************************************************

  The component entry and exit functions.

*****************************************************************************/

int EXPORT GB_INIT(void)
{
  /*************************************************************************
   When this component is loaded by Gambas runtime, it 'informs' to the
   compression component what functions must it call to perform its actions
   *************************************************************************/
  GB.GetInterface("gb.compress", COMPRESS_INTERFACE_VERSION, &COMPRESSION);
  COMPRESSION.Register(&_driver);

  return FALSE;
}

void EXPORT GB_EXIT()
{
}
