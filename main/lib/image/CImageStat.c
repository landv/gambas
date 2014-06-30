/***************************************************************************

  CImageStat.c

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

#define __CIMAGESTAT_C

#include <sys/mman.h>
#include "main.h"
#include "image_stat.h"
#include "CImageStat.h"

// static int my_mmap(const char *path, char **paddr, int *plen)
// {
//   struct stat info;
// 	int fd;
// 	void *addr;
// 	size_t len;
// 	  
// 	fd = open(path, O_RDONLY);
//   if (fd < 0)
//   	return -1;
// 
//   if (fstat(fd, &info) < 0)
//     return -1;
// 
//   len = info.st_size;
//   addr = mmap(NULL, len, PROT_READ, MAP_PRIVATE, fd, 0);
//   if (addr == MAP_FAILED)
//     return -1;
// 	
// 	*paddr = addr;
// 	*plen = len;
// 	return fd;
// }

BEGIN_METHOD(CIMAGESTAT_call, GB_STRING path)

	char *path = GB.FileName(STRING(path), LENGTH(path));
	IMAGE_STREAM stream;
	IMAGE_INFO info = {0};
	CIMAGESTAT *stat;
	
	if (GB.LoadFile(path, strlen(path), &stream.addr, &stream.len))
		return;
	
	stream.pos = 0;
	
	if (IMAGE_get_info(&stream, &info))
	{
		GB.Error("Unable to stat image: &1", IMAGE_error);
		stat = NULL;
	}
	else
	{
		stat = GB.New(GB.FindClass("ImageStat"), NULL, NULL);
		stat->path = GB.NewZeroString(path);
		stat->type = info.type;
		stat->width = info.width;
		stat->height = info.height;
		stat->depth = info.depth;
	}
	
	GB.ReleaseFile(stream.addr, stream.len);
	
	GB.ReturnObject(stat);
	
END_METHOD

BEGIN_METHOD_VOID(CIMAGESTAT_free)

	GB.FreeString(&THIS->path);

END_METHOD

BEGIN_PROPERTY(CIMAGESTAT_path)

	GB.ReturnString(THIS->path);

END_PROPERTY

BEGIN_PROPERTY(CIMAGESTAT_type)

	GB.ReturnConstZeroString(THIS->type);

END_PROPERTY

BEGIN_PROPERTY(CIMAGESTAT_width)

	GB.ReturnInteger(THIS->width);

END_PROPERTY

BEGIN_PROPERTY(CIMAGESTAT_height)

	GB.ReturnInteger(THIS->height);

END_PROPERTY

BEGIN_PROPERTY(CIMAGESTAT_depth)

	GB.ReturnInteger(THIS->depth);

END_PROPERTY

GB_DESC CImageStatDesc[] =
{
  GB_DECLARE("ImageStat", sizeof(CIMAGESTAT)),
  GB_NOT_CREATABLE(),

	GB_STATIC_METHOD("_call", "ImageStat", CIMAGESTAT_call, "(Path)s"),
	GB_METHOD("_free", NULL, CIMAGESTAT_free, NULL),
	
  GB_PROPERTY_READ("Path", "s", CIMAGESTAT_path),
  GB_PROPERTY_READ("Type", "s", CIMAGESTAT_type),
  GB_PROPERTY_READ("Width", "i", CIMAGESTAT_width),
  GB_PROPERTY_READ("Height", "i", CIMAGESTAT_height),
  GB_PROPERTY_READ("Depth", "i", CIMAGESTAT_depth),
  
  GB_END_DECLARE
};
	
