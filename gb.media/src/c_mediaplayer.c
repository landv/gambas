/***************************************************************************

  c_mediaplayer.c

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

#define __C_MEDIAPLAYER_C

#include "c_mediaplayer.h"

// GStreamer playbin2 documents a enum that is not defined anywhere...
// I have never see such a thing, and I have no idea why they did that!

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

static bool get_flag(CMEDIAPLAYER *_object, int flag)
{
	GstPlayFlags flags;
	
	g_object_get(G_OBJECT(ELEMENT), "flags", &flags, NULL);
	return (flags & flag) != 0;
}

static void set_flag(CMEDIAPLAYER *_object, int flag, bool value)
{
	GstPlayFlags flags;
	
	g_object_get(G_OBJECT(ELEMENT), "flags", &flags, NULL);
	if (value)
		flags |= flag;
	else
		flags &= ~flag;
	g_object_set(G_OBJECT(ELEMENT), "flags", flags, NULL);
}

#define IMPLEMENT_FLAG(_func, _flag) \
BEGIN_PROPERTY(_func) \
\
	if (READ_PROPERTY) \
		GB.ReturnBoolean(get_flag(THIS, _flag)); \
	else \
		set_flag(THIS, _flag, VPROP(GB_BOOLEAN)); \
\
END_PROPERTY

//---- MediaPlayerAudio --------------------------------------------------

BEGIN_PROPERTY(MediaPlayerAudio_Count)

	GB.ReturnInteger(get_int(THIS, "n-audio"));

END_PROPERTY

BEGIN_PROPERTY(MediaPlayerAudio_Current)

	if (READ_PROPERTY)
		GB.ReturnInteger(get_int(THIS, "audio-current"));
	else
		set_int(THIS, "audio-current", VPROP(GB_INTEGER));

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
		GB.ReturnFloat(get_boolean(THIS, "mute"));
	else
		set_boolean(THIS, "mute", VPROP(GB_BOOLEAN));

END_PROPERTY

BEGIN_PROPERTY(MediaPlayerAudio_Offset)

	if (READ_PROPERTY)
		GB.ReturnFloat(TO_SECOND(get_int64(THIS, "av-offset")));
	else
		set_int64(THIS, "av-offset", TO_TIME(VPROP(GB_FLOAT)));

END_PROPERTY

//---- MediaPlayerVideo --------------------------------------------------

BEGIN_PROPERTY(MediaPlayerVideo_Count)

	GB.ReturnInteger(get_int(THIS, "n-video"));

END_PROPERTY

BEGIN_PROPERTY(MediaPlayerVideo_Current)

	if (READ_PROPERTY)
		GB.ReturnInteger(get_int(THIS, "video-current"));
	else
		set_int(THIS, "video-current", VPROP(GB_INTEGER));

END_PROPERTY

IMPLEMENT_FLAG(MediaPlayerVideo_Enabled, GST_PLAY_FLAG_VIDEO)
IMPLEMENT_FLAG(MediaPlayerVideo_NativeOnly, GST_PLAY_FLAG_NATIVE_VIDEO)
IMPLEMENT_FLAG(MediaPlayerVideo_Visualisation, GST_PLAY_FLAG_VIS)
IMPLEMENT_FLAG(MediaPlayerVideo_Deinterlace, GST_PLAY_FLAG_DEINTERLACE)

//---- MediaPlayerSubtitles ----------------------------------------------

BEGIN_PROPERTY(MediaPlayerSubtitles_Count)

	GB.ReturnInteger(get_int(THIS, "n-text"));

END_PROPERTY

BEGIN_PROPERTY(MediaPlayerSubtitles_Current)

	if (READ_PROPERTY)
		GB.ReturnInteger(get_int(THIS, "text-current"));
	else
		set_int(THIS, "text-current", VPROP(GB_INTEGER));

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

//---- MediaPlayer -------------------------------------------------------

DECLARE_EVENT(EVENT_AboutToFinish);
DECLARE_EVENT(EVENT_AudioChanged);
DECLARE_EVENT(EVENT_SubtitlesChanged);
DECLARE_EVENT(EVENT_VideoChanged);
DECLARE_EVENT(EVENT_SourceSetup);

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

static void cb_source_setup(void *playbin, GstElement *source, void *_object)
{
	MEDIA_raise_event(THIS, EVENT_SourceSetup);
}

BEGIN_METHOD_VOID(MediaPlayer_new)

	g_signal_connect(ELEMENT, "about-to-finish", G_CALLBACK(cb_about_to_finish), THIS);
	g_signal_connect(ELEMENT, "audio-changed", G_CALLBACK(cb_audio_changed), THIS);
	g_signal_connect(ELEMENT, "text-changed", G_CALLBACK(cb_text_changed), THIS);
	g_signal_connect(ELEMENT, "video-changed", G_CALLBACK(cb_video_changed), THIS);
	g_signal_connect(ELEMENT, "source-setup", G_CALLBACK(cb_source_setup), THIS);

END_METHOD

BEGIN_PROPERTY(MediaPlayer_ConnectionSpeed)

	if (READ_PROPERTY)
		GB.ReturnInteger(get_int(THIS, "connection-speed"));
	else
		set_int(THIS, "connection-speed", VPROP(GB_INTEGER));

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
	GB_PROPERTY("Visualisation", "b", MediaPlayerVideo_Visualisation),
	GB_PROPERTY("Deinterlace", "b", MediaPlayerVideo_Deinterlace),
	
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
	
	GB_PROPERTY("ConnectionSpeed", "i", MediaPlayer_ConnectionSpeed),
	GB_PROPERTY("ProgressiveDownload", "b", MediaPlayer_ProgressiveDownload),
	GB_PROPERTY("Buffering", "b", MediaPlayer_Buffering),
	GB_PROPERTY("URL", "s", MediaPlayer_URL),
	
	GB_EVENT("AboutToFinish", NULL, NULL, &EVENT_AboutToFinish),
	GB_EVENT("AudioChanged", NULL, NULL, &EVENT_AudioChanged),
	GB_EVENT("SubtitlesChanged", NULL, NULL, &EVENT_SubtitlesChanged),
	GB_EVENT("VideoChanged", NULL, NULL, &EVENT_VideoChanged),
	GB_EVENT("SourceSetup", NULL, NULL, &EVENT_SourceSetup),
	
	GB_END_DECLARE
};

