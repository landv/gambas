/***************************************************************************

  CMessage.cpp

  (c) 2004-2006 - Daniel Campos Fernández <dcamposf@gmail.com>

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

#define __CMESSAGE_CPP

#include "CMessage.h"

static int _global_lock = 0;

typedef
	struct { 
		GB_STRING msg; 
		GB_STRING btn1; 
		GB_STRING btn2; 
		GB_STRING btn3;
	}
	MSG_PARAM;

static void show_message_box(int type, MSG_PARAM *_p)
{
	char *msg = GB.ToZeroString(ARG(msg));
	char *btn1, *btn2, *btn3;
	int ret;
	char *title;
	
	btn1 = MISSING(btn1) ? NULL : GB.ToZeroString(ARG(btn1));
	btn2 = (type == 0 || MISSING(btn2)) ? NULL : GB.ToZeroString(ARG(btn2));
	btn3 = (type == 0 || MISSING(btn3)) ? NULL : GB.ToZeroString(ARG(btn3));
	
  if (_global_lock)
  {
  	GB.Error("Message box already displayed");
  	return;
  }
	
  _global_lock++;
	
	title = gMessage::title();
	if (!title)
		title = GB.Application.Title();
	
	switch (type)
	{
		case 0: ret = gMessage::showInfo(msg, btn1); break;
		case 1: ret = gMessage::showWarning(msg, btn1, btn2, btn3); break;
		case 2: ret = gMessage::showQuestion(msg, btn1, btn2, btn3); break;
		case 3: ret = gMessage::showError(msg, btn1, btn2, btn3); break;
		case 4: ret = gMessage::showDelete(msg, btn1, btn2, btn3); break;
		default: ret = 0;
	}
  	
	gMessage::setTitle(NULL);
  
  GB.ReturnInteger(ret);
  
  _global_lock--;
}

BEGIN_METHOD(CMESSAGE_info, GB_STRING msg; GB_STRING btn)

	show_message_box(0, (MSG_PARAM *)_p);
	
END_METHOD


BEGIN_METHOD(CMESSAGE_warning, GB_STRING msg; GB_STRING btn1; GB_STRING btn2; GB_STRING btn3)

	show_message_box(1, (MSG_PARAM *)_p);

END_METHOD


BEGIN_METHOD(CMESSAGE_question, GB_STRING msg; GB_STRING btn1; GB_STRING btn2; GB_STRING btn3)

	show_message_box(2, (MSG_PARAM *)_p);

END_METHOD


BEGIN_METHOD(CMESSAGE_error, GB_STRING msg; GB_STRING btn1; GB_STRING btn2; GB_STRING btn3)
	
	show_message_box(3, (MSG_PARAM *)_p);
	 
END_METHOD


BEGIN_METHOD(CMESSAGE_delete, GB_STRING msg; GB_STRING btn1; GB_STRING btn2; GB_STRING btn3)
	
	show_message_box(4, (MSG_PARAM *)_p);

END_METHOD


BEGIN_PROPERTY(CMESSAGE_title)

	if (READ_PROPERTY)
		GB.ReturnNewZeroString(gMessage::title());
	else
		gMessage::setTitle(GB.ToZeroString(PROP(GB_STRING)));

END_PROPERTY

GB_DESC CMessageDesc[] =
{
  GB_DECLARE("Message", 0), GB_VIRTUAL_CLASS(),

  GB_STATIC_METHOD("_call", "i", CMESSAGE_info, "(Message)s[(Button)s]"),
  GB_STATIC_METHOD("Info", "i", CMESSAGE_info, "(Message)s[(Button)s]"),
  GB_STATIC_METHOD("Warning", "i", CMESSAGE_warning,  "(Message)s[(Button1)s(Button2)s(Button3)s]"),
  GB_STATIC_METHOD("Question", "i", CMESSAGE_question, "(Message)s[(Button1)s(Button2)s(Button3)s]"),
  GB_STATIC_METHOD("Error", "i", CMESSAGE_error, "(Message)s[(Button1)s(Button2)s(Button3)s]"),
  GB_STATIC_METHOD("Delete", "i", CMESSAGE_delete, "(Message)s[(Button1)s(Button2)s(Button3)s]"),
  GB_STATIC_PROPERTY("Title", "s", CMESSAGE_title),

  GB_END_DECLARE
};
