/***************************************************************************

  buffer.h

  Simple buffer management

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

#ifndef __GB_BUFFER_H
#define __GB_BUFFER_H

typedef
  struct {
    size_t length;
    size_t max;
    }
  BUFFER;


#define BUFFER_INC 256

PUBLIC void BUFFER_create(void *p_data);
PUBLIC void BUFFER_delete(void *p_data);
PUBLIC boolean BUFFER_load_file(void *p_data, const char *name);
PUBLIC offset_t BUFFER_add(void *p_data, const void *string, size_t len);
PUBLIC bool BUFFER_need(void *p_data, size_t size);

#define DATA_TO_BUFFER(_data) ((BUFFER *)((char *)(_data) - sizeof(BUFFER)))
#define BUFFER_TO_DATA(_buffer) ((char *)(_buffer) + sizeof(BUFFER))

#define BUFFER_length(_data) (DATA_TO_BUFFER(_data)->length)

#endif
