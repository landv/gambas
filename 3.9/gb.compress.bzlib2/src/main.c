/***************************************************************************

	main.c

	(c) 2003-2004 Daniel Campos Fernández <danielcampos@netcourrier.com>

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

#define __MAIN_C

/* Use 64 bits I/O */
#define _FILE_OFFSET_BITS 64

#include <stdio.h>
#include <stdint.h>
#include <errno.h>
#include <bzlib.h>

#include "main.h"

#define GB_Z_BUFFER 8192
#define MODE_READ 0
#define MODE_WRITE 1
#define P_MODE ((BZ2_STREAM*)stream)->info->mode
#define P_BZ   ((BZ2_STREAM*)stream)->info->bz
#define P_FILE ((BZ2_STREAM*)stream)->info->file
#define P_EOF  ((BZ2_STREAM*)stream)->info->eof
#define P_POS  ((BZ2_STREAM*)stream)->info->pos


typedef 
	struct
	{
		uint8_t mode;
		uint8_t eof;
		BZFILE  *bz;
		FILE    *file;
		int64_t pos;
	} 
	handleInfo; 

typedef 
	struct
	{
		GB_STREAM_BASE base;
		handleInfo *info;
	} 
	BZ2_STREAM;

GB_INTERFACE GB EXPORT;
COMPRESS_INTERFACE COMPRESSION EXPORT;

static COMPRESS_DRIVER _driver;

GB_STREAM_DESC BZStream = 
{
	open: BZ_stream_open,
	close: BZ_stream_close,
	read: BZ_stream_read,
	getchar: BZ_stream_getchar,
	write: BZ_stream_write,
	seek: BZ_stream_seek,
	tell: BZ_stream_tell,
	flush: BZ_stream_flush,
	eof: BZ_stream_eof,
	lof: BZ_stream_lof
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

static int BZ2_max_compression(void)
{
	return 9;
}

/*********************************************************************************
The Driver must provide this function:

int min_compression(void)

It must return the minimum compression level that user can
assign, for instance gzip (in this moment) provides ten values, from zero to
9 , beeing 0 the worst compression level (no compression at all),
so the driver here returns '0'
*********************************************************************************/
static int BZ2_min_compression(void)
{
	return 1;
}
/*********************************************************************************
The Driver must provide this function:

int default_compression(void)

It must return the default compression level, that is, the value to send when
Compression object methods are called without the optional parameter 'Level'.
In this gzip wrapper, for example, this value is -1
*********************************************************************************/
static int BZ2_default_compression(void)
{
	return 6;
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
static void BZ2_c_String(char **target,unsigned long *lent,char *source,unsigned long len,int level)
{
	*target=NULL;
	*lent=len + (len*0.1) + 600;
	GB.Alloc ((void**)target,sizeof(char)*(*lent));

	if (BZ_OK != BZ2_bzBuffToBuffCompress(*target,(unsigned int*)lent,source,(unsigned int)len,level,0,30))
	{
		*lent=0;
		GB.Free((void**)target);
		target=NULL;
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
static void BZ2_u_String(char **target,unsigned long *lent,char *source,unsigned long len)
{
	int myok=BZ_OUTBUFF_FULL;

	/* we assume src len * 1.8 as target len */
	*lent=1.8*len;
	GB.Alloc ((void**)target,(*lent)*sizeof(char));

	while (myok==BZ_OUTBUFF_FULL)
	{
		myok=BZ2_bzBuffToBuffDecompress(*target,(unsigned int*)lent,source,(unsigned int)len,0,0);
		switch (myok)
		{
			case BZ_OK: break;
			case BZ_DATA_ERROR:
			case BZ_DATA_ERROR_MAGIC:
			case BZ_UNEXPECTED_EOF:
				if (*target)GB.Free((void**)target);
				*target=NULL;
				*lent=0;
				GB.Error ("Invalid compressed string");
				return;

			case BZ_OUTBUFF_FULL: /* test and error method ! */
				if ((*lent)<=10)
					(*lent)+=(*lent);
				else
					(*lent)+=((*lent)*0.5);
				GB.Realloc ((void**)target,(*lent)*sizeof(char));
				break;

			case BZ_MEM_ERROR:
				if (*target)GB.Free((void**)target);
				*target=NULL;
				*lent=0;
				GB.Error ("Not enough memory: String too long");
				return;
			default:

				if (*target)GB.Free((void**)target);
				*target=NULL;
				*lent=0;
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
static void BZ2_c_File(char *source,char *target,int level)
{
	FILE *src;
	FILE *f_dst;
	BZFILE *dst;
	char buf[GB_Z_BUFFER];
	long len;
	int bzerror=BZ_OK;

	if ( (src=fopen(source,"rb"))==NULL) {
		GB.Error ("Unable to open file for reading");
		return;
	}

	if ( (f_dst=fopen(target,"wb"))==NULL) {
		fclose(src);
		GB.Error ("Unable to open file for writing");
		return;
	}
	dst=BZ2_bzWriteOpen ( &bzerror,f_dst,level,0,30);

	if (bzerror!=BZ_OK) {
		fclose(src);
		fclose(f_dst);
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
				BZ2_bzWriteClose (&bzerror,dst,0,NULL,NULL);
				fclose(f_dst);
				GB.Error("Error while reading data");
				return;
			}
		}
		BZ2_bzWrite (&bzerror,dst,(void*)buf,len);
		if (bzerror != BZ_OK)
		{
			fclose(src);
			BZ2_bzWriteClose (&bzerror,dst,0,NULL,NULL);
			fclose(f_dst);
			GB.Error("Error while writing data");
			return;
		}
	}

	fclose(src);
	BZ2_bzWriteClose (&bzerror,dst,0,NULL,NULL);
	fclose(f_dst);

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
static void BZ2_u_File(char *source,char *target)
{
	BZFILE *src;
	FILE *f_src;
	FILE *dst;
	char buf[GB_Z_BUFFER];
	long len;
	int bzerror;

	if ( (f_src=fopen(source,"rb"))==NULL)	{
		GB.Error ("Unable to open file for reading");
		return;
	}

	src=BZ2_bzReadOpen(&bzerror,f_src,0,0,NULL,0);

	if (bzerror) {
		fclose (f_src);
		GB.Error ("Unable to open file for reading");
		return;
	}

	if ( (dst=fopen(target,"w"))==NULL) {
		BZ2_bzReadClose (&bzerror,src);
		fclose(f_src);
		GB.Error ("Unable to open file for writing");
		return;
	}

	bzerror=BZ_OK;
	while (bzerror != BZ_STREAM_END)
	{
		len=BZ2_bzRead(&bzerror,src,buf,sizeof(char)*GB_Z_BUFFER);
		if ( (bzerror!=BZ_OK) && (bzerror!=BZ_STREAM_END) )
		{
			BZ2_bzReadClose (&bzerror,src);
			fclose(f_src);
			fclose(dst);
			GB.Error("Error while reading data");
			return;
		}
		if (len)
		{
			if (len != fwrite((void*)buf,sizeof(char),len,dst) )
			{
				BZ2_bzReadClose (&bzerror,src);
				fclose(f_src);
				fclose(dst);
				GB.Error("Error while writing data");
				return;
			}
		}
	}

	BZ2_bzReadClose (&bzerror,src);
	fclose(f_src);
	fclose(dst);
}

static void BZ2_c_Open(char *path,int level,BZ2_STREAM *stream)
{
	int bzerror;

	GB.Alloc(POINTER(&stream->info), sizeof(handleInfo) );

	P_FILE=fopen(path,"wb");
	if (!P_FILE)
	{
		GB.Free(POINTER(&stream->info));
		GB.Error("Unable to open file");
		return;
	}

	P_MODE=MODE_WRITE;
	P_BZ=BZ2_bzWriteOpen(&bzerror,P_FILE,level,0,30);


	if (bzerror)
	{
		fclose(P_FILE);
		GB.Free( POINTER(&((BZ2_STREAM*)stream)->info) );
		GB.Error("Unable to open file");
		return;
	}

	P_EOF=0;
	P_POS=0;
	stream->base.desc = &BZStream;
}

static void BZ2_u_Open(char *path,GB_STREAM *stream)
{
	int bzerror;

	GB.Alloc( POINTER(&((BZ2_STREAM*)stream)->info),sizeof(handleInfo) );

	P_FILE=fopen(path,"rb");
	if (!P_FILE)
	{
		GB.Free( POINTER(&((BZ2_STREAM*)stream)->info) );
		GB.Error("Unable to open file");
		return;
	}

	P_MODE=MODE_WRITE;
	P_BZ=BZ2_bzReadOpen (&bzerror,P_FILE,0,0,NULL,0);

	if (bzerror)
	{
		GB.Free( POINTER(&((BZ2_STREAM*)stream)->info) );
		fclose(P_FILE);
		GB.Error("Unable to open file");
		return;
	}

	P_EOF=0;
	P_POS=0;
	stream->desc=&BZStream;
}

/*************************************************************************
Stream related stuff
**************************************************************************/
/* not allowed stream methods */
static int BZ_stream_lof(GB_STREAM *stream, int64_t *len){return -1;}
static int BZ_stream_seek(GB_STREAM *stream, int64_t offset, int whence){	return -1;}
static int BZ_stream_open(GB_STREAM *stream, const char *path, int mode, void *data){return -1;}
/* allowed stream methods */
static int BZ_stream_tell(GB_STREAM *stream, int64_t *npos)
{
	*npos=P_POS;
	return 0;
}

static int BZ_stream_flush(GB_STREAM *stream)
{
	return 0;
}

static int BZ_stream_close(GB_STREAM *stream)
{
	int bzerror;

	if (P_MODE == MODE_WRITE)
		BZ2_bzWriteClose(&bzerror,P_BZ,0,NULL,NULL);
	else
		BZ2_bzReadClose(&bzerror,P_BZ);

	fclose(P_FILE);

	GB.Free( POINTER(&((BZ2_STREAM*)stream)->info) );
	stream->desc=NULL;
	return 0;
}

static int BZ_stream_write(GB_STREAM *stream, char *buffer, int len)
{
	int bzerror;

	if (P_MODE==MODE_READ) return -1;
	BZ2_bzWrite ( &bzerror,P_BZ, (void*)buffer, len);
	if (!bzerror) { P_POS+=len; return 0; }

	//BZ2_bzWriteClose (&bzerror,P_BZ,0,NULL,NULL);
	//fclose(P_FILE);

	//GB.Free( POINTER(&((BZ2_STREAM*)stream)->info) );
	//stream->desc=NULL;
	return -1;
}

static int BZ_stream_eof(GB_STREAM *stream)
{
	return P_EOF;
}

static int BZ_stream_read(GB_STREAM *stream, char *buffer, int len)
{
	int bzerror;
	int len2;

	if (P_MODE == MODE_WRITE) 
		return -1;

	len2 = BZ2_bzRead (&bzerror, P_BZ, (void*)buffer, len);
	
	if (!bzerror)
	{
		GB.Stream.SetBytesRead(stream, len2);
		P_POS += len2;
		return 0;
	}
	else
	{
		if ((len2 == len) && (bzerror == BZ_STREAM_END))
		{
			GB.Stream.SetBytesRead(stream, len2);
			P_POS += len2;
			P_EOF = 1;
			return 0;
		}
	}

	//BZ2_bzReadClose (&bzerror,P_BZ);
	//fclose(P_FILE);
	//GB.Free( POINTER(&((BZ2_STREAM*)stream)->info) );
	//stream->desc=NULL;
	return -1;
}

static int BZ_stream_getchar(GB_STREAM *stream, char *buffer)
{
	return BZ_stream_read(stream,buffer,1);
}


/****************************************************************************
This array of functions defines what functions the compression component
must call to perform its actions
****************************************************************************/
static COMPRESS_DRIVER _driver =
{
		"bzlib2",

		(void*)BZ2_max_compression,
		(void*)BZ2_min_compression,
		(void*)BZ2_default_compression,

		{
			(void*)BZ2_c_String,
	(void*)BZ2_c_File,
	(void*)BZ2_c_Open,
	(void*)BZ_stream_close,
		},

		{
			(void*)BZ2_u_String,
	(void*)BZ2_u_File,
	(void*)BZ2_u_Open,
	(void*)BZ_stream_close
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

	return 0;
}

void EXPORT GB_EXIT()
{
}

