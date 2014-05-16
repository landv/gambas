/***************************************************************************

  c_mediaplayer.c

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

#define __C_MEDIAPLAYER_C

#include "c_mediaplayer.h"

// GStreamer playbin documents a enum that is not defined anywhere!?
// If someone knows where I can find it...

typedef enum {
  GST_PLAY_FLAG_VIDEO         = (1 << 0),
  GST_PLAY_FLAG_AUDIO         = (1 << 1),
  GST_PLAY_FLAG_TEXT          = (1 << 2),
  GST_PLAY_FLAG_VIS           = (1 << 3),
  GST_PLAY_FLAG_SOFT_VOLUME   = (1 << 4),
  GST_PLAY_FLAG_NATIVE_AUDIO  = (1 << 5),
  GST_PLAY_FLAG_NATIVE_VIDEO  = (1 << 6),
  GST_PLAY_FLAG_DOWNLOAD      = (1 << 7),
  GST_PLAY_FLAG_BUFFERING     = (1 << 8),
  GST_PLAY_FLAG_DEINTERLACE   = (1 << 9)
} GstPlayFlags;


static int get_int(CMEDIAPLAYER *_object, char *name)
{
	int value;
	g_object_get(G_OBJECT(ELEMENT), name, &value, NULL);
	return value;
}

static void set_int(CMEDIAPLAYER *_object, char *name, int value)
{
	g_object_set(G_OBJECT(ELEMENT), name, value, NULL);
}

static int get_int64(CMEDIAPLAYER *_object, char *name)
{
	gint64 value;
	g_object_get(G_OBJECT(ELEMENT), name, &value, NULL);
	return value;
}

static void set_int64(CMEDIAPLAYER *_object, char *name, gint64 value)
{
	g_object_set(G_OBJECT(ELEMENT), name, value, NULL);
}

static double get_double(CMEDIAPLAYER *_object, char *name)
{
	double value;
	g_object_get(G_OBJECT(ELEMENT), name, &value, NULL);
	return value;
}

static void set_double(CMEDIAPLAYER *_object, char *name, double value)
{
	g_object_set(G_OBJECT(ELEMENT), name, value, NULL);
}

static gboolean get_boolean(CMEDIAPLAYER *_object, char *name)
{
	gboolean value;
	g_object_get(G_OBJECT(ELEMENT), name, &value, NULL);
	return value;
}

static void set_boolean(CMEDIAPLAYER *_object, char *name, gboolean value)
{
	g_object_set(G_OBJECT(ELEMENT), name, value, NULL);
}

static char *get_string(CMEDIAPLAYER *_object, char *name)
{
	gchar *value;
	g_object_get(G_OBJECT(ELEMENT), name, &value, NULL);
	return value;
}

static void set_string(CMEDIAPLAYER *_object, char *name, gchar *value)
{
	g_object_set(G_OBJECT(ELEMENT), name, value, NULL);
}

static CMEDIACONTROL *get_control(CMEDIAPLAYER *_object, char *name)
{
	GstElement *value;
	g_object_get(G_OBJECT(ELEMENT), name, &value, NULL);
	return MEDIA_get_control_from_element(value, TRUE);
}

static void set_control(CMEDIAPLAYER *_object, char *name, CMEDIACONTROL *control)
{
	GstElement *elt;
	GstBin *parent;
	
	if (!control)
	{
		g_object_set(G_OBJECT(ELEMENT), name, NULL, NULL);
		return;
	}
	
	elt = control->elt;
	parent = GST_BIN(gst_element_get_parent(elt));
	if (parent)
	{
		gst_object_ref(elt);
		gst_bin_remove(parent, elt);
	}
	
	g_object_set(G_OBJECT(ELEMENT), name, elt, NULL);
	
	if (parent)
		gst_object_unref(elt);
}

#define set_flag(_object, _flag, _value) MEDIA_set_flag(ELEMENT, "flags", _flag, _value)

#define IMPLEMENT_FLAG(_func, _flag) \
BEGIN_PROPERTY(_func) \
\
	if (READ_PROPERTY) \
		GB.ReturnBoolean(MEDIA_get_flag(ELEMENT, "flags", _flag)); \
	else \
		MEDIA_set_flag(ELEMENT, "flags", _flag, VPROP(GB_BOOLEAN)); \
\
END_PROPERTY

static GB_IMG *get_frame(void *_object)
{
	GstElement *play = GST_ELEMENT(ELEMENT);
	GstSample *sample;
	GstCaps *to_caps;
	const char *format;

	//g_return_val_if_fail (play != NULL, NULL);
	//g_return_val_if_fail (GST_IS_ELEMENT (play), NULL);

	switch (IMAGE.GetDefaultFormat())
	{
		case GB_IMAGE_BGRA:
			format = "BGR";
			break;

		case GB_IMAGE_RGBA:
			format = "RGB";
			break;

		default:
			GB.Error("Unsupported default image format");
			return NULL;
	}

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

	/* get frame */
	g_signal_emit_by_name (play, "convert-sample", to_caps, &sample);
	gst_caps_unref(to_caps);

	return MEDIA_get_image_from_sample(sample, FALSE);
}


//---- MediaPlayerAudio --------------------------------------------------

BEGIN_PROPERTY(MediaPlayerAudio_Count)

	GB.ReturnInteger(get_int(THIS, "n-audio"));

END_PROPERTY

BEGIN_PROPERTY(MediaPlayerAudio_Current)

	if (READ_PROPERTY)
		GB.ReturnInteger(get_int(THIS, "current-audio"));
	else
		set_int(THIS, "current-audio", VPROP(GB_INTEGER));

END_PROPERTY

IMPLEMENT_FLAG(MediaPlayerAudio_Enabled, GST_PLAY_FLAG_AUDIO)
IMPLEMENT_FLAG(MediaPlayerAudio_SoftwareVolume, GST_PLAY_FLAG_SOFT_VOLUME)
IMPLEMENT_FLAG(MediaPlayerAudio_NativeOnly, GST_PLAY_FLAG_NATIVE_AUDIO)

BEGIN_PROPERTY(MediaPlayerAudio_Volume)

	if (READ_PROPERTY)
		GB.ReturnFloat(get_double(THIS, "volume"));
	else
		set_double(THIS, "volume", VPROP(GB_FLOAT));

END_PROPERTY

BEGIN_PROPERTY(MediaPlayerAudio_Mute)

	if (READ_PROPERTY)
		GB.ReturnBoolean(get_boolean(THIS, "mute"));
	else
		set_boolean(THIS, "mute", VPROP(GB_BOOLEAN) != 0);

END_PROPERTY

BEGIN_PROPERTY(MediaPlayerAudio_Offset)

	if (READ_PROPERTY)
		GB.ReturnFloat(TO_SECOND(get_int64(THIS, "av-offset")));
	else
		set_int64(THIS, "av-offset", TO_TIME(VPROP(GB_FLOAT)));

END_PROPERTY

BEGIN_PROPERTY(MediaPlayerAudio_Output)

	if (READ_PROPERTY)
		GB.ReturnObject(get_control(THIS, "audio-sink"));
	else
		set_control(THIS, "audio-sink", VPROP(GB_OBJECT));

END_PROPERTY

//---- MediaPlayerVideo --------------------------------------------------

BEGIN_PROPERTY(MediaPlayerVideo_Count)

	GB.ReturnInteger(get_int(THIS, "n-video"));

END_PROPERTY

BEGIN_PROPERTY(MediaPlayerVideo_Current)

	if (READ_PROPERTY)
		GB.ReturnInteger(get_int(THIS, "current-video"));
	else
		set_int(THIS, "current-video", VPROP(GB_INTEGER));

END_PROPERTY

IMPLEMENT_FLAG(MediaPlayerVideo_Enabled, GST_PLAY_FLAG_VIDEO)
IMPLEMENT_FLAG(MediaPlayerVideo_NativeOnly, GST_PLAY_FLAG_NATIVE_VIDEO)
IMPLEMENT_FLAG(MediaPlayerVideo_Deinterlace, GST_PLAY_FLAG_DEINTERLACE)

BEGIN_PROPERTY(MediaPlayerVideo_Output)

	if (READ_PROPERTY)
		GB.ReturnObject(get_control(THIS, "video-sink"));
	else
		set_control(THIS, "video-sink", VPROP(GB_OBJECT));

END_PROPERTY

BEGIN_PROPERTY(MediaPlayerVideo_Visualisation)

	if (READ_PROPERTY)
		GB.ReturnObject(get_control(THIS, "vis-plugin"));
	else
	{
		CMEDIACONTROL *vis = VPROP(GB_OBJECT);
		//CMEDIACONTROL *old = get_control(THIS, "vis-plugin");
		bool playing;
		
		set_flag(THIS, GST_PLAY_FLAG_VIS, FALSE);
		
		playing = THIS_CONTROL->state == GST_STATE_PLAYING;
		if (playing)
			MEDIA_set_state(THIS, GST_STATE_PAUSED, FALSE);
		
		//set_control(THIS, "vis-plugin", NULL);
		//if (vis)
		set_control(THIS, "vis-plugin", vis);
		if (vis) 
			set_flag(THIS, GST_PLAY_FLAG_VIS, TRUE);
		
		if (playing)
			MEDIA_set_state(THIS, GST_STATE_PLAYING, FALSE);
	}

END_PROPERTY

BEGIN_PROPERTY(MediaPlayerVideo_Image)

	GB.ReturnObject(get_frame(THIS));

END_PROPERTY

//---- MediaPlayerSubtitles ----------------------------------------------

BEGIN_PROPERTY(MediaPlayerSubtitles_Count)

	GB.ReturnInteger(get_int(THIS, "n-text"));

END_PROPERTY

BEGIN_PROPERTY(MediaPlayerSubtitles_Current)

	if (READ_PROPERTY)
		GB.ReturnInteger(get_int(THIS, "current-text"));
	else
		set_int(THIS, "current-text", VPROP(GB_INTEGER));

END_PROPERTY

IMPLEMENT_FLAG(MediaPlayerSubtitles_Enabled, GST_PLAY_FLAG_TEXT)

BEGIN_PROPERTY(MediaPlayerSubtitles_Charset)

	if (READ_PROPERTY)
	{
		char *charset = get_string(THIS, "subtitle-encoding");
		GB.ReturnNewZeroString(charset);
		g_free(charset);
	}
	else
		set_string(THIS, "subtitle-encoding", GB.ToZeroString(PROP(GB_STRING)));

END_PROPERTY

BEGIN_PROPERTY(MediaPlayerSubtitles_URL)

	if (READ_PROPERTY)
	{
		char *charset = get_string(THIS, "suburi");
		GB.ReturnNewZeroString(charset);
		g_free(charset);
	}
	else
		set_string(THIS, "suburi", GB.ToZeroString(PROP(GB_STRING)));

END_PROPERTY

BEGIN_PROPERTY(MediaPlayerSubtitles_Output)

	if (READ_PROPERTY)
		GB.ReturnObject(get_control(THIS, "text-sink"));
	else
		set_control(THIS, "text-sink", VPROP(GB_OBJECT));

END_PROPERTY

//---- MediaPlayerBalanceChannel -----------------------------------------

static GstColorBalanceChannel *get_channel(void *_object)
{
	GList *channels = (GList *)gst_color_balance_list_channels(BALANCE);
	GstColorBalanceChannel *channel = (GstColorBalanceChannel *)g_list_nth_data(channels, THIS->channel);
	if (!channel) GB.Error(GB_ERR_ARG);
	return channel;
}

BEGIN_PROPERTY(MediaPlayerBalanceChannel_Name)

	GstColorBalanceChannel *channel = get_channel(THIS);
	if (channel)
		GB.ReturnNewZeroString(channel->label);

END_PROPERTY

BEGIN_PROPERTY(MediaPlayerBalanceChannel_Min)

	GstColorBalanceChannel *channel = get_channel(THIS);
	if (channel)
		GB.ReturnInteger(channel->min_value);

END_PROPERTY

BEGIN_PROPERTY(MediaPlayerBalanceChannel_Max)

	GstColorBalanceChannel *channel = get_channel(THIS);
	if (channel)
		GB.ReturnInteger(channel->max_value);

END_PROPERTY

BEGIN_PROPERTY(MediaPlayerBalanceChannel_Value)

	GstColorBalanceChannel *channel = get_channel(THIS);
	if (!channel)
		return;
	
	if (READ_PROPERTY)
		GB.ReturnInteger(gst_color_balance_get_value(BALANCE, channel));
	else
		gst_color_balance_set_value(BALANCE, channel, VPROP(GB_INTEGER));
	
END_PROPERTY

//---- MediaPlayerBalance ------------------------------------------------

BEGIN_PROPERTY(MediaPlayerBalance_Count)

	GB.ReturnInteger(g_list_length((GList *)gst_color_balance_list_channels(BALANCE)));

END_PROPERTY

BEGIN_PROPERTY(MediaPlayerBalance_Hardware)

	GB.ReturnBoolean(gst_color_balance_get_balance_type(BALANCE) == GST_COLOR_BALANCE_HARDWARE);
	
END_PROPERTY

BEGIN_METHOD(MediaPlayerBalance_get, GB_INTEGER index)

	GList *channels = (GList *)gst_color_balance_list_channels(BALANCE);
	int index = VARG(index);
	
	if (index < 0 || index >= g_list_length(channels))
		GB.Error(GB_ERR_BOUND);
	else
	{
		THIS->channel = index;
		RETURN_SELF();
	}

END_PROPERTY

//---- MediaPlayer -------------------------------------------------------

DECLARE_EVENT(EVENT_AboutToFinish);
DECLARE_EVENT(EVENT_AudioChanged);
DECLARE_EVENT(EVENT_SubtitlesChanged);
DECLARE_EVENT(EVENT_VideoChanged);
//DECLARE_EVENT(EVENT_SourceSetup);

static void cb_about_to_finish(void *playbin, void *_object)
{
	MEDIA_raise_event(THIS, EVENT_AboutToFinish);
}

static void cb_audio_changed(void *playbin, void *_object)
{
	MEDIA_raise_event(THIS, EVENT_AudioChanged);
}

static void cb_text_changed(void *playbin, void *_object)
{
	MEDIA_raise_event(THIS, EVENT_SubtitlesChanged);
}

static void cb_video_changed(void *playbin, void *_object)
{
	MEDIA_raise_event(THIS, EVENT_VideoChanged);
}

/*static void cb_source_setup(void *playbin, GstElement *source, void *_object)
{
	MEDIA_raise_event(THIS, EVENT_SourceSetup);
}*/

BEGIN_METHOD_VOID(MediaPlayer_new)

	g_signal_connect(ELEMENT, "about-to-finish", G_CALLBACK(cb_about_to_finish), THIS);
	g_signal_connect(ELEMENT, "audio-changed", G_CALLBACK(cb_audio_changed), THIS);
	g_signal_connect(ELEMENT, "text-changed", G_CALLBACK(cb_text_changed), THIS);
	g_signal_connect(ELEMENT, "video-changed", G_CALLBACK(cb_video_changed), THIS);
	//g_signal_connect(ELEMENT, "source-setup", G_CALLBACK(cb_source_setup), THIS);

END_METHOD

BEGIN_PROPERTY(MediaPlayer_ConnectionSpeed)

	if (READ_PROPERTY)
		GB.ReturnLong(get_int64(THIS, "connection-speed"));
	else
		set_int64(THIS, "connection-speed", VPROP(GB_LONG));

END_PROPERTY

IMPLEMENT_FLAG(MediaPlayer_ProgressiveDownload, GST_PLAY_FLAG_DOWNLOAD)
IMPLEMENT_FLAG(MediaPlayer_Buffering, GST_PLAY_FLAG_BUFFERING)

BEGIN_PROPERTY(MediaPlayer_URL)

	if (READ_PROPERTY)
	{
		char *charset = get_string(THIS, "uri");
		GB.ReturnNewZeroString(charset);
		g_free(charset);
	}
	else
		set_string(THIS, "uri", GB.ToZeroString(PROP(GB_STRING)));

END_PROPERTY

BEGIN_PROPERTY(MediaPlayer_Input)

	GB.ReturnObject(get_control(THIS, "source"));

END_PROPERTY

//-------------------------------------------------------------------------

GB_DESC MediaPlayerAudioDesc[] = 
{
	GB_DECLARE(".MediaPlayer.Audio", sizeof(CMEDIAPLAYER)),
	GB_VIRTUAL_CLASS(),
	
	GB_PROPERTY_READ("Count", "i", MediaPlayerAudio_Count),
	GB_PROPERTY("Current", "i", MediaPlayerAudio_Current),
	GB_PROPERTY("Enabled", "b", MediaPlayerAudio_Enabled),
	GB_PROPERTY("SoftwareVolume", "b", MediaPlayerAudio_SoftwareVolume),
	GB_PROPERTY("NativeOnly", "b", MediaPlayerAudio_NativeOnly),
	GB_PROPERTY("Volume", "f", MediaPlayerAudio_Volume),
	GB_PROPERTY("Mute", "b", MediaPlayerAudio_Mute),
	GB_PROPERTY("Offset", "f", MediaPlayerAudio_Offset),
	GB_PROPERTY("Output", "MediaControl", MediaPlayerAudio_Output),
	
	GB_END_DECLARE
};

GB_DESC MediaPlayerVideoDesc[] = 
{
	GB_DECLARE(".MediaPlayer.Video", sizeof(CMEDIAPLAYER)),
	GB_VIRTUAL_CLASS(),
	
	GB_PROPERTY_READ("Count", "i", MediaPlayerVideo_Count),
	GB_PROPERTY("Current", "i", MediaPlayerVideo_Current),
	GB_PROPERTY("Enabled", "b", MediaPlayerVideo_Enabled),
	GB_PROPERTY("NativeOnly", "b", MediaPlayerVideo_NativeOnly),
	GB_PROPERTY("Deinterlace", "b", MediaPlayerVideo_Deinterlace),
	GB_PROPERTY("Output", "MediaControl", MediaPlayerVideo_Output),
	GB_PROPERTY("Visualisation", "MediaControl", MediaPlayerVideo_Visualisation),
	GB_PROPERTY_READ("Image", "Image", MediaPlayerVideo_Image),
	
	GB_END_DECLARE
};

GB_DESC MediaPlayerSubtitlesDesc[] = 
{
	GB_DECLARE(".MediaPlayer.Subtitles", sizeof(CMEDIAPLAYER)),
	GB_VIRTUAL_CLASS(),
	
	GB_PROPERTY_READ("Count", "i", MediaPlayerSubtitles_Count),
	GB_PROPERTY("Current", "i", MediaPlayerSubtitles_Current),
	GB_PROPERTY("Enabled", "b", MediaPlayerSubtitles_Enabled),
	GB_PROPERTY("Charset", "s", MediaPlayerSubtitles_Charset),
	GB_PROPERTY("URL", "s", MediaPlayerSubtitles_URL),
	GB_PROPERTY("Output", "MediaControl", MediaPlayerSubtitles_Output),
	
	GB_END_DECLARE
};

GB_DESC MediaPlayerBalanceChannelDesc[] = 
{
	GB_DECLARE(".MediaPlayer.Balance.Channel", sizeof(CMEDIAPLAYER)),
	GB_VIRTUAL_CLASS(),
	
	GB_PROPERTY_READ("Name", "s", MediaPlayerBalanceChannel_Name),
	GB_PROPERTY_READ("Min", "i", MediaPlayerBalanceChannel_Min),
	GB_PROPERTY_READ("Max", "i", MediaPlayerBalanceChannel_Max),
	GB_PROPERTY("Value", "i", MediaPlayerBalanceChannel_Value),
	
	GB_END_DECLARE
};

GB_DESC MediaPlayerBalanceDesc[] = 
{
	GB_DECLARE(".MediaPlayer.Balance", sizeof(CMEDIAPLAYER)),
	GB_VIRTUAL_CLASS(),
	
	GB_PROPERTY_READ("Count", "i", MediaPlayerBalance_Count),
	GB_PROPERTY_READ("Hardware", "b", MediaPlayerBalance_Hardware),
	GB_METHOD("_get", ".MediaPlayer.Balance.Channel", MediaPlayerBalance_get, "(Channel)i"),
	
	GB_END_DECLARE
};

GB_DESC MediaPlayerDesc[] = 
{
	GB_DECLARE("MediaPlayer", sizeof(CMEDIAPLAYER)),
	GB_INHERITS("MediaPipeline"),
	
	GB_METHOD("_new", NULL, MediaPlayer_new, NULL),
	
	GB_PROPERTY_SELF("Audio", ".MediaPlayer.Audio"),
	GB_PROPERTY_SELF("Video", ".MediaPlayer.Video"),
	GB_PROPERTY_SELF("Subtitles", ".MediaPlayer.Subtitles"),
	GB_PROPERTY_SELF("Balance", ".MediaPlayer.Balance"),
	//GB_PROPERTY_SELF("Visualisation", ".MediaPlayer.Visualisation"),
	
	GB_PROPERTY("ConnectionSpeed", "l", MediaPlayer_ConnectionSpeed),
	GB_PROPERTY("ProgressiveDownload", "b", MediaPlayer_ProgressiveDownload),
	GB_PROPERTY("Buffering", "b", MediaPlayer_Buffering),
	GB_PROPERTY("URL", "s", MediaPlayer_URL),
	GB_PROPERTY_READ("Input", "MediaControl", MediaPlayer_Input),
	
	GB_EVENT("AboutToFinish", NULL, NULL, &EVENT_AboutToFinish),
	GB_EVENT("AudioChanged", NULL, NULL, &EVENT_AudioChanged),
	GB_EVENT("SubtitlesChanged", NULL, NULL, &EVENT_SubtitlesChanged),
	GB_EVENT("VideoChanged", NULL, NULL, &EVENT_VideoChanged),
	//GB_EVENT("SourceSetup", NULL, NULL, &EVENT_SourceSetup),
	
	GB_END_DECLARE
};

