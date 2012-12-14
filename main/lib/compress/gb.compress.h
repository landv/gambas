/***************************************************************************

	gb.compress.h

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

#ifndef __GB_COMPRESS_H
#define __GB_COMPRESS_H

#include "gambas.h"

typedef
	struct {
		char *type;
		}
	COMPRESS_DESC;


typedef
	struct {
		char *name;
		int (*max_compression)(void);
		int (*min_compression)(void);
		int (*default_compression)(void);
		
		struct {
			int  (*String) (char **target,unsigned int *lent,char *source,unsigned int len,int level);
			int  (*File)   (char *source,char *target,int level);
			void (*Open)   (char *path,int level,GB_STREAM *stream);
			int  (*Close)  (GB_STREAM *stream);
		} Compress;
		
		struct {
			int  (*String) (char **target,unsigned int *lent,char *source,unsigned int len);
			int  (*File)   (char *source,char *target);
			void (*Open)   (char *path,GB_STREAM *stream);
			int  (*Close)  (GB_STREAM *stream);
		} Uncompress;
	}
	COMPRESS_DRIVER;

typedef
	struct {
		intptr_t version;
		void (*Register)(COMPRESS_DRIVER *);
		}
	COMPRESS_INTERFACE;

#define COMPRESS_INTERFACE_VERSION 1

#endif 
