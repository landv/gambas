/***************************************************************************

  c_al.c

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

#define __C_AL_C

#include "c_al.h"

/*
 * LISTENER
 * Listener represents the location and orientation of the
 * 'user' in 3D-space.
 *
 * Properties include: -
 *
 * Gain         AL_GAIN         ALfloat
 * Position     AL_POSITION     ALfloat[3]
 * Velocity     AL_VELOCITY     ALfloat[3]
 * Orientation  AL_ORIENTATION  ALfloat[6] (Forward then Up vectors)
*/

static int get_listener_float_param_size(int param)
{
	switch(param)
	{
		case AL_GAIN: return 1;
		case AL_POSITION: return 3;
		case AL_VELOCITY: return 3;
		case AL_ORIENTATION: return 6;
		default: return 0;
	}
}

static int get_listener_integer_param_size(int param)
{
	return 0;
}

/*
 * SOURCE
 * Sources represent individual sound objects in 3D-space.
 * Sources take the PCM data provided in the specified Buffer,
 * apply Source-specific modifications, and then
 * submit them to be mixed according to spatial arrangement etc.
 *
 * Properties include: -
 *
 * Gain                              AL_GAIN                 ALfloat
 * Min Gain                          AL_MIN_GAIN             ALfloat
 * Max Gain                          AL_MAX_GAIN             ALfloat
 * Position                          AL_POSITION             ALfloat[3]
 * Velocity                          AL_VELOCITY             ALfloat[3]
 * Direction                         AL_DIRECTION            ALfloat[3]
 * Head Relative Mode                AL_SOURCE_RELATIVE      ALint (AL_TRUE or AL_FALSE)
 * Reference Distance                AL_REFERENCE_DISTANCE   ALfloat
 * Max Distance                      AL_MAX_DISTANCE         ALfloat
 * RollOff Factor                    AL_ROLLOFF_FACTOR       ALfloat
 * Inner Angle                       AL_CONE_INNER_ANGLE     ALint or ALfloat
 * Outer Angle                       AL_CONE_OUTER_ANGLE     ALint or ALfloat
 * Cone Outer Gain                   AL_CONE_OUTER_GAIN      ALint or ALfloat
 * Pitch                             AL_PITCH                ALfloat
 * Looping                           AL_LOOPING              ALint (AL_TRUE or AL_FALSE)
 * MS Offset                         AL_MSEC_OFFSET          ALint or ALfloat
 * Byte Offset                       AL_BYTE_OFFSET          ALint or ALfloat
 * Sample Offset                     AL_SAMPLE_OFFSET        ALint or ALfloat
 * Attached Buffer                   AL_BUFFER               ALint
 * State (Query only)                AL_SOURCE_STATE         ALint
 * Buffers Queued (Query only)       AL_BUFFERS_QUEUED       ALint
 * Buffers Processed (Query only)    AL_BUFFERS_PROCESSED    ALint
 */

static int get_source_float_param_size(int param)
{
	switch(param)
	{
		case AL_GAIN: case AL_MIN_GAIN: case AL_MAX_GAIN: case AL_REFERENCE_DISTANCE: case AL_MAX_DISTANCE: case AL_ROLLOFF_FACTOR:
		case AL_CONE_INNER_ANGLE: case AL_CONE_OUTER_ANGLE: case AL_CONE_OUTER_GAIN: case AL_PITCH:  case AL_BYTE_OFFSET:
		case AL_SAMPLE_OFFSET:
		//case AL_MSEC_OFFSET:
			return 1;

		case AL_POSITION: case AL_VELOCITY: case AL_DIRECTION:
			return 3;

		default:
			return 0;
	}
}

static int get_source_integer_param_size(int param)
{
	switch(param)
	{
		case AL_SOURCE_RELATIVE: case AL_CONE_INNER_ANGLE: case AL_CONE_OUTER_ANGLE: case AL_CONE_OUTER_GAIN: case AL_LOOPING:
		//case AL_MSEC_OFFSET:
		case AL_BYTE_OFFSET: case AL_SAMPLE_OFFSET: case AL_BUFFER: case AL_SOURCE_STATE: case AL_BUFFERS_QUEUED: case AL_BUFFERS_PROCESSED:
			return 1;

		default:
			return 0;
	}
}

/*
 * BUFFER
 * Buffer objects are storage space for sample data.
 * Buffers are referred to by Sources. One Buffer can be used
 * by multiple Sources.
 *
 * Properties include: -
 *
 * Frequency (Query only)    AL_FREQUENCY      ALint
 * Size (Query only)         AL_SIZE           ALint
 * Bits (Query only)         AL_BITS           ALint
 * Channels (Query only)     AL_CHANNELS       ALint
 */

static int get_buffer_float_param_size(int param)
{
	return 0;
}

static int get_buffer_integer_param_size(int param)
{
	switch(param)
	{
		case AL_FREQUENCY: case AL_SIZE: case AL_BITS: case AL_CHANNELS:
			return 1;

		default:
			return 0;
	}
}


//---------------------------------------------------------------------------


BEGIN_METHOD_VOID(AL_GetError)

	GB.ReturnInteger(alGetError());

END_METHOD

BEGIN_METHOD(AL_Enable, GB_INTEGER cap)

	alEnable(VARG(cap));

END_METHOD

BEGIN_METHOD(AL_Disable, GB_INTEGER cap)

	alDisable(VARG(cap));

END_METHOD

BEGIN_METHOD(AL_IsEnabled, GB_INTEGER cap)

	GB.ReturnBoolean(alIsEnabled(VARG(cap)));

END_METHOD

BEGIN_METHOD(AL_GenBuffers, GB_INTEGER count)

	int count = VARG(count);
	GB_ARRAY array;

	if (count <= 0)
	{
		GB.ReturnNull();
		return;
	}

	GB.Array.New(&array, GB_T_INTEGER, count);
	alGenBuffers(VARG(count), (ALuint *)GB.Array.Get(array, 0));
	GB.ReturnObject(array);

END_METHOD

BEGIN_METHOD(AL_GenSources, GB_INTEGER count)

	int count = VARG(count);
	GB_ARRAY array;

	if (count <= 0)
	{
		GB.ReturnNull();
		return;
	}

	GB.Array.New(&array, GB_T_INTEGER, count);
	alGenSources(VARG(count), (ALuint *)GB.Array.Get(array, 0));
	GB.ReturnObject(array);

END_METHOD

BEGIN_METHOD(AL_DeleteBuffers, GB_OBJECT array)

	GB_ARRAY array = (GB_ARRAY)VARG(array);
	int count;
	
	if (GB.CheckObject(array))
		return;
	
	count = GB.Array.Count(array);
	if (count <= 0)
		return;
	
	alDeleteBuffers(count, GB.Array.Get(array, 0));

END_METHOD

BEGIN_METHOD(AL_DeleteSources, GB_OBJECT array)

	GB_ARRAY array = (GB_ARRAY)VARG(array);
	int count;
	
	if (GB.CheckObject(array))
		return;
	
	count = GB.Array.Count(array);
	if (count <= 0)
		return;
	
	alDeleteSources(count, GB.Array.Get(array, 0));

END_METHOD

BEGIN_METHOD(AL_IsBuffer, GB_INTEGER name)

	GB.ReturnBoolean(alIsBuffer(VARG(name)));

END_METHOD

BEGIN_METHOD(AL_IsSource, GB_INTEGER name)

	GB.ReturnBoolean(alIsSource(VARG(name)));

END_METHOD

BEGIN_METHOD(AL_GetBoolean, GB_INTEGER param)

	GB.ReturnBoolean(alGetBoolean(VARG(param)));

END_METHOD

BEGIN_METHOD(AL_GetInteger, GB_INTEGER param)

	GB.ReturnInteger(alGetInteger(VARG(param)));

END_METHOD

BEGIN_METHOD(AL_GetFloat, GB_INTEGER param)

	GB.ReturnSingle(alGetFloat(VARG(param)));

END_METHOD

BEGIN_METHOD(AL_GetDouble, GB_INTEGER param)

	GB.ReturnFloat(alGetDouble(VARG(param)));

END_METHOD

BEGIN_METHOD(AL_GetString, GB_INTEGER param)

	GB.ReturnNewZeroString(alGetString(VARG(param)));

END_METHOD

BEGIN_METHOD(AL_IsExtensionPresent, GB_STRING name)

	GB.ReturnBoolean(alIsExtensionPresent(GB.ToZeroString(ARG(name))));

END_METHOD

BEGIN_METHOD(AL_GetEnumValue, GB_STRING name)

	GB.ReturnInteger(alGetEnumValue(GB.ToZeroString(ARG(name))));

END_METHOD

BEGIN_METHOD(AL_DistanceModel, GB_INTEGER model)

	alDistanceModel(VARG(model));

END_METHOD

BEGIN_METHOD(AL_DopplerFactor, GB_FLOAT factor)

	alDopplerFactor((float)VARG(factor));

END_METHOD

BEGIN_METHOD(AL_DopplerVelocity, GB_FLOAT velocity)

	alDopplerVelocity((float)VARG(velocity));

END_METHOD

BEGIN_METHOD(AL_SpeedOfSound, GB_FLOAT speed)

	alSpeedOfSound((float)VARG(speed));

END_METHOD

#define IMPLEMENT_X(_name, _type) \
BEGIN_METHOD(AL_##_name, ID_PARAM GB_INTEGER param; _type value) \
  al##_name(ID_ARG VARG(param), VARG(value)); \
END_METHOD

#define IMPLEMENT_3X(_name, _type) \
BEGIN_METHOD(AL_##_name, ID_PARAM GB_INTEGER param; _type value1; _type value2; _type value3) \
  al##_name(ID_ARG VARG(param), VARG(value1), VARG(value2), VARG(value3)); \
END_METHOD

#define IMPLEMENT_XV(_name, _type) \
BEGIN_METHOD(AL_##_name, ID_PARAM GB_INTEGER param; GB_OBJECT array) \
	GB_ARRAY array = VARG(array); \
	if (GB.CheckObject(array)) \
		return; \
	al##_name(ID_ARG VARG(param), GB.Array.Get(array, 0)); \
END_METHOD

#define IMPLEMENT_GET_X(_name, _return, _ctype) \
BEGIN_METHOD(AL_##_name, ID_PARAM GB_INTEGER param) \
	_ctype val; \
	al##_name(ID_ARG VARG(param), &val); \
	GB.Return##_return(val); \
END_METHOD

#define IMPLEMENT_GET_XV(_name, _type, _get_size) \
BEGIN_METHOD(AL_##_name, ID_PARAM GB_INTEGER param) \
	GB_ARRAY array; \
	int size = _get_size(VARG(param)); \
	if (size == 0) \
		GB.ReturnNull(); \
	else \
	{ \
		GB.Array.New(&array, _type, size); \
		al##_name(ID_ARG VARG(param), GB.Array.Get(array, 0)); \
		GB.ReturnObject(array); \
	} \
END_METHOD

#define ID_PARAM
#define ID_ARG

IMPLEMENT_X(Listenerf, GB_SINGLE)
IMPLEMENT_3X(Listener3f, GB_SINGLE)
IMPLEMENT_XV(Listenerfv, GB_SINGLE)
IMPLEMENT_X(Listeneri, GB_INTEGER)
IMPLEMENT_3X(Listener3i, GB_INTEGER)
IMPLEMENT_XV(Listeneriv, GB_INTEGER)

IMPLEMENT_GET_X(GetListenerf, Single, ALfloat)
IMPLEMENT_GET_XV(GetListenerfv, GB_T_SINGLE, get_listener_float_param_size)
IMPLEMENT_GET_X(GetListeneri, Single, ALint)
IMPLEMENT_GET_XV(GetListeneriv, GB_T_INTEGER, get_listener_integer_param_size)

#undef ID_PARAM
#define ID_PARAM GB_INTEGER id;
#undef ID_ARG
#define ID_ARG VARG(id),

IMPLEMENT_X(Sourcef, GB_SINGLE)
IMPLEMENT_3X(Source3f, GB_SINGLE)
IMPLEMENT_XV(Sourcefv, GB_SINGLE)
IMPLEMENT_X(Sourcei, GB_INTEGER)
IMPLEMENT_3X(Source3i, GB_INTEGER)
IMPLEMENT_XV(Sourceiv, GB_INTEGER)

IMPLEMENT_GET_X(GetSourcef, Single, ALfloat)
IMPLEMENT_GET_XV(GetSourcefv, GB_T_SINGLE, get_source_float_param_size)
IMPLEMENT_GET_X(GetSourcei, Single, ALint)
IMPLEMENT_GET_XV(GetSourceiv, GB_T_INTEGER, get_source_integer_param_size)

#define IMPLEMENT_ACTION(_name) \
BEGIN_METHOD(AL_##_name, GB_INTEGER id) \
	al##_name(VARG(id)); \
END_METHOD

#define IMPLEMENT_ACTION_V(_name) \
BEGIN_METHOD(AL_##_name, GB_OBJECT ids) \
	GB_ARRAY array = VARG(ids); \
	if (GB.CheckObject(array)) \
		return; \
	al##_name(GB.Array.Count(array), GB.Array.Get(array, 0)); \
END_METHOD

IMPLEMENT_ACTION(SourcePlay)
IMPLEMENT_ACTION(SourceStop)
IMPLEMENT_ACTION(SourceRewind)
IMPLEMENT_ACTION(SourcePause)

IMPLEMENT_ACTION_V(SourcePlayv)
IMPLEMENT_ACTION_V(SourceStopv)
IMPLEMENT_ACTION_V(SourceRewindv)
IMPLEMENT_ACTION_V(SourcePausev)

#define IMPLEMENT_QUEUE_BUFFER(_name) \
BEGIN_METHOD(AL_##_name, GB_INTEGER source; GB_OBJECT buffers) \
	GB_ARRAY buffers = VARG(buffers); \
	if (GB.CheckObject(buffers)) \
		return; \
	al##_name(VARG(source), GB.Array.Count(buffers), GB.Array.Get(buffers, 0)); \
END_METHOD

IMPLEMENT_QUEUE_BUFFER(SourceQueueBuffers)
IMPLEMENT_QUEUE_BUFFER(SourceUnqueueBuffers)

BEGIN_METHOD(AL_BufferData, GB_INTEGER buffer; GB_INTEGER format; GB_VARIANT data; GB_INTEGER size; GB_INTEGER freq)

	void *data;
	int size = VARGOPT(size, -1);
	int max_size = -1;

	if (VARG(data).type == GB_T_STRING)
	{
		data = VARG(data).value._string;
		max_size = GB.StringLength(data);
	}
	else if (VARG(data).type == GB_T_POINTER)
	{
		data = (void *)VARG(data).value._pointer;
		max_size = size;
		if (max_size < 0)
			max_size = 0;
	}
	else if (VARG(data).type == GB_T_OBJECT)
	{
		void *object = VARG(data).value._object;

		if (GB.Is(object, GB.FindClass("Byte[]"))
			  || GB.Is(object, GB.FindClass("Short[]"))
			  || GB.Is(object, GB.FindClass("Integer[]")))
		{
			int count = GB.Array.Count((GB_ARRAY)object);

			if (count == 0)
			{
				data = NULL;
				max_size = 0;
			}
			else
			{
				data = GB.Array.Get((GB_ARRAY)object, 0);
				max_size = count * (GB.Array.Get((GB_ARRAY)object, 1) - data);
			}
		}
	}

	if (max_size < 0)
	{
		GB.Error("Unsupported data type. String, Pointer, Byte[], Short[] or Integer[] expected");
		return;
	}

	if (size < 0)
		size = max_size;
	else if (size > max_size)
		size = max_size;

	if (size <= 0)
		return;

	alBufferData(VARG(buffer), VARG(format), data, size, VARGOPT(freq, 44100));

END_METHOD

#undef ID_PARAM
#define ID_PARAM GB_INTEGER id;
#undef ID_ARG
#define ID_ARG VARG(id),

IMPLEMENT_X(Bufferf, GB_SINGLE)
IMPLEMENT_3X(Buffer3f, GB_SINGLE)
IMPLEMENT_XV(Bufferfv, GB_SINGLE)
IMPLEMENT_X(Bufferi, GB_INTEGER)
IMPLEMENT_3X(Buffer3i, GB_INTEGER)
IMPLEMENT_XV(Bufferiv, GB_INTEGER)

IMPLEMENT_GET_X(GetBufferf, Single, ALfloat)
IMPLEMENT_GET_XV(GetBufferfv, GB_T_SINGLE, get_buffer_float_param_size)
IMPLEMENT_GET_X(GetBufferi, Single, ALint)
IMPLEMENT_GET_XV(GetBufferiv, GB_T_INTEGER, get_buffer_integer_param_size)


//---------------------------------------------------------------------------

GB_DESC AlDesc[] =
{
	GB_DECLARE_VIRTUAL("Al"),
	
	GB_STATIC_METHOD("GetError", "i", AL_GetError, NULL),
	
	GB_STATIC_METHOD("Enable", NULL, AL_Enable, "(Capability)i"),
	GB_STATIC_METHOD("Disable", NULL, AL_Disable, "(Capability)i"),
	GB_STATIC_METHOD("IsEnabled", "b", AL_IsEnabled, "(Capability)i"),
	
	GB_STATIC_METHOD("GetBoolean", "b", AL_GetBoolean, "(Param)i"),
	GB_STATIC_METHOD("GetInteger", "i", AL_GetInteger, "(Param)i"),
	GB_STATIC_METHOD("GetFloat", "g", AL_GetFloat, "(Param)i"),
	GB_STATIC_METHOD("GetDouble", "f", AL_GetDouble, "(Param)i"),

	//GB_STATIC_METHOD("GetBooleanv", "Boolean[]", AL_GetBooleanv, "(Param)i"),
	//GB_STATIC_METHOD("GetIntegerv", "Integer[]", AL_GetIntegerv, "(Param)i"),
	//GB_STATIC_METHOD("GetFloatv", "Single[]", AL_GetFloatv, "(Param)i"),
	//GB_STATIC_METHOD("GetDoublev", "Float[]", AL_GetDoublev, "(Param)i"),

	GB_STATIC_METHOD("GetString", "s", AL_GetString, "(Param)i"),

	GB_STATIC_METHOD("IsExtensionPresent", "b", AL_IsExtensionPresent, "(Name)s"),

	GB_STATIC_METHOD("GetEnumValue", "i", AL_GetEnumValue, "(Name)s"),

	GB_STATIC_METHOD("Listenerf", NULL, AL_Listenerf, "(Param)i(Value)g"),
	GB_STATIC_METHOD("Listener3f", NULL, AL_Listener3f, "(Param)i(Value1)g(Value2)g(Value3)g"),
	GB_STATIC_METHOD("Listenerfv", NULL, AL_Listenerfv, "(Param)i(Values)Single[];"),
	GB_STATIC_METHOD("Listeneri", NULL, AL_Listeneri, "(Param)i(Value)i"),
	GB_STATIC_METHOD("Listener3i", NULL, AL_Listener3i, "(Param)i(Value1)i(Value2)i(Value3)i"),
	GB_STATIC_METHOD("Listeneriv", NULL, AL_Listeneriv, "(Param)i(Values)Integer[];"),

	GB_STATIC_METHOD("GetListenerf", "g", AL_GetListenerf, "(Param)i"),
	GB_STATIC_METHOD("GetListener3f", "Single[]", AL_GetListenerfv, "(Param)i"),
	GB_STATIC_METHOD("GetListenerfv", "Single[]", AL_GetListenerfv, "(Param)i"),
	GB_STATIC_METHOD("GetListeneri", "i", AL_GetListenerf, "(Param)i"),
	GB_STATIC_METHOD("GetListener3i", "Integer[]", AL_GetListenerfv, "(Param)i"),
	GB_STATIC_METHOD("GetListeneriv", "Integer[]", AL_GetListenerfv, "(Param)i"),

	GB_STATIC_METHOD("GenSources", "Integer[]", AL_GenSources, "(Count)i"),
	GB_STATIC_METHOD("DeleteSources", NULL, AL_DeleteSources, "(Sources)Integer[];"),
	GB_STATIC_METHOD("IsSource", "b", AL_IsSource, "(Source)i"),

	GB_STATIC_METHOD("Sourcef", NULL, AL_Sourcef, "(Source)i(Param)i(Value)g"),
	GB_STATIC_METHOD("Source3f", NULL, AL_Source3f, "(Source)i(Param)i(Value1)g(Value2)g(Value3)g"),
	GB_STATIC_METHOD("Sourcefv", NULL, AL_Sourcefv, "(Source)i(Param)i(Values)Single[];"),
	GB_STATIC_METHOD("Sourcei", NULL, AL_Sourcei, "(Source)i(Param)i(Value)i"),
	GB_STATIC_METHOD("Source3i", NULL, AL_Source3i, "(Source)i(Param)i(Value1)i(Value2)i(Value3)i"),
	GB_STATIC_METHOD("Sourceiv", NULL, AL_Sourceiv, "(Source)i(Param)i(Values)Integer[];"),

	GB_STATIC_METHOD("GetSourcef", "g", AL_GetSourcef, "(Source)i(Param)i"),
	GB_STATIC_METHOD("GetSource3f", "Single[]", AL_GetSourcefv, "(Source)i(Param)i"),
	GB_STATIC_METHOD("GetSourcefv", "Single[]", AL_GetSourcefv, "(Source)i(Param)i"),
	GB_STATIC_METHOD("GetSourcei", "i", AL_GetSourcef, "(Source)i(Param)i"),
	GB_STATIC_METHOD("GetSource3i", "Integer[]", AL_GetSourcefv, "(Source)i(Param)i"),
	GB_STATIC_METHOD("GetSourceiv", "Integer[]", AL_GetSourcefv, "(Source)i(Param)i"),

	GB_STATIC_METHOD("SourcePlay", NULL, AL_SourcePlay, "(Source)i"),
	GB_STATIC_METHOD("SourceStop", NULL, AL_SourceStop, "(Source)i"),
	GB_STATIC_METHOD("SourceRewind", NULL, AL_SourceRewind, "(Source)i"),
	GB_STATIC_METHOD("SourcePause", NULL, AL_SourcePause, "(Source)i"),

	GB_STATIC_METHOD("SourcePlayv", NULL, AL_SourcePlayv, "(Sources)Integer[];"),
	GB_STATIC_METHOD("SourceStopv", NULL, AL_SourceStopv, "(Sources)Integer[];"),
	GB_STATIC_METHOD("SourceRewindv", NULL, AL_SourceRewindv, "(Sources)Integer[];"),
	GB_STATIC_METHOD("SourcePausev", NULL, AL_SourcePausev, "(Sources)Integer[];"),

	GB_STATIC_METHOD("SourceQueueBuffers", NULL, AL_SourceQueueBuffers, "(Source)i(Buffers)Integer[];"),
	GB_STATIC_METHOD("SourceUnqueueBuffers", NULL, AL_SourceUnqueueBuffers, "(Source)i(Buffers)Integer[];"),

	GB_STATIC_METHOD("GenBuffers", "Integer[]", AL_GenBuffers, "(Count)i"),
	GB_STATIC_METHOD("DeleteBuffers", NULL, AL_DeleteBuffers, "(Buffers)Integer[];"),
	GB_STATIC_METHOD("IsBuffer", "b", AL_IsBuffer, "(Buffer)i"),

	GB_STATIC_METHOD("BufferData", NULL, AL_BufferData, "(Buffer)i(Format)i(Data)v[(Size)i(Frequency)i]"),

	GB_STATIC_METHOD("Bufferf", NULL, AL_Bufferf, "(Buffer)i(Param)i(Value)g"),
	GB_STATIC_METHOD("Buffer3f", NULL, AL_Buffer3f, "(Buffer)i(Param)i(Value1)g(Value2)g(Value3)g"),
	GB_STATIC_METHOD("Bufferfv", NULL, AL_Bufferfv, "(Buffer)i(Param)i(Values)Single[];"),
	GB_STATIC_METHOD("Bufferi", NULL, AL_Bufferi, "(Buffer)i(Param)i(Value)i"),
	GB_STATIC_METHOD("Buffer3i", NULL, AL_Buffer3i, "(Buffer)i(Param)i(Value1)i(Value2)i(Value3)i"),
	GB_STATIC_METHOD("Bufferiv", NULL, AL_Bufferiv, "(Buffer)i(Param)i(Values)Integer[];"),

	GB_STATIC_METHOD("GetBufferf", "g", AL_GetBufferf, "(Buffer)i(Param)i"),
	GB_STATIC_METHOD("GetBuffer3f", "Single[]", AL_GetBufferfv, "(Buffer)i(Param)i"),
	GB_STATIC_METHOD("GetBufferfv", "Single[]", AL_GetBufferfv, "(Buffer)i(Param)i"),
	GB_STATIC_METHOD("GetBufferi", "i", AL_GetBufferf, "(Buffer)i(Param)i"),
	GB_STATIC_METHOD("GetBuffer3i", "Integer[]", AL_GetBufferfv, "(Buffer)i(Param)i"),
	GB_STATIC_METHOD("GetBufferiv", "Integer[]", AL_GetBufferfv, "(Buffer)i(Param)i"),

	GB_STATIC_METHOD("DopplerFactor", NULL, AL_DopplerFactor, "(DopplerFactor)f"),
	GB_STATIC_METHOD("DopplerVelocity", NULL, AL_DopplerVelocity, "(DopplerVelocity)f"),
	GB_STATIC_METHOD("SpeedOfSound", NULL, AL_SpeedOfSound, "(SpeedOfSound)f"),
	GB_STATIC_METHOD("DistanceModel", NULL, AL_DistanceModel, "(DistanceModel)i"),

	GB_CONSTANT("NONE", "i",                       AL_NONE),
	GB_CONSTANT("FALSE", "i",                      AL_FALSE),
	GB_CONSTANT("TRUE", "i",                       AL_TRUE),
	GB_CONSTANT("SOURCE_RELATIVE", "i",            AL_SOURCE_RELATIVE),
	GB_CONSTANT("CONE_INNER_ANGLE", "i",           AL_CONE_INNER_ANGLE),
	GB_CONSTANT("CONE_OUTER_ANGLE", "i",           AL_CONE_OUTER_ANGLE),
	GB_CONSTANT("PITCH", "i",                      AL_PITCH),
	GB_CONSTANT("POSITION", "i",                   AL_POSITION),
	GB_CONSTANT("DIRECTION", "i",                  AL_DIRECTION),
	GB_CONSTANT("VELOCITY", "i",                   AL_VELOCITY),
	GB_CONSTANT("LOOPING", "i",                    AL_LOOPING),
	GB_CONSTANT("BUFFER", "i",                     AL_BUFFER),
	GB_CONSTANT("GAIN", "i",                       AL_GAIN),
	GB_CONSTANT("MIN_GAIN", "i",                   AL_MIN_GAIN),
	GB_CONSTANT("MAX_GAIN", "i",                   AL_MAX_GAIN),
	GB_CONSTANT("ORIENTATION", "i",                AL_ORIENTATION),
	GB_CONSTANT("SOURCE_STATE", "i",               AL_SOURCE_STATE),
	GB_CONSTANT("INITIAL", "i",                    AL_INITIAL),
	GB_CONSTANT("PLAYING", "i",                    AL_PLAYING),
	GB_CONSTANT("PAUSED", "i",                     AL_PAUSED),
	GB_CONSTANT("STOPPED", "i",                    AL_STOPPED),
	GB_CONSTANT("BUFFERS_QUEUED", "i",             AL_BUFFERS_QUEUED),
	GB_CONSTANT("BUFFERS_PROCESSED", "i",          AL_BUFFERS_PROCESSED),
	GB_CONSTANT("SEC_OFFSET", "i",                 AL_SEC_OFFSET),
	GB_CONSTANT("SAMPLE_OFFSET", "i",              AL_SAMPLE_OFFSET),
	GB_CONSTANT("BYTE_OFFSET", "i",                AL_BYTE_OFFSET),
	GB_CONSTANT("SOURCE_TYPE", "i",                AL_SOURCE_TYPE),
	GB_CONSTANT("STATIC", "i",                     AL_STATIC),
	GB_CONSTANT("STREAMING", "i",                  AL_STREAMING),
	GB_CONSTANT("UNDETERMINED", "i",               AL_UNDETERMINED),
	GB_CONSTANT("FORMAT_MONO8", "i",               AL_FORMAT_MONO8),
	GB_CONSTANT("FORMAT_MONO16", "i",              AL_FORMAT_MONO16),
	GB_CONSTANT("FORMAT_STEREO8", "i",             AL_FORMAT_STEREO8),
	GB_CONSTANT("FORMAT_STEREO16", "i",            AL_FORMAT_STEREO16),
	GB_CONSTANT("REFERENCE_DISTANCE", "i",         AL_REFERENCE_DISTANCE),
	GB_CONSTANT("ROLLOFF_FACTOR", "i",             AL_ROLLOFF_FACTOR),
	GB_CONSTANT("CONE_OUTER_GAIN", "i",            AL_CONE_OUTER_GAIN),
	GB_CONSTANT("MAX_DISTANCE", "i",               AL_MAX_DISTANCE),
	GB_CONSTANT("FREQUENCY", "i",                  AL_FREQUENCY),
	GB_CONSTANT("BITS", "i",                       AL_BITS),
	GB_CONSTANT("CHANNELS", "i",                   AL_CHANNELS),
	GB_CONSTANT("SIZE", "i",                       AL_SIZE),
	GB_CONSTANT("UNUSED", "i",                     AL_UNUSED),
	GB_CONSTANT("PENDING", "i",                    AL_PENDING),
	GB_CONSTANT("PROCESSED", "i",                  AL_PROCESSED),
	GB_CONSTANT("NO_ERROR", "i",                   AL_NO_ERROR),
	GB_CONSTANT("INVALID_NAME", "i",               AL_INVALID_NAME),
	GB_CONSTANT("INVALID_ENUM", "i",               AL_INVALID_ENUM),
	GB_CONSTANT("INVALID_VALUE", "i",              AL_INVALID_VALUE),
	GB_CONSTANT("INVALID_OPERATION", "i",          AL_INVALID_OPERATION),
	GB_CONSTANT("OUT_OF_MEMORY", "i",              AL_OUT_OF_MEMORY),
	GB_CONSTANT("VENDOR", "i",                     AL_VENDOR),
	GB_CONSTANT("VERSION", "i",                    AL_VERSION),
	GB_CONSTANT("RENDERER", "i",                   AL_RENDERER),
	GB_CONSTANT("EXTENSIONS", "i",                 AL_EXTENSIONS),
	GB_CONSTANT("DOPPLER_FACTOR", "i",             AL_DOPPLER_FACTOR),
	GB_CONSTANT("DOPPLER_VELOCITY", "i",           AL_DOPPLER_VELOCITY),
	GB_CONSTANT("SPEED_OF_SOUND", "i",             AL_SPEED_OF_SOUND),
	GB_CONSTANT("DISTANCE_MODEL", "i",             AL_DISTANCE_MODEL),
	GB_CONSTANT("INVERSE_DISTANCE", "i",           AL_INVERSE_DISTANCE),
	GB_CONSTANT("INVERSE_DISTANCE_CLAMPED", "i",   AL_INVERSE_DISTANCE_CLAMPED),
	GB_CONSTANT("LINEAR_DISTANCE", "i",            AL_LINEAR_DISTANCE),
	GB_CONSTANT("LINEAR_DISTANCE_CLAMPED", "i",    AL_LINEAR_DISTANCE_CLAMPED),
	GB_CONSTANT("EXPONENT_DISTANCE", "i",          AL_EXPONENT_DISTANCE),
	GB_CONSTANT("EXPONENT_DISTANCE_CLAMPED", "i",  AL_EXPONENT_DISTANCE_CLAMPED),

	GB_END_DECLARE
};
