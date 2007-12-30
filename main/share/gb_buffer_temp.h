/***************************************************************************

  buffer_t.h

  Simple buffer management

  (c) 2000-2005 Beno�t Minisini <gambas@users.sourceforge.net>

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

#define __BUFFER_C

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "gb_common.h"
#include "gb_error.h"
#include "gb_alloc.h"
#include "gb_buffer.h"


#define DATA_TO_BUFFER(data) ((BUFFER *)((char *)data - sizeof(BUFFER)))
#define BUFFER_TO_DATA(buffer) ((char *)buffer + sizeof(BUFFER))


PUBLIC void BUFFER_create(void *p_data)
{
  BUFFER *buf;

  ALLOC(&buf, sizeof(BUFFER), "BUFFER_create");
  buf->max = 0;
  buf->length = 0;
  
  *((void **)p_data) = BUFFER_TO_DATA(buf);
}


PUBLIC void BUFFER_delete(void *p_data)
{
  void **data = (void **)p_data;
  BUFFER *buf = DATA_TO_BUFFER(*data);

  FREE(&buf, "BUFFER_delete");
  *data = NULL;
}

PUBLIC long BUFFER_length(void *data)
{
  return DATA_TO_BUFFER(data)->length;
}


PUBLIC boolean BUFFER_need(void *p_data, long size)
{
  void **data = (void **)p_data;
  BUFFER *buffer = DATA_TO_BUFFER(*data);

  buffer->length += size;
  if (buffer->length > buffer->max)
  {
    while (buffer->length > buffer->max)
      buffer->max += BUFFER_INC;
      
    REALLOC(&buffer, sizeof(char) * buffer->max + sizeof(BUFFER), "BUFFER_need");
    *data = BUFFER_TO_DATA(buffer);
  }

  return FALSE;
}


PUBLIC long BUFFER_add(void *p_data, const void *string, size_t len)
{
  void **data = (void **)p_data;
  BUFFER *buffer = DATA_TO_BUFFER(*data);
  long pos;

  pos = buffer->length;
  BUFFER_need(p_data, len);

  memcpy(*data + pos, string, len);
  return pos;
}


/*
PUBLIC long BUFFER_add_string(void *p_data, char *string, short len)
{
  long pos;
  
  pos = BUFFER_add(p_data, &len, sizeof(len));
  BUFFER_add(p_data, string, len);

  return pos;
}
*/

PUBLIC boolean BUFFER_load_file(void *p_data, const char *name)
{
  void **data = (void **)p_data;
  BUFFER *buffer = DATA_TO_BUFFER(*data);

  int fd;
  struct stat info;
  long len, lenr;
  BUFFER *new_buffer;
  void *p;
  
  fd = open(name, O_RDONLY);
  if (fd < 0) 
    return TRUE;
  
  fstat(fd, &info);
  
  len = info.st_size;

  ALLOC(&new_buffer, len + sizeof(BUFFER), "BUFFER_load_file");
  
  p = BUFFER_TO_DATA(new_buffer);
  for(;;)
  {
    lenr = read(fd, p, len);
    if (lenr < 0)
    {
      FREE(&new_buffer, "BUFFER_load_file");
      close(fd);
      return TRUE;
    }
    
    if (lenr == len)
      break;
      
    p += lenr;
    len -= lenr;
  }
  
  if (*data)
    FREE(&buffer, "BUFFER_load_file");
    
  new_buffer->length = len;
  new_buffer->max = len;
  *data = BUFFER_TO_DATA(new_buffer);
  
  close(fd);
  
  return FALSE;
}
