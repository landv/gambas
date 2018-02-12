/***************************************************************************

	gb_buffer.c

	(c) 2000-2017 Benoît Minisini <g4mba5@gmail.com>

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

#define __BUFFER_C

#include "main.h"
#include "gb_buffer.h"

#define ALLOC(a, b) GB.Alloc(POINTER(a), b)
#define FREE(a) GB.Free(POINTER(a))
#define REALLOC(a, b) GB.Realloc(POINTER(a), b)

void BUFFER_create(void *p_data)
{
	BUFFER *buf;

	ALLOC(&buf, sizeof(BUFFER));
	buf->max = 0;
	buf->length = 0;
	
	*((void **)p_data) = BUFFER_TO_DATA(buf);
}


void BUFFER_delete(void *p_data)
{
	void **data = (void **)p_data;
	BUFFER *buf = DATA_TO_BUFFER(*data);

	FREE(&buf);
	*data = NULL;
}


bool BUFFER_need(void *p_data, size_t size)
{
	void **data = (void **)p_data;
	BUFFER *buffer = DATA_TO_BUFFER(*data);

	buffer->length += size;
	//fprintf(stderr, "BUFFER_need: %ld (%ld / %ld)\n", size, buffer->length, buffer->max);
	
	if (buffer->length > buffer->max)
	{
		while (buffer->length >= buffer->max)
			buffer->max += BUFFER_INC;
			
		REALLOC(&buffer, sizeof(char) * buffer->max + sizeof(BUFFER));
		*data = BUFFER_TO_DATA(buffer);
	}

	return FALSE;
}


offset_t BUFFER_add(void *p_data, const void *string, int len)
{
	void **data = (void **)p_data;
	BUFFER *buffer = DATA_TO_BUFFER(*data);
	size_t pos;

	if (len < 0)
		len = strlen((const char *)string);
	
	pos = buffer->length;
	BUFFER_need(p_data, len);

	memcpy(*data + pos, string, len);

	//fprintf(stderr, ">> BUFFER_add\n");
	
	return pos;
}

void BUFFER_add_char(void *p_data, char c)
{
	void **data = (void **)p_data;
	BUFFER *buffer = DATA_TO_BUFFER(*data);
	size_t pos;

	pos = buffer->length;
	BUFFER_need(p_data, 1);
	*(char *)(*data + pos) = c;
}
