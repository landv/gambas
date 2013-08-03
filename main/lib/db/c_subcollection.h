/***************************************************************************

	c_subcollection.h

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

#ifndef __C_SUBCOLLECTION_H
#define __C_SUBCOLLECTION_H

#include "gambas.h"

/* SubCollection description */

typedef
	struct {
		char *klass;
		void *(*get)(void *, const char *);
		int (*exist)(void *, const char *);
		void (*list)(void *, char ***);
		void (*release)(void *, void *);
	}
	SUBCOLLECTION_DESC;

typedef
	struct {
		GB_BASE object;
		GB_HASHTABLE hash_table;
		int mode;
		void *container;
		SUBCOLLECTION_DESC *desc;
		char **list;
		}
	CSUBCOLLECTION;

#ifndef __C_SUBCOLLECTION_C

extern GB_DESC SubCollectionDesc[];

#else

#define THIS ((CSUBCOLLECTION *)_object)

#endif

void GB_SubCollectionNew(CSUBCOLLECTION **subcollection, SUBCOLLECTION_DESC *desc, void *container);
void *GB_SubCollectionContainer(void *_object);
void GB_SubCollectionAdd(void *_object, const char *key, int len, void *value);
void GB_SubCollectionRemove(void *_object, const char *key, int len);
void *GB_SubCollectionGet(void *_object, const char *key, int len);

#endif
