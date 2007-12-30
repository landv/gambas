/***************************************************************************

  CMessage.cpp

  (c) 2004-2006 - Daniel Campos Fernández <dcamposf@gmail.com>
  
  GTK+ component
  
  Realizado para la Junta de Extremadura. 
  Consejería de Educación Ciencia y Tecnología. 
  Proyecto gnuLinEx
  
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
#define __CMESSAGE_CPP

#include "CMessage.h"

#define MSG_FIELDS "(Message)s[(Button1)s(Button2)s(Button3)s]"

static int _global_lock = 0;

#define LOCK_MSG_BOX(_code) \
  if (_global_lock) \
  { \
  	GB.Error("Message box already displayed"); \
  	return; \
  } \
  _global_lock++; \
  GB.ReturnInteger(_code); \
  _global_lock--; \

BEGIN_METHOD(CMESSAGE_info, GB_STRING msg; GB_STRING btn)

	char *b=NULL;
	if (!MISSING(btn)) b=STRING(btn);
	
	LOCK_MSG_BOX(gMessage::showInfo(STRING(msg),b));
	
END_METHOD


BEGIN_METHOD(CMESSAGE_warning, GB_STRING msg; GB_STRING btn1; GB_STRING btn2; GB_STRING btn3)

	char *b1=NULL,*b2=NULL,*b3=NULL;
	if (!MISSING(btn1)) b1=STRING(btn1);
	if (!MISSING(btn2)) b2=STRING(btn2);
	if (!MISSING(btn3)) b3=STRING(btn3);
	LOCK_MSG_BOX(gMessage::showWarning(STRING(msg),b1,b2,b3));

END_METHOD


BEGIN_METHOD(CMESSAGE_question, GB_STRING msg; GB_STRING btn1; GB_STRING btn2; GB_STRING btn3)

	char *b1=NULL,*b2=NULL,*b3=NULL;
	if (!MISSING(btn1)) b1=STRING(btn1);
	if (!MISSING(btn2)) b2=STRING(btn2);
	if (!MISSING(btn3)) b3=STRING(btn3);
	LOCK_MSG_BOX(gMessage::showQuestion(STRING(msg),b1,b2,b3));

END_METHOD


BEGIN_METHOD(CMESSAGE_error, GB_STRING msg; GB_STRING btn1; GB_STRING btn2; GB_STRING btn3)
	
	char *b1=NULL,*b2=NULL,*b3=NULL;
	if (!MISSING(btn1)) b1=STRING(btn1);
	if (!MISSING(btn2)) b2=STRING(btn2);
	if (!MISSING(btn3)) b3=STRING(btn3);
	LOCK_MSG_BOX(gMessage::showError(STRING(msg),b1,b2,b3));
	 
END_METHOD

BEGIN_METHOD(CMESSAGE_delete, GB_STRING msg; GB_STRING btn1; GB_STRING btn2; GB_STRING btn3)
	
	char *b1=NULL,*b2=NULL,*b3=NULL;
	if (!MISSING(btn1)) b1=STRING(btn1);
	if (!MISSING(btn2)) b2=STRING(btn2);
	if (!MISSING(btn3)) b3=STRING(btn3);
	LOCK_MSG_BOX(gMessage::showDelete(STRING(msg),b1,b2,b3));

END_METHOD



GB_DESC CMessageDesc[] =
{
  GB_DECLARE("Message", 0), GB_VIRTUAL_CLASS(),


  GB_STATIC_METHOD("_call", "i", CMESSAGE_info, "(Message)s[(Button)s]"),
  GB_STATIC_METHOD("Info", "i", CMESSAGE_info, "(Message)s[(Button)s]"),
  GB_STATIC_METHOD("Warning", "i", CMESSAGE_warning, MSG_FIELDS),
  GB_STATIC_METHOD("Question", "i", CMESSAGE_question,MSG_FIELDS),
  GB_STATIC_METHOD("Error", "i", CMESSAGE_error,MSG_FIELDS ),
  GB_STATIC_METHOD("Delete", "i", CMESSAGE_delete,MSG_FIELDS),

  GB_END_DECLARE
};
