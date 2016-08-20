/***************************************************************************

  gb_buffer.h

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

#ifndef __GB_BUFFER_H
#define __GB_BUFFER_H

typedef
  struct {
    size_t length;
    size_t max;
    }
  BUFFER;


#define BUFFER_INC 256

void BUFFER_create(void *p_data);
void BUFFER_delete(void *p_data);
bool BUFFER_load_file(void *p_data, const char *name);
offset_t BUFFER_add(void *p_data, const void *string, int len);
bool BUFFER_need(void *p_data, size_t size);
void BUFFER_add_char(void *p_data, char c);

#define DATA_TO_BUFFER(_data) ((BUFFER *)(_data) - 1)
#define BUFFER_TO_DATA(_buffer) ((char *)((BUFFER *)(_buffer) + 1))

#define BUFFER_length(_data) (DATA_TO_BUFFER(_data)->length)

#endif
