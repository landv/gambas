/***************************************************************************

  c_media.c

  gb.media component

  (c) 2000-2013 Benoît Minisini <gambas@users.sourceforge.net>

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
//#include <gst/interfaces/xoverlay.h>
#include <gst/base/gstbasesink.h>
#include <gst/video/video.h>
#include <gst/video/videooverlay.h>

static void *_from_element = NULL;

void MEDIA_raise_event(void *_object, int event)
{
	gst_element_post_message(ELEMENT, gst_message_new_application(GST_OBJECT(ELEMENT), gst_structure_new("SendEvent", "event", G_TYPE_INT, event, NULL)));
}

/*void MEDIA_raise_event_arg(void *_object, int event, char *arg)
{
	gst_element_post_message(ELEMENT, gst_message_new_application(GST_OBJECT(ELEMENT), gst_structure_new("SendEvent", "event", G_TYPE_INT, event, "arg", G_TYPE_STRING, arg, NULL)));
}*/

static const char *get_element_type(GstElement *element)
{
	GstElementFactory *factory = gst_element_get_factory(element);
	if (!factory)
		return NULL;
	else
		return gst_plugin_feature_get_name(GST_PLUGIN_FEATURE(factory));
}

CMEDIACONTROL *MEDIA_get_control_from_element(void *element, bool create)
{
	CMEDIACONTROL *ctrl;
	GB_CLASS klass;
	//const char *type;
	
	if (!element)
		return NULL;
	
	ctrl = (CMEDIACONTROL *)g_object_get_data(G_OBJECT(element), "gambas-control");
	if (create)
	{
		if (!ctrl)
		{
			_from_element = element;
			
			/*type = get_element_type(element);
			
			if (type && strcmp(type, "v4l2src") == 0)
				klass = GB.FindClass("MediaVideo");
			else*/
			if (GST_IS_PIPELINE(element))
				klass = GB.FindClass("MediaPipeline");
			else if (GST_IS_BIN(element))
				klass = GB.FindClass("MediaContainer");
			else
				klass = GB.FindClass("MediaControl");
			
			ctrl = (CMEDIACONTROL *)GB.New(klass, NULL, NULL);
		}
	}
	else
	{
		if (ctrl && ctrl->borrow)
			ctrl = NULL;
	}
	
	return ctrl;
}

bool MEDIA_set_state(void *_object, int state, bool error)
{
	GstStateChangeReturn status;

	status = gst_element_set_state(ELEMENT, state);
	
	if (status == GST_STATE_CHANGE_ASYNC)
		status = gst_element_get_state(ELEMENT, NULL, NULL, GST_SECOND);
	
	if (status == GST_STATE_CHANGE_FAILURE)
	{
		if (error) GB.Error("Cannot set status");
		return TRUE;
	}
	
	return FALSE;
}

bool MEDIA_get_flag(void *element, char *property, int flag)
{
	int flags;
	
	g_object_get(G_OBJECT(element), property, &flags, NULL);
	return (flags & flag) != 0;
}

void MEDIA_set_flag(void *element, char *property, int flag, bool value)
{
	int flags;
	
	g_object_get(G_OBJECT(element), property, &flags, NULL);
	if (value)
		flags |= flag;
	else
		flags &= ~flag;
	g_object_set(G_OBJECT(element), property, flags, NULL);
}



static GB_TYPE to_gambas_type(const GValue *value)
{
	switch (G_VALUE_TYPE(value))
	{
		case G_TYPE_BOOLEAN: return GB_T_BOOLEAN;
		case G_TYPE_INT: return GB_T_INTEGER;
		case G_TYPE_UINT: return GB_T_INTEGER;
		case G_TYPE_UINT64: return GB_T_LONG;
		case G_TYPE_STRING: return GB_T_STRING;
		case G_TYPE_FLOAT: return GB_T_FLOAT;
		case G_TYPE_DOUBLE: return GB_T_FLOAT;
		default:
			if (G_VALUE_HOLDS(value, G_TYPE_DATE))
				return GB_T_DATE;
			else if (GST_VALUE_HOLDS_DATE_TIME(value))
				return GB_T_DATE;
			else
			{
				fprintf(stderr, "gb.media: warning: unsupported data type: %s\n", G_VALUE_TYPE_NAME(value));
				//GB.Error("Unsupported property datatype"); 
				return GB_T_NULL;
			}
	}
}

static void to_gambas_value(const GValue *value, GB_VALUE *gvalue)
{
	switch (G_VALUE_TYPE(value))
	{
		case G_TYPE_BOOLEAN:
			gvalue->type = GB_T_BOOLEAN;
			gvalue->_boolean.value = g_value_get_boolean(value) == 0 ? 0 : -1;
			break;
			
		case G_TYPE_INT:
			gvalue->type = GB_T_INTEGER;
			gvalue->_integer.value = g_value_get_int(value);
			break;
			
		case G_TYPE_UINT:
			gvalue->type = GB_T_INTEGER;
			gvalue->_integer.value = (int)g_value_get_uint(value);
			break;

		case G_TYPE_UINT64:
			gvalue->type = GB_T_LONG;
			gvalue->_long.value = (int64_t)g_value_get_uint64(value);
			break;
			
		case G_TYPE_STRING:
			gvalue->type = GB_T_STRING;
			gvalue->_string.value.addr = GB.NewZeroString(g_value_get_string(value));
			gvalue->_string.value.start = 0;
			gvalue->_string.value.len = GB.StringLength(gvalue->_string.value.addr);
			break;
			
		case G_TYPE_FLOAT:
			gvalue->type = GB_T_FLOAT;
			gvalue->_float.value = (double)g_value_get_float(value);
			break;
			
		case G_TYPE_DOUBLE:
			gvalue->type = GB_T_FLOAT;
			gvalue->_float.value = g_value_get_double(value);
			break;
			
		default:
			
			if (G_VALUE_HOLDS(value, G_TYPE_DATE))
			{
				GB_DATE_SERIAL ds;
				GDate *date = (GDate *)g_value_get_boxed(value);
				
				CLEAR(&ds);
				ds.year = date->year;
				ds.month = date->month;
				ds.day = date->day;
				
				if (ds.year && (ds.month == 0 || ds.day == 0))
				{
					ds.month = 1;
					ds.day = 1;
				}
				
				GB.MakeDate(&ds, (GB_DATE *)gvalue);
				break;
			}
			else if (GST_VALUE_HOLDS_DATE_TIME(value))
			{
				GB_DATE_SERIAL ds;
				GstDateTime *date = (GstDateTime *)g_value_get_boxed(value);
				
				CLEAR(&ds);
				if (gst_date_time_has_year(date))
					ds.year = gst_date_time_get_year(date);
				if (gst_date_time_has_month(date))
					ds.month = gst_date_time_get_month(date);
				if (gst_date_time_has_day(date))
					ds.day = gst_date_time_get_day(date);
				
				if (ds.year && (ds.month == 0 || ds.day == 0))
				{
					ds.month = 1;
					ds.day = 1;
				}
				
				if (gst_date_time_has_time(date))
				{
					ds.hour = gst_date_time_get_hour(date);
					ds.min = gst_date_time_get_minute(date);
					ds.sec = gst_date_time_get_second(date);
					ds.msec = gst_date_time_get_microsecond(date);
				}
				
				GB.MakeDate(&ds, (GB_DATE *)gvalue);
				break;
			}
			else
				gvalue->type = GB_T_NULL;
	}
}

static GParamSpec *get_property(GstElement *element, const char *property)
{
	GParamSpec *desc;
	
	desc = g_object_class_find_property(G_OBJECT_GET_CLASS(G_OBJECT(element)), property);
	if (!desc)
		GB.Error("Unknown property: '&1'", property);
	
	return desc;
}

static void return_value(const GValue *value)
{
	GType type = G_VALUE_TYPE(value);
	
	switch (type)
	{
		case G_TYPE_BOOLEAN: GB.ReturnBoolean(g_value_get_boolean(value)); break;
		case G_TYPE_INT: GB.ReturnInteger(g_value_get_int(value)); break;
		case G_TYPE_UINT: GB.ReturnInteger(g_value_get_uint(value)); break;
		case G_TYPE_UINT64: GB.ReturnLong(g_value_get_uint64(value)); break;
		case G_TYPE_STRING: GB.ReturnNewZeroString(g_value_get_string(value)); break;
		case G_TYPE_FLOAT: GB.ReturnFloat(g_value_get_float(value)); break;
		case G_TYPE_DOUBLE: GB.ReturnFloat(g_value_get_double(value)); break;
		
		default: 
			if (G_VALUE_HOLDS(value, G_TYPE_DATE) || GST_VALUE_HOLDS_DATE_TIME(value))
			{
				GB_VALUE date;
				to_gambas_value(value, &date);
				GB.ReturnDate((GB_DATE *)&date);
			}
			else if (G_TYPE_IS_ENUM(type))
			{
				GEnumValue *enum_value;
				
				enum_value = g_enum_get_value(G_ENUM_CLASS(g_type_class_ref(type)), g_value_get_enum(value));
				if (!enum_value)
				{
					char buf[16];
					sprintf(buf, "%d", g_value_get_enum(value));
					GB.ReturnNewZeroString(buf);
				}
				else
					GB.ReturnNewZeroString(enum_value->value_nick);
			}
			else if (type == GST_TYPE_CAPS)
			{
				char *caps;
				caps = gst_caps_to_string((GstCaps *)g_value_get_boxed(value));
				GB.ReturnNewZeroString(caps);
				g_free(caps);
			}
			else
			{
				fprintf(stderr, "gb.media: warning: unsupported datatype: %s\n", G_VALUE_TYPE_NAME(value));
				GB.ReturnNull();
			}
	}
}

void MEDIA_return_property(void *_object, const char *property)
{
	GParamSpec *desc;
	GValue value = G_VALUE_INIT;
	
	desc = get_property(ELEMENT, property);
	if (!desc)
		return;
	
	//fprintf(stderr, "type = %s\n", g_type_name(desc->value_type));
	g_value_init(&value, desc->value_type);
	g_object_get_property(G_OBJECT(ELEMENT), property, &value);
	return_value(&value);
	g_value_unset(&value);
}


static bool set_value(GValue *value, GB_VALUE *v, GParamSpec *desc)
{
	GType type = desc->value_type;

	g_value_init(value, type);
	
	switch (type)
	{
		case G_TYPE_BOOLEAN:
			if (GB.Conv(v, GB_T_BOOLEAN))
				return TRUE;
			g_value_set_boolean(value, v->_boolean.value);
			break;
			
		case G_TYPE_INT:
			if (GB.Conv(v, GB_T_INTEGER))
				return TRUE;
			g_value_set_int(value, v->_integer.value);
			break;
			
		case G_TYPE_UINT:
			if (GB.Conv(v, GB_T_INTEGER))
				return TRUE;
			g_value_set_uint(value, (uint)v->_integer.value);
			break;
			
		case G_TYPE_UINT64:
			if (GB.Conv(v, GB_T_LONG))
				return TRUE;
			g_value_set_uint64(value, (guint64)v->_long.value);
			break;
			
		case G_TYPE_STRING:
			if (GB.Conv(v, GB_T_STRING))
				return TRUE;
			g_value_set_string(value, GB.ToZeroString((GB_STRING *)v));
			break;
			
		case G_TYPE_FLOAT:
			if (GB.Conv(v, GB_T_FLOAT))
				return TRUE;
			g_value_set_float(value, v->_float.value);
			break;
			
		case G_TYPE_DOUBLE:
			if (GB.Conv(v, GB_T_FLOAT))
				return TRUE;
			g_value_set_double(value, v->_float.value);
			break;
		
		default:
			
			if (G_TYPE_IS_ENUM(type))
			{
				GEnumValue *enum_value;
				int real_value;
				
				if (!GB.Conv(v, GB_T_INTEGER))
				{
					real_value = ((GB_INTEGER *)v)->value;
				}
				else
				{
					if (GB.Conv(v, GB_T_STRING))
						return TRUE;

					enum_value = g_enum_get_value_by_nick(G_ENUM_CLASS(g_type_class_ref(type)), GB.ToZeroString((GB_STRING *)v));
					if (!enum_value)
					{
						GB.Error("Unknown enumeration value");
						return TRUE;
					}

					real_value = enum_value->value;
				}
				
				g_value_set_enum(value, real_value);
			}
			else if (type == GST_TYPE_CAPS)
			{
				GstCaps *caps;

				if (GB.Conv(v, GB_T_STRING))
					return TRUE;

				caps = gst_caps_from_string(GB.ToZeroString((GB_STRING *)v));
				if (!caps)
				{
					GB.Error("Incorrect filter");
					return TRUE;
				}

				g_value_take_boxed(value, caps);
			}
			else
			{
				GB.Error("Unsupported datatype: &1", g_type_name(type));
				return TRUE;
			}
	}
	
	return FALSE;
}

void MEDIA_set_property(void *_object, const char *property, GB_VALUE *v)
{
	GParamSpec *desc;
	GValue value = G_VALUE_INIT;

	desc = get_property(ELEMENT, property);
	if (!desc)
		return;
	
	if (set_value(&value, v, desc))
		return;
	
	g_object_set_property(G_OBJECT(ELEMENT), property, &value);
	g_value_unset(&value);
}

GB_IMG *MEDIA_get_image_from_sample(GstSample *sample, bool convert)
{
	GstSample *temp;
	GError *err = NULL;
	GstStructure *s;
	GstCaps *to_caps, *sample_caps;
	gint outwidth = 0;
	gint outheight = 0;
	GstMemory *memory;
	GstMapInfo info;
	const char *format;
	int gb_format;
	GB_IMG *img;

	switch (IMAGE.GetDefaultFormat())
	{
		case GB_IMAGE_BGRA:
			format = "BGR";
			gb_format = GB_IMAGE_BGR;
			break;

		case GB_IMAGE_RGBA:
			format = "RGB";
			gb_format = GB_IMAGE_RGB;
			break;

		default:
			GB.Error("Unsupported default image format");
			return NULL;
	}

	if (convert)
	{
		/* our desired output format (RGB or BGR) */
		to_caps = gst_caps_new_simple ("video/x-raw",
			"format", G_TYPE_STRING, format,
			/* Note: we don't ask for a specific width/height here, so that
				* videoscale can adjust dimensions from a non-1/1 pixel aspect
				* ratio to a 1/1 pixel-aspect-ratio. We also don't ask for a
				* specific framerate, because the input framerate won't
				* necessarily match the output framerate if there's a deinterlacer
				* in the pipeline. */
			"pixel-aspect-ratio", GST_TYPE_FRACTION, 1, 1,
			NULL);

		temp = gst_video_convert_sample(sample, to_caps, 25 * GST_SECOND, &err);

		if (temp == NULL && err)
		{
			GB.Error(err->message);
			gst_caps_unref(to_caps);
			gst_sample_unref(sample);
			g_error_free(err);
			return NULL;
		}

		gst_sample_unref(sample);
		gst_caps_unref(to_caps);

		sample = temp;
	}

	if (!sample)
	{
		GB.Error("Unable to retrieve or convert video frame");
		return NULL;
	}

	sample_caps = gst_sample_get_caps(sample);
	if (!sample_caps)
	{
		GB.Error("No caps on video frame");
		gst_sample_unref(sample);
		return NULL;
	}

	//fprintf(stderr, "frame caps: %" GST_PTR_FORMAT, sample_caps);

	s = gst_caps_get_structure (sample_caps, 0);
	gst_structure_get_int (s, "width", &outwidth);
	gst_structure_get_int (s, "height", &outheight);
	if (outwidth <= 0 || outheight <= 0)
	{
		GB.Error("Bad image dimensions");
		gst_sample_unref(sample);
		return NULL;
	}

	memory = gst_buffer_get_memory (gst_sample_get_buffer (sample), 0);
	gst_memory_map(memory, &info, GST_MAP_READ);

	/* create pixbuf from that - use our own destroy function */
	/*pixbuf = gdk_pixbuf_new_from_data (info.data,
			GDK_COLORSPACE_RGB, FALSE, 8, outwidth, outheight,
			GST_ROUND_UP_4 (outwidth * 3), destroy_pixbuf, sample);*/

	img = IMAGE.Create(outwidth, outheight, gb_format, info.data);

	gst_memory_unmap(memory, &info);

	gst_sample_unref(sample);
	return img;
}

static GB_IMG *get_last_image(void *_object)
{
	GstElement *elt = GST_ELEMENT(ELEMENT);
	GstSample *sample;

	if (!GST_IS_BASE_SINK(elt))
	{
		GB.Error("Not supported on this control");
		return NULL;
	}

	sample = gst_base_sink_get_last_sample(GST_BASE_SINK(elt));
	if (sample == NULL)
		return NULL;

	return MEDIA_get_image_from_sample(sample, TRUE);
}


#if 0
//---- MediaSignalArguments -----------------------------------------------

static int check_signal_arguments(void *_object)
{
	return THIS_ARG->param_values == NULL;
}

BEGIN_METHOD(MediaSignalArguments_get, GB_INTEGER index)

	int index = VARG(index);
	
	if (index < 0 || index >= THIS_ARG->n_param_values)
	{
		GB.Error(GB_ERR_BOUND);
		return;
	}
	
	return_value(&THIS_ARG->param_values[index]);

END_METHOD
#endif

//---- MediaTagList -------------------------------------------------------

static int MediaTagList_check(void *_object)
{
	return THIS_TAGLIST->tags == NULL;
}

static CMEDIATAGLIST *create_tag_list(GstTagList *tags)
{
	CMEDIATAGLIST *ob;
	
	ob = GB.New(GB.FindClass("MediaTagList"), NULL, NULL);
	ob->tags = tags;
	return ob;
}

BEGIN_METHOD(MediaTagList_get, GB_STRING name)

	char *name = GB.ToZeroString(ARG(name));
	GstTagList *tags = THIS_TAGLIST->tags;
	const GValue *value;
	int nvalue;

	nvalue = gst_tag_list_get_tag_size(tags, name);
	
	if (nvalue <= 0)
	{
		GB.ReturnNull();
	}
	else if (nvalue == 1)
	{
		value = gst_tag_list_get_value_index(tags, name, 0);
		return_value(value);
	}
	else
	{
		GB_ARRAY array;
		GB_TYPE type;
		GB_VALUE gvalue;
		int i;
		
		value = gst_tag_list_get_value_index(tags, name, 0);
		type = to_gambas_type(value);
		if (type == GB_T_NULL)
			GB.ReturnNull();
		else
		{
			GB.Array.New(&array, type, nvalue);
			for (i = 0; i < nvalue; i++)
			{
				value = gst_tag_list_get_value_index(tags, name, i);
				to_gambas_value(value, &gvalue);
				GB.Store(type, &gvalue, GB.Array.Get(array, i));
				GB.ReleaseValue(&gvalue);
			}
			
			GB.ReturnObject(array);
		}
	}

	GB.ReturnConvVariant();
	
END_METHOD

BEGIN_PROPERTY(MediaTagList_Tags)

	GB_ARRAY array;
	GstTagList *tags = THIS_TAGLIST->tags;
	int ntags, i;
	
	ntags = gst_tag_list_n_tags(tags);
	
	GB.Array.New(&array, GB_T_STRING, ntags);
	
	for (i = 0; i < ntags; i++)
		*((char **)GB.Array.Get(array, i)) = GB.NewZeroString(gst_tag_list_nth_tag_name(tags, i));
	
	GB.ReturnObject(array);

END_PROPERTY

//---- MediaLink ----------------------------------------------------------

static CMEDIALINK *create_link(GstPad *pad)
{
	CMEDIALINK *ob;

	ob = GB.New(GB.FindClass("MediaLink"), NULL, NULL);
	ob->pad = pad;
	return ob;
}

BEGIN_METHOD_VOID(MediaLink_free)

	gst_object_unref(LINK);

END_METHOD

BEGIN_PROPERTY(MediaLink_Name)

	char *name = gst_pad_get_name(LINK);
	GB.ReturnNewZeroString(name);
	g_free(name);

END_PROPERTY

BEGIN_PROPERTY(MediaLink_Peer)

	GstPad *peer = gst_pad_get_peer(LINK);

	if (!peer)
	{
		GB.ReturnNull();
		return;
	}

	GB.ReturnObject(MEDIA_get_control_from_element(gst_pad_get_parent_element(peer), TRUE));
	gst_object_unref(peer);

END_PROPERTY

static void return_peer_name(void *_object, GstPadDirection dir)
{
	if (gst_pad_get_direction(LINK) == dir)
	{
		GstPad *peer = gst_pad_get_peer(LINK);
		if (peer)
		{
			char *name = gst_pad_get_name(peer);
			GB.ReturnNewZeroString(name);
			g_free(name);
			gst_object_unref(peer);
			return;
		}
	}

	GB.ReturnVoidString();
}

BEGIN_PROPERTY(MediaLink_Input)

	return_peer_name(THIS_LINK, GST_PAD_SRC);

END_PROPERTY

BEGIN_PROPERTY(MediaLink_Output)

	return_peer_name(THIS_LINK, GST_PAD_SINK);

END_PROPERTY

//---- MediaControl -------------------------------------------------------

DECLARE_EVENT(EVENT_State);
//DECLARE_EVENT(EVENT_Signal);

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
	{ "MediaPlayer", "playbin" },
	{ "MediaFilter", "capsfilter" },
	{ "MediaVideo", "v4l2src" },
	//{ "MediaDecoder", "decodebin" },
	{ NULL, NULL }
};

static void cb_pad_added(GstElement *element, GstPad *pad, CMEDIACONTROL *_object)
{
	char *name;

	if (!THIS->dest)
		return;
	
	name = gst_pad_get_name(pad);
	gst_element_link_pads(ELEMENT, name, ((CMEDIACONTROL *)THIS->dest)->elt, NULL);
	g_free(name);
}

BEGIN_METHOD(MediaControl_new, GB_OBJECT parent; GB_STRING type)

	char *type;
	CMEDIACONTAINER *parent;
	MEDIA_TYPE *mtp;
	GB_CLASS klass;
	char *filter = NULL;
	
	//fprintf(stderr, "MediaControl_new: %p\n", THIS);

	THIS->tag.type = GB_T_NULL;
	
	if (_from_element)
	{
		//GstState state = GST_STATE_NULL;
		
		THIS->borrow = TRUE;
		THIS->elt = _from_element;
		_from_element = NULL;

		gst_object_ref(GST_OBJECT(ELEMENT));
		
		g_object_set_data(G_OBJECT(ELEMENT), "gambas-control", THIS);

		//gst_element_get_state(ELEMENT, &state, NULL, GST_SECOND);
		THIS->state = GST_STATE_NULL;
		
		//THIS->type = GB.NewZeroString(gst_plugin_feature_get_name(GST_PLUGIN_FEATURE(gst_element_get_factory(ELEMENT))));
	}
	else
	{
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
		{
			type = GB.ToZeroString(ARG(type));
			if (strchr(type, '/'))
			{
				filter = type;
				type = "capsfilter";
			}
		}
		
		THIS->state = GST_STATE_NULL;
		//THIS->type = GB.NewZeroString(type);
		

		ELEMENT = gst_element_factory_make(type, NULL);
		if (!ELEMENT)
		{
			GB.Error("Unable to create media control");
			return;
		}
		
		gst_object_ref(GST_OBJECT(ELEMENT));
		g_object_set_data(G_OBJECT(ELEMENT), "gambas-control", THIS);
		
		parent = VARGOPT(parent, NULL);
		if (parent)
		{
			gst_bin_add(GST_BIN(parent->elt), ELEMENT);
			gst_element_sync_state_with_parent(ELEMENT);
		}
		else if (!GST_IS_PIPELINE(ELEMENT))
			GB.CheckObject(parent);

		if (filter)
			MEDIA_set_property(THIS, "caps", (GB_VALUE *)ARG(type));
	}
		
END_METHOD

BEGIN_METHOD_VOID(MediaControl_free)

	//fprintf(stderr, "MediaControl_free: %p\n", THIS);

	GB.Unref(POINTER(&THIS->dest));
	//GB.FreeString(&THIS->type);
	GB.StoreVariant(NULL, &THIS->tag);
	
	if (ELEMENT)
	{
		if (!THIS->borrow)
			gst_element_set_state(ELEMENT, GST_STATE_NULL);
		
		g_object_set_data(G_OBJECT(ELEMENT), "gambas-control", NULL);
		gst_object_unref(GST_OBJECT(ELEMENT));
	}
	
END_METHOD

BEGIN_PROPERTY(MediaControl_Tag)

	if (READ_PROPERTY)
		GB.ReturnVariant(&THIS->tag);
	else
		GB.StoreVariant(PROP(GB_VARIANT), POINTER(&THIS->tag));

END_METHOD

BEGIN_PROPERTY(MediaControl_Type)

	const char *type = get_element_type(ELEMENT);
	
	if (!type)
		GB.ReturnConstZeroString("?");
	else
		GB.ReturnNewZeroString(type);
	
END_PROPERTY

BEGIN_PROPERTY(MediaControl_Parent)

	GstElement *parent = GST_ELEMENT(gst_element_get_parent(ELEMENT));
	GB.ReturnObject(MEDIA_get_control_from_element(parent, TRUE));
	if (parent)
		gst_object_unref(parent);

END_PROPERTY

BEGIN_PROPERTY(MediaControl_Name)

	if (READ_PROPERTY)
		GB.ReturnNewZeroString(gst_element_get_name(ELEMENT));
	else
		gst_element_set_name(ELEMENT, GB.ToZeroString(PROP(GB_STRING)));

END_PROPERTY

BEGIN_PROPERTY(MediaControl_State)

	if (READ_PROPERTY)
	{
		GB.ReturnInteger(THIS->state);
		/*GstState state;
		
		status = gst_element_get_state(ELEMENT, &state, NULL, GST_SECOND);
		
		if (status != GST_STATE_CHANGE_SUCCESS)
			GB.ReturnInteger(-1);
		else
			GB.ReturnInteger(state);*/
	}
	else
	{
		MEDIA_set_state(THIS, VPROP(GB_INTEGER), TRUE);
	}

END_PROPERTY

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
	return_value(&value);
	g_value_unset(&value);
	GB.ReturnConvVariant();
	
END_METHOD

BEGIN_METHOD(MediaControl_put, GB_VARIANT value; GB_STRING property)

	char *property = GB.ToZeroString(ARG(property));
	GB_VALUE *v = (GB_VALUE *)ARG(value);
	
	MEDIA_set_property(THIS, property, v);
	
END_METHOD

BEGIN_METHOD(MediaControl_LinkTo, GB_OBJECT dest; GB_STRING output; GB_STRING input)

	CMEDIACONTROL *dest = (CMEDIACONTROL *)VARG(dest);
	char *output;
	char *input;
	
	if (GB.CheckObject(dest))
		return;

	output = MISSING(output) ? NULL : GB.ToZeroString(ARG(output));
	if (output && !*output) output = NULL;
	input = MISSING(input) ? NULL : GB.ToZeroString(ARG(input));
	if (input && !*input) input = NULL;
	
	if (output)
	{
		GstPad *pad = gst_element_get_static_pad (ELEMENT, output);
		if (pad)
		{
			if (GST_PAD_IS_SRC(pad))
			{
				GstPad *peer = gst_pad_get_peer(pad);
				gst_pad_unlink(pad, peer);
				gst_object_unref(peer);
			}
			gst_object_unref(pad);
		}
	}

	if (!gst_element_link_pads(ELEMENT, output, dest->elt, input))
		GB.Error("Unable to link controls");

END_METHOD

BEGIN_METHOD(MediaControl_LinkLaterTo, GB_OBJECT dest)

	CMEDIACONTROL *dest = (CMEDIACONTROL *)VARG(dest);
	
	if (GB.CheckObject(dest))
		return;

	GB.Unref(POINTER(&THIS->dest));
	GB.Ref(dest);
	THIS->dest = dest;
	g_signal_connect(ELEMENT, "pad-added", G_CALLBACK(cb_pad_added), THIS);

END_METHOD

static GstIteratorResult iterator_next_pad(GstIterator *iter, GstPad **pad)
{
	GstIteratorResult ret;
	GValue value = G_VALUE_INIT;
	
	ret = gst_iterator_next(iter, &value);
	if (ret == GST_ITERATOR_OK)
	{
		if (G_VALUE_HOLDS_BOXED(&value))
			*pad = g_value_get_boxed(&value);
		else
			*pad = (GstPad *)g_value_get_object(&value);
	}
	
	return ret;
}

static void fill_pad_list(GB_ARRAY array, GstIterator *iter)
{
	bool done = FALSE;
	GstPad *pad;
	char *name;
	
	while (!done) 
	{
		switch (iterator_next_pad(iter, &pad)) 
		{
			case GST_ITERATOR_OK:
				name = gst_pad_get_name(pad);
				*((char **)GB.Array.Add(array)) = GB.NewZeroString(name);
				g_free(name);
				gst_object_unref(pad);
				break;
			case GST_ITERATOR_RESYNC:
				gst_iterator_resync(iter);
				break;
			case GST_ITERATOR_ERROR:
			case GST_ITERATOR_DONE:
				done = TRUE;
				break;
		}
	}
	
	gst_iterator_free(iter);
}

BEGIN_PROPERTY(MediaControl_Inputs)

	GstIterator *iter;
	GB_ARRAY array;
	
	GB.Array.New(&array, GB_T_STRING, 0);
	iter = gst_element_iterate_sink_pads(ELEMENT);
	fill_pad_list(array, iter);
	GB.ReturnObject(array);

END_PROPERTY

BEGIN_PROPERTY(MediaControl_Outputs)

	GstIterator *iter;
	GB_ARRAY array;
	
	GB.Array.New(&array, GB_T_STRING, 0);
	iter = gst_element_iterate_src_pads(ELEMENT);
	fill_pad_list(array, iter);
	GB.ReturnObject(array);

END_PROPERTY

#if 0
static void closure_marshal(GClosure     *closure,
                            GValue       *return_value,
                            guint         n_param_values,
                            const GValue *param_values,
                            gpointer      invocation_hint,
                            gpointer      marshal_data)
{
	GObject *src;
	CMEDIACONTROL *_object;
	CMEDIASIGNALARGUMENTS *arg;
	
	src = g_value_peek_pointer (param_values + 0);
	_object = get_control_from_element(src);
	
	arg = GB.New(GB.FindClass("MediaSignalArguments"), NULL, NULL);
	arg->n_param_values = n_param_values;
	arg->param_values = param_values;
	
	GB.Ref(arg);
	GB.Raise(THIS, EVENT_Signal, 1, GB_T_OBJECT, arg);
	MEDIA_raise_event_arg(THIS, EVENT_Signal, );
	
	arg->n_param_values = 0;
	arg->param_values = NULL;
	GB.Unref(POINTER(&arg));
}

static GClosure *get_closure()
{
	static GClosure *closure = NULL;
	
	if (!closure)
	{
		closure = g_closure_new_simple(sizeof(GClosure), NULL);
		g_closure_set_marshal(closure, closure_marshal);
	}
	
	return closure;
}

BEGIN_METHOD(MediaControl_Activate, GB_STRING signal)

	char *signal = GB.ToZeroString(ARG(signal));
	GClosure *closure = get_closure();
	
	if (g_signal_handler_find(ELEMENT, G_SIGNAL_MATCH_CLOSURE | G_SIGNAL_MATCH_ID, g_signal_lookup(signal, G_OBJECT_TYPE(ELEMENT)), (GQuark)0, closure, NULL, NULL))
	{
		GB.Error("Signal is already activated");
		return;
	}
		
	g_signal_connect_closure(ELEMENT, GB.ToZeroString(ARG(signal)), closure, FALSE);

END_METHOD
#endif

BEGIN_METHOD(MediaControl_SetWindow, GB_OBJECT control; GB_INTEGER x; GB_INTEGER y; GB_INTEGER w; GB_INTEGER h)

	void *control = VARG(control);
	long wid;
	int x, y, w, h;
	
	if (!GST_IS_VIDEO_OVERLAY(ELEMENT))
	{
		GB.Error("Not supported on this control");
		return;
	}
	
	if (control && GB.CheckObject(control))
		return;
	
	if (control)
	{
		wid = MAIN_get_x11_handle(control);
		if (wid == 0)
			return;
	}
	else
		wid = 0;
	
	gst_video_overlay_set_window_handle(GST_VIDEO_OVERLAY(ELEMENT), (guintptr)wid);
	
	if (wid && !MISSING(x) && !MISSING(y) && !MISSING(w) && !MISSING(h))
	{
		x = VARG(x);
		y = VARG(y);
		w = VARG(w);
		h = VARG(h);
		
		if (w > 0 && h > 0)
			gst_video_overlay_set_render_rectangle(GST_VIDEO_OVERLAY(ELEMENT), x, y, w, h);
	}
	gst_video_overlay_expose(GST_VIDEO_OVERLAY(ELEMENT));

END_PROPERTY

#if 0
BEGIN_PROPERTY(MediaControl_Protocols)

	char **protocols;
	GB_ARRAY array;
	
	if (!GST_IS_URI_HANDLER(ELEMENT))
	{
		GB.ReturnNull();
		return;
	}
	
	protocols = (char **)gst_uri_handler_get_protocols(GST_URI_HANDLER(ELEMENT));
	GB.Array.New(&array, GB_T_STRING, 0);
	while (*protocols)
	{
		*((char **)GB.Array.Add(array)) = GB.NewZeroString(*protocols);
		protocols++;
	}
	GB.ReturnObject(array);

END_PROPERTY
#endif

BEGIN_METHOD(MediaControl_GetLink, GB_STRING name)

	bool done = FALSE;
	GstIterator *iter;
	GstPad *pad;
	char *name;
	char *search;
	CMEDIALINK *link = NULL;

	search = GB.ToZeroString(ARG(name));
	iter = gst_element_iterate_pads(ELEMENT);

	while (!done)
	{
		switch (iterator_next_pad(iter, &pad))
		{
			case GST_ITERATOR_OK:
				name = gst_pad_get_name(pad);
				if (strcmp(name, search) == 0)
				{
					link = create_link(pad);
					done = TRUE;
					break;
				}
				g_free(name);
				gst_object_unref(pad);
				break;
			case GST_ITERATOR_RESYNC:
				gst_iterator_resync(iter);
				break;
			case GST_ITERATOR_ERROR:
			case GST_ITERATOR_DONE:
				done = TRUE;
				break;
		}
	}

	gst_iterator_free(iter);
	GB.ReturnObject(link);

END_METHOD

BEGIN_METHOD_VOID(MediaControl_GetLastImage)

	GB.ReturnObject(get_last_image(THIS));

END_PROPERTY


//---- MediaFilter --------------------------------------------------------

BEGIN_PROPERTY(MediaFilter_Filter)

	if (READ_PROPERTY)
		MEDIA_return_property(THIS, "caps");
	else
		MEDIA_set_property(THIS, "caps", PROP(GB_VALUE));

END_PROPERTY

//---- MediaContainerChildren ---------------------------------------------

BEGIN_PROPERTY(MediaContainerChildren_Count)

	GB.ReturnInteger(gst_child_proxy_get_children_count(GST_CHILD_PROXY(ELEMENT)));

END_PROPERTY

BEGIN_METHOD(MediaContainerChildren_get, GB_INTEGER index)

	int count = gst_child_proxy_get_children_count(GST_CHILD_PROXY(ELEMENT));
	int index = VARG(index);
	
	if (index < 0 || index >= count)
	{
		GB.Error(GB_ERR_BOUND);
		return;
	}

	GB.ReturnObject(MEDIA_get_control_from_element(gst_child_proxy_get_child_by_index(GST_CHILD_PROXY(ELEMENT), index), TRUE));

END_PROPERTY

BEGIN_METHOD_VOID(MediaContainerChildren_next)

	int count = gst_child_proxy_get_children_count(GST_CHILD_PROXY(ELEMENT));
	int *index = (int *)GB.GetEnum();

	if (*index < 0 || *index >= count)
		GB.StopEnum();
	else
	{
		GB.ReturnObject(MEDIA_get_control_from_element(gst_child_proxy_get_child_by_index(GST_CHILD_PROXY(ELEMENT), *index), TRUE));
		(*index)++;
	}

END_PROPERTY

//---- MediaContainer -----------------------------------------------------

static bool add_input_output(void *_object, CMEDIACONTROL *child, char *name, int direction, const char *dir_error, const char *unknown_error)
{
	GstPad *pad;
	GstIterator *iter;
	GstIteratorResult res;
	
	if (GB.CheckObject(child))
		return TRUE;
	
	if (!name)
	{
		if (direction == GST_PAD_SINK)
			iter = gst_element_iterate_sink_pads(child->elt);
		else
			iter = gst_element_iterate_src_pads(child->elt);
		
		for(;;)
		{
			res = iterator_next_pad(iter, &pad);
			if (res == GST_ITERATOR_RESYNC)
				gst_iterator_resync(iter);
			else
				break;
		}
		
		gst_iterator_free(iter);
		
		if (res != GST_ITERATOR_OK)
		{
			GB.Error(unknown_error);
			return TRUE;
		}
	}
	else
	{
		pad = gst_element_get_static_pad(child->elt, name);
		if (!pad)
		{
			GB.Error(unknown_error);
			return TRUE;
		}
		
		if (gst_pad_get_direction(pad) != direction)
		{
			gst_object_unref (GST_OBJECT(pad));
			GB.Error(dir_error);
			return TRUE;
		}
	}
	
	gst_element_add_pad(ELEMENT, gst_ghost_pad_new(name, pad));
	gst_object_unref(GST_OBJECT(pad));
	
	return FALSE;
}

// Just there for the documentation wiki!

BEGIN_METHOD_VOID(MediaContainer_new)

END_METHOD

BEGIN_METHOD(MediaContainer_AddInput, GB_OBJECT child; GB_STRING name)

	add_input_output(THIS, (CMEDIACONTROL *)VARG(child), MISSING(name) ? NULL : GB.ToZeroString(ARG(name)), GST_PAD_SINK, "Not an input", "Unknown input");

END_METHOD

BEGIN_METHOD(MediaContainer_AddOutput, GB_OBJECT child; GB_STRING name)

	add_input_output(THIS, (CMEDIACONTROL *)VARG(child), MISSING(name) ? NULL : GB.ToZeroString(ARG(name)), GST_PAD_SRC, "Not an output", "Unknown output");

END_METHOD

//---- MediaPipeline ------------------------------------------------------

DECLARE_EVENT(EVENT_End);
DECLARE_EVENT(EVENT_Message);
DECLARE_EVENT(EVENT_Tag);
DECLARE_EVENT(EVENT_Buffering);
DECLARE_EVENT(EVENT_Duration);
DECLARE_EVENT(EVENT_Progress);

static int cb_message(CMEDIAPIPELINE *_object)
{
	GstMessage *msg;
	GstMessageType type;
	int msg_type;
	GstBus *bus;
	CMEDIACONTROL *control;
	
	bus = gst_pipeline_get_bus(PIPELINE);
	
	while((msg = gst_bus_pop(bus)) != NULL) 
	{
		type = GST_MESSAGE_TYPE(msg);
		control = MEDIA_get_control_from_element(GST_MESSAGE_SRC(msg), FALSE);
		
		if (type == GST_MESSAGE_APPLICATION && control)
		{
			//CMEDIACONTROL *target = (CMEDIACONTROL *)g_value_get_pointer(gst_structure_get_value(gst_message_get_structure(msg), "control"));
			int event = g_value_get_int(gst_structure_get_value(gst_message_get_structure(msg), "event"));
			GB.Raise(control, event, 0);
		}
		else if (type == GST_MESSAGE_STATE_CHANGED && control)
		{
			GstState old_state, new_state;

			gst_message_parse_state_changed(msg, &old_state, &new_state, NULL);
			control->state = new_state;
			if (new_state == GST_STATE_NULL)
				control->error = FALSE;
			GB.Raise(control, EVENT_State, 0);
		}
		else //if (GST_MESSAGE_SRC(msg) == GST_OBJECT(PIPELINE))
		{
			switch (type)
			{
				case GST_MESSAGE_EOS:
					THIS->eos = TRUE;
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
						if (control)
							control->error = TRUE;
						THIS->error = TRUE;
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
					
					if (!control)
						control = THIS;
					
					GB.Ref(control);
					GB.Raise(THIS, EVENT_Message, 3, GB_T_OBJECT, control, GB_T_INTEGER, msg_type, GB_T_STRING, error->message, -1);
					g_error_free(error);
					GB.Unref(POINTER(&control));
					break;
				}
				
				case GST_MESSAGE_TAG:
				{
					GstTagList *tags = NULL;
					CMEDIATAGLIST *ob;
					//char *list;
		
					gst_message_parse_tag(msg, &tags);
					
					//list = gst_tag_list_to_string(tags);
					//fprintf(stderr, "--> %s\n", list);
					//g_free(list);
					
					ob = create_tag_list(tags);
					GB.Ref(ob);
					
					GB.Raise(THIS, EVENT_Tag, 1, GB_T_OBJECT, ob);
					ob->tags = NULL;
					GB.Unref(POINTER(&ob));
					
					gst_tag_list_free(tags);
					
					break;
				}
				
				case GST_MESSAGE_BUFFERING: GB.Raise(THIS, EVENT_Buffering, 0); break;
				case GST_MESSAGE_DURATION: GB.Raise(THIS, EVENT_Duration, 0); break;
				case GST_MESSAGE_PROGRESS: GB.Raise(THIS, EVENT_Progress, 0); break;
				default: break;
			}
			
			gst_message_unref(msg);
		}
	}
	
	gst_object_unref(bus);
	
	return FALSE;
}

static void stop_pipeline(CMEDIACONTROL *_object)
{
	int try;

	// "It is not allowed to post GST_MESSAGE_EOS when not in the PLAYING state." says the documentation
	if ((THIS->state == GST_STATE_PLAYING) && !THIS->eos)
	{
		try = 0;
		gst_element_send_event(ELEMENT, gst_event_new_eos());
		while (!THIS->eos)
		{
			try++;
			if (try > 25)
			{
				fprintf(stderr, "gb.media: warning: could not catch end of stream\n");
				break;
			}
			cb_message(THIS_PIPELINE);
			usleep(10000);
		}
	}

	MEDIA_set_state(THIS, GST_STATE_READY, TRUE);
	cb_message(THIS_PIPELINE);
}

BEGIN_METHOD(MediaPipeline_new, GB_INTEGER polling)
	
	if (!_from_element)
	{
		int polling = VARGOPT(polling, 250);

		if (polling <= 0)
			polling = 250;
		else if (polling < 10)
			polling = 10;
		else if (polling >= 1000)
			polling = 1000;

		THIS_PIPELINE->polling = polling;
		THIS_PIPELINE->watch = GB.Every(polling, (GB_TIMER_CALLBACK)cb_message, (intptr_t)THIS);
	}

END_METHOD

BEGIN_METHOD_VOID(MediaPipeline_free)

	stop_pipeline(THIS);
	if (THIS_PIPELINE->watch)
	{
		//GB.Wait(THIS_PIPELINE->polling);
		GB.Unref(POINTER(&THIS_PIPELINE->watch));
	}

END_METHOD

BEGIN_METHOD_VOID(MediaPipeline_Play)

	THIS->eos = FALSE;
	MEDIA_set_state(THIS, GST_STATE_PLAYING, TRUE);
	cb_message(THIS_PIPELINE);

END_METHOD

BEGIN_METHOD_VOID(MediaPipeline_Stop)

	stop_pipeline(THIS);

END_METHOD

BEGIN_METHOD_VOID(MediaPipeline_Close)

	MEDIA_set_state(THIS, GST_STATE_NULL, TRUE);
	cb_message(THIS_PIPELINE);

END_METHOD

BEGIN_METHOD_VOID(MediaPipeline_Pause)

	if (THIS->state != GST_STATE_PLAYING)
		return;
	
	MEDIA_set_state(THIS, GST_STATE_PAUSED, TRUE);
	cb_message(THIS_PIPELINE);

END_METHOD

BEGIN_PROPERTY(MediaPipeline_Position)

	if (READ_PROPERTY)
	{
		gint64 pos;
		
		if (THIS->state == GST_STATE_NULL || THIS->state == GST_STATE_READY || THIS->error || !gst_element_query_position(ELEMENT, GST_FORMAT_TIME, &pos))
			GB.ReturnFloat(0);
		else
			GB.ReturnFloat((double)(pos / 1000) / 1E6);
	}
	else
	{
		gint64 pos = VPROP(GB_FLOAT) * 1E9;
		
		if (pos < 0) 
			pos = 0;

		gst_element_seek_simple(ELEMENT, GST_FORMAT_TIME, GST_SEEK_FLAG_FLUSH | GST_SEEK_FLAG_KEY_UNIT, (guint64)pos);
	}

END_PROPERTY

BEGIN_PROPERTY(MediaPipeline_Duration)

	gint64 dur;
	
	if (THIS->state == GST_STATE_NULL || THIS->error || !gst_element_query_duration(ELEMENT, GST_FORMAT_TIME, &dur))
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
		if ((i == 0 && GB.CheckObject(c1)) || GB.CheckObject(c2))
			return;
		gst_element_link(c1->elt, c2->elt);
	}

END_METHOD

BEGIN_METHOD(Media_Time, GB_FLOAT second)

	GB.ReturnLong(VARG(second) * 1E9);

END_METHOD

BEGIN_METHOD(Media_URL, GB_STRING path)

	char *path = GB.RealFileName(STRING(path), LENGTH(path));
	
	path = g_filename_to_uri(path, NULL, NULL);
	GB.ReturnNewZeroString(path);
	g_free(path);

END_METHOD

//-------------------------------------------------------------------------

GB_DESC MediaTagListDesc[] = 
{
	GB_DECLARE("MediaTagList", sizeof(CMEDIATAGLIST)),
	GB_NOT_CREATABLE(),
	
	GB_HOOK_CHECK(MediaTagList_check),
	
	GB_METHOD("_get", "v", MediaTagList_get, "(Name)s"),
	GB_PROPERTY_READ("Tags", "String[]", MediaTagList_Tags),
	
	GB_END_DECLARE
};


GB_DESC MediaLinkDesc[] =
{
	GB_DECLARE("MediaLink", sizeof(CMEDIALINK)),
	GB_NOT_CREATABLE(),

	GB_METHOD("_free", NULL, MediaLink_free, NULL),

	GB_PROPERTY_READ("Name", "s", MediaLink_Name),
	GB_PROPERTY_READ("Peer", "MediaControl", MediaLink_Peer),
	GB_PROPERTY_READ("Input", "s", MediaLink_Input),
	GB_PROPERTY_READ("Output", "s", MediaLink_Output),

	GB_END_DECLARE
};


GB_DESC MediaControlDesc[] = 
{
	GB_DECLARE("MediaControl", sizeof(CMEDIACONTROL)),
	
	GB_METHOD("_new", NULL, MediaControl_new, "[(Parent)MediaContainer;(Type)s]"),
	GB_METHOD("_free", NULL, MediaControl_free, NULL),
	
	GB_PROPERTY("Name", "s", MediaControl_Name),
	GB_PROPERTY_READ("Type", "s", MediaControl_Type),
	GB_PROPERTY_READ("Parent", "MediaContainer", MediaControl_Parent),
	GB_PROPERTY("State", "i", MediaControl_State),
	GB_PROPERTY("Tag", "v", MediaControl_Tag),
			
	GB_METHOD("_put", NULL, MediaControl_put, "(Value)v(Property)s"),
	GB_METHOD("_get", "v", MediaControl_get, "(Property)s"),
	
	GB_METHOD("LinkTo", NULL, MediaControl_LinkTo, "(Target)MediaControl;[(Output)s(Input)s]"),
	GB_METHOD("LinkLaterTo", NULL, MediaControl_LinkLaterTo, "(Target)MediaControl;"),
	
	GB_PROPERTY_READ("Inputs", "String[]", MediaControl_Inputs),
	GB_PROPERTY_READ("Outputs", "String[]", MediaControl_Outputs),
	
	GB_METHOD("GetLink", "MediaLink", MediaControl_GetLink, "(Name)s"),

	GB_METHOD("SetWindow", NULL, MediaControl_SetWindow, "(Control)Control;[(X)i(Y)i(Width)i(Height)i]"),
	GB_METHOD("GetLastImage", "Image", MediaControl_GetLastImage, NULL),

	GB_EVENT("State", NULL, NULL, &EVENT_State),
	//GB_EVENT("Signal", NULL, "(Arg)MediaSignalArguments", &EVENT_Signal),
	
	//GB_PROPERTY_READ("Protocols", "String[]", MediaControl_Protocols),
	
	GB_END_DECLARE
};

GB_DESC MediaFilterDesc[] = 
{
	GB_DECLARE("MediaFilter", sizeof(CMEDIACONTROL)),
	GB_INHERITS("MediaControl"),
	
	//GB_METHOD("_new", NULL, MediaFilter_new, NULL),
	
	GB_PROPERTY("Filter", "s", MediaFilter_Filter),

	GB_END_DECLARE
};

GB_DESC MediaContainerChildrenDesc[] = 
{
	GB_DECLARE_VIRTUAL(".MediaContainer.Children"),
	
	GB_PROPERTY_READ("Count", "i", MediaContainerChildren_Count),
	GB_METHOD("_get", "MediaControl", MediaContainerChildren_get, "(Index)i"),
	GB_METHOD("_next", "MediaControl", MediaContainerChildren_next, NULL),
	
	GB_END_DECLARE
};

GB_DESC MediaContainerDesc[] = 
{
	GB_DECLARE("MediaContainer", sizeof(CMEDIACONTAINER)),
	GB_INHERITS("MediaControl"),
	
	GB_METHOD("_new", NULL, MediaContainer_new, NULL),

	GB_METHOD("AddInput", NULL, MediaContainer_AddInput, "(Child)MediaControl;[(Name)s]"),
	GB_METHOD("AddOutput", NULL, MediaContainer_AddOutput, "(Child)MediaControl;[(Name)s]"),
	
	GB_PROPERTY_SELF("Children", ".MediaContainer.Children"),
	
	GB_END_DECLARE
};

GB_DESC MediaPipelineDesc[] = 
{
	GB_DECLARE("MediaPipeline", sizeof(CMEDIAPIPELINE)),
	GB_INHERITS("MediaContainer"),
	
	GB_METHOD("_new", NULL, MediaPipeline_new, "[(Polling)i]"),
	GB_METHOD("_free", NULL, MediaPipeline_free, NULL),
	
	/*GB_CONSTANT("Null", "i", GST_STATE_NULL),
	GB_CONSTANT("Ready", "i", GST_STATE_READY),
	GB_CONSTANT("Paused", "i", GST_STATE_PAUSED),
	GB_CONSTANT("Playing", "i", GST_STATE_PLAYING),

	GB_CONSTANT("Info", "i", 0),
	GB_CONSTANT("Warning", "i", 1),
	GB_CONSTANT("Error", "i", 2),*/
	
	GB_PROPERTY("Position", "f", MediaPipeline_Position),
	GB_PROPERTY_READ("Duration", "f", MediaPipeline_Duration),
	GB_PROPERTY_READ("Length", "f", MediaPipeline_Duration),
	
	GB_METHOD("Play", NULL, MediaPipeline_Play, NULL),
	GB_METHOD("Stop", NULL, MediaPipeline_Stop, NULL),
	GB_METHOD("Pause", NULL, MediaPipeline_Pause, NULL),
	GB_METHOD("Close", NULL, MediaPipeline_Close, NULL),
	
	GB_EVENT("End", NULL, NULL, &EVENT_End),
	GB_EVENT("Message", NULL, "(Source)MediaControl;(Type)i(Message)s", &EVENT_Message),
	GB_EVENT("Tag", NULL, "(TagList)MediaTagList;", &EVENT_Tag),
	GB_EVENT("Buffering", NULL, NULL, &EVENT_Buffering),
	GB_EVENT("Duration", NULL, NULL, &EVENT_Duration),
	GB_EVENT("Progress", NULL, NULL, &EVENT_Progress),
	
	GB_END_DECLARE
};

GB_DESC MediaDesc[] = 
{
	GB_DECLARE_VIRTUAL("Media"),
	
	GB_CONSTANT("Unknown", "i", -1),

	GB_CONSTANT("Null", "i", GST_STATE_NULL),
	GB_CONSTANT("Ready", "i", GST_STATE_READY),
	GB_CONSTANT("Paused", "i", GST_STATE_PAUSED),
	GB_CONSTANT("Playing", "i", GST_STATE_PLAYING),
	
	GB_CONSTANT("Info", "i", 0),
	GB_CONSTANT("Warning", "i", 1),
	GB_CONSTANT("Error", "i", 2),

	GB_STATIC_METHOD("Link", NULL, Media_Link, "(FirstControl)MediaControl;(SecondControl)MediaControl;."),
	GB_STATIC_METHOD("Time", "l", Media_Time, "(Seconds)f"),
	GB_STATIC_METHOD("URL", "s", Media_URL, "(Path)s"),
	
	GB_END_DECLARE
};

