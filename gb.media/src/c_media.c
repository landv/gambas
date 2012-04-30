/***************************************************************************

  c_media.c

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

#define __C_MEDIA_C

#include "c_media.h"

//---- MediaControl -------------------------------------------------------

typedef
	struct {
		char *klass;
		char *type;
	}
	MEDIA_TYPE;

static MEDIA_TYPE _types[] =
{
	{ "MediaContainer", "bin" },
	{ "MediaPipeline", "pipeline" },
	{ "Media", "pipeline" },
	{ "MediaPlayer", "playerbin2" },
	{ "MediaDecoder", "decoderbin2" },
	{ NULL, NULL }
};

DECLARE_EVENT(EVENT_Ready);

/*static void cb_realized(GstElement *element, CMEDIACONTROL *_object)
{
	fprintf(stderr, "cb_realized: %s\n", gst_element_factory_get_klass(gst_element_get_factory(ELEMENT)));
	
	gst_element_post_message(element, gst_message_new_application(GST_OBJECT(element), gst_structure_new("ReadyEvent", "control", G_TYPE_POINTER, THIS, NULL)));
}*/

static void cb_pad_added(GstElement *element, GstPad *pad, CMEDIACONTROL *_object)
{
	char *name;
	//GstPad *sinkpad;
	
	fprintf(stderr, "cb_pad_added: %s\n", gst_element_factory_get_klass(gst_element_get_factory(ELEMENT)));

	if (!THIS->dest)
		return;
	
	/* We can now link this pad with the vorbis-decoder sink pad */
	//sinkpad = gst_element_get_static_pad (decoder, "sink");
	//gst_pad_link (pad, sinkpad);
	
	name = gst_pad_get_name(pad);
	gst_element_link_pads(ELEMENT, name, ((CMEDIACONTROL *)THIS->dest)->elt, NULL);
	g_free(name);
	
	//GB.Unref(POINTER(&THIS->dest));
	//gst_object_unref (sinkpad);
}

BEGIN_METHOD(MediaControl_new, GB_STRING type; GB_OBJECT parent)

	char *type;
	CMEDIACONTAINER *parent;
	MEDIA_TYPE *mtp;
	GB_CLASS klass;
	
	if (MISSING(type))
	{
		klass = GB.GetClass(THIS);
		type = NULL;
		
		for (mtp = _types; mtp->klass; mtp++)
		{
			if (GB.FindClass(mtp->klass) == klass)
			{
				type = mtp->type;
				break;
			}
		}
		
		if (!type)
		{
			GB.Error("The type must be specified");
			return;
		}
	}
	else
		type = GB.ToZeroString(ARG(type));
	
	ELEMENT = gst_element_factory_make(type, NULL);
	if (!ELEMENT)
	{
		GB.Error("Unable to create media control");
		return;
	}
	
	gst_object_ref(GST_OBJECT(ELEMENT));
	
	parent = VARGOPT(parent, NULL);
	if (parent)
	{
		gst_bin_add(GST_BIN(parent->elt), ELEMENT);
		gst_element_sync_state_with_parent(ELEMENT);
	}
	else if (!GST_IS_PIPELINE(ELEMENT))
		GB.CheckObject(parent);
		
END_METHOD

BEGIN_METHOD_VOID(MediaControl_free)

	gst_element_set_state(ELEMENT, GST_STATE_NULL);

	GB.Unref(POINTER(&THIS->dest));
	
	if (ELEMENT)
		gst_object_unref(GST_OBJECT(ELEMENT));

END_METHOD

BEGIN_PROPERTY(MediaControl_Type)

	GB.ReturnNewZeroString(gst_element_factory_get_klass(gst_element_get_factory(ELEMENT)));
	
END_PROPERTY

BEGIN_PROPERTY(MediaControl_Name)

	if (READ_PROPERTY)
		GB.ReturnNewZeroString(gst_element_get_name(ELEMENT));
	else
		gst_element_set_name(ELEMENT, GB.ToZeroString(PROP(GB_STRING)));

END_PROPERTY

BEGIN_PROPERTY(MediaControl_State)

	GstStateChangeReturn status;
	
	if (READ_PROPERTY)
	{
		GstState state;
		
		status = gst_element_get_state(ELEMENT, &state, NULL, GST_SECOND * 3);
		
		if (status != GST_STATE_CHANGE_SUCCESS)
			GB.ReturnInteger(-1);
		else
			GB.ReturnInteger(state);
	}
	else
	{
		status = gst_element_set_state(ELEMENT, VPROP(GB_INTEGER));
		
		if (status == GST_STATE_CHANGE_ASYNC)
			status = gst_element_get_state(ELEMENT, NULL, NULL, GST_SECOND * 3);
		
		if (status == GST_STATE_CHANGE_FAILURE)
		{
			GB.Error("Cannot set status");
			return;
		}
	}

END_PROPERTY

static GParamSpec *get_property(GstElement *element, char *property)
{
	GParamSpec *desc;
	
	desc = g_object_class_find_property(G_OBJECT_GET_CLASS(G_OBJECT(element)), property);
	if (!desc)
		GB.Error("Unknown property: '&1'", property);
	
	return desc;
}

BEGIN_METHOD(MediaControl_get, GB_STRING property)

	char *property = GB.ToZeroString(ARG(property));
	GParamSpec *desc;
	GValue value = G_VALUE_INIT;
	
	desc = get_property(ELEMENT, property);
	if (!desc)
		return;
	
	//fprintf(stderr, "type = %s\n", g_type_name(desc->value_type));
	g_value_init(&value, desc->value_type);
	g_object_get_property(G_OBJECT(ELEMENT), property, &value);

	switch (desc->value_type)
	{
		case G_TYPE_INT: GB.ReturnInteger(g_value_get_int(&value)); break;
		case G_TYPE_UINT: GB.ReturnInteger(g_value_get_uint(&value)); break;
		case G_TYPE_UINT64: GB.ReturnLong(g_value_get_uint64(&value)); break;
		case G_TYPE_STRING: GB.ReturnNewZeroString(g_value_get_string(&value)); break;
		case G_TYPE_FLOAT: GB.ReturnFloat(g_value_get_float(&value)); break;
		case G_TYPE_DOUBLE: GB.ReturnFloat(g_value_get_double(&value)); break;
		default: GB.Error("Unsupported property datatype"); GB.ReturnNull(); break;
	}
	
	g_value_unset(&value);
	
	GB.ReturnConvVariant();
	
END_METHOD

BEGIN_METHOD(MediaControl_put, GB_VARIANT value; GB_STRING property)

	char *property = GB.ToZeroString(ARG(property));
	GParamSpec *desc;
	GValue value = G_VALUE_INIT;
	GB_VALUE *v = (GB_VALUE *)ARG(value);
	
	desc = get_property(ELEMENT, property);
	if (!desc)
		return;
	
	g_value_init(&value, desc->value_type);

	switch (desc->value_type)
	{
		case G_TYPE_INT:
			if (GB.Conv(v, GB_T_INTEGER))
				return;
			g_value_set_int(&value, v->_integer.value);
			break;
			
		case G_TYPE_UINT:
			if (GB.Conv(v, GB_T_INTEGER))
				return;
			g_value_set_uint(&value, (uint)v->_integer.value);
			break;
			
		case G_TYPE_UINT64:
			if (GB.Conv(v, GB_T_LONG))
				return;
			g_value_set_uint64(&value, (guint64)v->_long.value);
			break;
			
		case G_TYPE_STRING:
			if (GB.Conv(v, GB_T_STRING))
				return;
			g_value_set_string(&value, GB.ToZeroString((GB_STRING *)v));
			break;
			
		case G_TYPE_FLOAT:
			if (GB.Conv(v, GB_T_FLOAT))
				return;
			g_value_set_float(&value, v->_float.value);
			break;
			
		case G_TYPE_DOUBLE:
			if (GB.Conv(v, GB_T_FLOAT))
				return;
			g_value_set_double(&value, v->_float.value);
			break;
			
		default:
			GB.Error("Unsupported property datatype");
			return;
	}
	
	g_object_set_property(G_OBJECT(ELEMENT), property, &value);
	g_value_unset(&value);
	
END_METHOD

BEGIN_METHOD(MediaControl_LinkTo, GB_OBJECT dest; GB_BOOLEAN later)

	CMEDIACONTROL *dest = (CMEDIACONTROL *)VARG(dest);
	
	if (GB.CheckObject(dest))
		return;

	if (VARGOPT(later, FALSE))
	{
		GB.Unref(POINTER(&THIS->dest));
		GB.Ref(dest);
		THIS->dest = dest;
		g_signal_connect(ELEMENT, "pad-added", G_CALLBACK(cb_pad_added), THIS);
	}
	else
	{
		gst_element_link(ELEMENT, dest->elt);
	}

END_METHOD

//---- MediaContainer -----------------------------------------------------

//---- MediaPipeline ------------------------------------------------------

DECLARE_EVENT(EVENT_End);
DECLARE_EVENT(EVENT_Message);
DECLARE_EVENT(EVENT_Tag);
DECLARE_EVENT(EVENT_Buffering);
DECLARE_EVENT(EVENT_State);
DECLARE_EVENT(EVENT_Duration);
DECLARE_EVENT(EVENT_Progress);

static int cb_media(CMEDIAPIPELINE *_object)
{
	GstMessage *msg;
	GstMessageType type;
	int msg_type;
	GstBus *bus;
	bool raise_state = FALSE;
	
	bus = gst_pipeline_get_bus(PIPELINE);
	
	while((msg = gst_bus_pop(bus)) != NULL) 
	{
		type = GST_MESSAGE_TYPE(msg);
		
		if (type == GST_MESSAGE_APPLICATION)
		{
			CMEDIACONTROL *src = (CMEDIACONTROL *) g_value_get_pointer(gst_structure_get_value(gst_message_get_structure(msg), "control"));
			GB.Raise(src, EVENT_Ready, 0);
		}
		else if (GST_MESSAGE_SRC(msg) == GST_OBJECT(PIPELINE))
		{
		
			switch (type)
			{
				case GST_MESSAGE_EOS: 
					GB.Raise(THIS, EVENT_End, 0); 
					break;
				
				case GST_MESSAGE_ERROR: 
				case GST_MESSAGE_WARNING: 
				case GST_MESSAGE_INFO: 
				{
					gchar *debug;
					GError *error;
					
					if (type == GST_MESSAGE_ERROR)
					{
						gst_message_parse_error(msg, &error, &debug);
						msg_type = 2;
					}
					else if (type == GST_MESSAGE_WARNING)
					{
						gst_message_parse_warning(msg, &error, &debug);
						msg_type = 1;
					}
					else
					{
						gst_message_parse_info(msg, &error, &debug);
						msg_type = 0;
					}
					
					g_free(debug);
					
					GB.Raise(THIS, EVENT_Message, 2, GB_T_INTEGER, msg_type, GB_T_STRING, error->message, -1);
					
					g_error_free(error);
					break;
				}
				
				case GST_MESSAGE_TAG: GB.Raise(THIS, EVENT_Tag, 0); break;
				case GST_MESSAGE_BUFFERING: GB.Raise(THIS, EVENT_Buffering, 0); break;
				
				case GST_MESSAGE_STATE_CHANGED:
					raise_state = TRUE;
					break;
				
				case GST_MESSAGE_DURATION: GB.Raise(THIS, EVENT_Duration, 0); break;
				case GST_MESSAGE_PROGRESS: GB.Raise(THIS, EVENT_Progress, 0); break;
				default: break;
			}
		}
		
		gst_message_unref(msg);
	}
	
	gst_object_unref(bus);
	
	if (raise_state)
		GB.Raise(THIS, EVENT_State, 0);
	
	return FALSE;
}

BEGIN_METHOD_VOID(MediaPipeline_new)
	
	THIS->watch = GB.Every(250, (GB_TIMER_CALLBACK)cb_media, (intptr_t)THIS);

END_METHOD

BEGIN_METHOD_VOID(MediaPipeline_free)

	GB.Unref(POINTER(&THIS->watch));

END_METHOD

BEGIN_METHOD_VOID(MediaPipeline_Play)

	gst_element_set_state(ELEMENT, GST_STATE_PLAYING);

END_METHOD

BEGIN_METHOD_VOID(MediaPipeline_Stop)

	gst_element_set_state(ELEMENT, GST_STATE_READY);

END_METHOD

BEGIN_METHOD_VOID(MediaPipeline_Close)

	gst_element_set_state(ELEMENT, GST_STATE_NULL);

END_METHOD

BEGIN_METHOD_VOID(MediaPipeline_Pause)

	gst_element_set_state(ELEMENT, GST_STATE_PAUSED);

END_METHOD

BEGIN_PROPERTY(MediaPipeline_Position)

	if (READ_PROPERTY)
	{
		GstFormat format = GST_FORMAT_TIME;
		gint64 pos;
		
		if (!gst_element_query_position(ELEMENT, &format, &pos) || format != GST_FORMAT_TIME)
			GB.ReturnFloat(0);
		else
			GB.ReturnFloat((double)(pos / 1000) / 1E6);
	}
	else
	{
		guint64 pos = VPROP(GB_FLOAT) * 1E9;
		
		if (pos < 0) 
			pos = 0;
		
		gst_element_seek_simple(ELEMENT, GST_FORMAT_TIME, GST_SEEK_FLAG_FLUSH | GST_SEEK_FLAG_KEY_UNIT, pos);
	}

END_PROPERTY

BEGIN_PROPERTY(MediaPipeline_Duration)

	GstFormat format = GST_FORMAT_TIME;
	gint64 dur;
	
	if (!gst_element_query_duration(ELEMENT, &format, &dur) || format != GST_FORMAT_TIME)
		GB.ReturnFloat(0);
	else
		GB.ReturnFloat((double)(dur / 1000) / 1E6);

END_PROPERTY

//---- Media --------------------------------------------------------------

BEGIN_METHOD(Media_Link, GB_OBJECT controls)

	GB_OBJECT *controls = ARG(controls);
	int i;
	CMEDIACONTROL *c1, *c2;
	
	if (GB.CheckObject(VARG(controls)))
		return;
	
	// GB.NParam() returns 0 when MediaPipeline_Link has just two arguments
	
	for (i = 0; i <= GB.NParam(); i++)
	{
		c1 = VALUE(&controls[i]);
		c2 = VALUE(&controls[i + 1]);
		if (GB.CheckObject(c1))
			return;
		gst_element_link(c1->elt, c2->elt);
	}

END_METHOD

BEGIN_METHOD(Media_Time, GB_FLOAT second)

	GB.ReturnLong(VARG(second) * 1E9);

END_METHOD

//-------------------------------------------------------------------------

GB_DESC MediaControlDesc[] = 
{
	GB_DECLARE("MediaControl", sizeof(CMEDIACONTROL)),
	
	GB_METHOD("_new", NULL, MediaControl_new, "[(Type)s(Parent)MediaContainer;]"),
	GB_METHOD("_free", NULL, MediaControl_free, NULL),
	
	GB_PROPERTY("Name", "s", MediaControl_Name),
	GB_PROPERTY_READ("Type", "s", MediaControl_Type),
	GB_PROPERTY("State", "i", MediaControl_State),
			
	GB_METHOD("_put", NULL, MediaControl_put, "(Value)v(Property)s"),
	GB_METHOD("_get", "v", MediaControl_get, "(Property)s"),
	
	GB_METHOD("LinkTo", NULL, MediaControl_LinkTo, "(Destination)MediaControl;[(Later)b]"),
	
	GB_END_DECLARE
};

GB_DESC MediaContainerDesc[] = 
{
	GB_DECLARE("MediaContainer", sizeof(CMEDIACONTAINER)),
	GB_INHERITS("MediaControl"),
	
	//GB_METHOD("_new", NULL, MediaContainer_new, NULL),
	//GB_METHOD("_new", NULL, MediaContainer_new, NULL),
	
	GB_END_DECLARE
};

GB_DESC MediaPipelineDesc[] = 
{
	GB_DECLARE("MediaPipeline", sizeof(CMEDIAPIPELINE)),
	GB_INHERITS("MediaContainer"),
	
	GB_METHOD("_new", NULL, MediaPipeline_new, NULL),
	GB_METHOD("_free", NULL, MediaPipeline_free, NULL),
	
	GB_CONSTANT("Null", "i", GST_STATE_NULL),
	GB_CONSTANT("Unknown", "i", -1),
	GB_CONSTANT("Ready", "i", GST_STATE_READY),
	GB_CONSTANT("Paused", "i", GST_STATE_PAUSED),
	GB_CONSTANT("Playing", "i", GST_STATE_PLAYING),

	GB_CONSTANT("Info", "i", 0),
	GB_CONSTANT("Warning", "i", 1),
	GB_CONSTANT("Error", "i", 2),
	
	GB_PROPERTY("Position", "f", MediaPipeline_Position),
	GB_PROPERTY_READ("Duration", "f", MediaPipeline_Duration),
	GB_PROPERTY_READ("Length", "f", MediaPipeline_Duration),
	
	GB_METHOD("Play", NULL, MediaPipeline_Play, NULL),
	GB_METHOD("Stop", NULL, MediaPipeline_Stop, NULL),
	GB_METHOD("Pause", NULL, MediaPipeline_Pause, NULL),
	GB_METHOD("Close", NULL, MediaPipeline_Close, NULL),
	
	GB_EVENT("End", NULL, NULL, &EVENT_End),
	GB_EVENT("Message", NULL, "(Type)i(Message)s", &EVENT_Message),
	GB_EVENT("Tag", NULL, NULL, &EVENT_Tag),
	GB_EVENT("Buffering", NULL, NULL, &EVENT_Buffering),
	GB_EVENT("State", NULL, NULL, &EVENT_State),
	GB_EVENT("Duration", NULL, NULL, &EVENT_Duration),
	GB_EVENT("Progress", NULL, NULL, &EVENT_Progress),
	
	GB_END_DECLARE
};

GB_DESC MediaDesc[] = 
{
	GB_DECLARE("Media", sizeof(CMEDIAPIPELINE)),
	GB_INHERITS("MediaPipeline"),
	
	GB_CONSTANT("Null", "i", GST_STATE_NULL),
	GB_CONSTANT("Unknown", "i", -1),
	GB_CONSTANT("Ready", "i", GST_STATE_READY),
	GB_CONSTANT("Paused", "i", GST_STATE_PAUSED),
	GB_CONSTANT("Playing", "i", GST_STATE_PLAYING),

	GB_CONSTANT("Info", "i", 0),
	GB_CONSTANT("Warning", "i", 1),
	GB_CONSTANT("Error", "i", 2),
	
	GB_STATIC_METHOD("Link", NULL, Media_Link, "(FirstControl)MediaControl;(SecondControl)MediaControl;."),
	GB_STATIC_METHOD("Time", "l", Media_Time, "(Seconds)f"),
	
	GB_END_DECLARE
};

