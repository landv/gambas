/***************************************************************************

  c_media.h

  gb.media component

  (c) 2012 Benoît Minisini <gambas@users.sourceforge.net>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 1, or (at your option)
  any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
  MA 02110-1301, USA.

***************************************************************************/

#ifndef __C_MEDIA_H
#define __C_MEDIA_H

#include "main.h"

#ifndef __C_MEDIA_C

//extern GB_DESC MediaSignalArgumentsDesc[];
extern GB_DESC MediaControlDesc[];
extern GB_DESC MediaContainerDesc[];
extern GB_DESC MediaPipelineDesc[];
extern GB_DESC MediaDesc[];

#else

#define THIS ((CMEDIACONTROL *)_object)
#define THIS_ARG ((CMEDIASIGNALARGUMENTS *)_object)
#define ELEMENT THIS->elt
#define PIPELINE ((GstPipeline *)THIS->elt)

#endif

typedef
	struct {
		GB_BASE ob;
		GstElement *elt;
		char *type;
		void *dest;
		GB_TIMER *watch;
		GB_VARIANT_VALUE tag;
		unsigned state : 3;
		unsigned error : 1;
	}
	CMEDIACONTROL;

typedef
	CMEDIACONTROL CMEDIACONTAINER;

typedef
	CMEDIACONTROL CMEDIAPIPELINE;

#if 0
typedef
	struct {
		GB_BASE ob;
		guint n_param_values;
		const GValue *param_values;
	}
	CMEDIASIGNALARGUMENTS;
#endif
	
#define TO_SECOND(_time) ((double)((_time) / 1000) / 1E6)
#define TO_TIME(_second) ((gint64)((_second) * 1E9))
	
void MEDIA_raise_event(void *_object, int event);
CMEDIACONTROL *MEDIA_get_control_from_element(void *element);
bool MEDIA_set_state(void *_object, int state, bool error);

#endif /* __C_MEDIA_H */
